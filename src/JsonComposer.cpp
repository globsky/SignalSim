//----------------------------------------------------------------------
// JsonComposer.cpp:
//   Implementation of functions to compose JSON object tree
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "JsonComposer.h"

#ifdef _MSC_VER
    #include <intrin.h>
    #pragma intrinsic(__popcnt64)
    #pragma intrinsic(_BitScanForward64)
    #define popcnt64(x) __popcnt64(x)
    static inline int ctzll(unsigned long long x) {
        unsigned long index;
        if (_BitScanForward64(&index, x)) {
            return (int)index;
        }
        return 64;  // or undefined, depending on your needs
    }
#elif defined(__GNUC__) || defined(__clang__)
    #define popcnt64(x) __builtin_popcountll(x)
    #define ctzll(x) __builtin_ctzll(x)
#else
    static inline int popcnt64(unsigned long long x) {
        int count = 0;
		for (int i = 0; i < 64; i ++) {
			if (x & 1)
				count ++;
            x >>= 1;
        }
        return count;
    }
    static inline int ctzll(unsigned long long x) {
        if (x == 0) return 64;
        int count = 0;
        while ((x & 1) == 0) {
            count++;
            x >>= 1;
        }
        return count;
    }
#endif

static const char *DictionaryListSystem[] = {
//    0      1      2        3          4
	"UTC", "GPS", "BDS", "Galileo", "GLONASS",
};
static const char *KeyDictionaryListTime[] = {
//     0      1        2          3         4      5        6       7        8
	"type", "week", "second", "leapYear", "day", "year", "month", "hour", "minute", 
};
static const char *KeyDictionaryListTrajectory[] = {
//     0           1                2                3           4        5          6           7           8        9    10   11
	"name", "initPosition", "initVelocity", "trajectoryList", "type", "format", "longitude", "latitude", "altitude", "x", "y", "z",
//     12             13        14        15       16      17      18
	"speedUnit", "angleUnit", "speed", "course", "east", "north", "up",
};
static const char *DictionaryListCoordinate[] = {
//    0      1       2      3     4     5     6      7        8       9      10     11      12
	"LLA", "ECEF", "SCU", "ENU", "d", "dm", "dms", "rad", "degree", "mps", "kph", "knot", "mph",
};
static const char *DictionaryListTrajectoryType[] = {
//     0          1            2           3          4
	"Const", "ConstAcc", "VerticalAcc", "Jerk", "HorizontalTurn",
};
static const char *KeyDictionaryListTrajectoryList[] = {
//     0         1             2           3        4       5        6
	"type", "duration", "acceleration", "speed", "rate", "angle", "radius",
};
static const char *DictionaryListSignal[] = {
//    0      1      2      3      4      5     6   7
	"L1CA","L1C", "L2C", "L2P", "L5",  "",    "", "",
	"B1C", "B1I", "B2I", "B3I", "B2a", "B2b", "", "",
	"E1",  "E5a", "E5b", "E5",  "E6",  "",    "", "",
	"G1",  "G2",  "G3",  "",    "",    "",    "", "",
};
static const char* DictionaryListPowerUnit[] = {
	//     0      1      2
		"dBHz", "dBm", "dBW",
};
#if 0
static const char *KeyDictionaryListParam[] = {
//       0            1             2           3         4         5        6      7
	"initTime", "trajectory", "ephemeris", "almanac", "output", "power", "delay", "time",
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
//       0             1              2                 3           4       5         6        7         8            9      10
	"noiseFloor", "initPower", "elevationAdjust", "signalPower", "unit", "value", "system", "svid", "powerValue", "epoch", "time",
};
static const char *DictionaryListOutputType[] = {
//      0             1            2          3
	"position", "observation", "IFdata", "baseband",
};
static const char *DictionaryListOutputFormat[] = {
//     0      1       2      3       4       5      6      7      8	
	"ECEF", "LLA", "NMEA", "KML", "RINEX", "IQ8", "IQ4", "IQ2", "IQ16",
};
#endif

static JsonObject *CreateObjects(const char *Key, int ContentNumber);
static int GetTrajSegmentTypeIndex(TrajectoryType Type);
static int GetTrajDataTypeIndex(TrajectoryDataType Type);

// create an object tree with number of ContentNumber object nodes
// if not all objects created, return NULL
JsonObject *CreateObjects(const char *Key, int ContentNumber)
{
	JsonObject *Root = new JsonObject, *CurObject = NULL, *NewObject;
	int i;

	if (Root == NULL)
		return NULL;

	// assign Root values
	Root->Type = JsonObject::ValueTypeObject;
	strcpy(Root->Key, Key);
	// generate subitem objects
	for (i = 0; i < ContentNumber; i ++)
	{
		NewObject = new JsonObject(Root);
		if (!NewObject)
			break;
		if (i == 0)
			Root->pObjectContent = NewObject;
		else
			CurObject->pNextObject = NewObject;
		CurObject = NewObject;
	}
	if (i != ContentNumber)	// not all subitem objects generated
	{
		JsonObject::DeleteTree(Root);
		return NULL;
	}
	return Root;
}

int GetTrajSegmentTypeIndex(TrajectoryType Type)
{
	switch (Type)
	{
	case TrajTypeConstSpeed: return 0;
	case TrajTypeConstAcc: return 1;
	case TrajTypeVerticalAcc: return 2;
	case TrajTypeJerk: return 3;
	case TrajTypeHorizontalCircular: return 4;
	default: return -1;
	}
}

int GetTrajDataTypeIndex(TrajectoryDataType Type)
{
	switch (Type)
	{
	case TrajDataTimeSpan: return 1;
	case TrajDataAcceleration: return 2;
	case TrajDataSpeed: return 3;
	case TrajDataAccRate: return 4;
	case TrajDataAngle: return 5;
	case TrajDataAngularRate: return 4;
	case TrajDataRadius: return 6;
	default: return -1;
	}
}

JsonObject *AssignValue(JsonObject *Object, const char *Key)
{
	Object->Type = JsonObject::ValueTypeNull;
	strcpy(Object->Key, Key);
	return Object->pNextObject;
}

JsonObject *AssignValue(JsonObject *Object, const char *Key, const char *Value)
{
	Object->Type = JsonObject::ValueTypeString;
	strcpy(Object->Key, Key);
	strcpy(Object->String, Value);
	return Object->pNextObject;
}

JsonObject *AssignValue(JsonObject *Object, const char *Key, int Value)
{
	Object->Type = JsonObject::ValueTypeIntNumber;
	strcpy(Object->Key, Key);
	Object->Number.l_data = Value;
	return Object->pNextObject;
}

JsonObject *AssignValue(JsonObject *Object, const char *Key, double Value)
{
	Object->Type = JsonObject::ValueTypeFloatNumber;
	strcpy(Object->Key, Key);
	Object->Number.d_data = Value;
	return Object->pNextObject;
}

JsonObject *AssignValue(JsonObject *Object, const char *Key, bool Value)
{
	Object->Type = Value ? JsonObject::ValueTypeTrue : JsonObject::ValueTypeFalse;
	strcpy(Object->Key, Key);
	return Object->pNextObject;
}

// if it is a number close to integer, assign as an integer value
JsonObject *AssignNumber(JsonObject *Object, const char *Key, double Value)
{
	if (fabs(Value - (int)Value) < 1e-6)	// integer value
		return AssignValue(Object, Key, (int)Value);
	else
		return AssignValue(Object, Key, Value);
}

double LonLatToRad(double Value, int Format)
{
	int Degree, Minute, Second;
	int Sign = (Value >= 0) ? 0 : 1;
	double AbsValue = fabs(Value);

	Degree = (int)(AbsValue);
	switch (Format)
	{
	case 0:	// "d"
		return DEG2RAD(Value);
	case 1:	// "dm"
		AbsValue -= Degree;
		Minute = (Degree % 100);
		Degree /= 100;
		AbsValue += Minute;
		AbsValue = Degree + AbsValue / 60.;
		Value = Sign ? -AbsValue : AbsValue;
		return DEG2RAD(Value);
	case 2:	// "dms"
		AbsValue -= Degree;
		Second = (Degree % 100);
		Minute = ((Degree / 100) % 100);
		Degree /= 10000;
		AbsValue += Second;
		AbsValue = Degree + Minute / 60. + AbsValue / 3600.;
		Value = Sign ? -AbsValue : AbsValue;
		return DEG2RAD(Value);
	default:
		return Value;
	}
}

double LonLatFromRad(double Value, int Format)
{
	int Degree, Minute;
	int Sign = (Value >= 0) ? 0 : 1;
	double AbsValue = fabs(Value);

	AbsValue = RAD2DEG(AbsValue);
	switch (Format)
	{
	case 0:	// "d"
		return RAD2DEG(Value);
	case 1:	// "dm"
		Degree = (int)AbsValue;
		AbsValue = (AbsValue - Degree) * 60 + (Degree * 100);
		Value = Sign ? -AbsValue : AbsValue;
		return Value;
	case 2:	// "dms"
		Degree = (int)AbsValue;
		AbsValue = (AbsValue - Degree) * 60;
		Minute = (int)AbsValue;
		AbsValue = (AbsValue - Minute) * 60;
		AbsValue += (Degree * 10000) + (Minute * 100);
		Value = Sign ? -AbsValue : AbsValue;
		return Value;
	default:
		return Value;
	}
}

double SpeedUnitToMPS(double Speed, int Unit)
{
	switch (Unit)
	{
	case 1:	// kilometers per hour
		return Speed / 3.6;
	case 2:	// knots
		return Speed * 1852 / 3600;
	case 3:	// miles per hour
		return Speed * 1609.344 / 3600;
	default:
		return Speed;
	}
}

double SpeedUnitFromMPS(double Speed, int Unit)
{
	switch (Unit)
	{
	case 1:	// kilometers per hour
		return Speed * 3.6;
	case 2:	// knots
		return Speed * 3600 / 1852;
	case 3:	// miles per hour
		return Speed * 3600 / 1609.344;
	default:
		return Speed;
	}
}

// return a JSON object tree containing the UTC time
// Type specifies the time format:
// 0: UTC
// 1: GPS time
// 2: BDS time
// 3: Galileo time
// 4: GLONASS time
JsonObject *ComposeStartTime(int Type, const UTC_TIME &UtcTime)
{
	JsonObject *Root = NULL, *CurObject = NULL;
	int ObjectNumber = (Type == 0) ? 7 : (Type == 4) ? 4 : 3;
	double Second;

	if (Type < 0 || Type > 4)
		return Root;
	Root = CreateObjects("initTime", ObjectNumber);
	if (Root == NULL)
		return Root;

	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, KeyDictionaryListTime[0], DictionaryListSystem[Type]);	// type
	if (Type == 0)	// UTC time
	{
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[5], UtcTime.Year);	// year
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[6], UtcTime.Month);	// month
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[4], UtcTime.Day);	// day
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[7], UtcTime.Hour);	// hour
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[8], UtcTime.Minute);	// minute
		Second = UtcTime.Second;
		AssignNumber(CurObject, KeyDictionaryListTime[2], Second);	// second
	}
	else if (Type == 4)	// GLONASS time
	{
		GLONASS_TIME GlonassTime = UtcToGlonassTime(UtcTime);
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[3], GlonassTime.LeapYear);	// leapYear
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[4], GlonassTime.Day);	// day
		Second = (GlonassTime.MilliSeconds + GlonassTime.SubMilliSeconds) / 1000.;
		AssignNumber(CurObject, KeyDictionaryListTime[2], Second);	// second
	}
	else
	{
		GNSS_TIME GnssTime;
		if (Type == 1)
			GnssTime = UtcToGpsTime(UtcTime);
		else if (Type == 2)
			GnssTime = UtcToBdsTime(UtcTime);
		else // if (Type == 3)
			GnssTime = UtcToGalileoTime(UtcTime);
		CurObject = AssignValue(CurObject, KeyDictionaryListTime[1], GnssTime.Week);	// week
		Second = (GnssTime.MilliSeconds + GnssTime.SubMilliSeconds) / 1000.;
		AssignNumber(CurObject, KeyDictionaryListTime[2], Second);	// second
	}

	return Root;
}

// return a JSON object tree containing the initial position
JsonObject *ComposeInitPosition(KINEMATIC_INFO InitPosition)
{
	JsonObject *Root = NULL, *CurObject = NULL;

	Root = CreateObjects(KeyDictionaryListTrajectory[1], 4);
	if (Root == NULL)
		return Root;

	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[4], DictionaryListCoordinate[1]);	// type
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[ 9], InitPosition.x);	// x
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[10], InitPosition.y);	// y
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[11], InitPosition.z);	// z

	return Root;
}

// return a JSON object tree containing the initial position
// Format specifies the lon/lat format:
// 0: d
// 1: dm
// 2: dms
// 3: rad
// Convert is 1 means value need to convert from rad to current format
JsonObject *ComposeInitPosition(LLA_POSITION InitPosition, int Format, int Convert)
{
	JsonObject *Root = NULL, *CurObject = NULL;

	Root = CreateObjects(KeyDictionaryListTrajectory[1], 5);
	if (Root == NULL)
		return Root;

	if (Convert)
	{
		InitPosition.lon = LonLatFromRad(InitPosition.lon, Format);
		InitPosition.lat = LonLatFromRad(InitPosition.lat, Format);
	}
	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[4], DictionaryListCoordinate[1]);	// type
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[4], DictionaryListCoordinate[1]);	// type
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[6], InitPosition.lon);	// longitude
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[7], InitPosition.lat);	// latitude
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[8], InitPosition.alt);	// altitude

	return Root;
}

// return a JSON object tree containing the initial velocity
// SpeedUnit specifies the unit of speed:
// 0: m/s
// 1: km/h
// 2: knot
// 3: miles/h
// Convert is 1 means value need to convert from m/s to current format
JsonObject *ComposeInitVelocity(KINEMATIC_INFO InitVelocity, int SpeedUnit, int Convert)
{
	JsonObject *Root = NULL, *CurObject = NULL;

	Root = CreateObjects(KeyDictionaryListTrajectory[2], 5);
	if (Root == NULL)
		return Root;

	if (Convert)
	{
		InitVelocity.vx = SpeedUnitFromMPS(InitVelocity.vx, SpeedUnit);
		InitVelocity.vy = SpeedUnitFromMPS(InitVelocity.vy, SpeedUnit);
		InitVelocity.vz = SpeedUnitFromMPS(InitVelocity.vz, SpeedUnit);
	}
	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[4], DictionaryListCoordinate[1]);	// type
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[12], DictionaryListCoordinate[9 + SpeedUnit]);	// speedUnit
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[ 9], InitVelocity.vx);	// x
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[10], InitVelocity.vy);	// y
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[11], InitVelocity.vz);	// z

	return Root;
}

// return a JSON object tree containing the initial velocity
// Type specifies the type of velocity
// 0: ENU
// 2: SCU
// SpeedUnit specifies the unit of speed:
// 0: m/s
// 1: km/h
// 2: knot
// 3: miles/h
// CourseUnit specifies the unit of speed:
// 0: degree
// other: rad
// Convert is 1 means value need to convert from m/s and rad to current format
JsonObject *ComposeInitVelocity(LOCAL_SPEED InitVelocity, int Type, int SpeedUnit, int CourseUnit, int Convert)
{
	JsonObject *Root = NULL, *CurObject = NULL;
	int ObjectNumber = (Type == 0) ? 5 : 6;

	Root = CreateObjects(KeyDictionaryListTrajectory[2], ObjectNumber);
	if (Root == NULL)
		return Root;

	if (Convert)
	{
		InitVelocity.speed = SpeedUnitFromMPS(InitVelocity.speed, SpeedUnit);
		InitVelocity.ve = SpeedUnitFromMPS(InitVelocity.ve, SpeedUnit);
		InitVelocity.vn = SpeedUnitFromMPS(InitVelocity.vn, SpeedUnit);
		InitVelocity.vu = SpeedUnitFromMPS(InitVelocity.vu, SpeedUnit);
		if (CourseUnit)
			InitVelocity.course = RAD2DEG(InitVelocity.course);
	}
	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[4], Type ? DictionaryListCoordinate[2] : DictionaryListCoordinate[3]);	// type
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[12], DictionaryListCoordinate[9 + SpeedUnit]);	// speedUnit
	if (Type == 0)	// ENU
	{
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[16], InitVelocity.ve);	// east
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[17], InitVelocity.vn);	// north
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[18], InitVelocity.vu);	// up
	}
	else	// SCU
	{
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[13], CourseUnit ? DictionaryListCoordinate[7] : DictionaryListCoordinate[8]);	// angleUnit
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[14], InitVelocity.speed);	// speed
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[15], InitVelocity.course);	// course
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectory[18], InitVelocity.vu);	// up
	}

	return Root;
}

// return a JSON object tree containing the trajectory segment
JsonObject *ComposeTrajectorySegment(TrajectoryType TrajType, TrajectoryDataType TrajDataType1, double TrajData1, TrajectoryDataType TrajDataType2, double TrajData2)
{
	JsonObject *Root = NULL, *CurObject = NULL;
	int SegmentTypeIndex = GetTrajSegmentTypeIndex(TrajType);
	int DataTypeIndex1 = GetTrajDataTypeIndex(TrajDataType1);
	int DataTypeIndex2 = (TrajType == TrajTypeConstSpeed) ? 0 : GetTrajDataTypeIndex(TrajDataType2);

	if (SegmentTypeIndex < 0 || DataTypeIndex1 < 0 || DataTypeIndex2 < 0)
		return NULL;

	Root = CreateObjects("", TrajType == TrajTypeConstSpeed ? 2 : 3);
	if (Root == NULL)
		return Root;
	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectoryList[0], DictionaryListTrajectoryType[SegmentTypeIndex]);	// type
	CurObject = AssignValue(CurObject, KeyDictionaryListTrajectoryList[DataTypeIndex1], TrajData1);
	if (TrajType != TrajTypeConstSpeed)
		CurObject = AssignValue(CurObject, KeyDictionaryListTrajectoryList[DataTypeIndex2], TrajData2);

	return Root;
}

JsonObject *ComposeSignalSelect(GnssSystem System, int SignalIndex, bool Enable)
{
	JsonObject *Root = NULL, *CurObject = NULL;

	Root = CreateObjects(KeyDictionaryListTrajectory[2], 3);
	if (Root == NULL)
		return Root;

	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, "system", DictionaryListSystem[System - GpsSystem + 1]);	// system
	CurObject = AssignValue(CurObject, "signal", DictionaryListSignal[System * 8 + SignalIndex]);	// signal
	CurObject = AssignValue(CurObject, "enable", Enable);	// signal

	return Root;
}

JsonObject *ComposeMaskOut(GnssSystem System, unsigned long long MaskOut)
{
	JsonObject *Root = NULL, *SvListObject = NULL, *CurObject = NULL;
	int SvNumber;
	int MaxSvid[] = { 32, 63, 36, 24 };

	MaskOut &= (((1ULL << MaxSvid[System]) - 1) << 1);
	SvNumber = (int)popcnt64(MaskOut);
	if (SvNumber == 0)
		return NULL;
	Root = CreateObjects("", (SvNumber == 1) ? 2 : 1);
	if (Root == NULL)
		return Root;

	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, "system", DictionaryListSystem[System - GpsSystem + 1]);	// system
	if (SvNumber == 1)
		CurObject = AssignValue(CurObject, "svid", ctzll(MaskOut));	// signal
	else	// add a list
	{
		SvListObject = CreateObjects("svid", SvNumber);
		if (SvListObject == NULL)
		{
			JsonObject::DeleteTree(Root);
			return NULL;
		}
		SvListObject->Type = JsonObject::ValueTypeArray;
		Root->AddObject(SvListObject, "system");
		CurObject = SvListObject->GetFirstObject();
		for (int i = 0; i < SvNumber; i ++)
		{
			int Pos = ctzll(MaskOut);
			CurObject = AssignValue(CurObject, "", Pos);
			MaskOut &= ~(1ULL << Pos);
		}
	}

	return Root;
}

JsonObject* ComposePowerValueItem(double Epoch, double Value, int Unit)
{
	JsonObject *Root = NULL, *CurObject = NULL;

	Root = CreateObjects(KeyDictionaryListTrajectory[2], 3);
	if (Root == NULL)
		return Root;

	CurObject = Root->GetFirstObject();
	CurObject = AssignValue(CurObject, "epoch", Epoch);	// epoch
	CurObject = AssignValue(CurObject, "unit", DictionaryListPowerUnit[Unit]);	// unit
	CurObject = AssignValue(CurObject, "value", Value);	// value

	return Root;
}

JsonObject* ComposeSvList(GnssSystem System, unsigned long long MaskOut)
{
	JsonObject* Root = NULL, * CurObject = NULL;
	int SvNumber;
	int MaxSvid[] = { 32, 63, 36, 24 };

	MaskOut &= (((1ULL << MaxSvid[System]) - 1) << 1);
	SvNumber = (int)popcnt64(MaskOut);
	if (SvNumber == 0)
		return NULL;
	else if (SvNumber == 1)
	{
		Root = new JsonObject;
		if (Root)
			AssignValue(Root, "svid", ctzll(MaskOut));
	}
	else
	{
		Root = CreateObjects("svid", SvNumber);
		if (Root == NULL)
			return NULL;
		Root->Type = JsonObject::ValueTypeArray;
		CurObject = Root->GetFirstObject();
		for (int i = 0; i < SvNumber; i++)
		{
			int Pos = ctzll(MaskOut);
			CurObject = AssignValue(CurObject, "", Pos);
			MaskOut &= ~(1ULL << Pos);
		}
	}

	return Root;
}
