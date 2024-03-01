//----------------------------------------------------------------------
// CNav2Bit.h:
//   Declaration of navigation bit synthesis class for CNAV2
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __CNAV2_BIT_H__
#define __CNAV2_BIT_H__

#include "NavBit.h"

#define L1C_SUBFRAME2_SYMBOL_LENGTH 600
#define L1C_SUBFRAME3_SYMBOL_LENGTH 274

class CNav2Bit : public NavBit
{
public:
	CNav2Bit();
	~CNav2Bit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]) { return 0; };
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam) { return 0; };

private:
	unsigned int Subframe2[32][18];	// 32 SVs, 576bits in subframe 2, 32bits in each DWORD MSB first, lowest address first
	unsigned int Subframe3[50][9];	// reserve 50 subframe3 contents, 32bits in each DWORD MSB first, lowest address first

	static const unsigned long long BCH_toi_table[256];	// BCH encode table for TOI
	static const unsigned int L1CMatrixGen2[L1C_SUBFRAME2_SYMBOL_LENGTH*19];
	static const unsigned int L1CMatrixGen3[L1C_SUBFRAME3_SYMBOL_LENGTH*9];

	void ComposeSubframe2(PGPS_EPHEMERIS Eph, unsigned int Subframe2[18]);
	void LDPCEncode(unsigned int Stream[], int bits[], int SymbolLength, int TableSize, const unsigned int MatrixGen[]);
	int XorBits(unsigned int Data);
};

#endif // __CNAV2_BIT_H__
