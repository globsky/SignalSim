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

private:
	static const unsigned int BCH_prn_table[64];		// BCH encode table for SVID
	static const unsigned long long BCH_soh_table[256];	// BCH encode table for SOH
	static const char B1CMatrixGen2[B1C_SUBFRAME2_SYMBOL_LENGTH*B1C_SUBFRAME2_SYMBOL_LENGTH+1];
	static const char B1CMatrixGen3[B1C_SUBFRAME3_SYMBOL_LENGTH*B1C_SUBFRAME3_SYMBOL_LENGTH+1];

	void ComposeSubframe2(int week, int how, int svid, unsigned int Frame2Data[25]);
	void ComposeSubframe3(int soh, unsigned int Flags, unsigned int AccurateIndex, unsigned int Frame3Data[11]);
};

#endif // __BCNAV1_BIT_H__
