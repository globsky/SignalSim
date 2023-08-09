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
	TravelTime = Distance / LIGHT_SPEED - GpsClockCorrection(Eph, SatelliteTime);
	TravelTime -= WGS_F_GTR * Eph->ecc * Eph->sqrtA * sin(Eph->Ek + TimeDiff * Eph->Ek_dot);		// relativity correction
	SatelliteParam->TravelTime = TravelTime;
	SatelliteParam->GroupDelay[0] = Eph->tgd;
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

double GetIonoDelay(double IonoDelayL1, int system, int FreqIndex)
{
	switch (system)
	{
	case GpsSystem:
		switch (FreqIndex)
		{
		case FREQ_INDEX_GPS_L1: return IonoDelayL1;
		case FREQ_INDEX_GPS_L2: return IonoDelayL1 * 1.6469444444444444444444444444444; // (154/120)^2
		case FREQ_INDEX_GPS_L5: return IonoDelayL1 * 1.7932703213610586011342155009452; // (154/115)^2
		default: return IonoDelayL1;
		}
	case BdsSystem:
		switch (FreqIndex)
		{
		case FREQ_INDEX_BDS_B1C: return IonoDelayL1;
		case FREQ_INDEX_BDS_B1I: return IonoDelayL1 * 1.0184327918525376651796986785624; // (1540/1526)^2
		case FREQ_INDEX_BDS_B2I:
		case FREQ_INDEX_BDS_B2b: return IonoDelayL1 * 1.7032461936225222637173226084458; // (154/118)^2
		case FREQ_INDEX_BDS_B3I: return IonoDelayL1 * 1.5424037460978147762747138397503; // (154/124)^2
		case FREQ_INDEX_BDS_B2a: return IonoDelayL1 * 1.7932703213610586011342155009452; // (154/115)^2
		default: return IonoDelayL1;
		}
	case GalileoSystem:
		switch (FreqIndex)
		{
		case FREQ_INDEX_GAL_E1:  return IonoDelayL1;
		case FREQ_INDEX_GAL_E5a: return IonoDelayL1 * 1.7932703213610586011342155009452; // (154/115)^2
		case FREQ_INDEX_GAL_E5b: return IonoDelayL1 * 1.7032461936225222637173226084458; // (154/118)^2
		case FREQ_INDEX_GAL_E6:  return IonoDelayL1 * 1.517824; // (154/125)^2
		default: return IonoDelayL1;
		}
	default: return IonoDelayL1;
	}
}

double GetWaveLength(int system, int FreqIndex)
{
	switch (system)
	{
	case GpsSystem:
		switch (FreqIndex)
		{
		case FREQ_INDEX_GPS_L1: return LIGHT_SPEED / FREQ_GPS_L1;
		case FREQ_INDEX_GPS_L2: return LIGHT_SPEED / FREQ_GPS_L2;
		case FREQ_INDEX_GPS_L5: return LIGHT_SPEED / FREQ_GPS_L5;
		default: return LIGHT_SPEED / FREQ_GPS_L1;
		}
	case BdsSystem:
		switch (FreqIndex)
		{
		case FREQ_INDEX_BDS_B1C: return LIGHT_SPEED / FREQ_BDS_B1C;
		case FREQ_INDEX_BDS_B1I: return LIGHT_SPEED / FREQ_BDS_B1I;
		case FREQ_INDEX_BDS_B2I:
		case FREQ_INDEX_BDS_B2b: return LIGHT_SPEED / FREQ_BDS_B2b;
		case FREQ_INDEX_BDS_B3I: return LIGHT_SPEED / FREQ_BDS_B3I;
		case FREQ_INDEX_BDS_B2a: return LIGHT_SPEED / FREQ_BDS_B2a;
		default: return LIGHT_SPEED / FREQ_BDS_B1C;
		}
	case GalileoSystem:
		switch (FreqIndex)
		{
		case FREQ_INDEX_GAL_E1:  return LIGHT_SPEED / FREQ_GAL_E1;
		case FREQ_INDEX_GAL_E5a: return LIGHT_SPEED / FREQ_GAL_E5a;
		case FREQ_INDEX_GAL_E5b: return LIGHT_SPEED / FREQ_GAL_E5b;
		case FREQ_INDEX_GAL_E6:  return LIGHT_SPEED / FREQ_GAL_E6;
		default: return LIGHT_SPEED / FREQ_GAL_E1;
		}
	default: return LIGHT_SPEED / FREQ_GPS_L1;
	}
}

double GetTravelTime(PSATELLITE_PARAM SatelliteParam, int FreqIndex)
{
	double TravelTime = SatelliteParam->TravelTime + SatelliteParam->GroupDelay[0];
	if (FreqIndex)
		TravelTime -= SatelliteParam->GroupDelay[FreqIndex];	// ISC adjustment
	TravelTime += GetIonoDelay(SatelliteParam->IonoDelay, SatelliteParam->system, FreqIndex) / LIGHT_SPEED;
	return TravelTime;
}

double GetCarrierPhase(PSATELLITE_PARAM SatelliteParam, int FreqIndex)
{
	double TravelTime = SatelliteParam->TravelTime + SatelliteParam->GroupDelay[0];
	if (FreqIndex)
		TravelTime -= SatelliteParam->GroupDelay[FreqIndex];	// ISC adjustment
	TravelTime = TravelTime * LIGHT_SPEED - GetIonoDelay(SatelliteParam->IonoDelay, SatelliteParam->system, FreqIndex);
	return TravelTime / GetWaveLength(SatelliteParam->system, FreqIndex);
}

double GetDoppler(PSATELLITE_PARAM SatelliteParam, int FreqIndex)
{
	return -SatelliteParam->RelativeSpeed / GetWaveLength(SatelliteParam->system, FreqIndex);
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
