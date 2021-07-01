//----------------------------------------------------------------------
// NavData.h:
//   Declaration of navigation data processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __NAV_DATA_H__
#define __NAV_DATA_H__

#include "BasicTypes.h"
#include "Rinex.h"

#define EPH_NUMBER_INC 100

class CNavData
{
public:
	CNavData();
	~CNavData();

	bool AddNavData(NavDataType Type, void *NavData);
	PGPS_EPHEMERIS FindGpsEphemeris(GNSS_TIME time, int svid);
	PIONO_PARAM GetGpsIono() { return &GpsIono; }
	void ReadNavFile(char *filename);

private:
	int EphemerisNumber;
	int GpsEphmerisPoolSize;
	PGPS_EPHEMERIS GpsEphmerisPool;
	IONO_PARAM GpsIono;
};

#endif // __NAV_DATA_H__
