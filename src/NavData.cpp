//----------------------------------------------------------------------
// NavData.cpp:
//   Implementation of navigation data processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <malloc.h>
#include <string.h>

#include "NavData.h"
#include "GnssTime.h"

CNavData::CNavData()
{
	GpsEphemerisNumber = BdsEphemerisNumber = GalileoEphemerisNumber = GlonassEphemerisNumber = 0;
	GpsEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	BdsEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GalileoEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GlonassEphmerisPool = (PGLONASS_EPHEMERIS)malloc(sizeof(GLONASS_EPHEMERIS) * EPH_NUMBER_INC);
	GpsEphmerisPoolSize = BdsEphmerisPoolSize = GalileoEphmerisPoolSize = GlonassEphmerisPoolSize = EPH_NUMBER_INC;
	memset(&GpsUtcParam, 0, sizeof(UTC_PARAM));
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
	free(GpsEphmerisPool);
	free(BdsEphmerisPool);
	free(GalileoEphmerisPool);
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
		if (GpsEphemerisNumber == GpsEphmerisPoolSize)
		{
			GpsEphmerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(GpsEphmerisPool, sizeof(GPS_EPHEMERIS) * GpsEphmerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			GpsEphmerisPool = NewEphmerisPool;
		}
		memcpy(&GpsEphmerisPool[GpsEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		GpsEphemerisNumber ++;
		break;
	case NavDataBdsD1D2:
	case NavDataBdsCnav1:
	case NavDataBdsCnav2:
	case NavDataBdsCnav3:
		if (BdsEphemerisNumber == BdsEphmerisPoolSize)
		{
			BdsEphmerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(BdsEphmerisPool, sizeof(GPS_EPHEMERIS) * BdsEphmerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			BdsEphmerisPool = NewEphmerisPool;
		}
		memcpy(&BdsEphmerisPool[BdsEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		BdsEphemerisNumber ++;
		break;
	case NavDataGalileoINav:
	case NavDataGalileoFNav:
		if (GalileoEphemerisNumber == GalileoEphmerisPoolSize)
		{
			GalileoEphmerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(GalileoEphmerisPool, sizeof(GPS_EPHEMERIS) * GalileoEphmerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			GalileoEphmerisPool = NewEphmerisPool;
		}
		memcpy(&GalileoEphmerisPool[GalileoEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		GalileoEphemerisNumber ++;
		break;
	case NavDataGlonassFdma:
		if (GlonassEphemerisNumber == GlonassEphmerisPoolSize)
		{
			GlonassEphmerisPoolSize += EPH_NUMBER_INC;
			NewEphmerisPool = (PGPS_EPHEMERIS)realloc(GlonassEphmerisPool, sizeof(GLONASS_EPHEMERIS) * GlonassEphmerisPoolSize);
			if (NewEphmerisPool == NULL)
				return false;
			GlonassEphmerisPool = (PGLONASS_EPHEMERIS)NewEphmerisPool;
		}
		memcpy(&GlonassEphmerisPool[GlonassEphemerisNumber], NavData, sizeof(GLONASS_EPHEMERIS));
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

PGPS_EPHEMERIS CNavData::FindEphemeris(GnssSystem system, GNSS_TIME time, int svid, unsigned char FirstPrioritySource)
{
	int i, time_diff, diff;
	PGPS_EPHEMERIS Eph = NULL;
	PGPS_EPHEMERIS EphmerisPool;
	int EphemerisNumber;
	int Week = time.Week;
	BOOL DoAssignment = 0;

	if (system == GpsSystem)
	{
		EphmerisPool = GpsEphmerisPool;
		EphemerisNumber = GpsEphemerisNumber;
	}
	else if (system == BdsSystem)
	{
		EphmerisPool = BdsEphmerisPool;
		EphemerisNumber = BdsEphemerisNumber;
		Week -= 1356;
	}
	else if (system == GalileoSystem)
	{
		EphmerisPool = GalileoEphmerisPool;
		EphemerisNumber = GalileoEphemerisNumber;
	}
	else
		return (PGPS_EPHEMERIS)0;


	for (i = 0; i < EphemerisNumber; i ++)
	{
		diff = (Week - EphmerisPool[i].week) * 604800 + (time.MilliSeconds / 1000 - EphmerisPool[i].toe);
		if (diff < 0)
			diff = -diff;
		if ((svid == EphmerisPool[i].svid) && (diff < 7200))	// same svid and within +-2 hours time span
		{
			if (Eph == NULL)	// not assigned, assign anyway
				DoAssignment = 1;
			else if (Eph->source != FirstPrioritySource && EphmerisPool[i].source == FirstPrioritySource)	// new ephemeris from desired source
				DoAssignment = 1;
			else if (diff < time_diff)	// either both old and new ephemeris do or do not from desired source, but has smaller time difference
				DoAssignment = 1;
			else
				DoAssignment = 0;

			if (DoAssignment)
			{
				Eph = &EphmerisPool[i];
				time_diff = diff;
			}
		}
	}

	return Eph;
}

PGLONASS_EPHEMERIS CNavData::FindGloEphemeris(GNSS_TIME time, int slot)
{
	int i, time_diff, diff;
	PGLONASS_EPHEMERIS Eph = NULL;
	UTC_TIME UtcTime = GpsTimeToUtc(time, FALSE);;
	GLONASS_TIME GloTime = UtcToGlonassTime(UtcTime);

	for (i = 0; i < GlonassEphemerisNumber; i ++)
	{
		if (slot != (int)GlonassEphmerisPool[i].n)
			continue;
		diff = GloTime.Day - GlonassEphmerisPool[i].day;
		// day range between -730~730
		if (diff > 730)
			diff -= 1461;
		else if (diff < -730)
			diff += 1461;
		diff = diff * 86400 + (GloTime.MilliSeconds / 1000 - GlonassEphmerisPool[i].tb);
		if (diff < 0)
			diff = -diff;
		if ((diff < 1800) && ((Eph == NULL) || ((Eph != NULL) && (diff < time_diff))))
		{
			Eph = &GlonassEphmerisPool[i];
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
		return;

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
