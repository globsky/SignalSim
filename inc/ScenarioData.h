//----------------------------------------------------------------------
// ScenarioData.h:
//   class than encapsulate scenario data analysis and process
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __SCENARIO_DATA_H__
#define __SCENARIO_DATA_H__

#include "BasicTypes.h"
#include "Trajectory.h"
#include "GnssTime.h"
#include "NavData.h"
#include "PowerControl.h"
#include "SatelliteParam.h"
#include "JsonParser.h"

class CScenarioData
{
public:
	CScenarioData();
	~CScenarioData();

public:
	// constants for maximum number of satellites in each system
	static const int TOTAL_GPS_SAT = 32;
	static const int TOTAL_BDS_SAT = 63;
	static const int TOTAL_GAL_SAT = 36;
	static const int TOTAL_GLO_SAT = 24;
public:
	// scenario data
	JsonStream JsonTree;
	CTrajectory Trajectory;
	CNavData NavData;
	CPowerControl PowerControl;
	//DELAY_CONFIG DelayConfig;
	OUTPUT_PARAM OutputParam;
	// variables to do data processing
	CIonoKlobuchar8 IonoModel;
	PGPS_EPHEMERIS GpsEph[TOTAL_GPS_SAT], GpsEphVisible[TOTAL_GPS_SAT];
	PGPS_EPHEMERIS BdsEph[TOTAL_BDS_SAT], BdsEphVisible[TOTAL_BDS_SAT];
	PGPS_EPHEMERIS GalEph[TOTAL_GAL_SAT], GalEphVisible[TOTAL_GAL_SAT];
	PGLONASS_EPHEMERIS GloEph[TOTAL_GLO_SAT], GloEphVisible[TOTAL_GLO_SAT];
	GNSS_TIME CurTime;
	UTC_TIME UtcTime;
	GNSS_TIME BdsTime;
	GLONASS_TIME GlonassTime;
	KINEMATIC_INFO CurPosEcef;
	LLA_POSITION CurPosLla;
	CSatelliteParam GpsSatParam[TOTAL_GPS_SAT], BdsSatParam[TOTAL_BDS_SAT], GalSatParam[TOTAL_GAL_SAT], GloSatParam[TOTAL_GLO_SAT];
	int GpsSatNumber, BdsSatNumber, GalSatNumber, GloSatNumber;
	unsigned long long GpsEphUpdateMask, BdsEphUpdateMask, GalEphUpdateMask, GloEphUpdateMask;
protected:
	void UpdateSatelliteParam(int TimeStepMs);
	void UpdateSatList();
	void FilterSignal();

public:
	// methods to process scenario data
	int LoadScenarioFile(const char* filename);
	int LoadScenarioObject(JsonObject *Object, const char* JsonFilePath = nullptr);
	int GetCurPosVel(KINEMATIC_INFO& CurPosVel, LLA_POSITION &CurPosLla);
	int GetCurTime(GNSS_TIME& CurGnssTime, UTC_TIME& CurUtcTime);
	int GetSatelliteParam(GnssSystem system, CSatelliteParam* SatParam[]);
	int StepForward(BOOL bUpdateSatList, BOOL bUpdateSatParam = TRUE, int TimeStepMs = 0);
};

#endif // __SCENARIO_DATA_H__
