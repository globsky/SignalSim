#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <chrono>

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
int QuantSamplesIQ4(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
int QuantSamplesIQ8(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
int QuantSamplesIQ16(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);


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
int TotalChannelNumber;
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

// thread synchronize variables
//const int NUM_THREADS = 3;  // Ïß³ÌÊýÁ¿
std::mutex mtx;
std::condition_variable cv_task;	// for start task
std::condition_variable cv_ready;	// for task ready
std::condition_variable cv_done;	// for task complete notification
int exec_cycle = 0;
int ready_tasks = 0;
int completed_tasks = 0;
bool shutdown = false;

// IF signal generation thread
void IF_generate_thread(CSatIfSignal* SatIfSignal)
{
	int thread_cycle = -1;

	while (true)
	{
		std::unique_lock<std::mutex> lock(mtx);

		++ready_tasks;
		if (ready_tasks == TotalChannelNumber)
			cv_ready.notify_one();  // notify main thread ready state

		// wait task or termineate
		cv_task.wait(lock, [&] { return (exec_cycle > thread_cycle) || shutdown; });

		if (shutdown)
			break;

		thread_cycle = exec_cycle;
		// unlock to have other threads continue
		lock.unlock();

//		std::cout << "[Sat " << SatIfSignal->Svid << "] for " << exec_cycle << " begin..." << std::endl;
		SatIfSignal->GetIfSample(CurTime);
//		std::cout << "[Sat " << SatIfSignal->Svid << "] for " << exec_cycle << " end" << std::endl;

        // update complete count
		{
			std::lock_guard<std::mutex> lock(mtx);
			++completed_tasks;
			if (completed_tasks == TotalChannelNumber)
				cv_done.notify_one();  // notify main thread work done
		}
	}
}

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
	int SignalIndex;
	int IfFreq, FdmaOffset;
	complex_number *NoiseArray;
	unsigned char *QuantArray;
	FILE* IfFile;
	clock_t start_time, end_time;

    std::vector<std::thread> threads;

	// read JSON file and assign parameters
    const char* jsonFile = "IfGenTest.json"; // Default JSON file

    // Check if a command-line argument is provided
    if (argc > 1)
    {
        if (std::strcmp(argv[1], "--help") == 0)
        {
            std::cout << "Usage: " << argv[0] << " [optional JSON file path]\n";
            return 0;
        }
        jsonFile = argv[1]; // Use the provided JSON file
    }

    // Read the JSON file
    if (JsonTree.ReadFile(jsonFile) != 0)
    {
        std::cerr << "Error: Unable to read JSON file: " << jsonFile << "\n";
        return 1;
    }
    Object = JsonTree.GetRootObject();

	AssignParameters(Object, &UtcTime, &StartPos, &StartVel, &Trajectory, &NavData, &OutputParam, &PowerControl, NULL);

	// initial variables
 	Trajectory.ResetTrajectoryTime();
	CurTime = UtcToGpsTime(UtcTime);
	GlonassTime = UtcToGlonassTime(UtcTime);
	BdsTime = UtcToBdsTime(UtcTime);
	CurPos = LlaToEcef(StartPos);
	SpeedLocalToEcef(StartPos, StartVel, CurPos);
	if ((IfFile = fopen(OutputParam.filename, "wb")) == NULL)
		return 0;

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
	printf("Generate IF data with following satellite signals:\n");
	printf("GPS visible SVs %d.\n", GpsSatNumber);
	printf("BDS visible SVs %d.\n", BdsSatNumber);
	printf("Galileo visible SVs %d.\n", GalSatNumber);
	printf("Glonass visible SVs %d.\n\n", GloSatNumber);
	for (SignalIndex = SIGNAL_INDEX_L1CA; SignalIndex <= SIGNAL_INDEX_L5; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[GpsSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[0][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("GPS %s %d visible SVs with IF %dkHz\n", SignalName[0][SignalIndex], GpsSatNumber, IfFreq / 1000);
		for (i = 0; i < GpsSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq, GpsSystem, SignalIndex, GpsEphVisible[i]->svid);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &GpsSatParam[GpsEphVisible[i]->svid-1], GetNavData(GpsSystem, SignalIndex, NavBitArray));
	        threads.emplace_back(IF_generate_thread, SatIfSignal[TotalChannelNumber]);
			TotalChannelNumber++;
			printf("\tSV%02d with Doppler %dHz\n", GpsEphVisible[i]->svid, (int)GetDoppler(&GpsSatParam[GpsEphVisible[i]->svid-1], SignalIndex));
		}
	}
	for (SignalIndex = SIGNAL_INDEX_B1C; SignalIndex <= SIGNAL_INDEX_B2b; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[BdsSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[1][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("BDS %s %d visible SVs with IF %dkHz\n", SignalName[1][SignalIndex], BdsSatNumber, IfFreq / 1000);
		for (i = 0; i < BdsSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq, BdsSystem, SignalIndex, BdsEphVisible[i]->svid);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &BdsSatParam[BdsEphVisible[i]->svid - 1], GetNavData(BdsSystem, SignalIndex, NavBitArray));
	        threads.emplace_back(IF_generate_thread, SatIfSignal[TotalChannelNumber]);
			TotalChannelNumber++;
			printf("\tSV%02d with Doppler %dHz\n", BdsEphVisible[i]->svid, (int)GetDoppler(&BdsSatParam[BdsEphVisible[i]->svid-1], SignalIndex));
		}
	}
	for (SignalIndex = SIGNAL_INDEX_E1; SignalIndex <= SIGNAL_INDEX_E6; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[GalileoSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[2][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("Galileo %s %d visible SVs with IF %dkHz\n", SignalName[2][SignalIndex], GalSatNumber, IfFreq / 1000);
		for (i = 0; i < GalSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq, GalileoSystem, SignalIndex, GalEphVisible[i]->svid);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &GalSatParam[GalEphVisible[i]->svid - 1], GetNavData(GalileoSystem, SignalIndex, NavBitArray));
	        threads.emplace_back(IF_generate_thread, SatIfSignal[TotalChannelNumber]);
			TotalChannelNumber++;
			printf("\tSV%02d with Doppler %dHz\n", GalEphVisible[i]->svid, (int)GetDoppler(&GalSatParam[GalEphVisible[i]->svid-1], SignalIndex));
		}
	}
	for (SignalIndex = SIGNAL_INDEX_G1; SignalIndex <= SIGNAL_INDEX_G2; SignalIndex++)
	{
		if (!(OutputParam.FreqSelect[GlonassSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[3][SignalIndex] - OutputParam.CenterFreq * 1000;
		printf("GLONASS %s %d visible SVs with IF %dkHz\n", SignalName[3][SignalIndex], GloSatNumber, IfFreq / 1000);
		for (i = 0; i < GloSatNumber; i++)
		{
			if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
				break;
			FdmaOffset = (SignalIndex == SIGNAL_INDEX_G1) ? GloEphVisible[i]->freq * 562500 : (SignalIndex == SIGNAL_INDEX_G2) ? GloEphVisible[i]->freq * 437500 : 0;
			SatIfSignal[TotalChannelNumber] = new CSatIfSignal(OutputParam.SampleFreq, IfFreq + FdmaOffset, GlonassSystem, SignalIndex, GloEphVisible[i]->n);
			SatIfSignal[TotalChannelNumber]->InitState(CurTime, &GloSatParam[GloEphVisible[i]->n - 1], GetNavData(GlonassSystem, SignalIndex, NavBitArray));
	        threads.emplace_back(IF_generate_thread, SatIfSignal[TotalChannelNumber]);
			TotalChannelNumber++;
			printf("\tSV%02d with Doppler %dHz\n", GloEphVisible[i]->n, (int)GetDoppler(&GloSatParam[GloEphVisible[i]->n-1], SignalIndex));
		}
	}
	printf("Total channels: %d\n\n", TotalChannelNumber);

	NoiseArray = new complex_number[OutputParam.SampleFreq];
	QuantArray = new unsigned char[OutputParam.SampleFreq * 4];

	long long TotalClippedSamples = 0;
	long long TotalSamples = 0;
	double AGCGain = 1.0;
	start_time = clock();

	while (!StepToNextMs())
	{
		// wait for all tasks ready
		{
			std::unique_lock<std::mutex> lock(mtx);
			while (ready_tasks < TotalChannelNumber)
				cv_ready.wait(lock);
//			std::cout << "all threads ready" << std::endl;
			exec_cycle ++;
		// reset ready count
			ready_tasks = 0;
        }

		// wake all threads
        cv_task.notify_all();

		// generate white noise
		for (i = 0; i < OutputParam.SampleFreq; i ++)
			NoiseArray[i] = GenerateNoise(1.0);

        // wait all threads complete
		{
			std::unique_lock<std::mutex> lock(mtx);
			while (completed_tasks < TotalChannelNumber)
				cv_done.wait(lock);
//            std::cout << "all threads done for " << exec_cycle << std::endl;
            completed_tasks = 0;
        }

		for (i = 0; i < TOTAL_SAT_CHANNEL; i++)
		{
			if (!SatIfSignal[i])
				continue;
//			SatIfSignal[i]->GetIfSample(CurTime);
			for (j = 0; j < OutputParam.SampleFreq; j++)
				NoiseArray[j] += SatIfSignal[i]->SampleArray[j];
		}
		if (OutputParam.Format == OutputFormatIQ4)
		{
			TotalClippedSamples += QuantSamplesIQ4(NoiseArray, OutputParam.SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char), OutputParam.SampleFreq, IfFile);
		}
		else if (OutputParam.Format == OutputFormatIQ16)
		{
			TotalClippedSamples += QuantSamplesIQ16(NoiseArray, OutputParam.SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char) * 4, OutputParam.SampleFreq, IfFile);
		}
		else
		{
			TotalClippedSamples += QuantSamplesIQ8(NoiseArray, OutputParam.SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char) * 2, OutputParam.SampleFreq, IfFile);
		}
		TotalSamples += OutputParam.SampleFreq * 2; // I and Q

#if 1
		// Adjust gain every 100ms
		if ((exec_cycle % 100) == 0)
		{
			double ClippingRate = (double)TotalClippedSamples / TotalSamples;
			if (ClippingRate > 0.01) // clipped rate over 1%
			{
				AGCGain *= 0.95; // reduce gain by 5%
				printf("AGC: Clipping %.2f%%, reducing gain to %.3f\n", ClippingRate * 100, AGCGain);
				TotalClippedSamples = TotalSamples = 0;	// reset statistic
			}
			else if (ClippingRate < 0.001 && AGCGain < 1.0) // clipped rate under 0.1%
			{
				AGCGain *= 1.02; // increase gain by 2%
				if (AGCGain > 1.0) AGCGain = 1.0;
				printf("AGC: Clipping %.2f%%, increasing gain to %.3f\n", ClippingRate * 100, AGCGain);
				TotalClippedSamples = TotalSamples = 0;	// reset statistic
			}
		}
#endif

//		for (j = 0; j < OutputParam.SampleFreq; j ++)
//			printf("%f %f\n", NoiseArray[j].real, NoiseArray[j].imag);
		if ((exec_cycle % 10) == 0)
			printf("Generate IF data completed %6d ms\r", exec_cycle);
//		if (length == 2) break;
	}

	// terminate all threads
	{
		std::lock_guard<std::mutex> lock(mtx);
		shutdown = true;
		std::cout << "\nterminate all threads" << std::endl;
	}
	cv_task.notify_all();

    // wait all threads complete
	for (auto& t : threads)
	{
		if (t.joinable()) t.join();
	}

	end_time = clock();
	std::cout << "All threads terminated. Generate IF file complete" << std::endl;
	printf("\nTotal samples: %lld\n", (long long)OutputParam.SampleFreq * exec_cycle);
	printf("Final Clipped rate %.4f%% (%lld out of %lld)\n", (double)TotalClippedSamples / TotalSamples * 100, TotalClippedSamples, TotalSamples);
	printf("Final AGC gain: %.3f\n", AGCGain);
	if ((double)TotalClippedSamples / TotalSamples > 0.05)
		printf("WARNING: High clipping rate! Consider reducing initPower in configuration file.\n");
	printf("Total execute time is %.6fs\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

	for (i = 0; i < TOTAL_GPS_SAT; i ++)
		if (SatIfSignal[i]) delete SatIfSignal[i];
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
#if 1
	double fvalue1, fvalue2, mag;

	// Marsaglia Polar method (improved Box-Muller method)
    do {
        fvalue1 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        fvalue2 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
        mag = fvalue1 * fvalue1 + fvalue2 * fvalue2;
    } while (mag >= 1.0 || mag == 0.0);
	mag = sqrt(-2.0 * log(mag) / mag) * Sigma;

	return complex_number(fvalue1 * mag, fvalue2 * mag);
#else
	int value1, value2;
	double fvalue1, fvalue2;
	complex_number noise;

	value1 = RAND_MAX + 1 - rand();	// range from 1 to RAND_MAX
	value2 = RAND_MAX + 1 - rand();	// range from 1 to RAND_MAX
	fvalue1 = (double)value1 / (RAND_MAX + 1);
	fvalue2 = (double)value2 / (RAND_MAX + 1);
	// scale noise power to be Sigma^2
	noise.real = sqrt(-log(fvalue1) * 2) * cos(PI2 * fvalue2) * Sigma;
	noise.imag = sqrt(-log(fvalue1) * 2) * sin(PI2 * fvalue2) * Sigma;

	return noise;
#endif
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

int QuantSamplesIQ16(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale)
{
    int i;
    int QuantValue;
    
    const double Gain = GainScale * 3277;
    int ClippedCount = 0;

    for (i = 0; i < Length; i++)
    {
        QuantValue = (int)(Samples[i].real * Gain);  
        if (QuantValue > 32767)  
        {
            QuantValue = 32767;
            ClippedCount++;
        }
        else if (QuantValue < -32768)
        {
            QuantValue = -32768;
            ClippedCount++;
        }
      
        QuantSamples[i * 4] = (unsigned char)(QuantValue & 0xff);    
        QuantSamples[i * 4 + 1] = (unsigned char)((QuantValue >> 8) & 0xff);

        QuantValue = (int)(Samples[i].imag * Gain); 
        if (QuantValue > 32767) 
        {
            QuantValue = 32767;
            ClippedCount++;
        }
        else if (QuantValue < -32768)
        {
            QuantValue = -32768;
            ClippedCount++;
        }

        QuantSamples[i * 4 + 2] = (unsigned char)(QuantValue & 0xff);  
        QuantSamples[i * 4 + 3] = (unsigned char)((QuantValue >> 8) & 0xff); 
    }

    return ClippedCount;
}
