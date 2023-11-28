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

	int AssignBits(unsigned int Data, int BitNumber, int BitStream[]);
	int AppendCRC(unsigned int DataStream[], int Length);
	int LDPCEncode(int SymbolStream[], int SymbolLength, const char *MatrixGen);
	int GF6IntMul(int a, int b);

private:
	static const unsigned int crc24q[256];				// CRC24Q table
	static const unsigned int e2v_table[128];
	static const unsigned int v2e_table[64];
};

#endif // __BCNAV_BIT_H__
