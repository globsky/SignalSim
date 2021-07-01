//----------------------------------------------------------------------
// GnssTime.cpp:
//   Convertion between GNSS time
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <math.h>

#include "ConstVal.h"
#include "BasicTypes.h"

static int DaysAcc[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

unsigned int InsertTime[] = { 46828800, 78364801, 109900802, 173059203, 252028804, 315187205,
                     346723206, 393984007, 425520008, 457056009, 504489610, 551750411,
					 599184012, 820108813, 914803214, 1025136015, 1119744016, 1167264017 };

UTC_TIME GlonassTimeToUtc(GLONASS_TIME GlonassTime);
GLONASS_TIME UtcToGlonassTime(UTC_TIME UtcTime);

static BOOL GetLeapSecond(unsigned int Seconds, int &LeapSecond)
{
	int len = sizeof(InsertTime) / sizeof(unsigned int), i;

	for (i = 0; i < len; i ++)
	{
		if (Seconds <= InsertTime[i])
		{
			LeapSecond = i;
			return (Seconds == InsertTime[i]);
		}
	}
	LeapSecond = len;
	return FALSE;
}

// This program handles the date from Jan. 1, 1984 00:00:00.00 UTC till year 2099
// date after 2020/12/31 may not have correct leap second correction
UTC_TIME GpsTimeToUtc(GNSS_TIME GnssTime, BOOL UseLeapSecond = TRUE)
{
	int Days, LeapSecond = 0;
	BOOL AtLeapSecond = UseLeapSecond;
	GLONASS_TIME GlonassTime;
	UTC_TIME time;

	// calculate total days and seconds
	// to prevent seconds less than zero after leap second adjust
	// add seconds of one week
	GlonassTime.Day = (GnssTime.Week - 1) * 7;
	GlonassTime.Seconds = GnssTime.Seconds + 604800.;
	if (UseLeapSecond)
		AtLeapSecond = GetLeapSecond((unsigned int)(GnssTime.Week * 604800 + GnssTime.Seconds), LeapSecond);
	GlonassTime.Seconds -= AtLeapSecond ? (LeapSecond + 1) : LeapSecond;
	
	Days = ((int)GlonassTime.Seconds) / 86400;
	GlonassTime.Day += Days;
	GlonassTime.Seconds -= Days * 86400;
	GlonassTime.Day -= 208 * 7;
	// calculate year
	GlonassTime.LeapYear = GlonassTime.Day / (366 + 365 * 3);
	GlonassTime.Day -= GlonassTime.LeapYear * (366 + 365 * 3);
	GlonassTime.LeapYear -= 2;
	GlonassTime.Day += 1;
	GlonassTime.Seconds += 10800.;

	time = GlonassTimeToUtc(GlonassTime);
	if (AtLeapSecond)
		time.Second += 1;

	return time;
}

GNSS_TIME UtcToGpsTime(UTC_TIME UtcTime, BOOL UseLeapSecond = TRUE)
{
	GLONASS_TIME GlonassTime;
	GNSS_TIME time;
	int TotalDays, Seconds, LeapSecond = 0;
	BOOL NextDay = (UtcTime.Hour == 0 && UtcTime.Minute == 0 && ((int)UtcTime.Second) == 0);
	BOOL AtLeapSecond;

	GlonassTime = UtcToGlonassTime(UtcTime);
	TotalDays = (GlonassTime.LeapYear + 3) * (366 + 365 * 3) + GlonassTime.Day - 6;
	time.Seconds = TotalDays * 86400 + GlonassTime.Seconds - 10800.;
	Seconds = (unsigned int)time.Seconds;
	if (UseLeapSecond)
	{
		AtLeapSecond = GetLeapSecond(Seconds, LeapSecond);
		Seconds += LeapSecond;
		AtLeapSecond = GetLeapSecond(Seconds, LeapSecond);
		time.Seconds += (AtLeapSecond && NextDay) ? (LeapSecond + 1) : LeapSecond;
	}
	time.Week = ((unsigned int)(time.Seconds)) / 604800;
	time.Seconds -= time.Week * 604800;

	return time;
}

UTC_TIME GlonassTimeToUtc(GLONASS_TIME GlonassTime)
{
	int i, Seconds, LeapDay = 0;
	UTC_TIME time;

	GlonassTime.Seconds -= 10800.;
	if (GlonassTime.Seconds < 0)
	{
		GlonassTime.Seconds += 86400.;
		GlonassTime.Day --;
	}
	time.Second = GlonassTime.Seconds;
	GlonassTime.LeapYear *= 4;
	GlonassTime.Day --;
	if (GlonassTime.Day >= (366 + 365 * 2))
	{
		GlonassTime.Day -= (366 + 365 * 2);
		GlonassTime.LeapYear += 3;
	}
	else if (GlonassTime.Day >= (366 + 365))
	{
		GlonassTime.Day -= (366 + 365);
		GlonassTime.LeapYear += 2;
	}
	else if (GlonassTime.Day >= 366)
	{
		GlonassTime.Day -= 366;
		GlonassTime.LeapYear ++;
	}
	else if (GlonassTime.Day >= 60)
		GlonassTime.Day --;
	else if (GlonassTime.Day == 59)
		LeapDay = 1;

	for (i = 1; i < 12; i ++)
	{
		if (GlonassTime.Day < DaysAcc[i])
			break;
	}
	if (LeapDay)
	{
		time.Month = 2;
		time.Day = 29;
	}
	else
	{
		time.Month = i;
		time.Day = GlonassTime.Day - (DaysAcc[i-1] - 1);
	}
	time.Year = 1992 + GlonassTime.LeapYear;
	Seconds = (int)GlonassTime.Seconds;
	time.Hour = Seconds / 3600;
	Seconds -= time.Hour * 3600;
	time.Minute = Seconds / 60;
	time.Second = GlonassTime.Seconds - time.Hour * 3600 - time.Minute * 60;

	return time;
}

GLONASS_TIME UtcToGlonassTime(UTC_TIME UtcTime)
{
	int Years, Days;
	GLONASS_TIME time;

	time.Seconds = ((UtcTime.Hour * 60) + UtcTime.Minute) * 60 + UtcTime.Second + 10800;
	Years = UtcTime.Year - 1992;
	Days = DaysAcc[UtcTime.Month - 1] + UtcTime.Day - 1;
	if ((Years % 4) != 0 || Days >= 59)
		Days ++;
	Days += (Years % 4) * 365;
	time.Day = Days + 1;
	time.LeapYear = Years / 4;

	return time;
}

GNSS_TIME UtcToBdsTime(UTC_TIME UtcTime)
{
	GNSS_TIME time = UtcToGpsTime(UtcTime);

	if (time.Seconds >= 14.0)
	{
		time.Seconds -= 14.0;
		time.Week -= 1356;
	}
	else
	{
		time.Seconds += (604800.0 - 14.0);
		time.Week -= 1357;
	}

	return time;
}

GNSS_TIME UtcToGalileoTime(UTC_TIME UtcTime)
{
	GNSS_TIME time = UtcToGpsTime(UtcTime);

	time.Week -= 1024;
	return time;
}

UTC_TIME BdsTimeToUtc(GNSS_TIME GnssTime)
{
	GnssTime.Seconds += 14.0;
	GnssTime.Week += 1356;
	return GpsTimeToUtc(GnssTime);
}

UTC_TIME GalileoTimeToUtc(GNSS_TIME GnssTime)
{
	GnssTime.Week += 1024;
	return GpsTimeToUtc(GnssTime);
}
