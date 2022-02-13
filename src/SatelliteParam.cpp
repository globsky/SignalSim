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

static void GetSatPosVel(GnssSystem system, double SatelliteTime, PGPS_EPHEMERIS Eph, PSATELLITE_PARAM SatelliteParam, PKINEMATIC_INFO pPosVel);

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
		if (!GpsSatPosSpeedEph(system, time.MilliSeconds / 1000., Eph[i], &SatPosition, NULL))
			continue;
		SatElAz(&Position, &SatPosition, &Elevation, &Azimuth);
		if (Elevation < OutputParam.ElevationMask)
			continue;
		EphVisible[SatNumber ++] = Eph[i];
	}

	return SatNumber;
}

#define USE_POSITION_PREDICTION 0

void GetSatelliteParam(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam, PSATELLITE_PARAM SatelliteParam)
{
	KINEMATIC_INFO SatPosition;
	double Distance, TravelTime, SatelliteTime = (time.MilliSeconds + time.SubMilliSeconds) / 1000.0;
	double TimeDiff;
	double Elevation, Azimuth;
	double LosVector[3];

	SatelliteParam->system= system;
	SatelliteParam->svid = Eph->svid;
	if (system == BdsSystem)	// subtract leap second difference
		SatelliteTime -= 14.0;

	// first estimate the travel time, ignore tgd, ionosphere and troposphere delay
#if USE_POSITION_PREDICTION
	GetSatPosVel(system, SatelliteTime, Eph, SatelliteParam, &SatPosition);
#else
	GpsSatPosSpeedEph(system, SatelliteTime, Eph, &SatPosition, NULL);
//	SatelliteParam->PosVel = SatPosition;
#endif
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, SatelliteParam->LosVector) / LIGHT_SPEED;
	SatPosition.x -= TravelTime * SatPosition.vx; SatPosition.y -= TravelTime * SatPosition.vy; SatPosition.z -= TravelTime * SatPosition.vz;
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, SatelliteParam->LosVector) / LIGHT_SPEED;
	SatelliteTime -= TravelTime;

	// calculate accurate transmit time
	// travel_time = (d + dtrop)/c + tgd - dts - trel + diono
#if USE_POSITION_PREDICTION
	TimeDiff = SatelliteTime - SatelliteParam->PosTimeTag;
	SatPosition.x = SatelliteParam->PosVel.x + (SatelliteParam->PosVel.vx + SatelliteParam->Acc[0] * TimeDiff * 0.5) * TimeDiff;
	SatPosition.y = SatelliteParam->PosVel.y + (SatelliteParam->PosVel.vy + SatelliteParam->Acc[1] * TimeDiff * 0.5) * TimeDiff;
	SatPosition.z = SatelliteParam->PosVel.z + (SatelliteParam->PosVel.vz + SatelliteParam->Acc[2] * TimeDiff * 0.5) * TimeDiff;
	SatPosition.vx = SatelliteParam->PosVel.vx + SatelliteParam->Acc[0] * TimeDiff;
	SatPosition.vy = SatelliteParam->PosVel.vy + SatelliteParam->Acc[1] * TimeDiff;
	SatPosition.vz = SatelliteParam->PosVel.vz + SatelliteParam->Acc[2] * TimeDiff;
#else
	TimeDiff = 0;
	GpsSatPosSpeedEph(system, SatelliteTime, Eph, &SatPosition, NULL);
#endif
	Distance = GeometryDistance(&PositionEcef, &SatPosition, LosVector);
	SatElAz(&PositionLla, LosVector, &Elevation, &Azimuth);
	SatelliteParam->IonoDelay = GpsIonoDelay(IonoParam, SatelliteTime, PositionLla.lat, PositionLla.lon, Elevation, Azimuth);
	Distance += TropoDelay(PositionLla.lat, PositionLla.alt, Elevation);
	TravelTime = Distance / LIGHT_SPEED + Eph->tgd - GpsClockCorrection(Eph, SatelliteTime);
	TravelTime -= WGS_F_GTR * Eph->ecc * Eph->sqrtA * sin(Eph->Ek + TimeDiff * Eph->Ek_dot);		// relativity correction
	SatelliteParam->TravelTime = TravelTime;
	SatelliteParam->Elevation = Elevation;
	SatelliteParam->Azimuth = Azimuth;
	SatelliteParam->RelativeSpeed = SatRelativeSpeed(&PositionEcef, &SatPosition) - LIGHT_SPEED * Eph->af1;
}

void GetSatelliteCN0(int PowerListCount, SIGNAL_POWER PowerList[], double DefaultCN0, enum ElevationAdjust Adjust, PSATELLITE_PARAM SatelliteParam)
{
	int i;
	double CN0;

	// adjust signal power
	for (i = 0; i < PowerListCount; i ++)
	{
		if ((PowerList[i].svid == SatelliteParam->svid || PowerList[i].svid == 0) && PowerList[i].system == SatelliteParam->system)
		{
			if (PowerList[i].CN0 < 0)
			{
				CN0 = DefaultCN0;
				switch (Adjust)
				{
				case ElevationAdjustNone:
					break;
				case ElevationAdjustSinSqrtFade:
					CN0 -= (1 - sqrt(SatelliteParam->Elevation)) * 25;
				}
			}
			else
				CN0 = PowerList[i].CN0;
			SatelliteParam->CN0 = (int)(CN0 * 100 + 0.5);
		}
	}
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

void GetSatPosVel(GnssSystem system, double SatelliteTime, PGPS_EPHEMERIS Eph, PSATELLITE_PARAM SatelliteParam, PKINEMATIC_INFO pPosVel)
{
	double TimeDiff = SatelliteTime - SatelliteParam->PosTimeTag;
	int TimeTag;

	// compensate week round
	if (TimeDiff > 600000)
		TimeDiff -= 604800;
	else if (TimeDiff < -600000)
		TimeDiff += 604800;

	if (SatelliteParam->PosTimeTag >= 0 && fabs(TimeDiff) <= 0.5)	// do prediction
	{
		pPosVel->x = SatelliteParam->PosVel.x + (SatelliteParam->PosVel.vx + SatelliteParam->Acc[0] * TimeDiff * 0.5) * TimeDiff;
		pPosVel->y = SatelliteParam->PosVel.y + (SatelliteParam->PosVel.vy + SatelliteParam->Acc[1] * TimeDiff * 0.5) * TimeDiff;
		pPosVel->z = SatelliteParam->PosVel.z + (SatelliteParam->PosVel.vz + SatelliteParam->Acc[2] * TimeDiff * 0.5) * TimeDiff;
		pPosVel->vx = SatelliteParam->PosVel.vx + SatelliteParam->Acc[0] * TimeDiff;
		pPosVel->vy = SatelliteParam->PosVel.vy + SatelliteParam->Acc[1] * TimeDiff;
		pPosVel->vz = SatelliteParam->PosVel.vz + SatelliteParam->Acc[2] * TimeDiff;
	}
	else	// new calculation
	{
		TimeTag = (int)(SatelliteTime + 0.5);
		GpsSatPosSpeedEph(system, (double)TimeTag, Eph, &(SatelliteParam->PosVel), (SatelliteParam->Acc));
		SatelliteParam->PosTimeTag = TimeTag;
		TimeDiff = SatelliteTime - TimeTag;
		if (TimeDiff != 0.0)
		{
			pPosVel->x = SatelliteParam->PosVel.x + (SatelliteParam->PosVel.vx + SatelliteParam->Acc[0] * TimeDiff * 0.5) * TimeDiff;
			pPosVel->y = SatelliteParam->PosVel.y + (SatelliteParam->PosVel.vy + SatelliteParam->Acc[1] * TimeDiff * 0.5) * TimeDiff;
			pPosVel->z = SatelliteParam->PosVel.z + (SatelliteParam->PosVel.vz + SatelliteParam->Acc[2] * TimeDiff * 0.5) * TimeDiff;
			pPosVel->vx = SatelliteParam->PosVel.vx + SatelliteParam->Acc[0] * TimeDiff;
			pPosVel->vy = SatelliteParam->PosVel.vy + SatelliteParam->Acc[1] * TimeDiff;
			pPosVel->vz = SatelliteParam->PosVel.vz + SatelliteParam->Acc[2] * TimeDiff;
		}
		else
			*pPosVel = SatelliteParam->PosVel;
	}
}
