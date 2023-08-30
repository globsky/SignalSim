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
	PGLONASS_EPHEMERIS FindGloEphemeris(GNSS_TIME time, int slot);
	PIONO_PARAM GetGpsIono() { return &GpsIono; }
	PIONO_PARAM GetBdsIono() { return &BdsIono[0]; }
	PIONO_PARAM GetGalileoIono() { return &GalileoIono; }
	PUTC_PARAM GetGpsUtcParam() { return &GpsUtcParam; }
	PUTC_PARAM GetBdsUtcParam() { return &BdsUtcParam; }
	PUTC_PARAM GetGalileoUtcParam() { return &GalileoUtcParam; }
	int GetGlonassSlotFreq(int slot) { return (slot > 0 && slot <= 24) ? GlonassSlotFreq[slot-1] : 7; }
	void ReadNavFile(char *filename);

private:
	int GpsEphemerisNumber;
	int BdsEphemerisNumber;
	int GalileoEphemerisNumber;
	int GlonassEphemerisNumber;
	int GpsEphmerisPoolSize;
	int BdsEphmerisPoolSize;
	int GalileoEphmerisPoolSize;
	int GlonassEphmerisPoolSize;
	PGPS_EPHEMERIS GpsEphmerisPool;
	PGPS_EPHEMERIS BdsEphmerisPool;
	PGPS_EPHEMERIS GalileoEphmerisPool;
	PGLONASS_EPHEMERIS GlonassEphmerisPool;
	IONO_PARAM GpsIono;
	IONO_PARAM BdsIono[24];
	IONO_PARAM GalileoIono;
	UTC_PARAM GpsUtcParam;
	UTC_PARAM BdsUtcParam;
	UTC_PARAM GalileoUtcParam;
//	UTC_PARAM GalileoGpsParam;
	int GlonassSlotFreq[24];	// FreqID for each slot
};

#endif // __NAV_DATA_H__
