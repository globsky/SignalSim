//----------------------------------------------------------------------
// JsonComposer.h:
//   Declaration of functions to compose JSON object tree
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __JSON_COMPOSER_H__
#define __JSON_COMPOSER_H__

#include <stdlib.h>

#include "BasicTypes.h"
#include "JsonParser.h"
#include "GnssTime.h"
#include "Trajectory.h"
#include "NavData.h"
#include "Coordinate.h"
#include "PowerControl.h"
#include "Tracking.h"

int popcnt64(unsigned long long x);
int ctzll(unsigned long long x);

JsonObject *AssignValue(JsonObject *Object, const char *Key);
JsonObject *AssignValue(JsonObject *Object, const char *Key, const char *Value);
JsonObject *AssignValue(JsonObject *Object, const char *Key, int Value);
JsonObject *AssignValue(JsonObject *Object, const char *Key, double Value);
JsonObject *AssignValue(JsonObject *Object, const char *Key, bool Value);

double LonLatToRad(double Value, int Format);
double LonLatFromRad(double Value, int Format);
double SpeedUnitToMPS(double Speed, int Unit);
double SpeedUnitFromMPS(double Speed, int Unit);

JsonObject *ComposeVersion(double Version);
JsonObject *ComposeDescription(const char *Description);
JsonObject *ComposeStartTime(int Type, const UTC_TIME &UtcTime);
JsonObject *ComposeTrajectory(const char *TrajName, KINEMATIC_INFO InitPosVel);
JsonObject *ComposeInitPosition(KINEMATIC_INFO InitPosition);
JsonObject *ComposeInitPosition(LLA_POSITION InitPosition, int Format, int Convert);
JsonObject *ComposeInitVelocity(KINEMATIC_INFO InitVelocity, int SpeedUnit, int Convert);
JsonObject *ComposeInitVelocity(LOCAL_SPEED InitVelocity, int Type, int SpeedUnit, int CourseUnit, int Convert);
JsonObject *ComposeTrajectorySegment(TrajectoryType TrajType, TrajectoryDataType TrajDataType1, double TrajData1, TrajectoryDataType TrajDataType2, double TrajData2);
JsonObject *ComposeOutput(int Type, int Format, const char *OutputName, int IntervalMs, double ElevationMask);
JsonObject *ComposeConfig(double ElevationMask);
JsonObject *ComposeSignalSelect(GnssSystem System, int SignalIndex, bool Enable);
JsonObject *ComposeMaskOut(GnssSystem System, unsigned long long MaskOut);
JsonObject *ComposePower(double NoiseFloor, int PowerUnit, double InitPower);
JsonObject* ComposeSignalPowerItem(GnssSystem System, double Epoch, double Value, int Unit);
JsonObject* ComposePowerValueItem(double Epoch, double Value, int Unit);
JsonObject* ComposeSvList(GnssSystem System, unsigned long long MaskOut);

#endif // __JSON_COMPOSER_H__
