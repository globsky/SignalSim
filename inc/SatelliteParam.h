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

typedef struct
{
	int svid;
	int CN0;	// scale factor 0.01
	double TravelTime;	// travel time including corrections except ionosphere delay in second
	double IonoDelay;	// ionosphere delay in meter
	double Elevation;	// satellite elevation in rad
	double Azimuth;		// satellite azimuth in rad
	double RelativeSpeed;	// satellite to receiver relative speed in m/s
	double LosVector[3];	// LOS vecter
} SATELLITE_PARAM, *PSATELLITE_PARAM;

int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[]);
SATELLITE_PARAM GetSatelliteParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam);
GNSS_TIME GetTransmitTime(GNSS_TIME ReceiverTime, double TravelTime);

#endif //!defined(__SATELLITE_PARAM_H__)
