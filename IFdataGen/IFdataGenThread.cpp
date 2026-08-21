#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <iostream>
#include <cstring>
#include <chrono>
#include <string>
#include <vector>
#include <ctime>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <thread>
#include <queue>

#include "SignalSim.h"
#include "ScenarioData.h"
#include "NavBitArray.h"

#define TOTAL_GPS_SAT 32
#define TOTAL_BDS_SAT 63
#define TOTAL_GAL_SAT 36
#define TOTAL_GLO_SAT 24
#define TOTAL_SAT_CHANNEL 128

struct CommandArguments
{
	std::string ConfigFile;
	std::string OutputFile;
	bool MultiThread;
	bool ValidateOnly;
	bool OutputTag;
};

int CreateSatIfSignal(const char *SignalName, int IfFreq, GnssSystem System, int SignalIndex, CSatelliteParam *SatParamList[], CSatIfSignal* SatIfSignal[], BOOL bCreateSignal);
complex_number GenerateNoise(double Sigma);
int QuantSamplesIQ2(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
int QuantSamplesIQ4(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
int QuantSamplesIQ8(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);
int QuantSamplesIQ16(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale);

void ShowHelp(const char* ProgramName);
bool ParseCommandLineArgs(int argc, char* argv[], CommandArguments &Arguments);
void CreateTagFile(const std::string& tagFilePath, const OUTPUT_PARAM& outputParam);

CScenarioData ScenarioData;
CNavBitArray NavBitArray;
KINEMATIC_INFO CurPosVel;
LLA_POSITION CurPosLla;
GNSS_TIME CurGnssTime;
UTC_TIME CurUtcTime;
/*CTrajectory Trajectory;
CPowerControl PowerControl;
CNavData NavData;
OUTPUT_PARAM OutputParam;
GNSS_TIME CurTime;
PGPS_EPHEMERIS GpsEph[TOTAL_GPS_SAT], GpsEphVisible[TOTAL_GPS_SAT];
PGPS_EPHEMERIS BdsEph[TOTAL_BDS_SAT], BdsEphVisible[TOTAL_BDS_SAT];
PGPS_EPHEMERIS GalEph[TOTAL_GAL_SAT], GalEphVisible[TOTAL_GAL_SAT];
PGLONASS_EPHEMERIS GloEph[TOTAL_GLO_SAT], GloEphVisible[TOTAL_GLO_SAT];
CSatelliteParam GpsSatParam[TOTAL_GPS_SAT], BdsSatParam[TOTAL_BDS_SAT], GalSatParam[TOTAL_GAL_SAT], GloSatParam[TOTAL_GLO_SAT];	// satellite parameter array at CurTime
int GpsSatNumber, BdsSatNumber, GalSatNumber, GloSatNumber;	// number of visible satellite*/
int TotalChannelNumber;
const int SignalCenterFreq[][8] = {
	{ FREQ_GPS_L1, FREQ_GPS_L1, FREQ_GPS_L2, FREQ_GPS_L2, FREQ_GPS_L5 },
	{ FREQ_BDS_B1C, FREQ_BDS_B1I, FREQ_BDS_B2I, FREQ_BDS_B3I, FREQ_BDS_B2a, FREQ_BDS_B2b, FREQ_BDS_B2ab },
	{ FREQ_GAL_E1, FREQ_GAL_E5a, FREQ_GAL_E5b, FREQ_GAL_E5, FREQ_GAL_E6 },
	{ FREQ_GLO_G1, FREQ_GLO_G2 },
};
const char *SignalName[][8] = {
	{ "GPS L1CA", "GPS L1C", "GPS L2C", "GPS L2P", "GPS L5", },
	{ "BDS B1C", "BDS B1I", "BDS B2I", "BDS B3I", "BDS B2a", "BDS B2b", "BDS B2ab", },
	{ "Galileo E1", "Galileo E5a", "Galileo E5b", "Galileo E5", "Galileo E6", },
	{ "GLONASS G1", "GLONASS G2", },
};

// thread synchronize variables
std::mutex mtx;
std::condition_variable cv_task;	// to notify task thread start
std::condition_variable cv_ready;	// to notify main thread ready
//std::condition_variable cv_done;	// for task complete notification

std::queue<CSatIfSignal*> task_queue;
int total_threads = 0;
int ready_tasks = 0;
int idle_threads = 0;
int ready_generation = 0;
int work_generation = 0;
bool shutdown = false;

int exec_cycle = 0;

// IF signal generation thread
void IF_generate_thread(int thread_id)
{
	while (true)
	{
		int current_ready_gen;
		// stage 1: wait for main thread to dispatch tasks (production mode)
		{
			std::unique_lock<std::mutex> lock(mtx);
			if (shutdown) break;

			current_ready_gen = ready_generation;
			ready_tasks ++;

			// all threads are ready, notify main thread to dispatch tasks
			if (ready_tasks == total_threads)
				cv_ready.notify_one();

			// wait for main thread to release (update cycle or issue stop command)
			cv_task.wait(lock, [&] { return ready_generation > current_ready_gen || shutdown; });

			if (shutdown) break;
		}

		// stage 2: acquire tasks from the queue and execute (consumer mode)
		while (true)
		{
			CSatIfSignal *task;
			bool has_task = false;

			{
				std::unique_lock<std::mutex> lock(mtx);
				if (!task_queue.empty())
				{
					task = task_queue.front();
					task_queue.pop();
					has_task = true;
				}
				else
				{
					// queue is empty, mark this thread as idle and wait for next round
					idle_threads++;
					// if all threads are idle, notify main thread to combine IF data
					if (idle_threads == total_threads)
						cv_ready.notify_one(); // notify main thread to combine IF data
					
					int current_work_gen = work_generation;
					// wait for main thread to complete data combination and start next round
					cv_task.wait(lock, [&] { return work_generation > current_work_gen || shutdown; });
					
					break; // exit the inner task acquisition loop and return to the top-level ready stage
				}
			}

			// if we have a task, execute it outside the lock to allow other threads to acquire tasks
			if (has_task)
				task->GetIfSample(CurGnssTime);
		}
		
		if (shutdown) break;
	}
}

int main(int argc, char* argv[])
{
	int i, j;
//	JsonStream JsonTree;
//	JsonObject *Object;
//	LOCAL_SPEED StartVel;
//	GLONASS_TIME GlonassTime;
//	GNSS_TIME BdsTime;
//	int ListCount;
//	PSIGNAL_POWER PowerList;
//	int FreqLow, FreqHigh;
	CSatelliteParam *GpsSatParam[TOTAL_GPS_SAT], *BdsSatParam[TOTAL_BDS_SAT], *GalSatParam[TOTAL_GAL_SAT], *GloSatParam[TOTAL_GLO_SAT];
	CSatIfSignal* SatIfSignal[TOTAL_SAT_CHANNEL];
	int SignalIndex;
	int IfFreq;
	complex_number *NoiseArray;
	unsigned char *QuantArray;
	FILE* IfFile = NULL;
	CommandArguments Arguments;
	int ThreadNumber = std::thread::hardware_concurrency();
	unsigned int *SignalSelect = ScenarioData.OutputParam.FreqSelect;
	OutputFormat DataFormat;
	int CenterFreq, SampleFreq;

	std::vector<std::thread> threads;

	// Default arguments
	Arguments.ConfigFile = "IfGenTest.json"; // Default JSON file
	Arguments.OutputFile = "";
	Arguments.MultiThread = true; // Default to use multi-threading
	Arguments.ValidateOnly = false;
	Arguments.OutputTag = false;

	SetOutputFile(stdout);
//	SetOutputLevel(MSG_LEVEL_INFO);

	// Show help if no arguments provided
	if (argc == 1)
	{
		ShowHelp(argv[0]);
		return 0;
	}

	if (!ParseCommandLineArgs(argc, argv, Arguments))
		return 1;
	if (Arguments.MultiThread && ThreadNumber <= 2)
	{
		std::cerr << "[WARNING] Not enough CPU cores for multi-threading\n";
		Arguments.MultiThread = false;
	}
	
	if (!Arguments.OutputFile.empty())
	{
		// Override output filename
		strncpy(ScenarioData.OutputParam.filename, Arguments.OutputFile.c_str(), 255);
		ScenarioData.OutputParam.filename[255] = '\0';
		printf("[INFO]\tUsing output file from command line: %s\n", ScenarioData.OutputParam.filename);
	}

	printf("\n================================================================================\n");
	printf("                          IF SIGNAL GENERATION \n");
	printf("================================================================================\n");

	// Read the JSON file
	printf("[INFO]\tLoading JSON file: %s\n", Arguments.ConfigFile.c_str());
	if (ScenarioData.LoadScenarioFile(Arguments.ConfigFile.c_str()) < 0)
	{
		std::cerr << "[ERROR]\tUnable to read JSON file: " << Arguments.ConfigFile << "\n";
		return 1;
	}
	else
	{
		printf("[INFO]\tJSON file read successfully: %s\n", Arguments.ConfigFile.c_str());
	}

	if (!Arguments.ValidateOnly)
	{
		printf("[INFO]\tOpening output file: %s\n", ScenarioData.OutputParam.filename);
		if ((IfFile = fopen(ScenarioData.OutputParam.filename, "wb")) == NULL)
		{
			printf("[ERROR]\tFailed to open output file: %s\n", ScenarioData.OutputParam.filename);
			return 0;
		}
		printf("[INFO]\tOutput file opened successfully.\n");
	}

	if (Arguments.OutputTag)
	{
		std::string TagFileName = ScenarioData.OutputParam.filename;
		TagFileName += ".tag";	// append .tag
		CreateTagFile(TagFileName, ScenarioData.OutputParam);
	}

	DataFormat = ScenarioData.OutputParam.Format;
	SampleFreq = ScenarioData.OutputParam.SampleFreq;
	CenterFreq = ScenarioData.OutputParam.CenterFreq;

	ScenarioData.GetCurTime(CurGnssTime, CurUtcTime);
	ScenarioData.NavData.CompleteAlmanac(BdsSystem, CurUtcTime);
	ScenarioData.NavData.CompleteAlmanac(GalileoSystem, CurUtcTime);

	// create naviagtion bit instances
	NavBitArray.CreateNavBitArray(SignalSelect);
	NavBitArray.SetEphemeris(GpsSystem, ScenarioData.GpsEphUpdateMask, ScenarioData.GpsEph);
	NavBitArray.SetEphemeris(BdsSystem, ScenarioData.BdsEphUpdateMask, ScenarioData.BdsEph);
	NavBitArray.SetEphemeris(GalileoSystem, ScenarioData.GalEphUpdateMask, ScenarioData.GalEph);
	NavBitArray.SetEphemeris(GlonassSystem, ScenarioData.GloEphUpdateMask, (PGPS_EPHEMERIS *)ScenarioData.GloEph);

	// create CSatIfSignal class for visible satellite, all other satellites clear pointer to NULL
	printf("[INFO]\tGenerating IF data with following satellite signals:\n\n");
	
	// Enhanced signal display with cleaner formatting
	printf("[INFO]\tEnabled Signals:\n");

	// Count total signals per system
	int GpsSignalCount = 0, BdsSignalCount = 0, GalSignalCount = 0, GloSignalCount = 0;
	for (SignalIndex = SIGNAL_INDEX_L1CA; SignalIndex <= SIGNAL_INDEX_L5; SignalIndex++)
		if (SignalSelect[GpsSystem] & (1 << SignalIndex)) GpsSignalCount++;
	for (SignalIndex = SIGNAL_INDEX_B1C; SignalIndex <= SIGNAL_INDEX_B2b; SignalIndex++)
		if (SignalSelect[BdsSystem] & (1 << SignalIndex)) BdsSignalCount++;
	for (SignalIndex = SIGNAL_INDEX_E1; SignalIndex <= SIGNAL_INDEX_E6; SignalIndex++)
		if (SignalSelect[GalileoSystem] & (1 << SignalIndex)) GalSignalCount++;
	for (SignalIndex = SIGNAL_INDEX_G1; SignalIndex <= SIGNAL_INDEX_G2; SignalIndex++)
		if (SignalSelect[GlonassSystem] & (1 << SignalIndex)) GloSignalCount++;


	printf("Signals Summary Table:\n");
	
	// Enhanced constellation summary table
	printf("+---------------+-------------+--------------+------------------------------+\n");
	printf("| Constellation | Visible SVs | Signals / SV | Total Signals / Visible SVs  |\n");
	printf("+---------------+-------------+--------------+------------------------------+\n");
	printf("| GPS           | %-11d | %-12d | %-28d |\n", ScenarioData.GpsSatNumber, GpsSignalCount, ScenarioData.GpsSatNumber * GpsSignalCount);
	printf("| BeiDou        | %-11d | %-12d | %-28d |\n", ScenarioData.BdsSatNumber, BdsSignalCount, ScenarioData.BdsSatNumber * BdsSignalCount);
	printf("| Galileo       | %-11d | %-12d | %-28d |\n", ScenarioData.GalSatNumber, GalSignalCount, ScenarioData.GalSatNumber * GalSignalCount);
	printf("| GLONASS       | %-11d | %-12d | %-28d |\n", ScenarioData.GloSatNumber, GloSignalCount, ScenarioData.GloSatNumber * GloSignalCount);
	printf("+---------------+-------------+--------------+------------------------------+\n");
	
	int TotalVisibleSVs = ScenarioData.GpsSatNumber + ScenarioData.BdsSatNumber + ScenarioData.GalSatNumber + ScenarioData.GloSatNumber;
	int TotalChannels = ScenarioData.GpsSatNumber * GpsSignalCount + ScenarioData.BdsSatNumber * BdsSignalCount + ScenarioData.GalSatNumber * GalSignalCount + ScenarioData.GloSatNumber * GloSignalCount;
	printf("Total Visible SVs = %d, Total channels = %d\n\n", TotalVisibleSVs, TotalChannels);

	memset(SatIfSignal, 0, sizeof(SatIfSignal));
	TotalChannelNumber = 0;
	// Detailed satellite and signal information in compact table format
	for (SignalIndex = SIGNAL_INDEX_L1CA; SignalIndex <= SIGNAL_INDEX_L5; SignalIndex++)
	{
		if (!(SignalSelect[GpsSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[0][SignalIndex] - CenterFreq * 1000;
		TotalChannelNumber += CreateSatIfSignal(SignalName[0][SignalIndex], IfFreq, GpsSystem, SignalIndex, GpsSatParam, SatIfSignal + TotalChannelNumber, !Arguments.ValidateOnly);
	}
	for (SignalIndex = SIGNAL_INDEX_B1C; SignalIndex <= SIGNAL_INDEX_B2b; SignalIndex++)
	{
		if (!(SignalSelect[BdsSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[1][SignalIndex] - CenterFreq * 1000;
		TotalChannelNumber += CreateSatIfSignal(SignalName[1][SignalIndex], IfFreq, BdsSystem, SignalIndex, BdsSatParam, SatIfSignal + TotalChannelNumber, !Arguments.ValidateOnly);
	}
	for (SignalIndex = SIGNAL_INDEX_E1; SignalIndex <= SIGNAL_INDEX_E6; SignalIndex++)
	{
		if (!(SignalSelect[GalileoSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[2][SignalIndex] - CenterFreq * 1000;
		TotalChannelNumber += CreateSatIfSignal(SignalName[2][SignalIndex], IfFreq, GalileoSystem, SignalIndex, GalSatParam, SatIfSignal + TotalChannelNumber, !Arguments.ValidateOnly);
	}
	
	for (SignalIndex = SIGNAL_INDEX_G1; SignalIndex <= SIGNAL_INDEX_G2; SignalIndex++)
	{
		if (!(SignalSelect[GlonassSystem] & (1 << SignalIndex)))
			continue;
		IfFreq = SignalCenterFreq[3][SignalIndex] - CenterFreq * 1000;
		TotalChannelNumber += CreateSatIfSignal(SignalName[3][SignalIndex], IfFreq, GlonassSystem, SignalIndex, GloSatParam, SatIfSignal + TotalChannelNumber, !Arguments.ValidateOnly);
	}
	printf("Total channels: %d\n\n", TotalChannelNumber);

	NoiseArray = new complex_number[SampleFreq];
	QuantArray = new unsigned char[SampleFreq * 4];

	// Calculate total data size and setup progress tracking
	int totalDurationMs = (int)(ScenarioData.Trajectory.GetTimeLength() * 1000);
	double bytesPerMs = SampleFreq * ((DataFormat == OutputFormatIQ2) ? 0.5 : (DataFormat == OutputFormatIQ4) ? 1.0: (DataFormat == OutputFormatIQ16) ? 4.0 : 2.0);
	double totalMB = (totalDurationMs * bytesPerMs) / (1024.0 * 1024.0);
	long long TotalClippedSamples = 0;
	long long TotalSamples = 0;
	double AGCGain = 1.0;
	printf("[INFO]\tStarting signal generation loop...\n");
	printf("[INFO]\tSignal Duration: %0.2f s\n", totalDurationMs/1000.0);
	printf("[INFO]\tSignal Size: %.2f MB\n", totalMB);
	printf("[INFO]\tSignal Data format: %s\n", (DataFormat == OutputFormatIQ2) ? "IQ2" : (DataFormat == OutputFormatIQ4) ? "IQ4":(DataFormat == OutputFormatIQ16) ? "IQ16" : "IQ8");
	printf("[INFO]\tSignal Center freq: %0.4f MHz\n", CenterFreq/1000.0);
	printf("[INFO]\tSignal Sample rate: %0.4f MHz\n\n", SampleFreq/1000.0);
	fflush(stdout);

	if (Arguments.ValidateOnly)
		return 0;

	if (Arguments.MultiThread)
	{
		total_threads = ThreadNumber;
		for (int i = 0; i < ThreadNumber; i ++)
			threads.emplace_back(IF_generate_thread, i);
	}

	auto start_time = std::chrono::high_resolution_clock::now();

	while (ScenarioData.StepForward(FALSE, TRUE, 1) > 0)
	{
		ScenarioData.GetCurTime(CurGnssTime, CurUtcTime);
		if (Arguments.MultiThread)
		{
			// wait for all threads ready
			{
				std::unique_lock<std::mutex> lock(mtx);
				cv_ready.wait(lock, [&] { return ready_tasks == total_threads; });

				// add tasks to the queue for this cycle
				for (i = 0; i < TOTAL_SAT_CHANNEL; i++)
				{
					if (!SatIfSignal[i])
						continue;
					task_queue.push(SatIfSignal[i]);
				}

				// reset counts and notify workers to start processing
				ready_tasks = 0;
				idle_threads = 0; // reset idle count
				ready_generation++;
				cv_task.notify_all(); 
			}
		}

		// generate white noise
		for (i = 0; i < SampleFreq; i ++)
			NoiseArray[i] = GenerateNoise(1.0);

		if (!Arguments.MultiThread)	// for single thread, just call GetIfSample() for each channel
		{
			for (i = 0; i < TOTAL_SAT_CHANNEL; i++)
			{
				if (!SatIfSignal[i])
					continue;
				SatIfSignal[i]->GetIfSample(CurGnssTime);
			}
		}
		else
		{
			// wait all tasks in queue completed and all threads idle
			{
				std::unique_lock<std::mutex> lock(mtx);
				cv_ready.wait(lock, [&] { return idle_threads == total_threads; });

				work_generation++;
				cv_task.notify_all();
			}
		}

		for (i = 0; i < TOTAL_SAT_CHANNEL; i++)
		{
			if (!SatIfSignal[i])
				continue;
			for (j = 0; j < SampleFreq; j++)
				NoiseArray[j] += SatIfSignal[i]->SampleArray[j];
		}
		exec_cycle ++;

		if (DataFormat == OutputFormatIQ2) 
		{
			TotalClippedSamples += QuantSamplesIQ2(NoiseArray, SampleFreq, QuantArray, AGCGain);
			// Pack 2 samples per byte
			fwrite(QuantArray, sizeof(unsigned char), SampleFreq / 2, IfFile);	// 1/2 byte/sample
		}
		else if (DataFormat == OutputFormatIQ4) 
		{
			TotalClippedSamples += QuantSamplesIQ4(NoiseArray, SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char), SampleFreq, IfFile); // 1 byte/sample
		}
		else if (DataFormat == OutputFormatIQ8) 
		{
			TotalClippedSamples += QuantSamplesIQ8(NoiseArray, SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char) * 2, SampleFreq, IfFile); // 2 bytes/sample
		}
		else
		{
			TotalClippedSamples += QuantSamplesIQ16(NoiseArray, SampleFreq, QuantArray, AGCGain);
			fwrite(QuantArray, sizeof(unsigned char) * 4, SampleFreq, IfFile); // 4 bytes/sample
		}
		TotalSamples += SampleFreq * 2; // I and Q

#if 1
		// Adjust gain every 100ms
		if ((exec_cycle % 100) == 0)
		{
			double ClippingRate = (double)TotalClippedSamples / TotalSamples;
			if (ClippingRate > 0.01) // clipped rate over 1%
			{
				AGCGain *= 0.95; // reduce gain by 5%
				printf("[WARNING]\tAGC: Clipping %.2f%%, reducing gain to %.3f\n", ClippingRate * 100, AGCGain);
				TotalClippedSamples = TotalSamples = 0;	// reset statistic
			}
			else if (ClippingRate < 0.001 && AGCGain < 1.0) // clipped rate under 0.1%
			{
				AGCGain *= 1.02; // increase gain by 2%
				if (AGCGain > 1.0) AGCGain = 1.0;
				printf("[WARNING]\tAGC: Clipping %.2f%%, increasing gain to %.3f\n", ClippingRate * 100, AGCGain);
				TotalClippedSamples = TotalSamples = 0;	// reset statistic
			}
		}
#endif

//		for (j = 0; j < OutputParam.SampleFreq; j ++)
//			printf("%f %f\n", NoiseArray[j].real, NoiseArray[j].imag);
		// Enhanced progress reporting with percentage, MB/s, and ETA
		if ((exec_cycle % 25) == 0)
		{
			auto current_time = std::chrono::high_resolution_clock::now();
			auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time).count();
			
			double percentage = (double)exec_cycle / totalDurationMs * 100.0;
			double currentMB = (exec_cycle * bytesPerMs) / (1024.0 * 1024.0);
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
				   exec_cycle, totalDurationMs, currentMB, totalMB, mbPerSec, etaStr);
			fflush(stdout);
		}
//		if (length == 2) break;
	}

	// terminate all threads
	{
		std::lock_guard<std::mutex> lock(mtx);
		shutdown = true;
//		std::cout << "\nterminate all threads" << std::endl;
	}
	cv_task.notify_all();

	// wait all threads complete
	for (auto& t : threads)
	{
		if (t.joinable()) t.join();
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
	double finalMB = (exec_cycle * bytesPerMs) / (1024.0 * 1024.0);
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

	for (i = 0; i < TOTAL_SAT_CHANNEL; i ++)
		if (SatIfSignal[i]) delete SatIfSignal[i];
	delete[] NoiseArray;
	delete[] QuantArray;
	fclose(IfFile);

	return 0;
}

int CreateSatIfSignal(const char *SignalName, int IfFreq, GnssSystem System, int SignalIndex, CSatelliteParam *SatParamList[], CSatIfSignal* SatIfSignal[], BOOL bCreateSignal)
{
	printf("GPS %s with IF %+dkHz:\n", SignalName, IfFreq / 1000);
	printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
	printf("| SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) | SV | Doppler (Hz) |\n");
	printf("+----+--------------+----+--------------+----+--------------+----+--------------+\n");
	int SatNumber = ScenarioData.GetSatelliteParam(System, SatParamList);
	int i, svCount = 0;
	for (i = 0; i < SatNumber; i ++)
	{
		if (TotalChannelNumber >= TOTAL_SAT_CHANNEL)
			break;
		if (bCreateSignal)
		{
			SatIfSignal[i] = new CSatIfSignal(ScenarioData.OutputParam.SampleFreq, IfFreq, System, SignalIndex, SatParamList[i]->svid);
			SatIfSignal[i]->InitState(CurGnssTime, SatParamList[i], NavBitArray.GetNavBit(GpsSystem, SignalIndex));
		}
		TotalChannelNumber++;
			
		if (svCount % 4 == 0) printf("|");
		printf(" %02d | %+12d |", SatParamList[i]->svid, (int)SatParamList[i]->GetDoppler(SignalIndex));
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
	return i;
}

complex_number GenerateNoise(double Sigma)
{
	double fvalue1, fvalue2, mag;

	// Marsaglia Polar method (improved Box-Muller method)
	do
	{
		fvalue1 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
		fvalue2 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
		mag = fvalue1 * fvalue1 + fvalue2 * fvalue2;
	} while (mag >= 1.0 || mag == 0.0);
	mag = sqrt(-2.0 * log(mag) / mag) * Sigma;

	return complex_number(fvalue1 * mag, fvalue2 * mag);
}

// PocketSDR compatible 2-bit IQ quantization 
// (TODO: Test)
// (FIXME: Optimize)
 int QuantSamplesIQ2(complex_number Samples[], int Length, unsigned char QuantSamples[], double GainScale)
 {
	int ClippedCount = 0;
	const double threshold = 1.1 / GainScale;	// the optimal threshold for Gauss noise is Sigma, increase a little to compensate added signal power
	const double ClippedThreshold = 5 * threshold;	// clip threshold set to 5 Sigma
	double Value;
	unsigned char QuantByte;

	 // Process 2 complex samples at a time to produce 1 byte of output.
	 // Bit definition within each byte is (from MSB): Sign-Q2, Mag-Q2, Sign-I2, Mag-I2, Sign-Q1, Mag-Q1, Sign-I1, Mag-I1
	for (int i = 0; i < Length; i += 2)
	{
		QuantByte = (Samples[i].real < 0.0) ? 2 : 0;
		Value = fabs(Samples[i].real);
		QuantByte |= (Value < threshold) ? 0 : 1;
		if (Value >= ClippedThreshold) ClippedCount ++;
		QuantByte |= (Samples[i].imag < 0.0) ? 8 : 0;
		Value = fabs(Samples[i].imag);
		QuantByte |= (Value < threshold) ? 0 : 4;
		if (Value >= ClippedThreshold) ClippedCount ++;

		QuantByte = (Samples[i+1].real < 0.0) ? 0x20 : 0;
		Value = fabs(Samples[i+1].real);
		QuantByte |= (Value < threshold) ? 0 : 0x10;
		if (Value >= ClippedThreshold) ClippedCount ++;
		QuantByte |= (Samples[i+1].imag < 0.0) ? 0x80 : 0;
		Value = fabs(Samples[i+1].imag);
		QuantByte |= (Value < threshold) ? 0 : 0x40;
		if (Value >= ClippedThreshold) ClippedCount ++;
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

void ShowHelp(const char* ProgramPath)
{
	// Extract just the executable name from the path
	std::string PathName = ProgramPath;
	std::string ProgramName;
	size_t pos = PathName.find_last_of("/\\");
	if (pos == std::string::npos)
		ProgramName = PathName;
	else
		ProgramName = PathName.substr(pos + 1);
	
	std::cout << "IFDataGen - GNSS IF Data Generator\n\n";
	std::cout << "Usage: " << ProgramName << " [options]\n\n";
	std::cout << "Available options:\n";
	std::cout << "   -c, 	--config <FILE>    Configuration file (JSON) [REQUIRED]\n";
	std::cout << "   -o, 	--output <FILE>    Output IF data file (overrides config)\n";
	std::cout << "   -vo, 	--validate-only    Validate configuration and exit\n";
	std::cout << "   -mt, 	--multi-thread     Force use multi-thread\n";
	std::cout << "   -st, 	--single-thread    Force use single-thread\n";
	std::cout << "   -t,  	--tag              Output tag file (output file name with .tag appended)\n";
	std::cout << "   -v, 	--version          Show version information\n";
	std::cout << "   -h, 	--help             Show this help message\n\n";
	std::cout << "Examples:\n";
	std::cout << "   " << ProgramName << " -c config.json\n";
	std::cout << "   " << ProgramName << " --config config.json --output mydata.bin\n";
	std::cout << "   " << ProgramName << " -c config.json -o output.bin -st\n";
	std::cout << "   " << ProgramName << " --config config.json -vo\n\n";
}

bool ParseCommandLineArgs(int argc, char* argv[], CommandArguments &Arguments)
{
	const std::vector<std::string> CommandList = {
		"--help", "-h",	// 0
		"--config", "-c",	// 1
		"--output", "-o",	// 2
		"--validate-only", "-vo",	// 3
		"--multi-thread", "-mt",	// 4
		"--single-thread", "-st",	// 5
		"--tag", "-t",	// 6
	};
	std::string arg;
	int i = 1, index;

	while (i < argc)
	{
		auto it = std::find(CommandList.begin(), CommandList.end(), argv[i]);
		index = (it == CommandList.end()) ? -1 : (it - CommandList.begin()) / 2;
		arg = argv[i];

		switch (index)
		{
		case 0:	// --help
			ShowHelp(argv[0]);
			exit(0);
		case 1:	// --config
			if (i + 1 >= argc || argv[i+1][0] == '-')
			{
				std::cerr << "[ERROR] " << arg << " requires a filename argument\n";
				return false;
			}
			Arguments.ConfigFile = argv[++i];
			break;
		case 2:	// --output
			if (i + 1 >= argc || argv[i+1][0] == '-')
			{
				std::cerr << "[ERROR] " << arg << " requires a filename argument\n";
				return false;
			}
			Arguments.OutputFile = argv[++i];
			break;
		case 3:	// --validate-only
			Arguments.ValidateOnly = true;
			break;
		case 4:	// --multi-thread
			Arguments.MultiThread = true;
			break;
		case 5:	// --single-thread
			Arguments.MultiThread = false;
			break;
		case 6:	// --tag
			Arguments.OutputTag = true;
			break;
		default:
			std::cout << "[WARNING] Unknown option " << arg << "\n";
		}
		i ++;	// move to next argument
	}

	return true;
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
	}else if (outputParam.Format == OutputFormatIQ16){
	formatStr = "INT16X2";
		iqStr = "1";
		bitsStr = "16"; 
	}else { // OutputFormatIQ8
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
