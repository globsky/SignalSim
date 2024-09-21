//----------------------------------------------------------------------
// SatelliteParam.h:
//   Definition of functions to calculate satellite parameters
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__SATELLITE_PARAM_H__)
#define __SATELLITE_PARAM_H__

#include "BasicTypes.h"
#include "XmlInterpreter.h"
#include "PowerControl.h"

// signal index in GroupDelay array
#define FREQ_INDEX_GPS_L1 0
#define FREQ_INDEX_GPS_L2 1
#define FREQ_INDEX_GPS_L5 2
#define FREQ_INDEX_BDS_B1C 0
#define FREQ_INDEX_BDS_B1I 1
#define FREQ_INDEX_BDS_B2I 2
#define FREQ_INDEX_BDS_B3I 3
#define FREQ_INDEX_BDS_B2a 4	// for pilot channel
#define FREQ_INDEX_BDS_B2b 5	// for pilot channel
#define FREQ_INDEX_BDS_B2ab 6
#define FREQ_INDEX_GAL_E1 0
#define FREQ_INDEX_GAL_E5a 1	// for pilot channel
#define FREQ_INDEX_GAL_E5b 2	// for pilot channel
#define FREQ_INDEX_GAL_E5 3
#define FREQ_INDEX_GAL_E6 4
#define FREQ_INDEX_GLO_G1 0
#define FREQ_INDEX_GLO_G2 1

// signal frequency
#define FREQ_GPS_L1 1575.42e6
#define FREQ_GPS_L2 1227.6e6
#define FREQ_GPS_L5 1176.45e6
#define FREQ_BDS_B1C 1575.42e6
#define FREQ_BDS_B1I 1561.098e6
#define FREQ_BDS_B2I 1207.14e6
#define FREQ_BDS_B3I 1268.52e6
#define FREQ_BDS_B2a 1176.45e6
#define FREQ_BDS_B2b 1207.14e6
#define FREQ_BDS_B2ab 1191.795e6
#define FREQ_GAL_E1 1575.42e6
#define FREQ_GAL_E5a 1176.45e6
#define FREQ_GAL_E5b 1207.14e6
#define FREQ_GAL_E5 1191.795e6
#define FREQ_GAL_E6 1278.75e6

typedef struct
{
	GnssSystem system;
	int svid;
	int FreqID;	// for GLONASS only
	int CN0;	// scale factor 0.01
	int PosTimeTag;
	KINEMATIC_INFO PosVel;
	double Acc[3];
	double TravelTime;	// travel time including corrections except group delay and ionosphere delay in second
	double IonoDelay;	// ionosphere delay in meter
	double GroupDelay[8];	// group delay in second, first element for TGD of L1, all followings for ISC
	double Elevation;	// satellite elevation in rad
	double Azimuth;		// satellite azimuth in rad
	double RelativeSpeed;	// satellite to receiver relative speed in m/s
	double LosVector[3];	// LOS vecter

} SATELLITE_PARAM, *PSATELLITE_PARAM;

int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[]);
int GetGlonassVisibleSatellite(KINEMATIC_INFO Position, GLONASS_TIME time, OUTPUT_PARAM OutputParam, PGLONASS_EPHEMERIS Eph[], int Number, PGLONASS_EPHEMERIS EphVisible[]);
void GetSatelliteParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam, PSATELLITE_PARAM SatelliteParam);
void GetSatelliteCN0(int PowerListCount, SIGNAL_POWER PowerList[], double DefaultCN0, enum ElevationAdjust Adjust, PSATELLITE_PARAM SatelliteParam);
double GetWaveLength(int system, int FreqIndex, int FreqID);
double GetTravelTime(PSATELLITE_PARAM SatelliteParam, int FreqIndex);
double GetCarrierPhase(PSATELLITE_PARAM SatelliteParam, int FreqIndex);
double GetDoppler(PSATELLITE_PARAM SatelliteParam, int FreqIndex);
GNSS_TIME GetTransmitTime(GNSS_TIME ReceiverTime, double TravelTime);

#endif //!defined(__SATELLITE_PARAM_H__)
