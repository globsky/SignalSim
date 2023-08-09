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

// put bit in Data from MSB ot LSB into BitStream, bit order from bit(BitNumber-1) to bit(0) of Data
int NavBit::AssignBits(unsigned int Data, int BitNumber, int BitStream[])
{
	int i;

	Data <<= (32 - BitNumber);
	for (i = 0; i < BitNumber; i++)
	{
		BitStream[i] = (Data & 0x80000000) ? 1 : 0;
		Data <<= 1;
	}

	return BitNumber;
}
