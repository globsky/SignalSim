//----------------------------------------------------------------------
// BCNavBit.h:
//   Declaration of navigation bit synthesis class for B-CNAV1
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __BCNAV_BIT_H__
#define __BCNAV_BIT_H__

#include "NavBit.h"

#define B1C_SUBFRAME2_SYMBOL_LENGTH 100
#define B1C_SUBFRAME3_SYMBOL_LENGTH 44

class BCNavBit : public NavBit
{
public:
	BCNavBit();
	~BCNavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int channel, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(int svid, PGPS_ALMANAC Alm) { return 0; };
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam) { return 0; };

private:
	unsigned int BdsSubframe2[63][25];	// 63 SVs, 600bits in subframe 2, 24bits (4 symbols) in bit23~0 of each DWORD MSB first, lowest address first
	unsigned int BdsSubframe3[63][11];	// 63 SVs, 264bits in subframe 3, 24bits (4 symbols) in bit23~0 of each DWORD MSB first, lowest address first

	static const unsigned int B1CSecondCode[63][57];	// 63x1800bit secondary code
	static const unsigned int BCH_prn_table[64];		// BCH encode table for SVID
	static const unsigned long long BCH_soh_table[256];	// BCH encode table for SOH
	static const unsigned int crc24q[256];				// CRC24Q table
	static const char B1CMatrixGen2[B1C_SUBFRAME2_SYMBOL_LENGTH*B1C_SUBFRAME2_SYMBOL_LENGTH+1];
	static const char B1CMatrixGen3[B1C_SUBFRAME3_SYMBOL_LENGTH*B1C_SUBFRAME3_SYMBOL_LENGTH+1];
	static const unsigned int e2v_table[128];
	static const unsigned int v2e_table[64];

	void ComposeSubframe2(PGPS_EPHEMERIS Eph, unsigned int Subframe2[25]);
	int AssignBits(unsigned int Data, int BitNumber, int BitStream[]);
	int AppendCRC(unsigned int DataStream[], int Length);
	int B1CLDPCEncode(int SymbolStream[], int Subframe);
	void GenParityMatrix(const char *MatrixGen, int *Symbols, int *Parity, int Length);
	int GF6IntMul(int a, int b);
};

#endif // __BCNAV_BIT_H__
