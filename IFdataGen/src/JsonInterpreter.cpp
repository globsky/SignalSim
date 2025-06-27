//----------------------------------------------------------------------
// JsonInterpreter.cpp:
//   Implementation of functions to interprete JSON object tree
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "JsonInterpreter.h"

static const char *KeyDictionaryListParam[] = {
//    0          1             2           3         4         5        6
	"time", "trajectory", "ephemeris", "almanac", "output", "power", "delay",
};
static const char *KeyDictionaryListTime[] = {
//     0      1        2          3         4      5        6       7        8
	"type", "week", "second", "leapYesr", "day", "year", "month", "hour", "minute", 
};
static const char *KeyDictionaryListTrajectory[] = {
//     0           1                2                3           4        5          6           7           8        9    10   11
	"name", "initPosition", "initVelocity", "trajectoryList", "type", "format", "longitude", "latitude", "altitude", "x", "y", "z",
//     12             13        14        15       16      17      18
	"speedUnit", "angleUnit", "speed", "course", "east", "north", "up",
};
static const char *KeyDictionaryListEphAlm[] = {
//     0       1
	"type", "name",
};
static const char *KeyDictionaryListOutput[] = {
//     0        1        2         3          4            5               6             7          8        9       10        11          12            13
	"type", "format", "name", "interval", "config", "systemSelect", "elevationMask", "maskOut", "system", "svid", "signal", "enable", "sampleFreq", "centerFreq",
};
static const char *KeyDictionaryListPower[] = {
//       0             1              2                 3           4       5         6        7         8           9
	"noiseFloor", "initPower", "elevationAdjust", "signalPower", "unit", "value", "system", "svid", "powerValue", "time",
};
static const char *DictionaryListSystem[] = {
//    0      1      2        3          4
	"UTC", "GPS", "BDS", "Galileo", "GLONASS",
};
static const char *DictionaryListCoordinate[] = {
//    0      1       2      3     4     5     6      7        8       9      10     11      12
	"LLA", "ECEF", "SCU", "ENU", "d", "dm", "dms", "rad", "degree", "mps", "kph", "knot", "mph",
};
static const char *KeyDictionaryListTrajectoryList[] = {
//     0       1           2           3        4       5        6        7
	"type", "time", "acceleration", "speed", "rate", "angle", "rate", "radius",	// "rate" at index 6 reserved for angle rate
};
static const char *DictionaryListTrajectoryType[] = {
//     0          1            2           3          4
	"Const", "ConstAcc", "VerticalAcc", "Jerk", "HorizontalTurn",
};
static const char *DictionaryListOutputType[] = {
//      0             1            2          3
	"position", "observation", "IFdata", "baseband",
};
static const char *DictionaryListOutputFormat[] = {
//     0      1       2      3       4       5      6
	"ECEF", "LLA", "NMEA", "KML", "RINEX", "IQ8", "IQ4",
};
static const char *DictionaryListSignal[] = {
//    0      1      2      3      4      5     6   7
	"L1CA","L1C", "L2C", "L2P", "L5",  "",    "", "",
	"B1C", "B1I", "B2I", "B3I", "B2a", "B2b", "", "",
	"E1",  "E5a", "E5b", "E5",  "E6",  "",    "", "",
	"G1",  "G2",  "G3",  "",    "",    "",    "", "",
};
static const char *DictionaryListPowerUnit[] = {
//     0      1      2
	"dBHz", "dBm", "dBW",
};

#define PARAMETER(Dictionary) Dictionary,sizeof(Dictionary)/sizeof(char *)
#define GET_DOUBLE_VALUE(Object) ((Object->Type == JsonObject::ValueTypeIntNumber) ? (double)(Object->Number.l_data) : Object->Number.d_data)

static int SearchDictionary(const char *Word, const char *DictionaryList[], int Length);
static BOOL AssignStartTime(JsonObject *Object, UTC_TIME &UtcTime);
static BOOL SetTrajectory(JsonObject *Object, LLA_POSITION &StartPos, LOCAL_SPEED &StartVel, CTrajectory &Trajectory);
static BOOL SetEphemeris(JsonObject *Object, CNavData &NavData);
static BOOL SetEphemerisFile(JsonObject *Object, CNavData &NavData);
static BOOL SetAlmanac(JsonObject *Object, CNavData &NavData);
static BOOL SetAlmanacFile(JsonObject *Object, CNavData &NavData);
static BOOL SetOutputParam(JsonObject *Object, OUTPUT_PARAM &OutputParam);
static BOOL SetPowerControl(JsonObject *Object, CPowerControl &PowerControl);
static BOOL SetDelayConfig(JsonObject *Object, DELAY_CONFIG &DelayConfig);
static BOOL AssignStartPosition(JsonObject *Object, LLA_POSITION &StartPos);
static int AssignStartVelocity(JsonObject *Object, LOCAL_SPEED &StartVel, KINEMATIC_INFO &Velotity);
static BOOL AssignTrajectoryList(JsonObject *Object, CTrajectory &Trajectory);
static TrajectoryType GetTrajectorySegment(JsonObject *Object, TrajectoryDataType &DataType1, double &Data1, TrajectoryDataType &DataType2, double &Data2);
static BOOL ProcessConfigParam(JsonObject *Object, OUTPUT_PARAM &OutputParam);
static BOOL ProcessMaskOut(JsonObject *Object, OUTPUT_PARAM &OutputParam);
static BOOL MaskOutSatellite(int system, int svid, OUTPUT_PARAM &OutputParam);
static BOOL ProcessSystemSelect(JsonObject *Object, OUTPUT_PARAM &OutputParam);
static BOOL ProcessSignalPower(JsonObject *Object, CPowerControl &PowerControl);
static BOOL ProcessPowerValue(JsonObject *Object, int system, int *svlist, int sv_number, CPowerControl &PowerControl);
static double FormatLonLat(double Value, int Format);
static double FormatSpeed(double Value, int Format);

BOOL AssignParameters(JsonObject *Object, PUTC_TIME UtcTime, PLLA_POSITION StartPos, PLOCAL_SPEED StartVel, CTrajectory *Trajectory, CNavData *NavData, POUTPUT_PARAM OutputParam, CPowerControl *PowerControl, PDELAY_CONFIG DelayConfig)
{
	Object = JsonStream::GetFirstObject(Object);
	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListParam)))
		{
		case 0:	// "time"
			if (UtcTime) AssignStartTime(JsonStream::GetFirstObject(Object), *UtcTime); break;
		case 1:	// "trajectory"
			if (StartPos && StartVel && Trajectory)
				SetTrajectory(JsonStream::GetFirstObject(Object), *StartPos, *StartVel, *Trajectory);
			break;
		case 2: // "ephemeris"
			if (NavData)
				SetEphemeris(Object, *NavData);
			break;
		case 3: // "almanac"
			if (NavData)
				SetAlmanac(Object, *NavData);
			break;
		case 4:	// "output"
			if (OutputParam) SetOutputParam(JsonStream::GetFirstObject(Object), *OutputParam); break;
		case 5:	// "power"
			if (PowerControl) SetPowerControl(JsonStream::GetFirstObject(Object), *PowerControl); break;
		case 6:	// "delay"
			if (DelayConfig) SetDelayConfig(JsonStream::GetFirstObject(Object), *DelayConfig); break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

BOOL AssignStartTime(JsonObject *Object, UTC_TIME &UtcTime)
{
	int Type = 1;	// 4 for UTC, 1 for GPS, 2 for BDS, 3 for Galileo, 4 for GLONASS
	int Week = 0, LeapYear = 0;
	GNSS_TIME GnssTime;
	GLONASS_TIME GlonassTime;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListTime)))
		{
		case 0:	// "type"
			if (Object->Type == JsonObject::ValueTypeString)
				Type = SearchDictionary(Object->String, PARAMETER(DictionaryListSystem));
			break;
		case 1:	// "week"
			Week = (int)(Object->Number.l_data); break;
		case 2:	// "second"
			UtcTime.Second = GET_DOUBLE_VALUE(Object); break;
		case 3:	// "leapYesr"
			LeapYear = (int)(Object->Number.l_data); break;
		case 4:	// "day"
			UtcTime.Day = (int)(Object->Number.l_data); break;
		case 5:	// "year"
			UtcTime.Year = (int)(Object->Number.l_data); break;
		case 6:	// "month"
			UtcTime.Month = (int)(Object->Number.l_data); break;
		case 7:	// "hour"
			UtcTime.Hour = (int)(Object->Number.l_data); break;
		case 8:	// "minute"
			UtcTime.Minute = (int)(Object->Number.l_data); break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	switch (Type)
	{
	case 1:	// GPS Time
	case 3:	// Galileo Time
		GnssTime.Week = Week;
		GnssTime.SubMilliSeconds = UtcTime.Second * 1000;
		GnssTime.MilliSeconds = (int)GnssTime.SubMilliSeconds;
		GnssTime.SubMilliSeconds -= GnssTime.MilliSeconds;
		UtcTime = GpsTimeToUtc(GnssTime);
		break;
	case 2:	// BDS Time
		GnssTime.Week = Week;
		GnssTime.SubMilliSeconds = UtcTime.Second * 1000;
		GnssTime.MilliSeconds = (int)GnssTime.SubMilliSeconds;
		GnssTime.SubMilliSeconds -= GnssTime.MilliSeconds;
		UtcTime = BdsTimeToUtc(GnssTime);
		break;
	case 4:	// GLONASS Time
		GlonassTime.LeapYear = LeapYear;
		GlonassTime.Day = UtcTime.Day;
		GlonassTime.SubMilliSeconds = UtcTime.Second * 1000;
		GlonassTime.MilliSeconds = (int)GlonassTime.SubMilliSeconds;
		GlonassTime.SubMilliSeconds -= GlonassTime.MilliSeconds;
		UtcTime = GlonassTimeToUtc(GlonassTime);
		break;
	}

	return TRUE;
}

BOOL SetTrajectory(JsonObject *Object, LLA_POSITION &StartPos, LOCAL_SPEED &StartVel, CTrajectory &Trajectory)
{
	int Content = 0, VelocityType;
	CONVERT_MATRIX ConvertMatrix;
	KINEMATIC_INFO Position, Velocity;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListTrajectory)))
		{
		case 0:	// "name"
			Trajectory.SetTrajectoryName(Object->String); break;
		case 1:	// "initPosition"
			if (AssignStartPosition(JsonStream::GetFirstObject(Object), StartPos)) Content |= 1; break;
		case 2:	// "initVelocity"
			if (VelocityType = AssignStartVelocity(JsonStream::GetFirstObject(Object), StartVel, Velocity)) Content |= 2; break;
		case 3:	// "trajectoryList"
			if ((Content & 3) != 3)
				return FALSE;
			if (VelocityType == 1)	// velocity in ECEF format and stored in Velotity
			{
				ConvertMatrix = CalcConvMatrix(StartPos);
				Position = LlaToEcef(StartPos);
				Position.vx = Velocity.vx; Position.vy = Velocity.vy; Position.vz = Velocity.vz;
				SpeedEcefToLocal(ConvertMatrix, Position, StartVel);
			}
			Trajectory.SetInitPosVel(StartPos, StartVel, FALSE);
			AssignTrajectoryList(JsonStream::GetFirstObject(Object), Trajectory);
			break;
		}
		Object = JsonStream::GetNextObject(Object);
	}
	return TRUE;
}

BOOL SetEphemeris(JsonObject *Object, CNavData &NavData)
{
	JsonObject *ObjectArray;

	if (Object->Type == JsonObject::ValueTypeObject)
		SetEphemerisFile(JsonStream::GetFirstObject(Object), NavData);
	else if (Object->Type == JsonObject::ValueTypeArray)
	{
		ObjectArray = JsonStream::GetFirstObject(Object);
		while (ObjectArray)
		{
			if (ObjectArray->Type == JsonObject::ValueTypeObject)
				SetEphemerisFile(JsonStream::GetFirstObject(ObjectArray), NavData);
			ObjectArray = JsonStream::GetNextObject(ObjectArray);
		}
	}
	return TRUE;
}

BOOL SetEphemerisFile(JsonObject *Object, CNavData &NavData)
{
	while (Object)
	{
		if (strcmp(Object->Key, "name") == 0)
			NavData.ReadNavFile(Object->String);
		Object = JsonStream::GetNextObject(Object);
	}
	return TRUE;
}

BOOL SetAlmanac(JsonObject *Object, CNavData &NavData)
{
	JsonObject *ObjectArray;

	if (Object->Type == JsonObject::ValueTypeObject)
		SetAlmanacFile(JsonStream::GetFirstObject(Object), NavData);
	else if (Object->Type == JsonObject::ValueTypeArray)
	{
		ObjectArray = JsonStream::GetFirstObject(Object);
		while (ObjectArray)
		{
			if (ObjectArray->Type == JsonObject::ValueTypeObject)
				SetAlmanacFile(JsonStream::GetFirstObject(ObjectArray), NavData);
			ObjectArray = JsonStream::GetNextObject(ObjectArray);
		}
	}
	return TRUE;
}

BOOL SetAlmanacFile(JsonObject *Object, CNavData &NavData)
{
	while (Object)
	{
		if (strcmp(Object->Key, "name") == 0)
			NavData.ReadAlmFile(Object->String);
		Object = JsonStream::GetNextObject(Object);
	}
	return TRUE;
}

BOOL SetOutputParam(JsonObject *Object, OUTPUT_PARAM &OutputParam)
{
	JsonObject *SystemSelectObject;

	// set default value
	OutputParam.filename[0] = 0;
	OutputParam.GpsMaskOut = OutputParam.GlonassMaskOut = 0;
	OutputParam.BdsMaskOut = OutputParam.GalileoMaskOut = 0LL;
	OutputParam.ElevationMask = DEG2RAD(5);
	OutputParam.Interval = 1000;
	// default output GPS L1 only
	OutputParam.FreqSelect[0] = 0x1;
	OutputParam.FreqSelect[1] = OutputParam.FreqSelect[2] = OutputParam.FreqSelect[3] = 0;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListOutput)))
		{
		case 0:	// "type"
			if (Object->Type == JsonObject::ValueTypeString)
				OutputParam.Type = (OutputType)SearchDictionary(Object->String, PARAMETER(DictionaryListOutputType));
			break;
		case 1:	// "format"
			if (Object->Type == JsonObject::ValueTypeString)
				OutputParam.Format = (OutputFormat)SearchDictionary(Object->String, PARAMETER(DictionaryListOutputFormat));
			break;
		case 2:	// "name"
			if (Object->Type == JsonObject::ValueTypeString)
				strncpy(OutputParam.filename, Object->String, 255);
			break;
		case 3:	// "interval"
			OutputParam.Interval = (int)(GET_DOUBLE_VALUE(Object) * 1000); break;
		case 4:	// "config"
			ProcessConfigParam(JsonStream::GetFirstObject(Object), OutputParam); break;
		case 5:	// "systemSelect"
			if (Object->Type == JsonObject::ValueTypeArray)
			{
				SystemSelectObject = JsonStream::GetFirstObject(Object);
				while (SystemSelectObject)
				{
					ProcessSystemSelect(JsonStream::GetFirstObject(SystemSelectObject), OutputParam);
					SystemSelectObject = JsonStream::GetNextObject(SystemSelectObject);
				}
			}
			break;
		case 12:	// "sampleFreq"
			OutputParam.SampleFreq = (int)(GET_DOUBLE_VALUE(Object) * 1000); break;
		case 13:	// "centerFreq"
			OutputParam.CenterFreq = (int)(GET_DOUBLE_VALUE(Object) * 1000); break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

int SearchDictionary(const char *Word, const char *DictionaryList[], int Length)
{
	int i;

	for (i = 0; i < Length; i ++)
		if (strcmp(Word, DictionaryList[i]) == 0)
			return i;
	return -1;
}

BOOL AssignStartPosition(JsonObject *Object, LLA_POSITION &StartPos)
{
	int Type = 0, Format = 4;
	KINEMATIC_INFO Position;
	double Longitude, Latitude;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListTrajectory)))
		{
		case 4:	// "type"
			if (Object->Type == JsonObject::ValueTypeString)
				Type = SearchDictionary(Object->String, PARAMETER(DictionaryListCoordinate));
			break;
		case 5:	// "format"
			if (Object->Type == JsonObject::ValueTypeString)
				Format = SearchDictionary(Object->String, PARAMETER(DictionaryListCoordinate));
			break;
		case 6:	// "longitude"
			Longitude = GET_DOUBLE_VALUE(Object); break;
		case 7:	// "latitude"
			Latitude = GET_DOUBLE_VALUE(Object); break;
		case 8:	// "altitude"
			StartPos.alt = GET_DOUBLE_VALUE(Object); break;
		case 9:	// "x"
			Position.x = GET_DOUBLE_VALUE(Object); break;
		case 10:	// "y"
			Position.y = GET_DOUBLE_VALUE(Object); break;
		case 11:	// "z"
			Position.z = GET_DOUBLE_VALUE(Object); break;
		}
		Object = JsonStream::GetNextObject(Object);
	}
	StartPos.lon = FormatLonLat(Longitude, Format);
	StartPos.lat = FormatLonLat(Latitude, Format);
	if (Type)
		StartPos = EcefToLla(Position);

	return TRUE;
}

int AssignStartVelocity(JsonObject *Object, LOCAL_SPEED &StartVel, KINEMATIC_INFO &Velotity)
{
	int Type = 2, SpeedUnit = 9, AngleUnit = 8;

	StartVel.vu = 0;	// set default up speed to 0
	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListTrajectory)))
		{
		case 4:	// "type"
			if (Object->Type == JsonObject::ValueTypeString)
				Type = SearchDictionary(Object->String, PARAMETER(DictionaryListCoordinate));
			break;
		case 9:	// "x"
			Velotity.x = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		case 10:	// "y"
			Velotity.y = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		case 11:	// "z"
			Velotity.z = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		case 12:	// "speedUnit"
			SpeedUnit = SearchDictionary(Object->Key, PARAMETER(DictionaryListCoordinate));
		case 13:	// "angleUnit"
			AngleUnit = SearchDictionary(Object->Key, PARAMETER(DictionaryListCoordinate));
		case 14:	// "speed"
			StartVel.speed = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		case 15:	// "course"
			StartVel.course = GET_DOUBLE_VALUE(Object); if (AngleUnit == 8) StartVel.course = DEG2RAD(StartVel.course); break;
		case 16:	// "east"
			StartVel.ve = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		case 17:	// "north"
			StartVel.vn = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		case 18:	// "up"
			StartVel.vu = FormatSpeed(GET_DOUBLE_VALUE(Object), SpeedUnit); break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	if (Type == 2)
		SpeedCourseToEnu(StartVel);
	else if (Type == 3)
		SpeedEnuToCourse(StartVel);
	return Type;
}

BOOL AssignTrajectoryList(JsonObject *Object, CTrajectory &Trajectory)
{
	TrajectoryType Type;
	TrajectoryDataType DataType1, DataType2;
	double Data1, Data2;

	Trajectory.ClearTrajectoryList();
	while (Object)
	{
		Type = GetTrajectorySegment(JsonStream::GetFirstObject(Object), DataType1, Data1, DataType2, Data2);
		if (Type != TrajTypeUnknown)
			Trajectory.AppendTrajectory(Type, DataType1, Data1, DataType2, Data2);
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

TrajectoryType GetTrajectorySegment(JsonObject *Object, TrajectoryDataType &DataType1, double &Data1, TrajectoryDataType &DataType2, double &Data2)
{
	TrajectoryType Type = TrajTypeUnknown;
	int KeyIndex, ParameterNumber = 0;
	TrajectoryDataType DataType = TrajDataTimeSpan;
	double Data = 0;

	while (Object)
	{
		switch (KeyIndex = SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListTrajectoryList)))
		{
		case 0:	// "type"
			if (Object->Type == JsonObject::ValueTypeString)
				Type = (TrajectoryType)(SearchDictionary(Object->String, PARAMETER(DictionaryListTrajectoryType)) + 1);
			break;
		case 1:	// time"
			DataType = TrajDataTimeSpan; Data = GET_DOUBLE_VALUE(Object); break;
		case 2:	// "acceleration"
			DataType = TrajDataAcceleration; Data = GET_DOUBLE_VALUE(Object); break;
		case 3:	// "speed"
			DataType = TrajDataSpeed; Data = GET_DOUBLE_VALUE(Object); break;
		case 4:	// "rate"
			DataType = (Type == TrajTypeJerk) ? TrajDataAccRate : TrajDataAngularRate; Data = GET_DOUBLE_VALUE(Object); break;
		case 5:	// "angle"
			DataType = TrajDataAngle; Data = DEG2RAD(GET_DOUBLE_VALUE(Object)); break;
		case 7:	// "radius"
			DataType = TrajDataRadius; Data = GET_DOUBLE_VALUE(Object); break;
		}
		if (KeyIndex > 0 && KeyIndex <= 7)
		{
			if (ParameterNumber == 0)
			{
				DataType1 = DataType;
				Data1 = Data;
			}
			else if (ParameterNumber == 1)
			{
				DataType2 = DataType;
				Data2 = Data;
			}
			ParameterNumber ++;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return Type;
}

BOOL ProcessConfigParam(JsonObject *Object, OUTPUT_PARAM &OutputParam)
{
	JsonObject *MaskOutArray;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListOutput)))
		{
		case 6:	// "elevationMask"
			OutputParam.ElevationMask = DEG2RAD(GET_DOUBLE_VALUE(Object)); break;
		case 7:	// "maskOut"
			if (Object->Type == JsonObject::ValueTypeArray)
			{
				MaskOutArray = JsonStream::GetFirstObject(Object);
				while (MaskOutArray)
				{
					ProcessMaskOut(JsonStream::GetFirstObject(MaskOutArray), OutputParam);
					MaskOutArray = JsonStream::GetNextObject(MaskOutArray);
				}
			}
			break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

BOOL ProcessMaskOut(JsonObject *Object, OUTPUT_PARAM &OutputParam)
{
	int system = GpsSystem, svid = 1;
	JsonObject *MaskOutSv;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListOutput)))
		{
		case 8:	// "system"
			if (Object->Type == JsonObject::ValueTypeString)
				system = (GnssSystem)(SearchDictionary(Object->String, PARAMETER(DictionaryListSystem)) - 1);
			break;
		case 9:	// "svid"
			if (Object->Type == JsonObject::ValueTypeIntNumber)
				MaskOutSatellite(system, (int)(Object->Number.l_data), OutputParam);
			else if (Object->Type == JsonObject::ValueTypeArray)
			{
				MaskOutSv = JsonStream::GetFirstObject(Object);
				while (MaskOutSv)
				{
					if (MaskOutSv->Type == JsonObject::ValueTypeIntNumber)
						MaskOutSatellite(system, (int)(MaskOutSv->Number.l_data), OutputParam);
					MaskOutSv = JsonStream::GetNextObject(MaskOutSv);
				}
			}
			break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

BOOL MaskOutSatellite(int system, int svid, OUTPUT_PARAM &OutputParam)
{
	switch (system)
	{
	case GpsSystem:
		if (svid >= 1 && svid <= 32)
			OutputParam.GpsMaskOut |= (1 << (svid-1));
		break;
	case BdsSystem:
		if (svid >= 1 && svid <= 63)
			OutputParam.BdsMaskOut |= (1LL << (svid-1));
		break;
	case GalileoSystem:
		if (svid >= 1 && svid <= 50)
			OutputParam.GalileoMaskOut |= (1LL << (svid-1));
		break;
	case GlonassSystem:
		if (svid >= 1 && svid <= 24)
			OutputParam.GlonassMaskOut |= (1 << (svid-1));
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

BOOL ProcessSystemSelect(JsonObject *Object, OUTPUT_PARAM &OutputParam)
{
	int system = GpsSystem, signal = -1;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListOutput)))
		{
		case 8:	// "system"
			if (Object->Type == JsonObject::ValueTypeString)
				system = (GnssSystem)(SearchDictionary(Object->String, PARAMETER(DictionaryListSystem)) - 1);
			break;
		case 10:	// "signal"
			if (Object->Type == JsonObject::ValueTypeString)
				signal = SearchDictionary(Object->String, PARAMETER(DictionaryListSignal));
			break;
		case 11:	// "enable"
			if (signal >= 0 && ((signal / 8) != system))	// freq and system do not match
				system = -1;
			if (system >= 0)
			{
				if (signal < 0)	// frequency not set, set as primary signal
					signal = 0;
				else
					signal %= 8;
				if (Object->Type == JsonObject::ValueTypeTrue)
					OutputParam.FreqSelect[system] |= (1 << signal);
				else if (Object->Type == JsonObject::ValueTypeFalse)
					OutputParam.FreqSelect[system] &= ~(1 << signal);
			}
			break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

BOOL SetPowerControl(JsonObject *Object, CPowerControl &PowerControl)
{
	int unit = 0;
	JsonObject *PowerObject;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListPower)))
		{
		case 0:	// "noiseFloor"
			PowerControl.NoiseFloor = GET_DOUBLE_VALUE(Object); break;
		case 1:	// "initPower"
			PowerObject = JsonStream::GetFirstObject(Object);
			while (PowerObject)
			{
				switch (SearchDictionary(PowerObject->Key, PARAMETER(KeyDictionaryListPower)))
				{
				case 4:	// "unit"
					unit = SearchDictionary(PowerObject->String, PARAMETER(DictionaryListPowerUnit)); break;
				case 5:	// "value"
					switch (unit)
					{
					case 0: PowerControl.InitCN0 = GET_DOUBLE_VALUE(PowerObject); break;
					case 1: PowerControl.InitCN0 = GET_DOUBLE_VALUE(PowerObject) - PowerControl.NoiseFloor; break;
					case 2: PowerControl.InitCN0 = GET_DOUBLE_VALUE(PowerObject) - PowerControl.NoiseFloor + 30; break;
					}
					break;
				}
				PowerObject = JsonStream::GetNextObject(PowerObject);
			}
			break;
		case 2:	// "elevationAdjust"
			if (Object->Type == JsonObject::ValueTypeTrue)
				PowerControl.Adjust = ElevationAdjustSinSqrtFade;
			else if (Object->Type == JsonObject::ValueTypeFalse)
				PowerControl.Adjust = ElevationAdjustNone;
			break;
		case 3:	// "signalPower"
			if (Object->Type == JsonObject::ValueTypeObject)
				ProcessSignalPower(JsonStream::GetFirstObject(Object), PowerControl);
			else if (Object->Type == JsonObject::ValueTypeArray)
			{
				PowerObject = JsonStream::GetFirstObject(Object);
				while (PowerObject)
				{
					if (PowerObject->Type == JsonObject::ValueTypeObject)
						ProcessSignalPower(JsonStream::GetFirstObject(PowerObject), PowerControl);
					PowerObject = JsonStream::GetNextObject(PowerObject);
				}
			}
			break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

BOOL ProcessSignalPower(JsonObject *Object, CPowerControl &PowerControl)
{
	int system, svlist[32], sv_number;
	JsonObject *ObjectArray;

	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListPower)))
		{
		case 6:	// "system"
			if (Object->Type == JsonObject::ValueTypeString)
				system = (GnssSystem)(SearchDictionary(Object->String, PARAMETER(DictionaryListSystem)) - 1);
			sv_number = 0;
			break;
		case 7:	// "svid"
			if (Object->Type == JsonObject::ValueTypeIntNumber)
				svlist[sv_number++] = (int)(Object->Number.l_data);
			else if (Object->Type == JsonObject::ValueTypeArray)
			{
				ObjectArray = JsonStream::GetFirstObject(Object);
				while (ObjectArray)
				{
					if (ObjectArray->Type == JsonObject::ValueTypeIntNumber)
						svlist[sv_number++] = (int)(ObjectArray->Number.l_data);
					ObjectArray = JsonStream::GetNextObject(ObjectArray);
				}
			}
			break;
		case 8:	// "powerValue"
			if (Object->Type == JsonObject::ValueTypeObject)
				ProcessPowerValue(JsonStream::GetFirstObject(Object), system, svlist, sv_number, PowerControl);
			else if (Object->Type == JsonObject::ValueTypeArray)
			{
				ObjectArray = JsonStream::GetFirstObject(Object);
				while (ObjectArray)
				{
					if (ObjectArray->Type == JsonObject::ValueTypeObject)
						ProcessPowerValue(JsonStream::GetFirstObject(ObjectArray), system, svlist, sv_number, PowerControl);
					ObjectArray = JsonStream::GetNextObject(ObjectArray);
				}
			}
			break;
		}
		Object = JsonStream::GetNextObject(Object);
	}

	return TRUE;
}

BOOL ProcessPowerValue(JsonObject *Object, int system, int *svlist, int sv_number, CPowerControl &PowerControl)
{
	int i, unit = 0;
	SIGNAL_POWER SignalPower;

	SignalPower.system = system;
	SignalPower.svid = 0;
	SignalPower.time = 0;
	SignalPower.CN0 = PowerControl.InitCN0;
	while (Object)
	{
		switch (SearchDictionary(Object->Key, PARAMETER(KeyDictionaryListPower)))
		{
		case 4:	// "unit"
			unit = SearchDictionary(Object->String, PARAMETER(DictionaryListPowerUnit)); break;
		case 5:	// "value"
			switch (unit)
			{
			case 0: SignalPower.CN0 = GET_DOUBLE_VALUE(Object); break;
			case 1: SignalPower.CN0 = GET_DOUBLE_VALUE(Object) - PowerControl.NoiseFloor; break;
			case 2: SignalPower.CN0 = GET_DOUBLE_VALUE(Object) - PowerControl.NoiseFloor + 30; break;
			}
			if (unit == 0 && SignalPower.CN0 < 0)
				SignalPower.CN0 = -1.0;
			break;
		case 9:	// "time"
			SignalPower.time = (int)(GET_DOUBLE_VALUE(Object) * 1000); break;
		}
		Object = JsonStream::GetNextObject(Object);
	}
	if (sv_number == 0)	// svlist is empty means for all satellites
	{
		SignalPower.svid = 0;
		PowerControl.AddControlElement(&SignalPower);
	}
	else
	{
		for (i = 0; i < sv_number; i ++)
		{
			SignalPower.svid = svlist[i];
			PowerControl.AddControlElement(&SignalPower);
		}
	}

	return TRUE;
}

BOOL SetDelayConfig(JsonObject *Object, DELAY_CONFIG &DelayConfig)
{
	return TRUE;
}

double FormatLonLat(double Value, int Format)
{
	int Degree, Minute, Second;
	int Sign = (Value >= 0) ? 0 : 1;
	double AbsValue = fabs(Value);

	Degree = (int)(AbsValue);
	switch (Format)
	{
	case 5:	// "dm"
		AbsValue -= Degree;
		Minute = (Degree % 100);
		Degree /= 100;
		AbsValue += Minute;
		AbsValue = Degree + AbsValue / 60.;
		Value = Sign ? -Value : Value;
		return DEG2RAD(Value);
	case 6:	// "dms"
		AbsValue -= Degree;
		Second = (Degree % 100);
		Minute = ((Degree / 100) % 100);
		Degree /= 10000;
		AbsValue += Second;
		AbsValue = Degree + Minute / 60. + AbsValue / 3600.;
		Value = Sign ? -Value : Value;
		return DEG2RAD(Value);
	case 7:
		return Value;
	default:
		return DEG2RAD(Value);
	}
}

double FormatSpeed(double Value, int Format)
{
	switch (Format)
	{
	case 10:	// kilometers per hour
		return Value / 3.6;
	case 11:	// knots
		return Value * 1852 / 3600;
	case 12:	// miles per hour
		return Value * 1609.344 / 3600;
	default:
		return Value;
	}
}
