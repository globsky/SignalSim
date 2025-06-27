//----------------------------------------------------------------------
// CNavBit.h:
//   Declaration of navigation bit synthesis class for CNAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __CNAV_BIT_H__
#define __CNAV_BIT_H__

#include "NavBit.h"

class CNavBit : public NavBit
{
public:
	CNavBit();
	~CNavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
	// Arrays allocate bits align with the method of putting 276bits (without CRC) of a message into 9 DWORD according to following table
	// WORD sequence:  DWORD0  DWORD1  DWORD2  DWORD3  DWORD4  DWORD5  DWORD6  DWORD7  DWORD8
	// bit order:       1-20   21-52   53-84   85-116 117-148 149-180 181-212 213-244 245-276
	unsigned int EphMessage[32][2][9];	// 32 SVs, Message 10/11 each has 276bits from bit19 DWORD0 to bit0 DWORD8
	unsigned int MidiAlm[32][4];		// 32 SVs, Message 37, 128bits from bit0 DWORD5 to bit0 DWORD8
	unsigned int ReducedAlm[32], TOA;	// 32 SVs, 31bit each, TOA combines WNa (13bit) and toa (8bit) in DWORD4 21LSB
	unsigned int ClockMessage[32][4];	// 32 SVs, Clock fields (top through af2) 89bits from bit13 DWORD1 to bit21 DWORD4
	unsigned int DelayMessage[32][3];	// 32 SVs, Group delay fields 65bits from bit20 DWORD4 to bit20 DWORD6
	unsigned int IonoMessage[3];		// 84bits from bit19 DWORD6 to bit0 DWORD8 (include last 12 reserved bits)
	unsigned int UTCMessage[4];			// 98bits from bit20 DWORD4 to bit19 DWORD7

	// save convolutional encode state
	unsigned char ConvEncodeBitsL2[32];
	unsigned char ConvEncodeBitsL5[32];

	int ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int EphData[2][9], unsigned int ClockData[4], unsigned int DelayData[3]);
	int ComposeAlmWords(GPS_ALMANAC Almanac[], unsigned int &ReducedAlmData, unsigned int MidiAlmData[6]);
	void GetMessageData(int svid, int message, int TOW, unsigned int Data[9]);
	unsigned char ConvolutionEncodePair(unsigned char &ConvEncodeBits, unsigned int &EncodeWord);
};

#endif // __CNAV_BIT_H__
