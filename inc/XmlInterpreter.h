//----------------------------------------------------------------------
// XmlInterpreter.h:
//   Declaration of functions to interprete XML element tree
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __XML_INTERPRETER_H__
#define __XML_INTERPRETER_H__

#include <stdlib.h>
#include "XmlElement.h"
#include "Trajectory.h"
#include "NavData.h"
#include "PowerControl.h"
#include "Tracking.h"

typedef int (*TagProcessFunction)(void *);

typedef struct
{
	const char *key;
	const char **values;
	int value_size;
	int default_value;
} ATTRIBUTE_TYPE, *PATTRIBUTE_TYPE;

typedef struct
{
	const char *TagName;
	TagProcessFunction ProcessFunction;
	void *Parameter;
} ELEMENT_PROCESS, *PELEMENT_PROCESS;

typedef struct
{
	PATTRIBUTE_TYPE *AttributeList;
	int AttributeNumber;
	ELEMENT_PROCESS *ProcessList;
	int ProcessListNumber;
} INTERPRETE_PARAM, *PINTERPRETE_PARAM;

int ElementProcTime(void *Param);
int ElementProcTrajectory(void *Param);
int ElementProcEphemeris(void *Param);
int ElementProcOutput(void *Param);
int ElementProcBasebandConfig(void *Param);
int ElementProcSatInitParam(void *Param);

extern PATTRIBUTE_TYPE InitTimeAttributes[];
extern PATTRIBUTE_TYPE TrajectoryAttributes[];
extern PATTRIBUTE_TYPE InitPosTypeAttributes[];
extern PATTRIBUTE_TYPE LatLonTypeAttributes[];
extern PATTRIBUTE_TYPE InitVelTypeAttributes[];
extern PATTRIBUTE_TYPE SpeedUnitAttributes[];
extern PATTRIBUTE_TYPE CourseUnitAttributes[];
extern PATTRIBUTE_TYPE EphAttributes[];
extern PATTRIBUTE_TYPE OutputAttributes[];
extern PATTRIBUTE_TYPE SatelliteAttributes[];
extern PATTRIBUTE_TYPE BasebandConfigAttributes[];
extern PATTRIBUTE_TYPE ElevationMaskAttributes[];
extern PATTRIBUTE_TYPE SystemAttributes[];
extern PATTRIBUTE_TYPE FreqIDAttributes[];
extern PATTRIBUTE_TYPE ChannelInitAttributes[];

extern const char *StartTimeElements[];
extern const char *StartPosElements[];
extern const char *StartVelElements[];
extern const char *TrajectoryElements[];
extern const char *TrajectoryTypeElements[];
extern const char *TrajectoryArgumentElements[];
extern const char *OutputParamElements[];
extern const char *PowerControlElements[];
extern const char *PowerParamElements[];
extern const char *SignalPowerElements[];
extern const char *DelayConfigElements[];
extern const char *ConfigParamElements[];
extern const char *BasebandConfigElements[];
extern const char *SatInitElements[];

BOOL AssignStartTime(CXmlElement *Element, UTC_TIME &UtcTime);
BOOL SetTrajectory(CXmlElement *Element, LLA_POSITION &StartPos, LOCAL_SPEED &StartVel, CTrajectory &Trajectory);
BOOL SetOutputParam(CXmlElement *Element, OUTPUT_PARAM &OutputParam);
BOOL SetPowerControl(CXmlElement *Element, CPowerControl &PowerControl);
BOOL SetDelayConfig(CXmlElement *Element, DELAY_CONFIG &DelayConfig);
BOOL SetBasebandConfig(CXmlElement *Element, BASEBAND_CONFIG &BasebandConfig);
BOOL SetSatInitParam(CXmlElement *Element, CHANNEL_INIT_PARAM SatInitParam[]);
BOOL AssignParameters(CXmlElement *RootElement, PUTC_TIME UtcTime, PLLA_POSITION StartPos, PLOCAL_SPEED StartVel, CTrajectory *Trajectory, CNavData *NavData, POUTPUT_PARAM OutputParam, CPowerControl *PowerControl, PDELAY_CONFIG DelayConfig);

int FindAttribute(char *key, PATTRIBUTE_TYPE *AttributeList);
int GetAttributeIndex(char *value, PATTRIBUTE_TYPE Attribute);
int GetElementIndex(const char *tag, const char **ElementList);
int FindTagIndex(char *Tag, ELEMENT_PROCESS *ProcessList, int ProcessListNumber);
int ProcessElement(CXmlElement *Element, PINTERPRETE_PARAM InterpreteParam);

int ElementProcTime(void *Param);
int ElementProcTrajectory(void *Param);
int ElementProcEphemeris(void *Param);
int ElementProcOutput(void *Param);
int ElementProcBasebandConfig(void *Param);
int ElementProcSatInitParam(void *Param);

#endif // __XML_INTERPRETER_H__
