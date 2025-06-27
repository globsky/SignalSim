//----------------------------------------------------------------------
// Tracking.h:
//   Declaration of functions for tracking configuration and implementation
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __TRACKING_H__
#define __TRACKING_H__

#include "BasicTypes.h"

typedef struct
{
	int ChannelNumber;
	int CorNumber;
	double NoiseFloor;
} BASEBAND_CONFIG, *PBASEBAND_CONFIG;

typedef struct
{
	int Enable;		// 0:default, 1:auto, 2:enable
	int CorInterval;
	int PeakCor;
	double InitFreqError;
	double InitPhaseError;
	double InitCodeError;
	double SnrRatio;
} CHANNEL_INIT_PARAM, *PCHANNEL_INIT_PARAM;




#endif // __TRACKING_H__
