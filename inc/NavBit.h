//----------------------------------------------------------------------
// NavBit.h:
//   Declaration of navigation bit synthesis base class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __NAV_BIT_H__
#define __NAV_BIT_H__

#include "BasicTypes.h"

#define COMPOSE_BITS(data, start, width) ((data & ((1 << width) - 1)) << start)

typedef union
{
	double d_data;
	unsigned int i_data[2];
} DOUBLE_INT_UNION;

class NavBit
{
public:
	NavBit();
	~NavBit();

	virtual int GetFrameData(GNSS_TIME StartTime, int svid, int channel, int *NavBits) = 0;	// channel: 0 for pilot, 1 for data
	int roundi(double data);
	int roundu(double data);
	double UnscaleDouble(double value, int scale);
};

#endif // __NAV_BIT_H__
