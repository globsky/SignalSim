#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <string>
#include <filesystem>
#include <ctime>
#ifdef _OPENMP
#include <omp.h>
#endif

#include "SignalSim.h"

#define TOTAL_GPS_SAT 32
#define TOTAL_BDS_SAT 63
#define TOTAL_GAL_SAT 36
#define TOTAL_GLO_SAT 24
#define TOTAL_SAT_CHANNEL 128

typedef enum {
    DataBitLNav, DataBitCNav, DataBitCNav2, // for GPS
    DataBitGNav, DataBitGNav2,	// for GLONASS
    DataBitD1D2, DataBitBCNav1, DataBitBCNav2, DataBitBCNav3,	// for BDS
    DataBitINav, DataBitFNav, DataBitECNav,	// for Galileo
    DataBitSbas, // for SBAS
} DataBitType;

void UpdateSatParamList(GNSS_TIME CurTime, KINEMATIC_INFO CurPos, int ListCount, PSIGNAL_POWER PowerList, PIONO_PARAM IonoParam);
int StepToNextMs();
complex_number GenerateNoise(double Sigma);
NavBit* GetNavData(GnssSystem SatSystem, int SatSignalIndex, NavBit* NavBitArray[]);
int QuantSamplesIQ2(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);	//TODO: Varify 2-bit quantization
int QuantSamplesIQ4(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
int QuantSamplesIQ8(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
void ShowHelp(const char* programName);
bool ParseCommandLineArgs(int argc, char* argv[], std::string& configFile, std::string& outputFile, bool& useParallel, bool& validateOnly);
std::string CreateOutputDirectory(const std::string& outputFilename);
void CreateTagFile(const std::string& tagFilePath, const OUTPUT_PARAM& outputParam);

CTrajectory Trajectory;
CPowerControl PowerControl;
CNavData NavData;
OUTPUT_PARAM OutputParam;
GNSS_TIME CurTime;
PGPS_EPHEMERIS GpsEph[TOTAL_GPS_SAT], GpsEphVisible[TOTAL_GPS_SAT];
PGPS_EPHEMERIS BdsEph[TOTAL_BDS_SAT], BdsEphVisible[TOTAL_BDS_SAT];
PGPS_EPHEMERIS GalEph[TOTAL_GAL_SAT], GalEphVisible[TOTAL_GAL_SAT];
PGLONASS_EPHEMERIS GloEph[TOTAL_GLO_SAT], GloEphVisible[TOTAL_GLO_SAT];
SATELLITE_PARAM GpsSatParam[TOTAL_GPS_SAT], BdsSatParam[TOTAL_BDS_SAT], GalSatParam[TOTAL_GAL_SAT], GloSatParam[TOTAL_GLO_SAT];	// satellite parameter array at CurTime
int GpsSatNumber, BdsSatNumber, GalSatNumber, GloSatNumber;	// number of visible satellite
const int SignalCenterFreq[][8] = {
	{ FREQ_GPS_L1, FREQ_GPS_L1, FREQ_GPS_L2, FREQ_GPS_L2, FREQ_GPS_L5 },
	{ FREQ_BDS_B1C, FREQ_BDS_B1I, FREQ_BDS_B2I, FREQ_BDS_B3I, FREQ_BDS_B2a, FREQ_BDS_B2b, FREQ_BDS_B2ab },
	{ FREQ_GAL_E1, FREQ_GAL_E5a, FREQ_GAL_E5b, FREQ_GAL_E5, FREQ_GAL_E6 },
	{ FREQ_GLO_G1, FREQ_GLO_G2 },
};
const char *SignalName[][8] = {
	{ "L1CA", "L1C", "L2C", "L2P", "L5", },
	{ "B1C", "B1I", "B2I", "B3I", "B2a", "B2b", "B2ab", },
	{ "E1", "E5a", "E5b", "E5", "E6", },
	{ "G1", "G2", },
};

int main(int argc, char* argv[])
{
	int i, j;
	JsonStream JsonTree;
	JsonObject *Object;
	UTC_TIME UtcTime;
	LLA_POSITION StartPos;
	KINEMATIC_INFO CurPos;
	LOCAL_SPEED StartVel;
	NavBit *NavBitArray[12];
	GLONASS_TIME GlonassTime;
	GNSS_TIME BdsTime;
	int ListCount;
	PSIGNAL_POWER PowerList;
	int FreqLow, FreqHigh;
	CSatIfSignal* SatIfSignal[TOTAL_SAT_CHANNEL];
	int TotalChannelNumber, SignalIndex;
	int IfFreq, FdmaOffset;
	complex_number *NoiseArray;
	unsigned char *QuantArray;
	FILE* IfFile;

	// Parse command-line arguments
	std::string configFile = "IfGenTest.json"; // Default JSON file
	std::string outputFile = "";
	bool useParallel = true; // Default to parallel execution
	bool validateOnly = false;

	// Show help if no arguments provided
	if (argc == 1) {
		ShowHelp(argv[0]);
            return 0;
        }

	if (!ParseCommandLineArgs(argc, argv, configFile, outputFile, useParallel, validateOnly)) {
		return 1;
    }

	
	printf("\n================================================================================\n");
	printf("                          IF SIGNAL GENERATION \n");
	printf("================================================================================\n");

    // Read the JSON file
	printf("[INFO]\tLoading JSON file: %s\n", configFile.c_str());
    if (JsonTree.ReadFile(configFile.c_str()) != 0)
    {
        std::cerr << "[ERROR]\tUnable to read JSON file: " << configFile << "\n";
        return 1;
    }
	else
	{
		printf("[INFO]\tJSON file read successfully: %s\n", configFile.c_str());
	}

    Object = JsonTree.GetRootObject();

	AssignParameters(Object, &UtcTime, &StartPos, &StartVel, &Trajectory, &NavData, &OutputParam, &PowerControl, NULL);

	if (!outputFile.empty()) {
		// Override output filename
		strncpy(OutputParam.filename, outputFile.c_str(), 255);
		OutputParam.filename[255] = '\0';
		printf("[INFO]\tUsing output file from command line: %s\n", outputFile.c_str());
	}

	// Validate configuration and exit if requested
	if (validateOnly) {	// TODO: Fully Implement Validation
		printf("[INFO]\tConfiguration validation To Be Implemented.\n");
		printf("[INFO]\tOutput file: %s\n", OutputParam.filename);
		printf("[INFO]\tSample frequency: %.2f MHz\n", OutputParam.SampleFreq / 1e3);
		printf("[INFO]\tCenter frequency: %.4f MHz\n", OutputParam.CenterFreq / 1e3);
		printf("[INFO]\tFormat: %s\n", (OutputParam.Format == OutputFormatIQ2) ? "IQ2" : (OutputParam.Format == OutputFormatIQ4) ? "IQ4" : "IQ8");
		return 0;
	}

	// initial variables
 	Trajectory.ResetTrajectoryTime();
	CurTime = UtcToGpsTime(UtcTime);
	GlonassTime = UtcToGlonassTime(UtcTime);
	BdsTime = UtcToBdsTime(UtcTime);
	CurPos = LlaToEcef(StartPos);
	SpeedLocalToEcef(StartPos, StartVel, CurPos);

	// Create output directory structure
	std::string outputDir = CreateOutputDirectory(OutputParam.filename);
	std::string binFilePath = outputDir + "/" + std::filesystem::path(OutputParam.filename).filename().string();
	std::string tagFilePath = binFilePath + ".tag";

	CreateTagFile(tagFilePath, OutputParam);

	printf("[INFO]\tOpening output file: %s\n", binFilePath.c_str());
	if ((IfFile = fopen(binFilePath.c_str(), "wb")) == NULL)
	{
		printf("[ERROR]\tFailed to open output file: %s\n", binFilePath.c_str());
		return 0;
	}
	printf("[INFO]\tOutput file opened successfully.\n");
	
	#ifdef _OPENMP
	if (useParallel)
	{
		printf("[INFO]\tOpenMP configured for PARALLEL execution (%d threads auto-detected)\n", omp_get_max_threads());
	}
	else
	{
		omp_set_num_threads(1);
		printf("[INFO]\tOpenMP configured for SERIAL execution (1 thread)\n");
	}
	#else
	if (useParallel)
		printf("[WARNING]\tParallel execution requested but OpenMP not available - using sequential processing\n");
	else
		printf("[INFO]\tOpenMP not available - using sequential processing\n");
	#endif

	for (i = 0; i < TOTAL_GPS_SAT; i ++)
		GpsSatParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
	for (i = 0; i < TOTAL_BDS_SAT; i ++)
		BdsSatParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
	for (i = 0; i < TOTAL_GAL_SAT; i ++)
		GalSatParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
	for (i = 0; i < TOTAL_GLO_SAT; i++)
		GloSatParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
	// create naviagtion bit instances
	for (i = 0; i < sizeof(NavBitArray) / sizeof(NavBit*); i++)
	{
		switch (i)
		{
		case DataBitLNav:   NavBitArray[i] = new LNavBit; break;
		case DataBitCNav:   NavBitArray[i] = new CNavBit; break;
		case DataBitCNav2:  NavBitArray[i] = new CNav2Bit; break;
		case DataBitGNav:   NavBitArray[i] = new GNavBit; break;
		case DataBitGNav2:  NavBitArray[i] = (NavBit*)0; break;
		case DataBitD1D2:   NavBitArray[i] = new D1D2NavBit; break;
		case DataBitBCNav1: NavBitArray[i] = new BCNav1Bit; break;
		case DataBitBCNav2: NavBitArray[i] = new BCNav2Bit; break;
		case DataBitBCNav3: NavBitArray[i] = new BCNav3Bit; break;
		case DataBitINav:   NavBitArray[i] = new INavBit; break;
		case DataBitFNav:   NavBitArray[i] = new FNavBit; break;
		case DataBitECNav:  NavBitArray[i] = (NavBit*)0; break;
		case DataBitSbas:   NavBitArray[i] = (NavBit*)0; break;
		default:            NavBitArray[i] = (NavBit*)0; break;
		}
	}

	// determine whether signal within IF band
	FreqLow = (OutputParam.CenterFreq - OutputParam.SampleFreq / 2) * 1000;
	FreqHigh = (OutputParam.CenterFreq + OutputParam.SampleFreq / 2) * 1000;
	if (OutputParam.FreqSelect[GpsSystem])
	{
		if ((OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L1CA)) && (FREQ_GPS_L1 < FreqLow || FREQ_GPS_L1 > FreqHigh))
			OutputParam.FreqSelect[GpsSystem] &= ~(1 << SIGNAL_INDEX_L1CA);
		if ((OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L1C)) && (FREQ_GPS_L1 < FreqLow || FREQ_GPS_L1 > FreqHigh))
			OutputParam.FreqSelect[GpsSystem] &= ~(1 << SIGNAL_INDEX_L1C);
		if ((OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L2C)) && (FREQ_GPS_L2 < FreqLow || FREQ_GPS_L2 > FreqHigh))
			OutputParam.FreqSelect[GpsSystem] &= ~(1 << SIGNAL_INDEX_L2C);
		if ((OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L2P)) && (FREQ_GPS_L2 < FreqLow || FREQ_GPS_L2 > FreqHigh))
			OutputParam.FreqSelect[GpsSystem] &= ~(1 << SIGNAL_INDEX_L2P);
		if ((OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L5)) && (FREQ_GPS_L5 < FreqLow || FREQ_GPS_L5 > FreqHigh))
			OutputParam.FreqSelect[GpsSystem] &= ~(1 << SIGNAL_INDEX_L5);
	}
	if (OutputParam.FreqSelect[BdsSystem])
	{
		if ((OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B1C)) && (FREQ_BDS_B1C < FreqLow || FREQ_BDS_B1C > FreqHigh))
			OutputParam.FreqSelect[BdsSystem] &= ~(1 << SIGNAL_INDEX_B1C);
		if ((OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B1I)) && (FREQ_BDS_B1I < FreqLow || FREQ_BDS_B1I > FreqHigh))
			OutputParam.FreqSelect[BdsSystem] &= ~(1 << SIGNAL_INDEX_B1I);
		if ((OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2I)) && (FREQ_BDS_B2I < FreqLow || FREQ_BDS_B2I > FreqHigh))
			OutputParam.FreqSelect[BdsSystem] &= ~(1 << SIGNAL_INDEX_B2I);
		if ((OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B3I)) && (FREQ_BDS_B3I < FreqLow || FREQ_BDS_B3I > FreqHigh))
			OutputParam.FreqSelect[BdsSystem] &= ~(1 << SIGNAL_INDEX_B3I);
		if ((OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2a)) && (FREQ_BDS_B2a < FreqLow || FREQ_BDS_B2a > FreqHigh))
			OutputParam.FreqSelect[BdsSystem] &= ~(1 << SIGNAL_INDEX_B2a);
		if ((OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2b)) && (FREQ_BDS_B2b < FreqLow || FREQ_BDS_B2b > FreqHigh))
			OutputParam.FreqSelect[BdsSystem] &= ~(1 << SIGNAL_INDEX_B2b);
	}
	if (OutputParam.FreqSelect[GalileoSystem])
	{
		if ((OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E1)) && (FREQ_GAL_E1 < FreqLow || FREQ_GAL_E1 > FreqHigh))
			OutputParam.FreqSelect[GalileoSystem] &= ~(1 << SIGNAL_INDEX_E1);
		if ((OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E5a)) && (FREQ_GAL_E5a < FreqLow || FREQ_GAL_E5a > FreqHigh))
			OutputParam.FreqSelect[GalileoSystem] &= ~(1 << SIGNAL_INDEX_E5a);
		if ((OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E5b)) && (FREQ_GAL_E5b < FreqLow || FREQ_GAL_E5b > FreqHigh))
			OutputParam.FreqSelect[GalileoSystem] &= ~(1 << SIGNAL_INDEX_E5b);
		if ((OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E6)) && (FREQ_GAL_E6 < FreqLow || FREQ_GAL_E6 > FreqHigh))
			OutputParam.FreqSelect[GalileoSystem] &= ~(1 << SIGNAL_INDEX_E6);
	}
	if (OutputParam.FreqSelect[GlonassSystem])
	{
		if ((OutputParam.FreqSelect[GlonassSystem] & (1 << SIGNAL_INDEX_G1)) && (FREQ_GLO_G1 < FreqLow || FREQ_GLO_G1 > FreqHigh))
			OutputParam.FreqSelect[GlonassSystem] &= ~(1 << SIGNAL_INDEX_G1);
		if ((OutputParam.FreqSelect[GlonassSystem] & (1 << SIGNAL_INDEX_G2)) && (FREQ_GLO_G2 < FreqLow || FREQ_GLO_G2 > FreqHigh))
			OutputParam.FreqSelect[GlonassSystem] &= ~(1 << SIGNAL_INDEX_G2);
	}

	// set Ionosphere and UTC parameter for different navigation data bit
	NavBitArray[DataBitLNav]->SetIonoUtc(NavData.GetGpsIono(), NavData.GetGpsUtcParam());
	NavBitArray[DataBitCNav]->SetIonoUtc(NavData.GetGpsIono(), NavData.GetGpsUtcParam());
	NavBitArray[DataBitCNav2]->SetIonoUtc(NavData.GetGpsIono(), NavData.GetGpsUtcParam());
	NavBitArray[DataBitD1D2]->SetIonoUtc(NavData.GetBdsIono(), NavData.GetBdsUtcParam());
	NavBitArray[DataBitINav]->SetIonoUtc(NavData.GetGalileoIono(), NavData.GetGalileoUtcParam());
	NavBitArray[DataBitFNav]->SetIonoUtc(NavData.GetGalileoIono(), NavData.GetGalileoUtcParam());
	// Find ephemeris match current time and fill in data to generate bit stream
	for (i = 1; i <= TOTAL_GPS_SAT; i ++)
	{
		GpsEph[i-1] = NavData.FindEphemeris(GpsSystem, CurTime, i);
		NavBitArray[DataBitLNav]->SetEphemeris(i, GpsEph[i - 1]);
		NavBitArray[DataBitCNav]->SetEphemeris(i, GpsEph[i - 1]);
		NavBitArray[DataBitCNav2]->SetEphemeris(i, GpsEph[i - 1]);
	}
	for (i = 1; i <= TOTAL_BDS_SAT; i ++)
	{
		BdsEph[i-1] = NavData.FindEphemeris(BdsSystem, BdsTime, i);
		NavBitArray[DataBitD1D2]->SetEphemeris(i, BdsEph[i - 1]);
		NavBitArray[DataBitBCNav1]->SetEphemeris(i, BdsEph[i - 1]);
		NavBitArray[DataBitBCNav2]->SetEphemeris(i, BdsEph[i - 1]);
		NavBitArray[DataBitBCNav3]->SetEphemeris(i, BdsEph[i - 1]);
	}
	for (i = 1; i <= TOTAL_GAL_SAT; i++)
	{
		GalEph[i - 1] = NavData.FindEphemeris(GalileoSystem, CurTime, i);
		NavBitArray[DataBitINav]->SetEphemeris(i, GalEph[i - 1]);
		NavBitArray[DataBitFNav]->SetEphemeris(i, GalEph[i - 1]);
	}
	for (i = 1; i <= TOTAL_GLO_SAT; i++)
	{
		GloEph[i - 1] = NavData.FindGloEphemeris(GlonassTime, i);
		NavBitArray[DataBitGNav]->SetEphemeris(i, (PGPS_EPHEMERIS)GloEph[i - 1]);
	}
	NavData.CompleteAlmanac(BdsSystem, UtcTime);
	NavBitArray[DataBitLNav]->SetAlmanac(NavData.GetGpsAlmanac());
	NavBitArray[DataBitCNav]->SetAlmanac(NavData.GetGpsAlmanac());
	NavBitArray[DataBitCNav2]->SetAlmanac(NavData.GetGpsAlmanac());
	NavBitArray[DataBitD1D2]->SetAlmanac(NavData.GetBdsAlmanac());
	NavBitArray[DataBitBCNav1]->SetAlmanac(NavData.GetBdsAlmanac());
	NavBitArray[DataBitBCNav2]->SetAlmanac(NavData.GetBdsAlmanac());
	NavBitArray[DataBitBCNav3]->SetAlmanac(NavData.GetBdsAlmanac());
	NavBitArray[DataBitINav]->SetAlmanac(NavData.GetGalileoAlmanac());
	NavBitArray[DataBitFNav]->SetAlmanac(NavData.GetGalileoAlmanac());
	NavBitArray[DataBitGNav]->SetAlmanac((PGPS_ALMANAC)NavData.GetGlonassAlmanac());

	// calculate visible satellite at start time and calculate satellite parameters
	GpsSatNumber = (OutputParam.FreqSelect[GpsSystem]) ? GetVisibleSatellite(CurPos, CurTime, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
	BdsSatNumber = (OutputParam.FreqSelect[BdsSystem]) ? GetVisibleSatellite(CurPos, CurTime, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
	GalSatNumber = (OutputParam.FreqSelect[GalileoSystem]) ? GetVisibleSatellite(CurPos, CurTime, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
	GloSatNumber = (OutputParam.FreqSelect[GlonassSystem]) ? GetGlonassVisibleSatellite(CurPos, GlonassTime, OutputParam, GloEph, TOTAL_GLO_SAT, GloEphVisible) : 0;
	ListCount = PowerControl.GetPowerControlList(0, PowerList);
	UpdateSatParamList(CurTime, CurPos, ListCount, PowerList, NavData.GetGpsIono());

	// create CSatIfSignal class for visible satellite, all other satellites clear pointer to NULL
	memset(SatIfSignal, 0, sizeof(SatIfSignal));
	TotalChannelNumber = 0;
	printf("[INFO]\tGenerating IF data with following satellite signals:\n\n");
	
	// Enhanced signal display with cleaner formatting
	printf("[INFO]\tEnabled Signals:\n");

	// GPS signals
	if (OutputParam.FreqSelect[GpsSystem]) {
		printf("\tGPS : [ ");
		if (OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L1CA)) printf("L1CA ");
		if (OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L1C)) printf("L1C ");
		if (OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L2C)) printf("L2C ");
		if (OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L2P)) printf("L2P ");
		if (OutputParam.FreqSelect[GpsSystem] & (1 << SIGNAL_INDEX_L5)) printf("L5 ");
		printf("]\n");
	}
	
	// BDS signals
	if (OutputParam.FreqSelect[BdsSystem]) {
		printf("\tBDS : [ ");
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B1C)) printf("B1C ");
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B1I)) printf("B1I ");
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2I)) printf("B2I ");
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2a)) printf("B2a ");
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2b)) printf("B2b ");
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SIGNAL_INDEX_B3I)) printf("B3I ");
		printf("]\n");
	}
	
	// Galileo signals
	if (OutputParam.FreqSelect[GalileoSystem]) {
		printf("\tGAL : [ ");
		if (OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E1)) printf("E1 ");
		if (OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E5a)) printf("E5a ");
		if (OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E5b)) printf("E5b ");
		if (OutputParam.FreqSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E6)) printf("E6 ");
		printf("]\n");
	}
	
	// GLONASS signals
	if (OutputParam.FreqSelect[GlonassSystem]) {
		printf("\tGLO : [ ");
		if (OutputParam.FreqSelect[GlonassSystem] & (1 << SIGNAL_INDEX_G1)) printf("G1 ");
		if (OutputParam.FreqSelect[GlonassSystem] & (1 << SIGNAL_INDEX_G2)) printf("G2 ");
		printf("]\n");
	}
	printf("\n");
	// Count total signals per system
	int GpsSignalCount = 0, BdsSignalCount = 0, GalSignalCount = 0, GloSignalCount = 0;
	for (SignalIndex = SIGNAL_INDEX_L1CA; SignalIndex <= SIGNAL_INDEX_L5; SignalIndex++)
		if (OutputParam.FreqSelect[GpsSystem] & (1 << SignalIndex)) GpsSignalCount++;
	for (SignalIndex = SIGNAL_INDEX_B1C; SignalIndex <= SIGNAL_INDEX_B2b; SignalIndex++)
		if (OutputParam.FreqSelect[BdsSystem] & (1 << SignalIndex)) BdsSignalCount++;
	for (SignalIndex = SIGNAL_INDEX_E1; SignalIndex <= SIGNAL_INDEX_E6; SignalIndex++)
		if (OutputParam.FreqSelect[GalileoSystem] & (1 << SignalIndex)) GalSignalCount++;
	for (SignalIndex = SIGNAL_INDEX_G1; SignalIndex <= SIGNAL_INDEX_G2; SignalIndex++)
		if (OutputParam.FreqSelect[GlonassSystem] & (1 << SignalIndex)) GloSignalCount++;


	printf("Signals Summary Table:\n");
	
	// Enhanced constellation summary table
	printf("+---------------+-------------+--------------+------------------------------+\n");
	printf("| Constellation | Visible SVs | Signals / SV | Total Signals / Visible SVs |\n");
	printf("+---------------+-------------+--------------+------------------------------+\n");
	printf("| GPS           | %-11d | %-12d | %-28d |\n", GpsSatNumber, GpsSignalCount, GpsSatNumber * GpsSignalCount);
	printf("| BeiDou        | %-11d | %-12d | %-28d |\n", BdsSatNumber, BdsSignalCount, BdsSatNumber * BdsSignalCount);
	printf("| Galileo       | %-11d | %-12d | %-28d |\n", GalSatNumber, GalSignalCount, GalSatNumber * GalSignalCount);
	printf("| GLONASS       | %-11d | %-12d | %-28d |\n", GloSatNumber, GloSignalCount, GloSatNumber * GloSignalCount);
	printf("+---------------+-------------+--------------+------------------------------+\n");
	
	int TotalVisibleSVs = GpsSatNumber + BdsSatNumber + GalSatNumber + GloSatNumber;
	int TotalChannels = GpsSatNumber * GpsSignalCount + BdsSatNumber * BdsSignalCount + GalSatNumber * GalSignalCount + GloSatNumber * GloSignalCount;
	printf("Total Visible SVs = %d, Total channels = %d\n\n", TotalVisibleSVs, TotalChannels);

	// Detailed satellite and signal information in compact table format
	for (SignalIndex = SIGNAL_INDEX_L1CA; SignalIndex <= SIGNAL_INDEX_L5; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[GpsSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[0][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("GPS %s with IF %+dkHz:\n", SignalName[0][SignalIndex], IfFreq / 1000);
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
		printf("| SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) |\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
		int svCount = 0;
		for (i = 0; i < GpsSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq, GpsSystem, SignalIndex, GpsEphVisible[i]->svid);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &GpsSatParam[GpsEphVisible[i]->svid-1], GetNavData(GpsSystem, SignalIndex, NavBitArray));
			TotalChannelNumber++;
			
			if (svCount % 4 == 0) printf("|");
			printf(" %02d | %+12d |", GpsEphVisible[i]->svid, (int)GetDoppler(&GpsSatParam[GpsEphVisible[i]->svid-1], SignalIndex));
			svCount++;
			if (svCount % 4 == 0) printf("\n");
		}
		// Fill remaining columns if needed
		while (svCount % 4 != 0) {
			printf("    |              |");
			svCount++;
		}
		if (svCount > 0 && (svCount-1) % 4 == 3) printf("\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n\n");
	}
	
	for (SignalIndex = SIGNAL_INDEX_B1C; SignalIndex <= SIGNAL_INDEX_B2b; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[BdsSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[1][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("BeiDou %s with IF %+dkHz:\n", SignalName[1][SignalIndex], IfFreq / 1000);
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
		printf("| SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) |\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");

		int svCount = 0;
		for (i = 0; i < BdsSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq, BdsSystem, SignalIndex, BdsEphVisible[i]->svid);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &BdsSatParam[BdsEphVisible[i]->svid - 1], GetNavData(BdsSystem, SignalIndex, NavBitArray));
			TotalChannelNumber++;
			
			if (svCount % 4 == 0) printf("|");
			printf(" %02d | %+12d |", BdsEphVisible[i]->svid, (int)GetDoppler(&BdsSatParam[BdsEphVisible[i]->svid-1], SignalIndex));
			svCount++;
			if (svCount % 4 == 0) printf("\n");
		}
		while (svCount % 4 != 0) {
			printf("    |              |");
			svCount++;
		}
		if (svCount > 0 && (svCount-1) % 4 == 3) printf("\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n\n");
	}
	
	for (SignalIndex = SIGNAL_INDEX_E1; SignalIndex <= SIGNAL_INDEX_E6; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[GalileoSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[2][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("Galileo %s with IF %+dkHz:\n", SignalName[2][SignalIndex], IfFreq / 1000);
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
		printf("| SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) |\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
		
		int svCount = 0;
		for (i = 0; i < GalSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq, GalileoSystem, SignalIndex, GalEphVisible[i]->svid);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &GalSatParam[GalEphVisible[i]->svid - 1], GetNavData(GalileoSystem, SignalIndex, NavBitArray));
			TotalChannelNumber++;
			
			if (svCount % 4 == 0) printf("|");
			printf(" %02d | %+12d |", GalEphVisible[i]->svid, (int)GetDoppler(&GalSatParam[GalEphVisible[i]->svid-1], SignalIndex));
			svCount++;
			if (svCount % 4 == 0) printf("\n");
		}
		while (svCount % 4 != 0) {
			printf("    |              |");
			svCount++;
		}
		if (svCount > 0 && (svCount-1) % 4 == 3) printf("\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n\n");
	}
	
	for (SignalIndex = SIGNAL_INDEX_G1; SignalIndex <= SIGNAL_INDEX_G2; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[GlonassSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[3][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("GLONASS %s with IF %+dkHz:\n", SignalName[3][SignalIndex], IfFreq / 1000);
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
		printf("| SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) |\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");

		int svCount = 0;
		for (i = 0; i < GloSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			FdmaOffset = (SignalIndex == SIGNAL_INDEX_G1) ? GloEphVisible[i]->freq * 562500 : (SignalIndex == SIGNAL_INDEX_G2) ? GloEphVisible[i]->freq * 437500 : 0;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq + FdmaOffset, GlonassSystem, SignalIndex, GloEphVisible[i]->n);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &GloSatParam[GloEphVisible[i]->n - 1], GetNavData(GlonassSystem, SignalIndex, NavBitArray));
			TotalChannelNumber++;
			
			if (svCount % 4 == 0) printf("|");
			printf(" %02d | %+12d |", GloEphVisible[i]->n, (int)GetDoppler(&GloSatParam[GloEphVisible[i]->n-1], SignalIndex));
			svCount++;
			if (svCount % 4 == 0) printf("\n");
		}
		while (svCount % 4 != 0) {
			printf("    |              |");
			svCount++;
		}
		if (svCount > 0 && (svCount-1) % 4 == 3) printf("\n");
		printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n\n\n");

	}

	NoiseArray = new complex_number[OutputParam.SampleFreq];
	QuantArray = new unsigned char[OutputParam.SampleFreq * 2];

	// Calculate total data size and setup progress tracking
	int totalDurationMs = (int)(Trajectory.GetTimeLength() * 1000);
	double bytesPerMs = OutputParam.SampleFreq *  ((OutputParam.Format == OutputFormatIQ2) ? 0.5 : (OutputParam.Format == OutputFormatIQ4) ? 1.0 : 2.0);
	double totalMB = (totalDurationMs * bytesPerMs) / (1024.0 * 1024.0);
	
	int length = 0;
	long long TotalClippedSamples = 0;
	long long TotalSamples = 0;
	double AGCGain = 1.0;
	printf("[INFO]\tStarting signal generation loop...\n");
	printf("[INFO]\tSignal Duration: %0.2f s\n", totalDurationMs/1000.0);
	printf("[INFO]\tSignal Size: %.2f MB\n", totalMB);
	printf("[INFO]\tSignal Data format: %s\n", (OutputParam.Format == OutputFormatIQ2) ? "IQ2" : (OutputParam.Format == OutputFormatIQ4) ? "IQ4" : "IQ8");
	printf("[INFO]\tSignal Center freq: %0.4f MHz\n", OutputParam.CenterFreq/1000.0);
	printf("[INFO]\tSignal Sample rate: %0.2f MHz\n\n", OutputParam.SampleFreq/1000.0);
	fflush(stdout);
	
	auto start_time = std::chrono::high_resolution_clock::now();
	
	while (!StepToNextMs())
	{
		// generate white noise
		for (i = 0; i < OutputParam.SampleFreq; i ++)
			NoiseArray[i] = GenerateNoise(1.0);
		
		// Use parallel or serial processing based on command line flag
		if (useParallel)
		{
			#ifdef _OPENMP
			// Parallel satellite signal generation using OpenMP (auto-detects thread count)
			#pragma omp parallel for schedule(dynamic)
			for (i = 0; i < TotalChannelNumber; i++)	// TOTAL_SAT_CHANNEL
				SatIfSignal[i]->GetIfSample(CurTime);

			#else
			// OpenMP not available, fall back to sequential processing
			for (i = 0; i < TotalChannelNumber; i++)
				SatIfSignal[i]->GetIfSample(CurTime);
			
			#endif
		}
		else
		{
			// True serial execution - no OpenMP overhead
			for (i = 0; i < TotalChannelNumber; i++)
				SatIfSignal[i]->GetIfSample(CurTime);

		}

		// Sequential accumulation to avoid race conditions (Dont nest this loop, causes issues with OpenMP)
		for (i = 0; i < TotalChannelNumber; i++)
		{
			for (j = 0; j < OutputParam.SampleFreq; j++)
				NoiseArray[j] += SatIfSignal[i]->SampleArray[j];
		}

		
		if (OutputParam.Format == OutputFormatIQ2) 
		{
			TotalClippedSamples += QuantSamplesIQ2(NoiseArray, OutputParam.SampleFreq, QuantArray, AGCGain);
			// Pack 2 samples per byte
			fwrite(QuantArray, sizeof(unsigned char), OutputParam.SampleFreq / 2, IfFile);	// 1/2 byte/sample
		}
		else if (OutputParam.Format == OutputFormatIQ4) 
		{
			TotalClippedSamples += QuantSamplesIQ4(NoiseArray, OutputParam.SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char), OutputParam.SampleFreq, IfFile); // 1 byte/sample
		}
		else
		{
			TotalClippedSamples += QuantSamplesIQ8(NoiseArray, OutputParam.SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char) * 2, OutputParam.SampleFreq, IfFile); // 2 bytes/sample
		}
		TotalSamples += OutputParam.SampleFreq * 2; // I and Q

#if 1
		// Adjust gain every 100ms
		if ((length % 100) == 99)
		{
			double ClippingRate = (double)TotalClippedSamples / TotalSamples;
			if (ClippingRate > 0.01) // clipped rate over 1%
			{
				AGCGain *= 0.95; // reduce gain by 5%
				printf("[WARNING]\tAGC: Clipping %.2f%%, reducing gain to %.3f\n", ClippingRate * 100, AGCGain);
			}
			else if (ClippingRate < 0.001 && AGCGain < 1.0) // clipped rate under 0.1%
			{
				AGCGain *= 1.02; // increase gain by 2%
				if (AGCGain > 1.0) AGCGain = 1.0;
				printf("[WARNING]\tAGC: Clipping %.2f%%, increasing gain to %.3f\n", ClippingRate * 100, AGCGain);
			}
		}
#endif

//		for (j = 0; j < OutputParam.SampleFreq; j ++)
//			printf("%f %f\n", NoiseArray[j].real, NoiseArray[j].imag);
		
		length++;
		
		// Enhanced progress reporting with percentage, MB/s, and ETA
		if ((length % 10) == 0 || length == totalDurationMs) {
			auto current_time = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
			
			double percentage = (double)length / totalDurationMs * 100.0;
			double currentMB = (length * bytesPerMs) / (1024.0 * 1024.0);
			double mbPerSec = (elapsed > 0) ? (currentMB * 1000.0) / elapsed : 0.0;
			
			// Calculate estimated time remaining
			long etaMs = 0;
			if (percentage > 0 && elapsed > 0) {
				etaMs = (long)((elapsed * (100.0 - percentage)) / percentage);
			}
			
			// Progress bar with percentage in center
			int barWidth = 50;
			int progress = (int)(percentage * barWidth / 100.0);
			char progressStr[8];
			sprintf(progressStr, "%.1f%%", percentage);
			int progressStrLen = strlen(progressStr);
			int centerPos = (barWidth - progressStrLen) / 2;
			
			printf("\r[");
			for (int k = 0; k < barWidth; k++) {
				if (k >= centerPos && k < centerPos + progressStrLen) {
					printf("%c", progressStr[k - centerPos]);
				} else if (k < progress) {
					printf("=");
				} else if (k == progress && percentage < 100.0) {
					printf(">");
				} else if (percentage >= 100.0 && k < barWidth) {
					printf("=");
				} else {
					printf(" ");
				}
			}
			
			// Format ETA
			char etaStr[32];
			if (etaMs > 0) {
				int etaSeconds = (int)(etaMs / 1000);
				int etaMinutes = etaSeconds / 60;
				etaSeconds %= 60;
				if (etaMinutes > 0) {
					sprintf(etaStr, "ETA: %dm%02ds   ", etaMinutes, etaSeconds);
				} else {
					sprintf(etaStr, "ETA: %02ds   ", etaSeconds);
				}
			} else {
				strcpy(etaStr, "ETA: --   ");
			}
			
			printf("] %d/%d ms | %.2f/%.2f MB @ %.2f MB/s | %s",
				   length, totalDurationMs, currentMB, totalMB, mbPerSec, etaStr);
			fflush(stdout);
		}
//		if (length == 2) break;
	}
	
	// Final progress bar update to ensure 100% is shown
	printf("\r[");
	for (int k = 0; k < 50; k++) {
		if (k >= 22 && k < 28) {
			printf("%c", "100.0%"[k - 22]);
		} else {
			printf("=");
		}
	}
	printf("] %d/%d ms | %.2f/%.2f MB | \tCOMPLETED\n",
		   totalDurationMs, totalDurationMs, totalMB, totalMB); 
	
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
	double finalMB = (length * bytesPerMs) / (1024.0 * 1024.0);
	double avgMbPerSec = (duration.count() > 0) ? (finalMB * 1000.0) / duration.count() : 0.0;

	printf("\n[INFO]\tIF Signal generation completed!\n");
	printf("------------------------------------------------------------------\n");
	printf("[INFO]\tTotal samples: %lld\n", TotalSamples);
	printf("[INFO]\tClipped samples: %lld (%.4f%%)\n", TotalClippedSamples, (double)TotalClippedSamples / TotalSamples * 100);
	printf("[INFO]\tFinal AGC gain: %.3f\n", AGCGain);
	if ((double)TotalClippedSamples / TotalSamples > 0.05)
	{
		printf("[WARNING]\tHigh clipping rate! Consider reducing initPower in JSON config.\n");
	}
	printf("[INFO]\tTotal time taken: %0.2f s\n", duration.count()/1000.0);
	printf("[INFO]\tData generated: %.2f MB\n", finalMB);
	printf("[INFO]\tAverage rate: %.2f MB/s\n", avgMbPerSec);
	printf("------------------------------------------------------------------\n\n");

	// Memory cleanup.
	for (i = 0; i < TOTAL_SAT_CHANNEL; ++i)
		delete SatIfSignal[i];
	for (i = 0; i < static_cast<int>(sizeof(NavBitArray) / sizeof(NavBitArray[0])); ++i)
		delete NavBitArray[i];

	delete[] NoiseArray;
	delete[] QuantArray;
	fclose(IfFile);

	return 0;
}

void UpdateSatParamList(GNSS_TIME CurTime, KINEMATIC_INFO CurPos, int ListCount, PSIGNAL_POWER PowerList, PIONO_PARAM IonoParam)
{
	int i, index;
	LLA_POSITION PosLLA = EcefToLla(CurPos);
	int TotalSatNumber = 0;

	for (i = 0; i < GpsSatNumber; i ++)
	{
		index = GpsEphVisible[i]->svid - 1;
		GetSatelliteParam(CurPos, PosLLA, CurTime, GpsSystem, GpsEphVisible[i], IonoParam, &GpsSatParam[index]);
		GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GpsSatParam[index]);
	}
	for (i = 0; i < BdsSatNumber; i ++)
	{
		index = BdsEphVisible[i]->svid - 1;
		GetSatelliteParam(CurPos, PosLLA, CurTime, BdsSystem, BdsEphVisible[i], IonoParam, &BdsSatParam[index]);
		GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &BdsSatParam[index]);
	}
	for (i = 0; i < GalSatNumber; i ++)
	{
		index = GalEphVisible[i]->svid - 1;
		GetSatelliteParam(CurPos, PosLLA, CurTime, GalileoSystem, GalEphVisible[i], IonoParam, &GalSatParam[index]);
		GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GalSatParam[index]);
	}
	for (i = 0; i < GloSatNumber; i++)
	{
		index = GloEphVisible[i]->n - 1;
		GetSatelliteParam(CurPos, PosLLA, CurTime, GlonassSystem, (PGPS_EPHEMERIS)GloEphVisible[i], IonoParam, &GloSatParam[index]);
		GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GloSatParam[index]);
	}
}

int StepToNextMs()
{
	KINEMATIC_INFO CurPos;
	int ListCount = 0;
	PSIGNAL_POWER PowerList = NULL;
//	UTC_TIME UtcTime;
//	GLONASS_TIME GlonassTime;

	if (!Trajectory.GetNextPosVelECEF(0.001, CurPos))
		return -1;

	// calculate new satellite parameter
	ListCount = PowerControl.GetPowerControlList(1, PowerList);
	if (++CurTime.MilliSeconds > 604800000)
	{
		CurTime.Week ++;
		CurTime.MilliSeconds -= 604800000;
	}
/*	UtcTime = GpsTimeToUtc(CurTime);
	if ((CurTime.MilliSeconds % 60000) == 0)	// recalculate visible satellite at minute boundary
	{
		GlonassTime = UtcToGlonassTime(UtcTime);
		GpsSatNumber = (OutputParam.FreqSelect[GpsSystem]) ? GetVisibleSatellite(CurPos, CurTime, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
		BdsSatNumber = (OutputParam.FreqSelect[BdsSystem]) ? GetVisibleSatellite(CurPos, CurTime, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
		GalSatNumber = (OutputParam.FreqSelect[GalileoSystem]) ? GetVisibleSatellite(CurPos, CurTime, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
		GloSatNumber = (OutputParam.FreqSelect[GlonassSystem]) ? GetGlonassVisibleSatellite(CurPos, GlonassTime, OutputParam, GloEph, TOTAL_GLO_SAT, GloEphVisible) : 0;
	}*/
	UpdateSatParamList(CurTime, CurPos, ListCount, PowerList, NavData.GetGpsIono());
	return 0;
}

complex_number GenerateNoise(double Sigma)
{
	double fvalue1, fvalue2, mag;

	// Marsaglia Polar method (improved Box-Muller method)
    do {
        fvalue1 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        fvalue2 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        mag = fvalue1 * fvalue1 + fvalue2 * fvalue2;
    } while (mag >= 1.0 || mag == 0.0);
	mag = sqrt(-2.0 * log(mag) / mag) * Sigma;

	return complex_number(fvalue1 * mag, fvalue2 * mag);
}

NavBit* GetNavData(GnssSystem SatSystem, int SatSignalIndex, NavBit* NavBitArray[])
{
	switch (SatSystem)
	{
	case GpsSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_L1CA: return NavBitArray[DataBitLNav];
		case SIGNAL_INDEX_L1C:  return NavBitArray[DataBitCNav2];
		case SIGNAL_INDEX_L2C:  return NavBitArray[DataBitCNav];
		case SIGNAL_INDEX_L2P:  return NavBitArray[DataBitLNav];
		case SIGNAL_INDEX_L5:   return NavBitArray[DataBitCNav];
		default: return NavBitArray[DataBitLNav];
		}
		break;
	case BdsSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_B1C: return NavBitArray[DataBitBCNav1];
		case SIGNAL_INDEX_B1I: return NavBitArray[DataBitD1D2];
		case SIGNAL_INDEX_B2I: return NavBitArray[DataBitD1D2];
		case SIGNAL_INDEX_B3I: return NavBitArray[DataBitD1D2];
		case SIGNAL_INDEX_B2a: return NavBitArray[DataBitBCNav2];
		case SIGNAL_INDEX_B2b: return NavBitArray[DataBitBCNav3];
		default: return NavBitArray[DataBitD1D2];
		}
		break;
	case GalileoSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_E1:  return NavBitArray[DataBitINav];
		case SIGNAL_INDEX_E5a: return NavBitArray[DataBitFNav];
		case SIGNAL_INDEX_E5b: return NavBitArray[DataBitINav];
		case SIGNAL_INDEX_E5:  return NavBitArray[DataBitFNav];
		case SIGNAL_INDEX_E6:  return NavBitArray[DataBitECNav];
		default: return NavBitArray[DataBitINav];
		}
		break;
	case GlonassSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_G1: return NavBitArray[DataBitGNav];
		case SIGNAL_INDEX_G2: return NavBitArray[DataBitGNav];
		default: return NavBitArray[DataBitINav];
		}
		break;
	default: return NavBitArray[DataBitLNav];
	}
}

// PocketSDR compatible 2-bit IQ quantization 
// (TODO: Test)
// (FIXME: Optimize)
 int QuantSamplesIQ2(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale)
 {
     int ClippedCount = 0;
     // A single threshold separates small and large magnitudes.
     // The gain should be tuned so that signal-plus-noise standard deviation maps appropriately.
     const double threshold = 1.0 / GainScale;

     // Process 2 complex samples at a time to produce 1 byte of output.
     // Each byte contains four 2-bit values: [Q2, I2, Q1, I1]
    for (int i = 0; i < Length; i += 2)
    {
        // --- Process Sample 1 ---
		double i1_val = Samples[i].real;
		double q1_val = Samples[i].imag;

		bool i1_sign = i1_val < 0;
		bool i1_mag  = fabs(i1_val) >= threshold;
		bool q1_sign = q1_val < 0;
		bool q1_mag  = fabs(q1_val) >= threshold;

		// Increment clipped count for large magnitude values
		if (i1_mag) ClippedCount++;
		if (q1_mag) ClippedCount++;

		// Combine sign (1 bit) and magnitude (1 bit) into a 2-bit index {0,1,2,3}
		// This maps to levels {+1, +3, -1, -3}
		unsigned char i1_idx = (i1_sign << 1) | i1_mag;
		unsigned char q1_idx = (q1_sign << 1) | q1_mag;

		// --- Process Sample 2 ---
		double i2_val = Samples[i+1].real;
		double q2_val = Samples[i+1].imag;

		bool i2_sign = i2_val < 0;
		bool i2_mag  = fabs(i2_val) >= threshold;
		bool q2_sign = q2_val < 0;
		bool q2_mag  = fabs(q2_val) >= threshold;

		if (i2_mag) ClippedCount++;
		if (q2_mag) ClippedCount++;

		unsigned char i2_idx = (i2_sign << 1) | i2_mag;
		unsigned char q2_idx = (q2_sign << 1) | q2_mag;

		// --- Pack into a single byte ---
		// Each sample's I/Q takes 4 bits (a nibble). Two samples fit in one byte.
		// Sample 1 -> Low Nibble, Sample 2 -> High Nibble
		unsigned char nibble1 = (i1_idx << 2) | q1_idx;
		unsigned char nibble2 = (i2_idx << 2) | q2_idx;

        QuantSamples[i/2] = (nibble2 << 4) | nibble1;
	}

	return ClippedCount;
}

int QuantSamplesIQ4(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale)
{
	int i;
	double Value;
	unsigned char QuantValue, QuantSample;
	const double Gain = GainScale * 3.0;
	int ClippedCount = 0;

	for (i = 0; i < Length; i++)
	{
		Value = fabs(Samples[i].real);
		QuantValue = (int)(Value * Gain);	// optimal quantization for sigma=1 noise
		if (QuantValue > 7)
		{
			QuantValue = 7;
			ClippedCount ++;
		}
		QuantValue += ((Samples[i].real >= 0) ? 0 : (1 << 3));	// add sign bit as MSB
		QuantSample = QuantValue << 4;
		Value = fabs(Samples[i].imag);
		QuantValue = (int)(Value * Gain);	// optimal quantization for sigma=1 noise
		if (QuantValue > 7)
		{
			QuantValue = 7;
			ClippedCount ++;
		}
		QuantValue += ((Samples[i].imag >= 0) ? 0 : (1 << 3));	// add sign bit as MSB
		QuantSample |= QuantValue;
		QuantSamples[i] = QuantSample;
	}

	return ClippedCount;
}

int QuantSamplesIQ8(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale)
{
	int i;
	int QuantValue;
	const double Gain = GainScale * 25.0;
	int ClippedCount = 0;

	for (i = 0; i < Length; i++)
	{
		QuantValue = (int)(Samples[i].real * Gain);	// sigma scaled at 25 -> +-5 sigma scaled within range of INT8
		if (QuantValue > 127)	// saturate at -128~127
		{
			QuantValue = 127;
			ClippedCount ++;
		}
		else if (QuantValue < -128)
		{
			QuantValue = -128;
			ClippedCount ++;
		}
		QuantSamples[i * 2] = (unsigned char)(QuantValue & 0xff);
		QuantValue = (int)(Samples[i].imag * Gain);	// sigma scaled at 25 -> +-5 sigma scaled within range of INT8
		if (QuantValue > 127)	// saturate at -128~127
		{
			QuantValue = 127;
			ClippedCount ++;
		}
		else if (QuantValue < -128)
		{
			QuantValue = -128;
			ClippedCount ++;
		}
		QuantSamples[i * 2 + 1] = (unsigned char)(QuantValue & 0xff);
	}

	return ClippedCount;
}

void ShowHelp(const char* programPath)
{
	// Extract just the executable name from the path
	std::string programName = std::filesystem::path(programPath).filename().string();
	
	std::cout << "IFDataGen - GNSS IF Data Generator\n\n";
	std::cout << "Usage: " << programName << " [options]\n\n";
	std::cout << "Available options:\n";
	std::cout << "  --config, -c <FILE>     Configuration file (JSON) [REQUIRED]\n";
	std::cout << "  --output, -o <FILE>     Output IF data file (overrides config)\n";
	std::cout << "  --validate-only, -vo    Validate configuration and exit\n";
	std::cout << "  --parallel, -p          Use parallel execution (default)\n";
	std::cout << "  --serial, -s            Force serial execution\n";
	std::cout << "  --help, -h              Show this help message\n\n";
	std::cout << "Examples:\n";
	std::cout << "  " << programName << " -c config.json\n";
	std::cout << "  " << programName << " --config config.json --output mydata.bin\n";
	std::cout << "  " << programName << " -c config.json -o output.bin -s\n";
	std::cout << "  " << programName << " --config config.json -vo\n\n";
	std::cout << "Output structure:\n";
	std::cout << "  filename/\n";
    std::cout << "	|-- filename.bin     (IF data)\n";
    std::cout << "	|-- filename.bin.tag (metadata)\n\n";
}

bool ParseCommandLineArgs(int argc, char* argv[], std::string& configFile, std::string& outputFile, bool& useParallel, bool& validateOnly)
{
	configFile = "";
	outputFile = "";
	useParallel = true;
	validateOnly = false;

	for (int i = 1; i < argc; i++) {
		std::string arg = argv[i];
		
		if (arg == "--help" || arg == "-h") {
			ShowHelp(argv[0]);
			exit(0);
		}
		else if (arg == "--config" || arg == "-c") {
			if (i + 1 >= argc) {
				std::cerr << "[Error] " << arg << " requires a filename argument\n";
				return false;
			}
			configFile = argv[++i];
		}
		else if (arg == "--output" || arg == "-o") {
			if (i + 1 >= argc) {
				std::cerr << "[Error] " << arg << " requires a filename argument\n";
				return false;
			}
			outputFile = argv[++i];
		}
		else if (arg == "--validate-only" || arg == "-vo") {
			validateOnly = true;
		}
		else if (arg == "--parallel" || arg == "-p") {
			useParallel = true;
		}
		else if (arg == "--serial" || arg == "-s") {
			useParallel = false;
		}
		else if (arg[0] == '-') {
			std::cerr << "[Error] Unknown option: " << arg << "\n";
			std::cerr << "Use --help, -h for usage information\n";
			return false;
		}
		else {
			// Positional argument - treat as config file if not already set
			if (configFile.empty()) {
				configFile = arg;
			} else {
				std::cerr << "Error: Unexpected argument: " << arg << "\n";
				return false;
			}
		}
	}

	// Check if config file is provided
	if (configFile.empty()) {
		std::cerr << "Error: Configuration file is required. Use --config or -c to specify.\n";
		std::cerr << "Use --help for usage information\n";
		return false;
	}

	return true;
}

std::string CreateOutputDirectory(const std::string& outputFilename)
{
	std::filesystem::path filePath(outputFilename);
	std::string dirName = "bin_signals/" + filePath.stem().string(); 
	if (!std::filesystem::exists(dirName)) {
		std::filesystem::create_directories(dirName);
}

	
	try {
		printf("[INFO]\tCreating output directory: %s\n", dirName.c_str());
		if (!std::filesystem::exists(dirName)) {
			std::filesystem::create_directories(dirName);
		}
		printf("[INFO]\tOutput directory created successfully: %s\n", dirName.c_str());

	} catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "[ERROR]\tCreating directory: " << e.what() << std::endl;
		throw;
	}
	
	return dirName;
}

void CreateTagFile(const std::string& tagFilePath, const OUTPUT_PARAM& outputParam)
{
    printf("[INFO]\tCreating tag file: %s\n", tagFilePath.c_str());
    FILE* tagFile = fopen(tagFilePath.c_str(), "w");
    
    if (!tagFile) {
        std::cerr << "[WARNING]\tCould not create tag file: " << tagFilePath << std::endl;
        return;
    }

    // Get current time in UTC (PocketSDR uses UTC time)
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    struct tm* timeinfo = gmtime(&time_t);  // Use gmtime() instead of localtime()
    
    // Determine format strings
    const char* formatStr;
    const char* iqStr;
    const char* bitsStr;

    if (outputParam.Format == OutputFormatIQ2) {
        formatStr = "INT2X2";
        iqStr = "1";
        bitsStr = "2";
    } else if (outputParam.Format == OutputFormatIQ4) {
        formatStr = "INT4X2";
        iqStr = "1";
        bitsStr = "4";
    } else { // OutputFormatIQ8
        formatStr = "INT8X2";
        iqStr = "1";
        bitsStr = "8";
    }

    // Write tag file contents matching PocketSDR format exactly
    fprintf(tagFile, "PROG = IFDataGen\n");
    fprintf(tagFile, "TIME = %04d/%02d/%02d %02d:%02d:%02d.%03d\n",
        timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
        timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, (int)ms.count());
    fprintf(tagFile, "FMT  = %s\n", formatStr);
    fprintf(tagFile, "F_S  = %.6f\n", outputParam.SampleFreq/1e3);		// Sample frequency in MHz
    fprintf(tagFile, "F_LO = %.6f\n", outputParam.CenterFreq/1e3);		// Center frequency in MHz
    fprintf(tagFile, "IQ   = %s\n", iqStr);
    fprintf(tagFile, "BITS = %s\n", bitsStr);
    fprintf(tagFile, "SCALE = 1.0\n");

    fclose(tagFile);
    printf("[INFO]\tTag file created: %s\n", tagFilePath.c_str());
}

