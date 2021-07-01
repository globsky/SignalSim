//----------------------------------------------------------------------
// XmlArguments.cpp:
//   Definition of XML file arguments (attribute keys and values, element name)
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include "XmlInterpreter.h"

char *FlexibleName[] = { "" };
char *SystemType[] = { "GPS", "GLONASS", "BDS", "Galileo", "UTC" };
char *InitPosType[] = { "LLA", "ECEF" };
char *LatLonType[] = { "d", "dm", "dms" };
char *InitVelType[] = { "SCU", "ENU", "ECEF" };
char *SpeedUnit[] = { "mps", "kph", "knot", "mph" };
char *AngleUnit[] = { "degree", "rad" };
char *EphSrc[] = { "file" };
char *EphType[] = { "RINEX" };
char *OutputType[] = { "position", "observation" };
char *OutputFormat[] = { "ECEF", "LLA", "NMEA", "RINEX" };
char *TimeUnit[] = { "s", "ms" };
char *ChannelEnable[] = { "auto", "true" };

ATTRIBUTE_TYPE TimeTypeAttr = { "type", SystemType, sizeof(SystemType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE TrajectoryNameAttr = { "name", FlexibleName, sizeof(FlexibleName) / sizeof(char *), -1 };
ATTRIBUTE_TYPE InitPosTypeAttr = { "type", InitPosType, sizeof(InitPosType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE LatLonTypeAttr = { "type", LatLonType, sizeof(LatLonType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE InitVelTypeAttr = { "type", InitVelType, sizeof(InitVelType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE SpeedUnitAttr = { "unit", SpeedUnit, sizeof(SpeedUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE CourseUnitAttr = { "unit", AngleUnit, sizeof(AngleUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE EphSrcAttr = { "source", EphSrc, sizeof(EphSrc) / sizeof(char *), 0 };
ATTRIBUTE_TYPE EphTypeAttr = { "type", EphType, sizeof(EphType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE OutputTypeAttr = { "type", OutputType, sizeof(OutputType) / sizeof(char *), -1 };
ATTRIBUTE_TYPE OutputFormatAttr = { "format", OutputFormat, sizeof(OutputFormat) / sizeof(char *), 0 };
ATTRIBUTE_TYPE IntervalUnitAttr = { "unit", TimeUnit, sizeof(TimeUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE ElevationMaskAttr = { "unit", AngleUnit, sizeof(AngleUnit) / sizeof(char *), 0 };
ATTRIBUTE_TYPE SystemAttr = { "system", SystemType, sizeof(SystemType) / sizeof(char *), 0 };
ATTRIBUTE_TYPE SvidAttr = { "svid", FlexibleName, sizeof(FlexibleName) / sizeof(char *), -1 };
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
PATTRIBUTE_TYPE BasebandConfigAttributes[] = { &TimeTypeAttr, NULL };
PATTRIBUTE_TYPE ElevationMaskAttributes[] = { &ElevationMaskAttr, NULL };
PATTRIBUTE_TYPE MaskOutAttributes[] = { &SystemAttr, NULL };
PATTRIBUTE_TYPE ChannelInitAttributes[] = { &SystemAttr, &SvidAttr, &ChannelEnableAttr, NULL };

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
