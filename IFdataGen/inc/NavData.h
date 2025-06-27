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
	static const int GpsSatNumber = 32;
	static const int BdsSatNumber = 63;
	static const int GalileoSatNumber = 36;
	static const int GlonassSatNumber = 24;

	CNavData();
	~CNavData();

	bool AddNavData(NavDataType Type, void *NavData);
	PGPS_EPHEMERIS FindEphemeris(GnssSystem system, GNSS_TIME time, int svid, int IgnoreTimeLimit = 0, unsigned char FirstPrioritySource = 0);
	PGLONASS_EPHEMERIS FindGloEphemeris(GLONASS_TIME GlonassTime, int slot);
	PGPS_ALMANAC GetGpsAlmanac() { return GpsAlmanac; }
	PGPS_ALMANAC GetBdsAlmanac() { return BdsAlmanac; }
	PGPS_ALMANAC GetGalileoAlmanac() { return GalileoAlmanac; }
	PGLONASS_ALMANAC GetGlonassAlmanac() { return GlonassAlmanac; }
	PIONO_PARAM GetGpsIono() { return &GpsIono; }
	PIONO_PARAM GetBdsIono() { return &BdsIono[0]; }
	PIONO_PARAM GetGalileoIono() { return (PIONO_PARAM)&GalileoIono; }
	PUTC_PARAM GetGpsUtcParam() { return &GpsUtcParam; }
	PUTC_PARAM GetBdsUtcParam() { return &BdsUtcParam; }
	PUTC_PARAM GetGalileoUtcParam() { return &GalileoUtcParam; }
	int GetGlonassSlotFreq(int slot) { return (slot > 0 && slot <= 24) ? GlonassSlotFreq[slot-1] : 7; }
	void ReadNavFile(char *filename);
	void ReadAlmFile(char *filename);
	void CompleteAlmanac(GnssSystem system, UTC_TIME time);
	void CompleteGlonassAlmanac(GLONASS_TIME time);

private:
	int GpsEphemerisNumber;
	int BdsEphemerisNumber;
	int GalileoEphemerisNumber;
	int GlonassEphemerisNumber;
	int GpsEphemerisPoolSize;
	int BdsEphemerisPoolSize;
	int GalileoEphemerisPoolSize;
	int GlonassEphemerisPoolSize;
	PGPS_EPHEMERIS GpsEphemerisPool;
	PGPS_EPHEMERIS BdsEphemerisPool;
	PGPS_EPHEMERIS GalileoEphemerisPool;
	PGLONASS_EPHEMERIS GlonassEphemerisPool;
	GPS_ALMANAC GpsAlmanac[GpsSatNumber];
	GPS_ALMANAC BdsAlmanac[BdsSatNumber];
	GPS_ALMANAC GalileoAlmanac[GalileoSatNumber];
	GLONASS_ALMANAC GlonassAlmanac[GlonassSatNumber];
	IONO_PARAM GpsIono;
	IONO_PARAM BdsIono[24];
	IONO_NEQUICK GalileoIono;
	UTC_PARAM GpsUtcParam;
	UTC_PARAM BdsUtcParam;
	UTC_PARAM GalileoUtcParam;
//	UTC_PARAM GalileoGpsParam;
	int GlonassSlotFreq[24];	// FreqID for each slot
};

#endif // __NAV_DATA_H__
