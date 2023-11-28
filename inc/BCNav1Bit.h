//----------------------------------------------------------------------
// BCNav1Bit.h:
//   Declaration of navigation bit synthesis class for B-CNAV1
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __BCNAV1_BIT_H__
#define __BCNAV1_BIT_H__

#include "BCNavBit.h"

#define B1C_SUBFRAME2_SYMBOL_LENGTH 100
#define B1C_SUBFRAME3_SYMBOL_LENGTH 44

class BCNav1Bit : public BCNavBit
{
public:
	BCNav1Bit();
	~BCNav1Bit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(int svid, PGPS_ALMANAC Alm) { return 0; };
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam) { return 0; };

private:
	unsigned int BdsSubframe2[63][25];	// 63 SVs, 600bits in subframe 2, 24bits (4 symbols) in bit23~0 of each DWORD MSB first, lowest address first
	unsigned int BdsSubframe3[63][11];	// 63 SVs, 264bits in subframe 3, 24bits (4 symbols) in bit23~0 of each DWORD MSB first, lowest address first

	static const unsigned int BCH_prn_table[64];		// BCH encode table for SVID
	static const unsigned long long BCH_soh_table[256];	// BCH encode table for SOH
	static const char B1CMatrixGen2[B1C_SUBFRAME2_SYMBOL_LENGTH*B1C_SUBFRAME2_SYMBOL_LENGTH+1];
	static const char B1CMatrixGen3[B1C_SUBFRAME3_SYMBOL_LENGTH*B1C_SUBFRAME3_SYMBOL_LENGTH+1];

	void ComposeSubframe2(PGPS_EPHEMERIS Eph, unsigned int Subframe2[25]);
};

#endif // __BCNAV1_BIT_H__
