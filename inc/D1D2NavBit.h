//----------------------------------------------------------------------
// D1D2NavBit.h:
//   Declaration of navigation bit synthesis class for BDS2 D1/D2
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __D1D2NAV_BIT_H__
#define __D1D2NAV_BIT_H__

#include "NavBit.h"

class D1D2NavBit : public NavBit
{
public:
	D1D2NavBit();
	~D1D2NavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
	unsigned int BdsStream123[53][3*9];	// 53 MEO/IGSO SVs, WORD 2-10 in each 3 subframes
	unsigned int BdsStreamAlm[63][9];	// WORD 3-10 of Almanac
	unsigned int BdsStreamInfo[4][9];	// page 7~10 of subframe 5
	unsigned int BdsStreamHealth[3][9];	// page 24 of subframe 5 (AmID = 01~11)
	unsigned int BdsStreamD2[10][10*4];	// 10 GEO SVs, WORD 2-5 in each 10 subframes
	IONO_PARAM IonoParamSave;	// Ionosphere parameters saved to put into subframe 1
	static const unsigned int BCHPoly[4];

	int ComposeBdsStream123(PGPS_EPHEMERIS Ephemeris, PIONO_PARAM IonoParam, unsigned int Stream[3*9]);
	int ComposeBdsStreamD2(PGPS_EPHEMERIS Ephemeris, PIONO_PARAM IonoParam, unsigned int Stream[10*4]);
	int FillBdsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int Stream[9]);
	int FillBdsHealthPage(PGPS_ALMANAC Almanac, int Length, unsigned int Stream[9]);
	unsigned int GetBCH(unsigned int word);
	unsigned int Interleave(unsigned int data);
};

#endif // __D1D2NAV_BIT_H__
