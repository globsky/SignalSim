//----------------------------------------------------------------------
// XmlArguments.cpp:
//   Definition of XML file arguments (attribute keys and values, element name)
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include "XmlInterpreter.h"

const char *FlexibleName[] = { "" };
const char *SystemType[] = { "GPS", "BDS", "Galileo", "GLONASS", "UTC" };
const char *InitPosType[] = { "LLA", "ECEF" };
const char *LatLonType[] = { "d", "dm", "dms" };
const char *InitVelType[] = { "SCU", "ENU", "ECEF" };
const char *SpeedUnit[] = { "mps", "kph", "knot", "mph" };
const char *AngleUnit[] = { "degree", "rad" };
const char *EphSrc[] = { "file" };
const char *EphType[] = { "RINEX" };
const char *OutputTypeStr[] = { "position", "observation" };
const char *OutputFormatStr[] = { "ECEF", "LLA", "NMEA", "KML", "RINEX" };
const char *TimeUnit[] = { "s", "ms" };
const char *ChannelEnable[] = { "auto", "true" };
const char *FreqIDs[] = { "L1CA", "L1C", "L2C", "L2P", "L5", "", "", "",
                    "B1C", "B1I", "B2I", "B3I", "B2a", "B2b", "", "",
					"E1", "E5a", "E5b", "E5", "E6", "", "", "",
					"G1", "G2", "G3", "", "", "", "", "", };

ATTRIBUTE_TYPE TimeTypeAttr = { "type", SystemType, sizeof(SystemType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE TrajectoryNameAttr = { "name", FlexibleName, sizeof(FlexibleName) / sizeof(char *), -1 };
ATTRIBUTE_TYPE InitPosTypeAttr = { "type", InitPosType, sizeof(InitPosType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE LatLonTypeAttr = { "type", LatLonType, sizeof(LatLonType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE InitVelTypeAttr = { "type", InitVelType, sizeof(InitVelType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE SpeedUnitAttr = { "unit", SpeedUnit, sizeof(SpeedUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE CourseUnitAttr = { "unit", AngleUnit, sizeof(AngleUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE EphSrcAttr = { "source", EphSrc, sizeof(EphSrc) / sizeof(char *), 0 };
ATTRIBUTE_TYPE EphTypeAttr = { "type", EphType, sizeof(EphType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE OutputTypeAttr = { "type", OutputTypeStr, sizeof(OutputTypeStr) / sizeof(char *), -1 };
ATTRIBUTE_TYPE OutputFormatAttr = { "format", OutputFormatStr, sizeof(OutputFormatStr) / sizeof(char *), 0 };
ATTRIBUTE_TYPE IntervalUnitAttr = { "unit", TimeUnit, sizeof(TimeUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE ElevationMaskAttr = { "unit", AngleUnit, sizeof(AngleUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE SystemAttr = { "system", SystemType, sizeof(SystemType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE SvidAttr = { "svid", FlexibleName, sizeof(FlexibleName) / sizeof(char *), -1 };
ATTRIBUTE_TYPE FreqIDAttr = { "freq", FreqIDs, sizeof(FreqIDs) / sizeof(char *), -1 };
ATTRIBUTE_TYPE ChannelEnableAttr = { "enable", ChannelEnable, sizeof(ChannelEnable) / sizeof(char *), 0 };

PATTRIBUTE_TYPE InitTimeAttributes[] = { &TimeTypeAttr, NULL };
PATTRIBUTE_TYPE TrajectoryAttributes[] = { &TrajectoryNameAttr, NULL };
PATTRIBUTE_TYPE InitPosTypeAttributes[] = { &InitPosTypeAttr, NULL };
PATTRIBUTE_TYPE LatLonTypeAttributes[] = { &LatLonTypeAttr, NULL };
PATTRIBUTE_TYPE InitVelTypeAttributes[] = { &InitVelTypeAttr, NULL };
PATTRIBUTE_TYPE SpeedUnitAttributes[] = { &SpeedUnitAttr, NULL };
PATTRIBUTE_TYPE CourseUnitAttributes[] = { &CourseUnitAttr, NULL };
PATTRIBUTE_TYPE EphAttributes[] = { &EphSrcAttr, &EphTypeAttr, NULL };
PATTRIBUTE_TYPE OutputAttributes[] = { &OutputTypeAttr, &OutputFormatAttr, NULL };
PATTRIBUTE_TYPE SatelliteAttributes[] = { &SystemAttr, &SvidAttr, NULL };
PATTRIBUTE_TYPE BasebandConfigAttributes[] = { &TimeTypeAttr, NULL };
PATTRIBUTE_TYPE ElevationMaskAttributes[] = { &ElevationMaskAttr, NULL };
PATTRIBUTE_TYPE SystemAttributes[] = { &SystemAttr, NULL };
PATTRIBUTE_TYPE FreqIDAttributes[] = { &SystemAttr, &FreqIDAttr, NULL };
PATTRIBUTE_TYPE ChannelInitAttributes[] = { &SystemAttr, &SvidAttr, &ChannelEnableAttr, NULL };

const char *StartTimeElements[] = { "Week", "LeapYear", "Day", "Second", "Year", "Month", "Hour", "Minute", NULL };
const char *StartPosElements[] = { "Longitude", "Latitude", "Altitude", "x", "y", "z", NULL };
const char *StartVelElements[] = { "Speed", "Course", "East", "North", "Up", NULL };
const char *TrajectoryElements[] = { "InitPosition", "InitVelocity", "TrajectoryList", NULL };
const char *TrajectoryTypeElements[] = { "Const", "ConstAcc", "VerticalAcc", "Jerk", "HorizontalTurn", NULL };
const char *TrajectoryArgumentElements[] = { "TimeSpan", "Acceleration", "Speed", "AccRate", "TurnAngle", "AngularRate", "Radius", NULL };
const char *OutputParamElements[] = { "Interval", "Name", "ConfigParam", "SystemSelect", NULL };
const char *ConfigParamElements[] = { "ElevationMask", "MaskOut", NULL };
const char *PowerControlElements[] = { "PowerParam", "SignalPower", NULL };
const char *PowerParamElements[] = { "NoiseFloor", "InitPower", "ElevationAdjust", NULL };
const char *SignalPowerElements[] = { "Time", "Power", NULL };
const char *DelayConfigElements[] = { "SystemDelay", "ReceiverDelay", NULL };
const char *BasebandConfigElements[] = { "ChannelNumber", "CorrelatorNumber", "NoiseFloor", NULL };
const char *SatInitElements[] = { "CorrelatorInterval", "PeakCorrelator", "InitFreqError", "InitPhaseError", "InitCodeError", "SNR", NULL };

ELEMENT_PROCESS RootProcess[] = {
	{ "Time",           &ElementProcTime,           NULL},
	{ "Trajectory",     &ElementProcTrajectory,     NULL},
	{ "Ephemeris",      &ElementProcEphemeris,      NULL},
	{ "Output",         &ElementProcOutput,         NULL},
	{ "BasebandConfig", &ElementProcBasebandConfig, NULL},
	{ "ChannelInit",    &ElementProcSatInitParam,   NULL},
};

INTERPRETE_PARAM RootInterpretParam = { NULL, 0, RootProcess, sizeof(RootProcess) / sizeof(ELEMENT_PROCESS) };
INTERPRETE_PARAM InitTimeParam = { InitTimeAttributes, sizeof(InitTimeAttributes) / sizeof(PATTRIBUTE_TYPE), RootProcess, sizeof(RootProcess) / sizeof(ELEMENT_PROCESS) };
