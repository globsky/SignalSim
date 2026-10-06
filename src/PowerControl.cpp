//----------------------------------------------------------------------
// PowerControl.cpp:
//   Definition of signal power control class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <algorithm>

#include "ConstVal.h"
#include "PowerControl.h"

CPowerControl::CPowerControl()
{
	Adjust = ElevationAdjustNone;
	NoiseFloor = -172.;
	InitCN0 = 47.;
	ResetTime();
}

CPowerControl::~CPowerControl()
{
}

void CPowerControl::Clear()
{
	PowerControlArray.clear();
}

void CPowerControl::AddControlElement(PSIGNAL_POWER pControlElement)
{
	PowerControlArray.push_back(*pControlElement);
}

void CPowerControl::Sort()
{
	std::sort(PowerControlArray.begin(), PowerControlArray.end(), [](const SIGNAL_POWER &a, const SIGNAL_POWER &b) { return a.time < b.time; });
}

void CPowerControl::ResetTime()
{
	TimeElapsMs = 0;
	NextIndex = 0;
}

int CPowerControl::GetPowerControlList(int TimeStepMs, PSIGNAL_POWER &PowerList)
{
	size_t InitIndex = NextIndex;

	PowerList = (NextIndex >= PowerControlArray.size()) ? NULL : &PowerControlArray[NextIndex];
	TimeElapsMs += TimeStepMs;

	while (NextIndex < PowerControlArray.size())
	{
		if (PowerControlArray[NextIndex].time > TimeElapsMs)
			break;
		NextIndex ++;
	}

	return NextIndex - InitIndex;
}
