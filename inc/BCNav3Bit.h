//----------------------------------------------------------------------
// BCNav3Bit.h:
//   Declaration of navigation bit synthesis class for B-CNAV3
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __BCNAV3_BIT_H__
#define __BCNAV3_BIT_H__

#include "BCNavBit.h"

#define B2b_SYMBOL_LENGTH 81

class BCNav3Bit : public BCNavBit
{
public:
	BCNav3Bit();
	~BCNav3Bit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(int svid, PGPS_ALMANAC Alm) { return 0; };
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam) { return 0; };

private:
	unsigned int Ephemeris1[63][7];	// ephemeris I (203 bits)
	unsigned int Ephemeris2[63][7];	// ephemeris II (222 bits)
	unsigned int ClockParam[63][3];	// clock + IODC (79 bits)
	unsigned int IntegrityFlags[63];	// B2a DIF/SIF/AIF + SISMAI + B1C DIF/SIF/AIF (10 bits)

	static const char B2bMatrixGen[B2b_SYMBOL_LENGTH*B2b_SYMBOL_LENGTH+1];

	void ComposeMessage(int MessageType, int week, int sow, int svid, unsigned int FrameData[]);
};

#endif // __BCNAV3_BIT_H__
