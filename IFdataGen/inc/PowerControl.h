//----------------------------------------------------------------------
// PowerControl.h:
//   Declaration of signal power control class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __POWER_CONTROL_H__
#define __POWER_CONTROL_H__

#include "BasicTypes.h"

typedef struct
{
	int system;
	int svid;
	int time;
	double CN0;
} SIGNAL_POWER, *PSIGNAL_POWER;

enum ElevationAdjust { ElevationAdjustNone, ElevationAdjustSinSqrtFade };

class CPowerControl
{
public:
	CPowerControl();
	~CPowerControl();
	enum ElevationAdjust Adjust;
	double NoiseFloor;
	double InitCN0;
	int ArraySize;
	int NextIndex;
	PSIGNAL_POWER PowerControlArray;
	int TimeElapsMs;

	void AddControlElement(PSIGNAL_POWER pControlElement);
	void Sort();
	void ResetTime();
	int GetPowerControlList(int TimeStepMs, PSIGNAL_POWER &PowerList);
};

#endif // __POWER_CONTROL_H__
