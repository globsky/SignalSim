//----------------------------------------------------------------------
// BCNavBit.h:
//   Declaration of navigation bit synthesis class for BDS3 navigation bit
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __BCNAV_BIT_H__
#define __BCNAV_BIT_H__

#include "NavBit.h"

class BCNavBit : public NavBit
{
public:
	BCNavBit();
	~BCNavBit();

	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam) { return 0; };

	int AppendWord(unsigned int *Dest, int StartBit, unsigned int *Src, int Length);
	int AssignBits(unsigned int Data, int BitNumber, int BitStream[]);
	int AppendCRC(unsigned int DataStream[], int Length);
	int LDPCEncode(int SymbolStream[], int SymbolLength, const char *MatrixGen);
	int GF6IntMul(int a, int b);

	unsigned int Ephemeris1[63][9];		// IODE + ephemeris I (211 bits)
	unsigned int Ephemeris2[63][10];	// ephemeris II (222 bits)
	unsigned int ClockParam[63][4];		// clock + IODC
	unsigned int IntegrityFlags[63];	// SISMAI + B1C/B2a/B2b DIF/SIF/AIF (13 bits)
	unsigned int TgsIscParam[63][3];	// TGD+ISC for B1C/B2a/B2b
	unsigned int ReducedAlmanac[63][2];	// reduced almanac (38 bits)
	unsigned int MidiAlmanac[63][7];	// midi almanac (156 bits)
	unsigned int BdGimIono[4];			// BDGIM ionosphere parameters (74 bits)
	unsigned int BdtUtcParam[5];		// BDT-UTC parameters (97 bits)
	unsigned int EopParam[6];			// EOP parameters (138 bits)
	unsigned int BgtoParam[7][3];		// BGTO parameters (68 bits)

private:
	static const unsigned int crc24q[256];				// CRC24Q table
	static const unsigned int e2v_table[128];
	static const unsigned int v2e_table[64];

	int FillBdsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int MidiAlm[8], unsigned int ReducedAlm[2]);
};

#endif // __BCNAV_BIT_H__
