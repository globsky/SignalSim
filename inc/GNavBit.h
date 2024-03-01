//----------------------------------------------------------------------
// GNavBit.h:
//   Declaration of navigation bit synthesis class for GNAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __GNAV_BIT_H__
#define __GNAV_BIT_H__

#include "NavBit.h"

class GNavBit : public NavBit
{
public:
	GNavBit();
	~GNavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
	// string contents: 3DOWRD to hold 85bits with index 0 bit20 as first bit to index 2 bit0 as last bit
	// index bit20 always 0, index 2 bit 7~0 left as 0s to fill page number and check sum
	unsigned int StringEph[24][4][3];	// 24 SVs, String 1~4 and 85bits for each string
	unsigned int StringAlm[5][11][3];	// 5 frames, string 5~15 and 85bits for each string
	static const unsigned int CheckSumTable[8][3];

	int ComposeStringEph(PGLONASS_EPHEMERIS Ephemeris, unsigned int String[][3]);
	int ComposeStringAlm(PGLONASS_ALMANAC Almanac, int slot, unsigned int StringEven[3], unsigned int StringOdd[3]);
	unsigned int CheckSum(unsigned int Data[3]);
};

#endif // __GNAV_BIT_H__
