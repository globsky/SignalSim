//----------------------------------------------------------------------
// PowerControl.h:
//   Declaration of signal power control class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __POWER_CONTROL_H__
#define __POWER_CONTROL_H__

#ifdef NULL
	#pragma push_macro("NULL")
	#undef NULL
	#define NULL 0
#endif
#include <vector>
#ifdef NULL
	#pragma pop_macro("NULL")
#endif

#include "BasicTypes.h"

typedef struct _tag_SIGNAL_POWER
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
	size_t NextIndex;
	std::vector <_tag_SIGNAL_POWER> PowerControlArray;
	int TimeElapsMs;

	void Clear();
	void AddControlElement(PSIGNAL_POWER pControlElement);
	void Sort();
	void ResetTime();
	int GetPowerControlList(int TimeStepMs, PSIGNAL_POWER &PowerList);
};

#endif // __POWER_CONTROL_H__
