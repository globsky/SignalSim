//----------------------------------------------------------------------
// PowerControl.cpp:
//   Definition of signal power control class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>

#include "ConstVal.h"
#include "PowerControl.h"

CPowerControl::CPowerControl()
{
	ArraySize = 0;
	PowerControlArray = NULL;
	Adjust = ElevationAdjustNone;
	NoiseFloor = -172.;
	InitCN0 = 47.;
}

CPowerControl::~CPowerControl()
{
	if (PowerControlArray)
		free(PowerControlArray);
}

void CPowerControl::AddControlElement(PSIGNAL_POWER pControlElement)
{
	// allocate array or expand size
	if (!PowerControlArray)
		PowerControlArray = (PSIGNAL_POWER)malloc(100 * sizeof(SIGNAL_POWER));
	else if ((ArraySize % 100) == 0)
		PowerControlArray = (PSIGNAL_POWER)realloc(PowerControlArray, (ArraySize + 100) * sizeof(SIGNAL_POWER));

	memcpy(PowerControlArray + ArraySize, pControlElement, sizeof(SIGNAL_POWER));
	ArraySize ++;
}

void CPowerControl::Sort()
{
	int i, j;
	int min_index;
	SIGNAL_POWER temp;

	for (i = 0; i < ArraySize - 1; i ++)
	{
		min_index = i;
		for (j = i + 1; j < ArraySize; j ++)
		{
			if (PowerControlArray[j].time < PowerControlArray[min_index].time)
				min_index = j;
		}
		// swap min time element to position i
		if (min_index != i)
		{
			temp = PowerControlArray[i];
			PowerControlArray[i] = PowerControlArray[min_index];
			PowerControlArray[min_index] = temp;
		}
	}
}

void CPowerControl::ResetTime()
{
	TimeElapsMs = 0;
	NextIndex = 0;
}

int CPowerControl::GetPowerControlList(int TimeStepMs, PSIGNAL_POWER &PowerList)
{
	int InitIndex = NextIndex;

	PowerList = PowerControlArray + NextIndex;
	TimeElapsMs += TimeStepMs;

	while (NextIndex < ArraySize)
	{
		if (PowerControlArray[NextIndex].time > TimeElapsMs)
			break;
		NextIndex ++;
	}

	return NextIndex - InitIndex;
}
