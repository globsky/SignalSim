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

CNavData::CNavData()
{
	GpsEphemerisNumber = BdsEphemerisNumber = GalileoEphemerisNumber = 0;
	GpsEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	BdsEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GalileoEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GpsEphmerisPoolSize = BdsEphmerisPoolSize = GalileoEphmerisPoolSize = EPH_NUMBER_INC;
}

CNavData::~CNavData()
{
	free(GpsEphmerisPool);
	free(BdsEphmerisPool);
	free(GalileoEphmerisPool);
}

bool CNavData::AddNavData(NavDataType Type, void *NavData)
{
	PGPS_EPHEMERIS NewGpsEphmerisPool;
	PIONO_PARAM Iono = (PIONO_PARAM)NavData;
	int TimeMark;

	if (Type == NavDataGpsIono)
	{
		memcpy(&GpsIono, NavData, sizeof(IONO_PARAM));
		return true;
	}
	else if (Type == NavDataBdsIonoA)
	{
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
	}
	else if (Type == NavDataBdsIonoB)
	{
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
	}
	else if (Type == NavDataGpsEph)
	{
		if (GpsEphemerisNumber == GpsEphmerisPoolSize)
		{
			GpsEphmerisPoolSize += EPH_NUMBER_INC;
			NewGpsEphmerisPool = (PGPS_EPHEMERIS)realloc(GpsEphmerisPool, sizeof(GPS_EPHEMERIS) * GpsEphmerisPoolSize);
			if (NewGpsEphmerisPool == NULL)
				return false;
			GpsEphmerisPool = NewGpsEphmerisPool;
		}
		memcpy(&GpsEphmerisPool[GpsEphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
		GpsEphemerisNumber ++;
	}
	return true;
}

PGPS_EPHEMERIS CNavData::FindEphemeris(GnssSystem system, GNSS_TIME time, int svid)
{
	int i, time_diff, diff;
	PGPS_EPHEMERIS Eph = NULL;
	PGPS_EPHEMERIS EphmerisPool;
	int EphemerisNumber;

	if (system == SystemGps)
	{
		EphmerisPool = GpsEphmerisPool;
		EphemerisNumber = GpsEphemerisNumber;
	}
	else if (system == SystemBds)
	{
		EphmerisPool = BdsEphmerisPool;
		EphemerisNumber = BdsEphemerisNumber;
	}
	else if (system == SystemGalileo)
	{
		EphmerisPool = GalileoEphmerisPool;
		EphemerisNumber = GalileoEphemerisNumber;
	}

	for (i = 0; i < EphemerisNumber; i ++)
	{
		diff = (time.Week - EphmerisPool[i].week) * 604800 + ((int)(time.Seconds) - EphmerisPool[i].toe);
		if (diff < 0)
			diff = -diff;
		if ((svid == EphmerisPool[i].svid) && (diff < 7200) && ((Eph == NULL) || ((Eph != NULL) && (diff < time_diff))))
		{
			Eph = &EphmerisPool[i];
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

	if ((fp = fopen(filename, "r")) == NULL)
		return;

	while ((DataType = LoadNavFileHeader(fp, (void *)&NavData)) != NavDataEnd)
	{
		if (DataType != NavDataUnknown)
			AddNavData(DataType, (void *)&NavData);
	}
	while ((DataType = LoadNavFileEphemeris(fp, (void *)&NavData)) != NavDataEnd)
	{
		if (DataType != NavDataUnknown)
			AddNavData(DataType, (void *)&NavData);
	}
}
