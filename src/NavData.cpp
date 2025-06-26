//----------------------------------------------------------------------
// NavData.cpp:
//   Implementation of navigation data processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "NavData.h"
#include "Almanac.h"
#include "GnssTime.h"

CNavData::CNavData()
{
	GpsEphemerisNumber = BdsEphemerisNumber = GalileoEphemerisNumber = GlonassEphemerisNumber = 0;
	GpsEphemerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	BdsEphemerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GalileoEphemerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GlonassEphemerisPool = (PGLONASS_EPHEMERIS)malloc(sizeof(GLONASS_EPHEMERIS) * EPH_NUMBER_INC);
	GpsEphemerisPoolSize = BdsEphemerisPoolSize = GalileoEphemerisPoolSize = GlonassEphemerisPoolSize = EPH_NUMBER_INC;
	memset(&GpsUtcParam, 0, sizeof(UTC_PARAM));
	memset(GpsAlmanac, 0, sizeof(GpsAlmanac));
	memset(BdsAlmanac, 0, sizeof(BdsAlmanac));
	memset(GalileoAlmanac, 0, sizeof(GalileoAlmanac));
	memset(GlonassAlmanac, 0, sizeof(GlonassAlmanac));
	// set default FreqID for each glonass SLOT
	GlonassSlotFreq[ 0] =  1; GlonassSlotFreq[ 1] = -4; GlonassSlotFreq[ 2] =  5; GlonassSlotFreq[ 3] =  6;
	GlonassSlotFreq[ 4] =  1; GlonassSlotFreq[ 5] = -4; GlonassSlotFreq[ 6] =  5; GlonassSlotFreq[ 7] =  6;
	GlonassSlotFreq[ 8] = -2; GlonassSlotFreq[ 9] = -7; GlonassSlotFreq[10] =  0; GlonassSlotFreq[11] = -1;
	GlonassSlotFreq[12] = -2; GlonassSlotFreq[13] = -7; GlonassSlotFreq[14] =  0; GlonassSlotFreq[15] = -1;
	GlonassSlotFreq[16] =  4; GlonassSlotFreq[17] = -3; GlonassSlotFreq[18] =  3; GlonassSlotFreq[19] =  2;
	GlonassSlotFreq[20] =  4; GlonassSlotFreq[21] = -3; GlonassSlotFreq[22] =  3; GlonassSlotFreq[23] =  2;
}

CNavData::~CNavData()
{
	free(GpsEphemerisPool);
	free(BdsEphemerisPool);
	free(GalileoEphemerisPool);
	free(GlonassEphemerisPool);
}

bool CNavData::AddNavData(NavDataType Type, void *NavData)
{
	PGPS_EPHEMERIS NewEphmerisPool;
	PIONO_PARAM Iono = (PIONO_PARAM)NavData;
	PUTC_PARAM UtcParam = (PUTC_PARAM)NavData;
	int TimeMark;

	switch (Type)
	{
	case NavDataGpsIono:
	case NavDataIonGps:
		memcpy(&GpsIono, NavData, sizeof(IONO_PARAM));
		return true;
	case NavDataIonBds:
		memcpy(&BdsIono[0], NavData, sizeof(IONO_PARAM));
		return true;
	case NavDataIonGalileo:
		memcpy(&GalileoIono, NavData, sizeof(IONO_NEQUICK));
		return true;
	case NavDataBdsIonoA:
		TimeMark = Iono->flag & 0x1f;
		if (TimeMark >= 0 && TimeMark < 24)
		{
			BdsIono[TimeMark].a0 = Iono->a0;
			BdsIono[TimeMark].a1 = Iono->a1;
			BdsIono[TimeMark].a2 = Iono->a2;
			BdsIono[TimeMark].a3 = Iono->a3;
			BdsIono[TimeMark].flag = Iono->flag & 0x3f00;	// assign svid
		}
		return true;
	case NavDataBdsIonoB:
		TimeMark = Iono->flag & 0x1f;
		if (TimeMark >= 0 && TimeMark < 24 && (BdsIono[TimeMark].flag == (Iono->flag & 0x3f00)))
		{
			BdsIono[TimeMark].b0 = Iono->b0;
			BdsIono[TimeMark].b1 = Iono->b1;
			BdsIono[TimeMark].b2 = Iono->b2;
			BdsIono[TimeMark].b3 = Iono->b3;
			BdsIono[TimeMark].flag |= 1;	// set valid flag
		}
		return true;
	case NavDataGpsLnav:
	case NavDataGpsCnav:
	case NavDataGpsCnav2:
		if (GpsEphemerisNumber == GpsEphemerisPoolSize)
		{
			GpsEphemerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(GpsEphemerisPool, sizeof(GPS_EPHEMERIS) * GpsEphemerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			GpsEphemerisPool = NewEphmerisPool;
		}
		memcpy(&GpsEphemerisPool[GpsEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		GpsEphemerisNumber ++;
		break;
	case NavDataBdsD1D2:
	case NavDataBdsCnav1:
	case NavDataBdsCnav2:
	case NavDataBdsCnav3:
		if (BdsEphemerisNumber == BdsEphemerisPoolSize)
		{
			BdsEphemerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(BdsEphemerisPool, sizeof(GPS_EPHEMERIS) * BdsEphemerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			BdsEphemerisPool = NewEphmerisPool;
		}
		memcpy(&BdsEphemerisPool[BdsEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		BdsEphemerisNumber ++;
		break;
	case NavDataGalileoINav:
	case NavDataGalileoFNav:
		if (GalileoEphemerisNumber == GalileoEphemerisPoolSize)
		{
			GalileoEphemerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(GalileoEphemerisPool, sizeof(GPS_EPHEMERIS) * GalileoEphemerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			GalileoEphemerisPool = NewEphmerisPool;
		}
		memcpy(&GalileoEphemerisPool[GalileoEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		GalileoEphemerisNumber ++;
		break;
	case NavDataGlonassFdma:
		if (GlonassEphemerisNumber == GlonassEphemerisPoolSize)
		{
			GlonassEphemerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(GlonassEphemerisPool, sizeof(GLONASS_EPHEMERIS) * GlonassEphemerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			GlonassEphemerisPool = (PGLONASS_EPHEMERIS)NewEphmerisPool;
		}
		memcpy(&GlonassEphemerisPool[GlonassEphemerisNumber], NavData, sizeof(GLONASS_EPHEMERIS));
		GlonassEphemerisNumber ++;
		break;
	case NavDataGpsUtc:
		UtcParam->TLS = UtcParam->TLSF = GpsUtcParam.TLS;	// set TLS/TLSF field in UtcParam with correct before memcpy()
		UtcParam->flag = GpsUtcParam.flag | 1;
		memcpy(&GpsUtcParam, UtcParam, sizeof(UTC_PARAM));
		break;
	case NavDataBdsUtc:
		UtcParam->TLS = UtcParam->TLSF = BdsUtcParam.TLS;	// set TLS/TLSF field in UtcParam with correct before memcpy()
		UtcParam->flag = BdsUtcParam.flag | 1;
		memcpy(&BdsUtcParam, UtcParam, sizeof(UTC_PARAM));
		break;
	case NavDataGalileoUtc:
		UtcParam->TLS = UtcParam->TLSF = GalileoUtcParam.TLS;	// set TLS/TLSF field in UtcParam with correct before memcpy()
		UtcParam->flag = GalileoUtcParam.flag | 1;
		memcpy(&GalileoUtcParam, UtcParam, sizeof(UTC_PARAM));
		break;
	case NavDataGalileoGps:
	case NavDataLeapSecond:
		GpsUtcParam.TLS = GpsUtcParam.TLSF = UtcParam->TLS;	// set both TLS and TLSF
		GpsUtcParam.flag |= 2;	// and set valid flag
		GalileoUtcParam.TLS = GalileoUtcParam.TLSF = UtcParam->TLS;	// set both TLS and TLSF
		GalileoUtcParam.flag |= 2;	// and set valid flag
		BdsUtcParam.TLS = BdsUtcParam.TLSF = UtcParam->TLS - 14;	// set both TLS and TLSF
		BdsUtcParam.flag |= 2;	// and set valid flag
		break;
	}
	return true;
}

PGPS_EPHEMERIS CNavData::FindEphemeris(GnssSystem system, GNSS_TIME time, int svid, int IgnoreTimeLimit, unsigned char FirstPrioritySource)
{
	int i, time_diff, diff;
	PGPS_EPHEMERIS Eph = NULL;
	PGPS_EPHEMERIS EphemerisPool;
	int EphemerisNumber;
	int Week = time.Week;
	BOOL DoAssignment = 0;

	if (system == GpsSystem)
	{
		EphemerisPool = GpsEphemerisPool;
		EphemerisNumber = GpsEphemerisNumber;
	}
	else if (system == BdsSystem)
	{
		EphemerisPool = BdsEphemerisPool;
		EphemerisNumber = BdsEphemerisNumber;
	}
	else if (system == GalileoSystem)
	{
		EphemerisPool = GalileoEphemerisPool;
		EphemerisNumber = GalileoEphemerisNumber;
	}
	else
		return (PGPS_EPHEMERIS)0;


	for (i = 0; i < EphemerisNumber; i ++)
	{
		if (EphemerisPool[i].health != 0)
			continue;
		diff = (Week - EphemerisPool[i].week) * 604800 + (time.MilliSeconds / 1000 - EphemerisPool[i].toe);
		if (diff < 0)
			diff = -diff;
		if (svid == EphemerisPool[i].svid)	// same svid
		{
			if (!IgnoreTimeLimit && diff > 7200) // exceed +-2 hours time span
				DoAssignment = 0;
			else if (Eph == NULL)	// not assigned, assign anyway
				DoAssignment = 1;
			else if (Eph->source != FirstPrioritySource && EphemerisPool[i].source == FirstPrioritySource)	// new ephemeris from desired source
				DoAssignment = 1;
			else if (diff < time_diff)	// either both old and new ephemeris do or do not from desired source, but has smaller time difference
				DoAssignment = 1;
			else
				DoAssignment = 0;

			if (DoAssignment)
			{
				Eph = &EphemerisPool[i];
				time_diff = diff;
			}
		}
	}

	return Eph;
}

PGLONASS_EPHEMERIS CNavData::FindGloEphemeris(GLONASS_TIME GlonassTime, int slot)
{
	int i, time_diff, diff;
	PGLONASS_EPHEMERIS Eph = NULL;

	for (i = 0; i < GlonassEphemerisNumber; i ++)
	{
		if (slot != (int)GlonassEphemerisPool[i].n)
			continue;
		diff = GlonassTime.Day - GlonassEphemerisPool[i].day;
		// day range between -730~730
		if (diff > 730)
			diff -= 1461;
		else if (diff < -730)
			diff += 1461;
		diff = diff * 86400 + (GlonassTime.MilliSeconds / 1000 - GlonassEphemerisPool[i].tb);
		if (diff < 0)
			diff = -diff;
		if ((diff < 1800) && ((Eph == NULL) || ((Eph != NULL) && (diff < time_diff))))
		{
			Eph = &GlonassEphemerisPool[i];
			time_diff = diff;
		}
	}

	return Eph;
}

void CNavData::ReadNavFile(char *filename)
{
	FILE *fp;
	NavDataType DataType;
	GPS_EPHEMERIS NavData;
	PGLONASS_EPHEMERIS pEph = (PGLONASS_EPHEMERIS)(&NavData);

	if ((fp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "Error: Unable to open ephemeris file: %s\n", filename);
		fprintf(stderr, "Cannot continue without ephemeris data. Exiting.\n");
		exit(1);
	}

	while ((DataType = LoadNavFileHeader(fp, (void *)&NavData)) != NavDataEnd)
	{
		if (DataType != NavDataUnknown)
			AddNavData(DataType, (void *)&NavData);
	}
	while ((DataType = LoadNavFileContents(fp, (void *)&NavData)) != NavDataEnd)
	{
		if (DataType == NavDataGlonassFdma)
			if (pEph->n > 0 && pEph->n <= 24)
				GlonassSlotFreq[pEph->n-1] = pEph->freq;
		if (DataType != NavDataUnknown)
			AddNavData(DataType, (void *)&NavData);
	}
}

void CNavData::ReadAlmFile(char *filename)
{
	FILE *fp;
	AlmanacType Type;

	if ((fp = fopen(filename, "r")) == NULL)
	{
		fprintf(stderr, "Error: Unable to open almanac file: %s\n", filename);
		return;
	}
	Type = CheckAlmnanacType(fp);
	switch (Type)
	{
	case AlmanacGps:
		ReadAlmanacGps(fp, GpsAlmanac);
		break;
	case AlmanacBds:
		ReadAlmanacBds(fp, BdsAlmanac);
		break;
	case AlmanacGalileo:
		ReadAlmanacGalileo(fp, GalileoAlmanac);
		break;
	case AlmanacGlonass:
		ReadAlmanacGlonass(fp, GlonassAlmanac);
		break;
	}
}

void CNavData::CompleteAlmanac(GnssSystem system, UTC_TIME time)
{
	PGPS_ALMANAC Almanac;
	PGPS_EPHEMERIS Eph = NULL;
	int AlmanacNumber;
	GNSS_TIME gnss_time;
	GLONASS_TIME glonass_time;
	int i, toa = -1, week;

	if (system == GpsSystem)
	{
		Almanac = GpsAlmanac;
		AlmanacNumber = GpsSatNumber;
		gnss_time = UtcToGpsTime(time);
	}
	else if (system == BdsSystem)
	{
		Almanac = BdsAlmanac;
		AlmanacNumber = BdsSatNumber;
		gnss_time = UtcToBdsTime(time);
	}
	else if (system == GalileoSystem)
	{
		Almanac = GalileoAlmanac;
		AlmanacNumber = GalileoSatNumber;
		gnss_time = UtcToGpsTime(time);
	}
	else if (system == GlonassSystem)
	{
		glonass_time = UtcToGlonassTime(time);
		CompleteGlonassAlmanac(glonass_time);
		return;
	}
	else
		return;

	// if there is valid almanac, use toa of valid almanac as first priority
	for (i = 0; i < AlmanacNumber; i ++)
		if (Almanac[i].valid & 1)
		{
			toa = Almanac[i].toa;
			week = Almanac[i].week;
			break;
		}
	// else assign toa with given system time
	if (toa < 0)
	{
		toa = (gnss_time.MilliSeconds + 2048000) / 4096000 * 4096;	// round to nearest 2^12
		week = gnss_time.Week;
	}

	// for any almanac not valid, find whether there is valid ephemeris
	for (i = 0; i < AlmanacNumber; i ++)
	{
		if (Almanac[i].valid & 1)
			continue;
		if ((Eph = FindEphemeris(system, gnss_time, i + 1, 1)) == NULL)
			continue;
		Almanac[i] = GetAlmanacFromEphemeris(Eph, week, toa);
	}
}

#define GLONASS_PERIOD 40544
void CNavData::CompleteGlonassAlmanac(GLONASS_TIME time)
{
	int i, j;
	int day = -1, leap_year;
	PGLONASS_EPHEMERIS Eph;
	double t;

	// if there is valid almanac, use day number of valid almanac as first priority
	for (i = 0; i < GlonassSatNumber; i ++)
		if (GlonassAlmanac[i].flag)
		{
			day = GlonassAlmanac[i].day;
			leap_year = GlonassAlmanac[i].leap_year;
			break;
		}
	// else assign day with given system time
	if (day < 0)
	{
		day = time.Day;
		leap_year = time.LeapYear;
	}

	// for any almanac not valid, find whether there is valid ephemeris
	for (i = 0; i < GlonassSatNumber; i ++)
	{
		if (GlonassAlmanac[i].flag)
			continue;
		// find first ephemeris within target date to estimate time of ascending
		Eph = NULL;
		for (j = 0; j < GlonassEphemerisNumber; j ++)
		{
			if ((int)GlonassEphemerisPool[j].n != i + 1)
				continue;
			if (GlonassEphemerisPool[j].day == day)
			{
				Eph = &GlonassEphemerisPool[j];
				break;
			}
		}
		if (Eph == NULL)	// not found
			continue;
		t = atan2(-Eph->z, Eph->vz * GLONASS_PERIOD / PI2) * GLONASS_PERIOD / PI2;
		if (t < 0) t += GLONASS_PERIOD;
		t += Eph->tb;
		time.MilliSeconds = (int)(t * 1000);
		if ((Eph = FindGloEphemeris(time, i + 1)) == NULL)
			continue;
		GlonassAlmanac[i] = GetAlmanacFromEphemeris(Eph, day, leap_year);
/*		printf("%02d\t%10.4f\t%10.4f\t%8.6f\t%9.6f\t%11.6f\t%11.6f\t%e\t%2d\n",
			i + 1, GlonassAlmanac[i].t, GlonassAlmanac[i].dt + 43200, GlonassAlmanac[i].ecc, GlonassAlmanac[i].di * 180 + 63,
			GlonassAlmanac[i].lambda * 180, GlonassAlmanac[i].w * 180, GlonassAlmanac[i].clock_error, GlonassAlmanac[i].freq);*/
	}
}
