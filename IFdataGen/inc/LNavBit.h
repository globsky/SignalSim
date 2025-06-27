//----------------------------------------------------------------------
// LNavBit.h:
//   Declaration of navigation bit synthesis class for LNAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __LNAV_BIT_H__
#define __LNAV_BIT_H__

#include "NavBit.h"

class LNavBit : public NavBit
{
public:
	LNavBit();
	~LNavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
	unsigned int GpsStream123[32][3*8];	// 32 SVs, WORD 3-10 in each 3 subframes
	unsigned int GpsStream45[2][25][8];	// WORD 3-10 in 25 pages of 2 subframes
	static const unsigned char ParityTable[6][16];
	static const unsigned int ParityAdjust[4];
	static const int PageId[2][25];

	int ComposeGpsStream123(PGPS_EPHEMERIS Ephemeris, unsigned int Stream[3*8]);
	int FillGpsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int Stream[8]);
	int FillGpsHealthPage(GPS_ALMANAC Almanac[], unsigned int Stream4[8], unsigned int Stream5[8]);
	unsigned int GpsGetParity(unsigned int word);
};

#endif // __LNAV_BIT_H__
