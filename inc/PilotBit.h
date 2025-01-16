//----------------------------------------------------------------------
// PilotBit.h:
//   Pilot channel secondary bit for different signal
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__PILOT_BIT_H__)
#define __PILOT_BIT_H__

#include "BasicTypes.h"

const unsigned int *GetPilotBits(GnssSystem System, int SatSignal, int svid, int &Length);

#endif //!defined(__PILOT_BIT_H__)
