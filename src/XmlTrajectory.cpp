//----------------------------------------------------------------------
// XmlTrajectory.cpp:
//   Implementation of functions to put XML element tree to trajectory
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "GnssTime.h"
#include "XmlTrajectory.h"

static BOOL AssignStartPosition(CXmlElement *Element, LLA_POSITION &StartPos);
static BOOL AssignStartVelocity(CXmlElement *Element, LOCAL_SPEED &StartVel);
static BOOL AssignTrajectoryList(CXmlElement *Element, CTrajectory &Trajectory);
static TrajectoryType GetTrajectorySegment(CXmlElement *Element, TrajectoryDataType &DataType1, double &Data1, TrajectoryDataType &DataType2, double &Data2);
static BOOL GetTrajectorySegmentData(CXmlElement *Element, TrajectoryDataType &DataType, double &Data);


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
		if (strcmp(Attributes->Dictionary[i].key, "type") == 0)
		{
			if (strcmp(Attributes->Dictionary[i].value, "GPS") == 0)
				Type = 0;
			else if (strcmp(Attributes->Dictionary[i].value, "GLONASS") == 0)
				Type = 1;
			else if (strcmp(Attributes->Dictionary[i].value, "UTC") == 0)
				Type = 4;
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

	i = 0;
	while ((SubElement = Element->GetElement(i ++)) != NULL)
	{
		if (strcmp(SubElement->GetTag(), "Longitude") == 0)
			StartPos.lon = DEG2RAD(atof(SubElement->GetText()));
		else if (strcmp(SubElement->GetTag(), "Latitude") == 0)
			StartPos.lat = DEG2RAD(atof(SubElement->GetText()));
		else if (strcmp(SubElement->GetTag(), "Altitude") == 0)
			StartPos.alt = atof(SubElement->GetText());
	}

	return TRUE;
}

BOOL AssignStartVelocity(CXmlElement *Element, LOCAL_SPEED &StartVel)
{
	int i, Type = 0;	// 0 for SCU, 1 for ENU
	CXmlElement *SubElement;
	CSimpleDict *Attributes = Element->GetAttributes();

	for (i = 0; i < Attributes->DictItemNumber; i ++)
	{
		if (strcmp(Attributes->Dictionary[i].key, "type") == 0)
		{
			if (strcmp(Attributes->Dictionary[i].value, "SCU") == 0)
				Type = 0;
			else if (strcmp(Attributes->Dictionary[i].value, "ENU") == 0)
				Type = 1;
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
		CTrajectorySegment::SpeedCourseToEnu(StartVel);
	else
		CTrajectorySegment::SpeedEnuToCourse(StartVel);

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
