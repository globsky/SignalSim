//----------------------------------------------------------------------
// INavBit.h:
//   Declaration of navigation bit synthesis class for I/NAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __INAV_BIT_H__
#define __INAV_BIT_H__

#include "NavBit.h"

class INavBit : public NavBit
{
public:
	INavBit();
	~INavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
/*	unsigned int GpsStream123[32][3*8];	// 32 SVs, WORD 3-10 in each 3 subframes
	unsigned int GpsStream45[2][25][8];	// WORD 3-10 in 25 pages of 2 subframes
	static const unsigned char ParityTable[6][16];
	static const unsigned int ParityAdjust[4];

	int ComposeGpsStream123(PGPS_EPHEMERIS Ephemeris, unsigned int Stream[3*8]);
	int FillGpsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int Stream[8]);*/
	unsigned int GalEphData[36][5*4];	// 36 SVs, Word1~5 as ephemeris, 4DWORD each Word
	unsigned int GalUtcData[4];	// for Word6
	unsigned int GalSpareData[4];	// for Word0
	unsigned int GalDummyData[4];	// for Word63

	static const int WordAllocationE1[15];
	static const int WordAllocationE5[15];
	static const int SyncPattern[10];

	int ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int *EphData);
	unsigned int *GetWordData(int svid, int Word);
	unsigned char GalConvolutionEncode(unsigned char &ConvEncodeBits, unsigned int EncodeWord);
};

#endif // __INAV_BIT_H__
