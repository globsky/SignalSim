//----------------------------------------------------------------------
// SatelliteParam.cpp:
//   Implementation of functions to calculate satellite parameters
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <math.h>

#include "ConstVal.h"
#include "SatelliteParam.h"
#include "Coordinate.h"
#include "XmlInterpreter.h"

int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[])
{
	int i;
	int SatNumber = 0;
	KINEMATIC_INFO SatPosition;
	double Elevation, Azimuth;

	for (i = 0; i < Number; i ++)
	{
		if (Eph[i] == NULL || Eph[i]->flag == 0 || Eph[i]->health != 0)
			continue;
		if (OutputParam.GpsMaskOut & (1 << (i-1)))
			continue;
		if (!GpsSatPosSpeedEph(system, time.MilliSeconds / 1000., Eph[i], &SatPosition))
			continue;
		SatElAz(&Position, &SatPosition, &Elevation, &Azimuth);
		if (Elevation < OutputParam.ElevationMask)
			continue;
		EphVisible[SatNumber ++] = Eph[i];
	}

	return SatNumber;
}

SATELLITE_PARAM GetSatelliteParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam)
{
	KINEMATIC_INFO SatPosition;
	double Distance, TravelTime, SatelliteTime = (time.MilliSeconds + time.SubMilliSeconds) / 1000.0;
	double Elevation, Azimuth;
	double LosVector[3];
	SATELLITE_PARAM SatelliteParam;

	SatelliteParam.svid = Eph->svid;
	if (system == BdsSystem)	// subtract leap second difference
		SatelliteTime -= 14.0;

	// first estimate the travel time, ignore tgd, ionosphere and troposphere delay
	GpsSatPosSpeedEph(system, SatelliteTime, Eph, &SatPosition);
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, SatelliteParam.LosVector) / LIGHT_SPEED;
	SatPosition.x -= TravelTime * SatPosition.vx; SatPosition.y -= TravelTime * SatPosition.vy; SatPosition.z -= TravelTime * SatPosition.vz;
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, SatelliteParam.LosVector) / LIGHT_SPEED;
	SatelliteTime -= TravelTime;

	// calculate accurate transmit time
	// travel_time = (d + dtrop)/c + tgd - dts - trel + diono
	GpsSatPosSpeedEph(system, SatelliteTime, Eph, &SatPosition);
	Distance = GeometryDistance(&PositionEcef, &SatPosition, LosVector);
	SatElAz(&PositionLla, LosVector, &Elevation, &Azimuth);
	SatelliteParam.IonoDelay = GpsIonoDelay(IonoParam, SatelliteTime, PositionLla.lat, PositionLla.lon, Elevation, Azimuth);
	Distance += TropoDelay(PositionLla.lat, PositionLla.alt, Elevation);
	TravelTime = Distance / LIGHT_SPEED + Eph->tgd - GpsClockCorrection(Eph, SatelliteTime);
	TravelTime -= WGS_F_GTR * Eph->ecc * Eph->sqrtA * sin(Eph->Ek);		// relativity correction
	SatelliteParam.TravelTime = TravelTime;
	SatelliteParam.Elevation = Elevation;
	SatelliteParam.Azimuth = Azimuth;
	SatelliteParam.RelativeSpeed = SatRelativeSpeed(&PositionEcef, &SatPosition) - LIGHT_SPEED * Eph->af1;

	return SatelliteParam;
}

int GetSatelliteCN0(int PrevCN0, int PowerListCount, SIGNAL_POWER PowerList[], double DefaultCN0, enum ElevationAdjust Adjust, int svid, double Elevation)
{
	int i;
	double CN0;

	// adjust signal power
	for (i = 0; i < PowerListCount; i ++)
	{
		if (PowerList[i].svid == svid || PowerList[i].svid == 0)
		{
			if (PowerList[i].CN0 < 0)
			{
				CN0 = DefaultCN0;
				switch (Adjust)
				{
				case ElevationAdjustNone:
					break;
				case ElevationAdjustSinSqrtFade:
					CN0 -= (1 - sqrt(Elevation)) * 25;
				}
			}
			else
				CN0 = PowerList[i].CN0;
			PrevCN0 = (int)(CN0 * 100 + 0.5);
		}
	}

	return PrevCN0;
}

GNSS_TIME GetTransmitTime(GNSS_TIME ReceiverTime, double TravelTime)
{
	GNSS_TIME TransmitTime = ReceiverTime;

	TravelTime *= 1000.0;	// convert to ms
	TransmitTime.MilliSeconds -= (int)TravelTime;
	TravelTime -= (int)TravelTime;
	TransmitTime.SubMilliSeconds -= TravelTime;
	if (TransmitTime.SubMilliSeconds < 0)
	{
		TransmitTime.SubMilliSeconds += 1.0;
		TransmitTime.MilliSeconds --;
	}
	if (TransmitTime.MilliSeconds < 0)
		TransmitTime.MilliSeconds += 604800000;

	return TransmitTime;
}
