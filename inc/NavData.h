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
	PGPS_EPHEMERIS FindEphemeris(GnssSystem system, GNSS_TIME time, int svid);
	PIONO_PARAM GetGpsIono() { return &GpsIono; }
	PIONO_PARAM GetBdsIono() { return &BdsIono[0]; }
	double *GetGalileoIono() { return GalileoIono; }
	PUTC_PARAM GetGpsUtcParam() { return &GpsUtcParam; }
	PUTC_PARAM GetBdsUtcParam() { return &BdsUtcParam; }
	PUTC_PARAM GetGalileoUtcParam() { return &GalileoUtcParam; }
	void ReadNavFile(char *filename);

private:
	int GpsEphemerisNumber;
	int BdsEphemerisNumber;
	int GalileoEphemerisNumber;
	int GpsEphmerisPoolSize;
	int BdsEphmerisPoolSize;
	int GalileoEphmerisPoolSize;
	PGPS_EPHEMERIS GpsEphmerisPool;
	PGPS_EPHEMERIS BdsEphmerisPool;
	PGPS_EPHEMERIS GalileoEphmerisPool;
	IONO_PARAM GpsIono;
	IONO_PARAM BdsIono[24];
	double GalileoIono[3];
	UTC_PARAM GpsUtcParam;
	UTC_PARAM BdsUtcParam;
	UTC_PARAM GalileoUtcParam;
//	UTC_PARAM GalileoGpsParam;
};

#endif // __NAV_DATA_H__
