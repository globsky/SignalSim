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
	EphemerisNumber = 0;
	GpsEphmerisPool = (PGPS_EPHEMERIS)malloc(sizeof(GPS_EPHEMERIS) * EPH_NUMBER_INC);
	GpsEphmerisPoolSize = EPH_NUMBER_INC;
}

CNavData::~CNavData()
{
	free(GpsEphmerisPool);
}

bool CNavData::AddNavData(NavDataType Type, void *NavData)
{
	PGPS_EPHEMERIS NewGpsEphmerisPool;

	if (Type == NavDataGpsIono)
	{
		memcpy(&GpsIono, NavData, sizeof(IONO_PARAM));
		return true;
	}

	if (Type != NavDataGpsEph)
		return false;

	if (EphemerisNumber == GpsEphmerisPoolSize)
	{
		GpsEphmerisPoolSize += EPH_NUMBER_INC;
		NewGpsEphmerisPool = (PGPS_EPHEMERIS)realloc(GpsEphmerisPool, sizeof(GPS_EPHEMERIS) * GpsEphmerisPoolSize);
		if (NewGpsEphmerisPool == NULL)
			return false;
		GpsEphmerisPool = NewGpsEphmerisPool;
	}
	memcpy(&GpsEphmerisPool[EphemerisNumber], NavData, sizeof(GPS_EPHEMERIS));
	EphemerisNumber ++;

	return true;
}

PGPS_EPHEMERIS CNavData::FindGpsEphemeris(GNSS_TIME time, int svid)
{
	int i, time_diff, diff;
	PGPS_EPHEMERIS Eph = NULL;

	for (i = 0; i < EphemerisNumber; i ++)
	{
		diff = (time.Week - GpsEphmerisPool[i].week) * 604800 + ((int)(time.Seconds) - GpsEphmerisPool[i].toe);
		if (diff < 0)
			diff = -diff;
		if ((svid == GpsEphmerisPool[i].svid) && (diff < 7200) && ((Eph == NULL) || ((Eph != NULL) && (diff < time_diff))))
		{
			Eph = &GpsEphmerisPool[i];
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
