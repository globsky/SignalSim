//----------------------------------------------------------------------
// XmlInterpreter.cpp:
//   Implementation of functions to interprete XML element tree
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "GnssTime.h"
#include "Coordinate.h"
#include "XmlInterpreter.h"
#include "Tracking.h"

static BOOL AssignStartPosition(CXmlElement *Element, LLA_POSITION &StartPos);
static BOOL AssignStartVelocity(CXmlElement *Element, LOCAL_SPEED &StartVel);
static BOOL AssignTrajectoryList(CXmlElement *Element, CTrajectory &Trajectory);
static TrajectoryType GetTrajectorySegment(CXmlElement *Element, TrajectoryDataType &DataType1, double &Data1, TrajectoryDataType &DataType2, double &Data2);
static BOOL GetTrajectorySegmentData(CXmlElement *Element, TrajectoryDataType &DataType, double &Data);
static BOOL ProcessConfigParam(CXmlElement *Element, OUTPUT_PARAM &OutputParam);

BOOL AssignStartTime(CXmlElement *Element, UTC_TIME &UtcTime)
{
	int i, Type = 0;	// 0 for GPS, 1 for GLONASS, 4 for UTC
	int Week = 0, LeapYear = 0;
	GNSS_TIME GnssTime;
	GLONASS_TIME GlonassTime;
	CSimpleDict *Attributes = Element->GetAttributes();
	CXmlElement *SubElement = 0;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, InitTimeAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, InitTimeAttributes[0]); break;
		}
	}

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), StartTimeElements))
		{
		case 0: Week = atoi(SubElement->GetText()); break;
		case 1: LeapYear = atoi(SubElement->GetText()); break;
		case 2: UtcTime.Day = atoi(SubElement->GetText()); break;
		case 3: UtcTime.Second = atof(SubElement->GetText()); break;
		case 4: UtcTime.Year = atoi(SubElement->GetText()); break;
		case 5: UtcTime.Month = atoi(SubElement->GetText()); break;
		case 6: UtcTime.Hour = atoi(SubElement->GetText()); break;
		case 7: UtcTime.Minute = atoi(SubElement->GetText()); break;
		}
	}

	if (Type == 0)
	{
		GnssTime.Week = Week;
		GnssTime.Seconds = UtcTime.Second;
		UtcTime = GpsTimeToUtc(GnssTime);
	}
	else if (Type == 1)
	{
		GlonassTime.LeapYear = LeapYear;
		GlonassTime.Day = UtcTime.Day;
		GlonassTime.Seconds = UtcTime.Second;
		UtcTime = GlonassTimeToUtc(GlonassTime);
	}

	return TRUE;
}

BOOL SetTrajectory(CXmlElement *Element, LLA_POSITION &StartPos, LOCAL_SPEED &StartVel, CTrajectory &Trajectory)
{
	int i, Content = 0;
	CXmlElement *SubElement = 0;
	CSimpleDict *Attributes = Element->GetAttributes();

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		if (strcmp(Attributes->Dictionary[i].key, "name") == 0)
			Trajectory.SetTrajectoryName(Attributes->Dictionary[i].value);
	}

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), TrajectoryElements))
		{
		case 0: AssignStartPosition(SubElement, StartPos); Content |= 1; break;
		case 1: AssignStartVelocity(SubElement, StartVel); Content |= 2; break;
		case 2:
			if ((Content & 3) != 3)
				return FALSE;
			Trajectory.SetInitPosVel(StartPos, StartVel, FALSE);
			AssignTrajectoryList(SubElement, Trajectory);
			break;
		}
	}

	return TRUE;
}

BOOL AssignStartPosition(CXmlElement *Element, LLA_POSITION &StartPos)
{
	int i;
	CXmlElement *SubElement = 0;
	int Type = 0;
	CSimpleDict *Attributes = Element->GetAttributes();
	KINEMATIC_INFO Position;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, InitPosTypeAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, InitPosTypeAttributes[0]); break;
		}
	}

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), StartPosElements))
		{
		case 0: StartPos.lon = DEG2RAD(atof(SubElement->GetText())); break;
		case 1: StartPos.lat = DEG2RAD(atof(SubElement->GetText())); break;
		case 2: StartPos.alt = atof(SubElement->GetText()); break;
		case 3: Position.x = atof(SubElement->GetText()); break;
		case 4: Position.y = atof(SubElement->GetText()); break;
		case 5: Position.z = atof(SubElement->GetText()); break;
		}
	}
	if (Type)
		StartPos = EcefToLla(Position);

	return TRUE;
}

BOOL AssignStartVelocity(CXmlElement *Element, LOCAL_SPEED &StartVel)
{
	int i, Type = 0;	// 0 for SCU, 1 for ENU
	CXmlElement *SubElement = 0;
	CSimpleDict *Attributes = Element->GetAttributes();

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, InitVelTypeAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, InitVelTypeAttributes[0]); break;
		}
	}

	StartVel.vu = 0;
	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), StartVelElements))
		{
		case 0: StartVel.speed = atof(SubElement->GetText()); break;
		case 1: StartVel.course = DEG2RAD(atof(SubElement->GetText())); break;
		case 2: StartVel.ve = atof(SubElement->GetText()); break;
		case 3: StartVel.vn = atof(SubElement->GetText()); break;
		case 4: StartVel.vu = atof(SubElement->GetText()); break;
		}
	}

	if (Type == 0)
		SpeedCourseToEnu(StartVel);
	else
		SpeedEnuToCourse(StartVel);

	return TRUE;
}

BOOL AssignTrajectoryList(CXmlElement *Element, CTrajectory &Trajectory)
{
	CXmlElement *SubElement = 0;
	TrajectoryType Type;
	TrajectoryDataType DataType1, DataType2;
	double Data1, Data2;

	Trajectory.ClearTrajectoryList();
	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		Type = GetTrajectorySegment(SubElement, DataType1, Data1, DataType2, Data2);
		if (Type != TrajTypeUnknown)
			Trajectory.AppendTrajectory(Type, DataType1, Data1, DataType2, Data2);
	}

	return TRUE;
}

TrajectoryType GetTrajectorySegment(CXmlElement *Element, TrajectoryDataType &DataType1, double &Data1, TrajectoryDataType &DataType2, double &Data2)
{
	TrajectoryType Type = TrajTypeUnknown;
	CXmlElement *SubElement = 0;

	Type = (TrajectoryType)(GetElementIndex(Element->GetTag(), TrajectoryTypeElements) + 1);

	if (Type == TrajTypeUnknown)
		return TrajTypeUnknown;

	if ((SubElement = Element->EnumSubElement(SubElement)) == NULL)
		return TrajTypeUnknown;
	if (!GetTrajectorySegmentData(SubElement, DataType1, Data1))
		return TrajTypeUnknown;
	if (Type != TrajTypeConstSpeed && (SubElement = Element->EnumSubElement(SubElement)) == NULL)
		return TrajTypeUnknown;
	if (!GetTrajectorySegmentData(SubElement, DataType2, Data2))
		return TrajTypeUnknown;

	return Type;
}

BOOL GetTrajectorySegmentData(CXmlElement *Element, TrajectoryDataType &DataType, double &Data)
{
	DataType = (TrajectoryDataType)(GetElementIndex(Element->GetTag(), TrajectoryArgumentElements));
	Data = atof(Element->GetText());
	if (DataType == TrajDataAngle)
		Data = DEG2RAD(Data);
	return DataType >= 0;
}

BOOL SetOutputParam(CXmlElement *Element, OUTPUT_PARAM &OutputParam)
{
	int i;
	CXmlElement *SubElement = 0;
	CSimpleDict *Attributes = Element->GetAttributes();

	// set default value
	OutputParam.filename[0] = 0;
	OutputParam.GpsMaskOut = OutputParam.GlonassMaskOut = 0;
	OutputParam.BdsMaskOut = OutputParam.GalileoMaskOut = 0LL;
	OutputParam.ElevationMask = DEG2RAD(5);
	OutputParam.Interval = 1.0;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, OutputAttributes))
		{
		case 0: OutputParam.Type = (OutputType)GetAttributeIndex(Attributes->Dictionary[i].value, OutputAttributes[0]); break;
		case 1: OutputParam.Format = (OutputFormat)GetAttributeIndex(Attributes->Dictionary[i].value, OutputAttributes[1]); break;
		}
	}

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), OutputParamElements))
		{
		case 0: OutputParam.Interval = atof(SubElement->GetText()); break;
		case 1: strcpy(OutputParam.filename, SubElement->GetText()); break;
		case 2: ProcessConfigParam(SubElement, OutputParam); break;
		}
	}

	return TRUE;
}

BOOL ProcessConfigParam(CXmlElement *Element, OUTPUT_PARAM &OutputParam)
{
	int index;
	CXmlElement *SubElement = 0;
	CSimpleDict *Attributes;
	int system = 0, svid;

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), ConfigParamElements))
		{
		case 0:
			Attributes = SubElement->GetAttributes();
			index = Attributes->Find("unit");
			if ((index >= 0) && strcmp(Attributes->Dictionary[index].value, "rad") == 0)
				OutputParam.ElevationMask = atof(SubElement->GetText());
			else
				OutputParam.ElevationMask = DEG2RAD(atof(SubElement->GetText()));
			break;
		case 1:
			Attributes = SubElement->GetAttributes();
			index = Attributes->Find("system");
			if (index >= 0)
			{
				if (strcmp(Attributes->Dictionary[index].value, "GLONASS") == 0)
					system = 1;
				else if (strcmp(Attributes->Dictionary[index].value, "BDS") == 0)
					system = 2;
				else if (strcmp(Attributes->Dictionary[index].value, "Galileo") == 0)
					system = 3;
			}
			svid = atoi(SubElement->GetText());
			switch (system)
			{
			case 0:
				if (svid >= 1 && svid <= 32)
					OutputParam.GpsMaskOut |= (1 << (svid-1));
				break;
			case 1:
				if (svid >= 1 && svid <= 24)
					OutputParam.GlonassMaskOut |= (1 << (svid-1));
				break;
			case 2:
				if (svid >= 1 && svid <= 64)
					OutputParam.BdsMaskOut |= (1LL << (svid-1));
				break;
			case 3:
				if (svid >= 1 && svid <= 64)
					OutputParam.GalileoMaskOut |= (1LL << (svid-1));
				break;
			}
			break;
		}
	}

	return TRUE;
}

BOOL SetBasebandConfig(CXmlElement *Element, BASEBAND_CONFIG &BasebandConfig)
{
	int i, Type = 0;	// 0 for GPS, 1 for GLONASS, 4 for UTC
	CSimpleDict *Attributes = Element->GetAttributes();
	CXmlElement *SubElement = 0;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, BasebandConfigAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, BasebandConfigAttributes[0]); break;
		}
	}

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), BasebandConfigElements))
		{
		case 0: BasebandConfig.ChannelNumber = atoi(SubElement->GetText()); break;
		case 1: BasebandConfig.CorNumber = atoi(SubElement->GetText()); break;
		case 2: BasebandConfig.NoiseFloor = atof(SubElement->GetText()); break;
		}
	}

	return TRUE;
}

BOOL SetSatInitParam(CXmlElement *Element, CHANNEL_INIT_PARAM SatInitParam[])
{
	int i, svid = 0, enable = 0, Type = 0;	// 0 for GPS, 1 for GLONASS, 4 for UTC
	CSimpleDict *Attributes = Element->GetAttributes();
	CXmlElement *SubElement = 0;
	static CHANNEL_INIT_PARAM DefaultParam = {0, 2, 4, 0.0, 0.0, 0.0, 5.0 };
	CHANNEL_INIT_PARAM CurrentParam = DefaultParam;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, ChannelInitAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, ChannelInitAttributes[0]); break;
		case 1: svid = atoi(Attributes->Dictionary[i].value); break;
		case 2: enable = GetAttributeIndex(Attributes->Dictionary[i].value, ChannelInitAttributes[2]); break;
		}
	}

	if (svid > 0)
		CurrentParam.Enable = enable + 1;

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		switch (GetElementIndex(SubElement->GetTag(), SatInitElements))
		{
		case 0: CurrentParam.CorInterval = atoi(SubElement->GetText()); break;
		case 1: CurrentParam.PeakCor = atoi(SubElement->GetText()); break;
		case 2: CurrentParam.InitFreqError = atof(SubElement->GetText()); break;
		case 3: CurrentParam.InitPhaseError = atof(SubElement->GetText()); break;
		case 4: CurrentParam.InitCodeError = atof(SubElement->GetText()); break;
		case 5: CurrentParam.SnrRatio = atof(SubElement->GetText()); break;
		}
	}

	if (svid > 0 && svid <= 32)
		SatInitParam[svid-1]= CurrentParam;
	else if (svid == 0)
	{
		DefaultParam = CurrentParam;
		for (i = 0; i < 32; i ++)
			if (SatInitParam[i].Enable == 0)
				SatInitParam[i]= CurrentParam;
	}

	return TRUE;
}

int FindAttribute(char *key, PATTRIBUTE_TYPE *AttributeList)
{
	int i = 0;

	while (AttributeList[i])
	{
		if (strcmp(key, AttributeList[i]->key) == 0)
			return i;
		i ++;
	}
	return -1;
}

int GetAttributeIndex(char *value, PATTRIBUTE_TYPE Attribute)
{
	int i;

	for (i = 0; i < Attribute->value_size; i ++)
		if (strcmp(value, Attribute->values[i]) == 0)
			break;
	return (i == Attribute->value_size) ? Attribute->default_value : i;
}

int GetElementIndex(char *tag, char **ElementList)
{
	int i = 0;

	while (ElementList[i] != NULL)
	{
		if (strcmp(tag, ElementList[i]) == 0)
			break;
		i ++;
	}

	return (ElementList[i] ? i : -1);
}

int FindTagIndex(char *Tag, ELEMENT_PROCESS *ProcessList, int ProcessListNumber)
{
	int i;

	for (i = 0; i < ProcessListNumber; i ++)
		if (strcmp(Tag, ProcessList[i].TagName) == 0)
			break;
	return i;
}

int ProcessElement(CXmlElement *Element, PINTERPRETE_PARAM InterpreteParam)
{
	CXmlElement *SubElement = NULL;
	int index;

	while ((SubElement = Element->EnumSubElement(SubElement)) != NULL)
	{
		if ((index = FindTagIndex(SubElement->GetTag(), InterpreteParam->ProcessList, InterpreteParam->ProcessListNumber)) < InterpreteParam->ProcessListNumber)
			InterpreteParam->ProcessList[index].ProcessFunction(InterpreteParam->ProcessList[index].Parameter);
	}

	return 0;
}

int ElementProcTime(void *Param)
{
	return 0;
}

int ElementProcTrajectory(void *Param)
{
	return 0;
}

int ElementProcEphemeris(void *Param)
{
	return 0;
}

int ElementProcOutput(void *Param)
{
	return 0;
}

int ElementProcBasebandConfig(void *Param)
{
	return 0;
}

int ElementProcSatInitParam(void *Param)
{
	return 0;
}
