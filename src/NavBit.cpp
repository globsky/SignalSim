//----------------------------------------------------------------------
// NavBit.h:
//   Implementation of navigation bit synthesis base class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include "NavBit.h"

NavBit::NavBit()
{
}

NavBit::~NavBit()
{
}
int NavBit::roundi(double data)
{
	if (data >= 0)
		return (int)(data + 0.5);
	else
		return (int)(data - 0.5);
}

int NavBit::roundu(double data)
{
	return (unsigned int)(data + 0.5);
}

double NavBit::UnscaleDouble(double value, int scale)
{
	DOUBLE_INT_UNION data;

	data.d_data = value;
	data.i_data[1] -= (scale << 20);
	return data.d_data;
}
