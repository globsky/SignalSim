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
#include "GnssTime.h"
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
		if (Eph[i] == NULL || Eph[i]->valid == 0 || Eph[i]->health != 0)
			continue;
		if (system == GpsSystem)
		{
			if (OutputParam.GpsMaskOut & (1 << i))
				continue;
		}
		else if (system == BdsSystem)
		{
			if (OutputParam.BdsMaskOut & (1LL << i))
				continue;
		}
		else if (system == GalileoSystem)
		{
			if (OutputParam.GalileoMaskOut & (1LL << i))
				continue;
		}
		else
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

int GetGlonassVisibleSatellite(KINEMATIC_INFO Position, GLONASS_TIME time, OUTPUT_PARAM OutputParam, PGLONASS_EPHEMERIS Eph[], int Number, PGLONASS_EPHEMERIS EphVisible[])
{
	int i;
	int SatNumber = 0;
	KINEMATIC_INFO SatPosition;
	double Elevation, Azimuth;

	for (i = 0; i < Number; i ++)
	{
		if (Eph[i] == NULL || Eph[i]->flag == 0)
			continue;
		if (OutputParam.GlonassMaskOut & (1 << (i-1)))
			continue;
		if (!GlonassSatPosSpeedEph(time.MilliSeconds / 1000., Eph[i], &SatPosition, NULL))
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
	PGLONASS_EPHEMERIS GloEph = (PGLONASS_EPHEMERIS)Eph;
	int Seconds, LeapSecond;

	SatelliteParam->system= system;
	if (system == BdsSystem)	// subtract leap second difference
		time.MilliSeconds -= 14000;
	else if (system == GlonassSystem)	// subtract leap second, add 3 hours
	{
		Seconds = (unsigned int)(time.Week * 604800 + time.MilliSeconds / 1000);
		GetLeapSecond(Seconds, LeapSecond);
		time.MilliSeconds = (time.MilliSeconds + 10800000 - LeapSecond * 1000) % 86400000;
	}
	SatelliteTime = (time.MilliSeconds + time.SubMilliSeconds) / 1000.0;

	// first estimate the travel time, ignore tgd, ionosphere and troposphere delay
	if (system == GlonassSystem)
	{
		SatelliteParam->svid = GloEph->n;
		SatelliteParam->FreqID = GloEph->freq;
		GlonassSatPosSpeedEph(SatelliteTime, GloEph, &SatPosition, NULL);
	}
	else
	{
		SatelliteParam->svid = Eph->svid;
		SatelliteParam->FreqID = 0;
#if USE_POSITION_PREDICTION
		GetSatPosVel(system, SatelliteTime, Eph, SatelliteParam, &SatPosition);
#else
		GpsSatPosSpeedEph(system, SatelliteTime, Eph, &SatPosition, NULL);
	//	SatelliteParam->PosVel = SatPosition;
#endif
	}
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, SatelliteParam->LosVector) / LIGHT_SPEED;
	SatPosition.x -= TravelTime * SatPosition.vx; SatPosition.y -= TravelTime * SatPosition.vy; SatPosition.z -= TravelTime * SatPosition.vz;
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, SatelliteParam->LosVector) / LIGHT_SPEED;
	SatelliteTime -= TravelTime;

	// calculate accurate transmit time
	// travel_time = (d + dtrop)/c + tgd - dts - trel + diono
	if (system == GlonassSystem)
		GlonassSatPosSpeedEph(SatelliteTime, GloEph, &SatPosition, NULL);
	else
	{
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
	}
	Distance = GeometryDistance(&PositionEcef, &SatPosition, LosVector);
	SatElAz(&PositionLla, LosVector, &Elevation, &Azimuth);
	SatelliteParam->IonoDelay = GpsIonoDelay(IonoParam, SatelliteTime, PositionLla.lat, PositionLla.lon, Elevation, Azimuth);
	Distance += TropoDelay(PositionLla.lat, PositionLla.alt, Elevation);
	if (system == GlonassSystem)
		TravelTime = Distance / LIGHT_SPEED - GlonassClockCorrection(GloEph, SatelliteTime);
	else
	{
		TravelTime = Distance / LIGHT_SPEED - GpsClockCorrection(Eph, SatelliteTime);
		TravelTime -= WGS_F_GTR * Eph->ecc * Eph->sqrtA * sin(Eph->Ek + TimeDiff * Eph->Ek_dot);		// relativity correction
		// assign GroupDelay[]
		switch (system)
		{
		case GpsSystem:
			SatelliteParam->GroupDelay[SIGNAL_INDEX_L1CA] = Eph->tgd;	// L1C/A
			SatelliteParam->GroupDelay[SIGNAL_INDEX_L1C] = Eph->tgd_ext[1];	// L1C
			SatelliteParam->GroupDelay[SIGNAL_INDEX_L2C] = Eph->tgd2;	// L2C
			SatelliteParam->GroupDelay[SIGNAL_INDEX_L5] = Eph->tgd_ext[3];	// L5
			break;
		case BdsSystem:
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B1C] = Eph->tgd_ext[1];	// B1C
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B1I] = Eph->tgd;	// B1I
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B2I] = Eph->tgd2;	// B2I
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B3I] = 0;	// B3I
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B2a] = Eph->tgd_ext[3];	// B2a
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B2b] = Eph->tgd_ext[4];	// B2b
			SatelliteParam->GroupDelay[SIGNAL_INDEX_B2ab] = (Eph->tgd_ext[3] + Eph->tgd_ext[4]) / 2;	// B2a+B2b
			break;
		case GalileoSystem:
			SatelliteParam->GroupDelay[SIGNAL_INDEX_E1] = Eph->tgd;	// E1
			SatelliteParam->GroupDelay[SIGNAL_INDEX_E5a] = Eph->tgd_ext[2];	// E5a
			SatelliteParam->GroupDelay[SIGNAL_INDEX_E5b] = Eph->tgd_ext[4];	// E5b
			SatelliteParam->GroupDelay[SIGNAL_INDEX_E5] = (Eph->tgd_ext[2] + Eph->tgd_ext[4]) / 2;	// E5
			SatelliteParam->GroupDelay[SIGNAL_INDEX_E6] = Eph->tgd_ext[4];	// E6
			break;
		}
	}
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

double GetIonoDelay(double IonoDelayL1, int system, int SignalIndex)
{
	switch (system)
	{
	case GpsSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_L1CA: return IonoDelayL1;
		case SIGNAL_INDEX_L1C : return IonoDelayL1;
		case SIGNAL_INDEX_L2C : return IonoDelayL1 * 1.6469444444444444444444444444444; // (154/120)^2
		case SIGNAL_INDEX_L5  : return IonoDelayL1 * 1.7932703213610586011342155009452; // (154/115)^2
		default: return IonoDelayL1;
		}
	case BdsSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_B1C : return IonoDelayL1;
		case SIGNAL_INDEX_B1I : return IonoDelayL1 * 1.0184327918525376651796986785624; // (1540/1526)^2
		case SIGNAL_INDEX_B2I :
		case SIGNAL_INDEX_B2b : return IonoDelayL1 * 1.7032461936225222637173226084458; // (154/118)^2
		case SIGNAL_INDEX_B3I : return IonoDelayL1 * 1.5424037460978147762747138397503; // (154/124)^2
		case SIGNAL_INDEX_B2a : return IonoDelayL1 * 1.7932703213610586011342155009452; // (154/115)^2
		case SIGNAL_INDEX_B2ab: return IonoDelayL1 * 1.7473889738252684705925693971154; // (154/116.5)^2
		default: return IonoDelayL1;
		}
	case GalileoSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_E1 : return IonoDelayL1;
		case SIGNAL_INDEX_E5a: return IonoDelayL1 * 1.7932703213610586011342155009452; // (154/115)^2
		case SIGNAL_INDEX_E5b: return IonoDelayL1 * 1.7032461936225222637173226084458; // (154/118)^2
		case SIGNAL_INDEX_E5 : return IonoDelayL1 * 1.7473889738252684705925693971154; // (154/116.5)^2
		case SIGNAL_INDEX_E6 : return IonoDelayL1 * 1.517824; // (154/125)^2
		default: return IonoDelayL1;
		}
	default: return IonoDelayL1;
	}
}

double GetWaveLength(int system, int SignalIndex, int FreqID)
{
	double Freq;

	switch (system)
	{
	case GpsSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_L1CA:
		case SIGNAL_INDEX_L1C : return LIGHT_SPEED / FREQ_GPS_L1;
		case SIGNAL_INDEX_L2C : return LIGHT_SPEED / FREQ_GPS_L2;
		case SIGNAL_INDEX_L5  : return LIGHT_SPEED / FREQ_GPS_L5;
		default: return LIGHT_SPEED / FREQ_GPS_L1;
		}
	case BdsSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_B1C : return LIGHT_SPEED / FREQ_BDS_B1C;
		case SIGNAL_INDEX_B1I : return LIGHT_SPEED / FREQ_BDS_B1I;
		case SIGNAL_INDEX_B2I :
		case SIGNAL_INDEX_B2b : return LIGHT_SPEED / FREQ_BDS_B2b;
		case SIGNAL_INDEX_B3I : return LIGHT_SPEED / FREQ_BDS_B3I;
		case SIGNAL_INDEX_B2a : return LIGHT_SPEED / FREQ_BDS_B2a;
		case SIGNAL_INDEX_B2ab: return LIGHT_SPEED / FREQ_BDS_B2ab;
		default: return LIGHT_SPEED / FREQ_BDS_B1C;
		}
	case GalileoSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_E1 : return LIGHT_SPEED / FREQ_GAL_E1;
		case SIGNAL_INDEX_E5a: return LIGHT_SPEED / FREQ_GAL_E5a;
		case SIGNAL_INDEX_E5b: return LIGHT_SPEED / FREQ_GAL_E5b;
		case SIGNAL_INDEX_E5 : return LIGHT_SPEED / FREQ_GAL_E5;
		case SIGNAL_INDEX_E6 : return LIGHT_SPEED / FREQ_GAL_E6;
		default: return LIGHT_SPEED / FREQ_GAL_E1;
		}
	case GlonassSystem:
		Freq = (SignalIndex == SIGNAL_INDEX_G1) ? (1602e6 + 562500 * FreqID) : (1246e6 + 437500 * FreqID);
		return LIGHT_SPEED / Freq;
	default: return LIGHT_SPEED / FREQ_GPS_L1;
	}
}

double GetTravelTime(PSATELLITE_PARAM SatelliteParam, int SignalIndex)
{
	double TravelTime = SatelliteParam->TravelTime + SatelliteParam->GroupDelay[SignalIndex];
	TravelTime += GetIonoDelay(SatelliteParam->IonoDelay, SatelliteParam->system, SignalIndex) / LIGHT_SPEED;
	return TravelTime;
}

double GetCarrierPhase(PSATELLITE_PARAM SatelliteParam, int SignalIndex)
{
	double TravelTime = SatelliteParam->TravelTime + SatelliteParam->GroupDelay[SignalIndex];
	TravelTime = TravelTime * LIGHT_SPEED - GetIonoDelay(SatelliteParam->IonoDelay, SatelliteParam->system, SignalIndex);
	return TravelTime / GetWaveLength(SatelliteParam->system, SignalIndex, SatelliteParam->FreqID);
}

double GetDoppler(PSATELLITE_PARAM SatelliteParam, int SignalIndex)
{
	return -SatelliteParam->RelativeSpeed / GetWaveLength(SatelliteParam->system, SignalIndex, SatelliteParam->FreqID);
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
