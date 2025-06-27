//----------------------------------------------------------------------
// JsonInterpreter.h:
//   Declaration of functions to interprete JSON object tree
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __JSON_INTERPRETER_H__
#define __JSON_INTERPRETER_H__

#include <stdlib.h>

#include "BasicTypes.h"
#include "JsonParser.h"
#include "GnssTime.h"
#include "Trajectory.h"
#include "NavData.h"
#include "Coordinate.h"
#include "PowerControl.h"
#include "Tracking.h"

BOOL AssignParameters(JsonObject *Object, PUTC_TIME UtcTime, PLLA_POSITION StartPos, PLOCAL_SPEED StartVel, CTrajectory *Trajectory, CNavData *NavData, POUTPUT_PARAM OutputParam, CPowerControl *PowerControl, PDELAY_CONFIG DelayConfig);

#endif // __JSON_INTERPRETER_H__
