//----------------------------------------------------------------------
// SatelliteParam.h:
//   Definition of functions to calculate satellite parameters
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__SATELLITE_PARAM_H__)
#define __SATELLITE_PARAM_H__

#include "ConstVal.h"
#include "BasicTypes.h"
#include "PowerControl.h"
#include "DelayModel.h"

int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[]);
int GetGlonassVisibleSatellite(KINEMATIC_INFO Position, GLONASS_TIME time, OUTPUT_PARAM OutputParam, PGLONASS_EPHEMERIS Eph[], int Number, PGLONASS_EPHEMERIS EphVisible[]);
void GetSatelliteParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam, PSATELLITE_PARAM SatelliteParam);
void GetSatelliteCN0(int PowerListCount, SIGNAL_POWER PowerList[], double DefaultCN0, enum ElevationAdjust Adjust, PSATELLITE_PARAM SatelliteParam);
double GetWaveLength(int system, int SignalIndex, int FreqID);
double GetTravelTime(PSATELLITE_PARAM SatelliteParam, int SignalIndex);
double GetCarrierPhase(PSATELLITE_PARAM SatelliteParam, int SignalIndex);
double GetDoppler(PSATELLITE_PARAM SatelliteParam, int SignalIndex);
GNSS_TIME GetTransmitTime(GNSS_TIME ReceiverTime, double TravelTime);

// new CSatelliteParam class to support ephemeris change, precise ephemeris and different troposphere delay/ionosphere delay model
class CSatelliteParam
{
public:
	CSatelliteParam();
	~CSatelliteParam();

	void Initialize(GnssSystem SatSystem, PGPS_EPHEMERIS Eph, CIonoDelay *IonoModel, double InitCN0, enum ElevationAdjust Adjust);
	void UpdateEphemeris(PGPS_EPHEMERIS Eph);
	void CalculateParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time);
	void UpdateCN0(int PowerListCount, SIGNAL_POWER PowerList[]);
	double GetTravelTime(int SignalIndex);
	double GetCarrierPhase(int SignalIndex);
	double GetDoppler(int SignalIndex);

	GnssSystem system;
	int svid;
	int FreqID;	// for GLONASS only
	PGPS_EPHEMERIS EphCur, EphPrev;
	PGLONASS_EPHEMERIS GloEphCur, GloEphPrev;
	CIonoDelay *IonoDelayModel;
	double CN0Default;
	enum ElevationAdjust CN0Adjust;
	int EphTransition;

	int CN0;	// scale factor 0.01
	int TimeTag;
	KINEMATIC_INFO PosVel;
	double Acc[3];
	double TravelTime;	// travel time including corrections except group delay and ionosphere delay in second
	double IonoDelayMeter, TropoDelayMeter;	// ionosphere delay, troposphere delay and satellite clock error in meter
	double ClockError;	// satellite clock error in seconds
	double GroupDelay[8];	// group delay in second, first element for TGD of L1, all followings for ISC using SIGNAL_INDEX_XXXX as index
	double Elevation;	// satellite elevation in rad
	double Azimuth;		// satellite azimuth in rad
	double RelativeSpeed;	// satellite to receiver relative speed in m/s
	double LosVector[3];	// LOS vecter

private:
	double GetWaveLength(int SignalIndex);
	double GetIonoDelayFactor(int SignalIndex);
};

#endif //!defined(__SATELLITE_PARAM_H__)
