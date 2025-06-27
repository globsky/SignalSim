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

int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[]);
int GetGlonassVisibleSatellite(KINEMATIC_INFO Position, GLONASS_TIME time, OUTPUT_PARAM OutputParam, PGLONASS_EPHEMERIS Eph[], int Number, PGLONASS_EPHEMERIS EphVisible[]);
void GetSatelliteParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam, PSATELLITE_PARAM SatelliteParam);
void GetSatelliteCN0(int PowerListCount, SIGNAL_POWER PowerList[], double DefaultCN0, enum ElevationAdjust Adjust, PSATELLITE_PARAM SatelliteParam);
double GetWaveLength(int system, int SignalIndex, int FreqID);
double GetTravelTime(PSATELLITE_PARAM SatelliteParam, int SignalIndex);
double GetCarrierPhase(PSATELLITE_PARAM SatelliteParam, int SignalIndex);
double GetDoppler(PSATELLITE_PARAM SatelliteParam, int SignalIndex);
GNSS_TIME GetTransmitTime(GNSS_TIME ReceiverTime, double TravelTime);

#endif //!defined(__SATELLITE_PARAM_H__)
