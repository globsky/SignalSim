//----------------------------------------------------------------------
// FNavBit.h:
//   Declaration of navigation bit synthesis class for F/NAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __FNAV_BIT_H__
#define __FNAV_BIT_H__

#include "NavBit.h"

class FNavBit : public NavBit
{
public:
	FNavBit();
	~FNavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
	unsigned int GalEphData[36][4][7];	// 36 SVs, PageType 1~4, each page has 214bits before CRC, leaving 10MSB of first DWORD as 0s
	unsigned int GalAlmData[12][2][7];	// 36 SVs, PageType 5~6, each page has 214bits before CRC, leaving 10MSB of first DWORD as 0s
	unsigned int GalUtcData[4];	// GST-UTC part in PageType 4, from bit5 index0 to bit3 index3 totally 99 bits
	unsigned int GalIonoData[2];	// Iono correction part in PageType 1, from bit15 index0 to bit7 index1 totally 41 bits

	static const int SyncPattern[12];

	int ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int EphData[4][7]);
	int ComposeAlmWords(GPS_ALMANAC Almanac[], unsigned int AlmData[2][7], int week);
	void GetPageData(int svid, int page, int subframe, unsigned int GST, unsigned int Data[7]);
	unsigned char GalConvolutionEncode(unsigned char &ConvEncodeBits, unsigned int &EncodeWord);
};

#endif // __FNAV_BIT_H__
