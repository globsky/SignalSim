//----------------------------------------------------------------------
// BCNav2Bit.h:
//   Declaration of navigation bit synthesis class for B-CNAV2
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __BCNAV2_BIT_H__
#define __BCNAV2_BIT_H__

#include "BCNavBit.h"

#define B2a_SYMBOL_LENGTH 48

class BCNav2Bit : public BCNavBit
{
public:
	BCNav2Bit();
	~BCNav2Bit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);

private:
	static const char B2aMatrixGen[B2a_SYMBOL_LENGTH*B2a_SYMBOL_LENGTH+1];
	static const int MessageOrder[20];

	void ComposeMessage(int MessageType, int week, int sow, int svid, unsigned int FrameData[]);
};

#endif // __BCNAV2_BIT_H__
