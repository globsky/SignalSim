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

private:

	static const char B2bMatrixGen[B2b_SYMBOL_LENGTH*B2b_SYMBOL_LENGTH+1];
	static const int MessageOrder[6];

	void ComposeMessage(int MessageType, int week, int sow, int svid, unsigned int FrameData[]);
};

#endif // __BCNAV3_BIT_H__
