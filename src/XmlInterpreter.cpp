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
	CXmlElement *SubElement;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, InitTimeAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, InitTimeAttributes[0]); break;
		}
	}

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "Week") == 0)
			Week = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "LeapYear") == 0)
			LeapYear = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Day") == 0)
			UtcTime.Day = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Second") == 0)
			UtcTime.Second = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Year") == 0)
			UtcTime.Year = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Month") == 0)
			UtcTime.Month = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Hour") == 0)
			UtcTime.Hour = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Minute") == 0)
			UtcTime.Minute = atoi(SubElement->GetText());
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
	int i;
	CXmlElement *SubElement;

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "InitPosition") == 0)
			AssignStartPosition(SubElement, StartPos);
		else if (strcmp(SubElement->GetTag(), "InitVelocity") == 0)
			AssignStartVelocity(SubElement, StartVel);
		else if (strcmp(SubElement->GetTag(), "TrajectoryList") == 0)
		{
			Trajectory.SetInitPosVel(StartPos, StartVel, FALSE);
			AssignTrajectoryList(SubElement, Trajectory);
		}
	}

	return TRUE;
}

BOOL AssignStartPosition(CXmlElement *Element, LLA_POSITION &StartPos)
{
	int i;
	CXmlElement *SubElement;
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

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "Longitude") == 0)
			StartPos.lon = DEG2RAD(atof(SubElement->GetText()));
		else if (strcmp(SubElement->GetTag(), "Latitude") == 0)
			StartPos.lat = DEG2RAD(atof(SubElement->GetText()));
		else if (strcmp(SubElement->GetTag(), "Altitude") == 0)
			StartPos.alt = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "x") == 0)
			Position.x = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "y") == 0)
			Position.y = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "z") == 0)
			Position.z = atof(SubElement->GetText());
	}
	if (Type)
		StartPos = EcefToLla(Position);

	return TRUE;
}

BOOL AssignStartVelocity(CXmlElement *Element, LOCAL_SPEED &StartVel)
{
	int i, Type = 0;	// 0 for SCU, 1 for ENU
	CXmlElement *SubElement;
	CSimpleDict *Attributes = Element->GetAttributes();

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, InitVelTypeAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, InitVelTypeAttributes[0]); break;
		}
	}

	StartVel.vu = 0;
	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "Speed") == 0)
			StartVel.speed = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Course") == 0)
			StartVel.course = DEG2RAD(atof(SubElement->GetText()));
		else if (strcmp(SubElement->GetTag(), "East") == 0)
			StartVel.ve = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "North") == 0)
			StartVel.vn = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Up") == 0)
			StartVel.vu = atof(SubElement->GetText());
	}

	if (Type == 0)
		SpeedCourseToEnu(StartVel);
	else
		SpeedEnuToCourse(StartVel);

	return TRUE;
}

BOOL AssignTrajectoryList(CXmlElement *Element, CTrajectory &Trajectory)
{
	int i;
	CXmlElement *SubElement;
	TrajectoryType Type;
	TrajectoryDataType DataType1, DataType2;
	double Data1, Data2;

	Trajectory.ClearTrajectoryList();
	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
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
	CXmlElement *SubElement;

	if (strcmp(Element->GetTag(), "Const") == 0)
		Type = TrajTypeConstSpeed;
	else if (strcmp(Element->GetTag(), "ConstAcc") == 0)
		Type = TrajTypeConstAcc;
	else if (strcmp(Element->GetTag(), "VerticalAcc") == 0)
		Type = TrajTypeVerticalAcc;
	else if (strcmp(Element->GetTag(), "Jerk") == 0)
		Type = TrajTypeJerk;
	else if (strcmp(Element->GetTag(), "HorizontalTurn") == 0)
		Type = TrajTypeHorizontalCircular;

	if (Type == TrajTypeUnknown)
		return TrajTypeUnknown;

	if ((SubElement = Element->GetElement(0)) == NULL)
		return TrajTypeUnknown;
	GetTrajectorySegmentData(SubElement, DataType1, Data1);
	if (Type != TrajTypeConstSpeed && (SubElement = Element->GetElement(1)) == NULL)
		return TrajTypeUnknown;
	GetTrajectorySegmentData(SubElement, DataType2, Data2);

	return Type;
}

BOOL GetTrajectorySegmentData(CXmlElement *Element, TrajectoryDataType &DataType, double &Data)
{
	if (strcmp(Element->GetTag(), "TimeSpan") == 0)
	{
		DataType = TrajDataTimeSpan;
		Data = atof(Element->GetText());
	}
	else if (strcmp(Element->GetTag(), "Acceleration") == 0)
	{
		DataType = TrajDataAcceleration;
		Data = atof(Element->GetText());
	}
	else if (strcmp(Element->GetTag(), "Speed") == 0)
	{
		DataType = TrajDataSpeed;
		Data = atof(Element->GetText());
	}
	else if (strcmp(Element->GetTag(), "AccRate") == 0)
	{
		DataType = TrajDataAccRate;
		Data = atof(Element->GetText());
	}
	else if (strcmp(Element->GetTag(), "TurnAngle") == 0)
	{
		DataType = TrajDataAngle;
		Data = DEG2RAD(atof(Element->GetText()));
	}
	else if (strcmp(Element->GetTag(), "AngularRate") == 0)
	{
		DataType = TrajDataAngularRate;
		Data = atof(Element->GetText());
	}
	else if (strcmp(Element->GetTag(), "Radius") == 0)
	{
		DataType = TrajDataRadius;
		Data = atof(Element->GetText());
	}

	return TRUE;
}

BOOL SetOutputParam(CXmlElement *Element, OUTPUT_PARAM &OutputParam)
{
	int i;
	CXmlElement *SubElement;
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

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "Interval") == 0)
			OutputParam.Interval = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "Name") == 0)
			strcpy(OutputParam.filename, SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "ConfigParam") == 0)
			ProcessConfigParam(SubElement, OutputParam);
	}

	return TRUE;
}

BOOL ProcessConfigParam(CXmlElement *Element, OUTPUT_PARAM &OutputParam)
{
	int i, index;
	CXmlElement *SubElement;
	CSimpleDict *Attributes;
	int system = 0, svid;

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "ElevationMask") == 0)
		{
			Attributes = SubElement->GetAttributes();
			index = Attributes->Find("unit");
			if ((index >= 0) && strcmp(Attributes->Dictionary[index].value, "rad") == 0)
				OutputParam.ElevationMask = atof(SubElement->GetText());
			else
				OutputParam.ElevationMask = DEG2RAD(atof(SubElement->GetText()));
		}
		else if (strcmp(SubElement->GetTag(), "MaskOut") == 0)
		{
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
		}
	}

	return TRUE;
}

BOOL SetBasebandConfig(CXmlElement *Element, BASEBAND_CONFIG &BasebandConfig)
{
	int i, Type = 0;	// 0 for GPS, 1 for GLONASS, 4 for UTC
	CSimpleDict *Attributes = Element->GetAttributes();
	CXmlElement *SubElement;

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		switch (FindAttribute(Attributes->Dictionary[i].key, BasebandConfigAttributes))
		{
		case 0: Type = GetAttributeIndex(Attributes->Dictionary[i].value, BasebandConfigAttributes[0]); break;
		}
	}

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "ChannelNumber") == 0)
			BasebandConfig.ChannelNumber = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "CorrelatorNumber") == 0)
			BasebandConfig.CorNumber = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "NoiseFloor") == 0)
			BasebandConfig.NoiseFloor = atof(SubElement->GetText());
	}

	return TRUE;
}

BOOL SetSatInitParam(CXmlElement *Element, CHANNEL_INIT_PARAM SatInitParam[])
{
	int i, svid = 0, enable = 0, Type = 0;	// 0 for GPS, 1 for GLONASS, 4 for UTC
	CSimpleDict *Attributes = Element->GetAttributes();
	CXmlElement *SubElement;
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

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "CorrelatorInterval") == 0)
			CurrentParam.CorInterval = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "PeakCorrelator") == 0)
			CurrentParam.PeakCor = atoi(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "InitFreqError") == 0)
			CurrentParam.InitFreqError = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "InitPhaseError") == 0)
			CurrentParam.InitPhaseError = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "InitCodeError") == 0)
			CurrentParam.InitCodeError = atof(SubElement->GetText());
		else if (strcmp(SubElement->GetTag(), "SNR") == 0)
			CurrentParam.SnrRatio = atof(SubElement->GetText());
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
