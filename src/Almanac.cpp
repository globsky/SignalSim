//----------------------------------------------------------------------
// Almanac.cpp:
//   Almanac file read/write/convert functions
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "BasicTypes.h"
#include "GnssTime.h"
#include "Almanac.h"

#define SQRT_A0 5440.588203494177338011974948823
#define NORMINAL_I0 0.97738438111682456307726683035362

int GetAlmanacGps(FILE *fp, PGPS_ALMANAC Alm);
int GetAlmanacBds(FILE *fp, PGPS_ALMANAC Alm);
int GetAlmanacGalileo(FILE *fp, PGPS_ALMANAC Alm, int RefWeek);
int GetAlmanacGlonass(FILE *fp, PGLONASS_ALMANAC Alm);
GPS_ALMANAC GetAlmanacFromEphemeris(PGPS_EPHEMERIS Eph);
GLONASS_ALMANAC GetAlmanacFromEphemeris(PGLONASS_EPHEMERIS Eph);

AlmanacType CheckAlmnanacType(FILE *fp)
{
	char str[256], *p = str;
	AlmanacType Type;

	fseek(fp, 0, SEEK_SET);
	fgets(str, 255, fp);

	if (str[0] == '*')	// this is a YUMA GPS almanac file
		Type = AlmanacGps;
	else if (str[0] == '<')	// this is a XML Galileo almanac file
		Type = AlmanacGalileo;
	else
	{
		// check second parameter
		while (*p != '\0' && *p != ' ' && *p != '\t') p ++;	// skip to first space
		while (*p == ' ' || *p == '\t') p ++;	// skip first space
		if (*p == '\0')
			Type = AlmanacUnknown;
		else if (p[2] == '.' && p[5] == '.')	// date format, GLONASS almanac
			Type = AlmanacGlonass;
//		else if (p[1] == '.')	// scientific notation of ecc, BDS almanac
//			Type = AlmanacBds;
		else
			Type = AlmanacUnknown;
	}
	fseek(fp, 0, SEEK_SET);	// return to beginning of file
	return Type;
}

int ReadAlmanacGps(FILE *fp, GPS_ALMANAC Almanac[])
{
	char str[128];
	int svid;
	GPS_ALMANAC Alm;
	int AlmCount = 0;

	while (fgets(str, 127, fp))
	{
		if (str[0] == '*')	// start of new almanac
		{
			if ((svid = GetAlmanacGps(fp, &Alm)) > 0)
			{
				AlmCount ++;
				if ((Almanac[svid-1].flag & 1) == 0 || Alm.week > Almanac[svid-1].week)		// not a valid almanac or has newer one
					Almanac[svid-1] = Alm;
			}
		}
	}

	return AlmCount;
}

int GetAlmanacGps(FILE *fp, PGPS_ALMANAC Alm)
{
	int svid = 0, data;
	char str[128];

	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%d", &svid);
	Alm->svid = (unsigned char)svid;
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%d", &data);
	Alm->health = (unsigned char)data;
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->ecc));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%d", &data);
	Alm->toa = data;
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->i0));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->omega_dot));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->sqrtA));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->omega0));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->w));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->M0));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->af0));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%lf", &(Alm->af1));
	if (!fgets(str, 127, fp))
		return 0;
	sscanf(str+27, "%d", &data);
	Alm->week = data;
	Alm->health = 0;
	Alm->flag = 1;

	return (svid <= 0 || svid > 32) ? 0 : svid;
}

int ReadAlmanacBds(FILE *fp, GPS_ALMANAC Almanac[])
{
	int svid;
	GPS_ALMANAC Alm;
	int AlmCount = 0;

	while ((svid = GetAlmanacBds(fp, &Alm)) > 0)
	{
		AlmCount ++;
		if ((Almanac[svid-1].flag & 1) == 0 || Alm.week > Almanac[svid-1].week)		// not a valid almanac or has newer one
			Almanac[svid-1] = Alm;
	}

	return AlmCount;
}

int GetAlmanacBds(FILE *fp, PGPS_ALMANAC Alm)
{
	char str[256];
	int svid = 0;

	fgets(str, 255, fp);
	sscanf(str, "%d %lf %d %lf %lf %lf %lf %lf %lf %lf %lf %d", &svid, &(Alm->ecc), &(Alm->toa),
		&(Alm->i0), &(Alm->omega_dot), &(Alm->sqrtA), &(Alm->omega0),
		&(Alm->w), &(Alm->M0), &(Alm->af0), &(Alm->af1), &(Alm->week));
	if (svid <= 0 || svid > 63)
		return 0;
	Alm->svid = svid;
	Alm->health = 0;
	Alm->flag = 1;

	return svid;
}

int ReadAlmanacGalileo(FILE *fp, GPS_ALMANAC Almanac[])
{
	char str[256], *p;
	int svid;
	GPS_ALMANAC Alm;
	int AlmCount = 0;
	UTC_TIME UtcTime;
	GNSS_TIME Time;

	// first find out the WN from the header
	while (fgets(str, 255, fp))
	{
		if (strstr(str, "</header>"))
			break;
		if ((p = strstr(str, "<issueDate>")) != 0)
		{
			sscanf(p + 11, "%d-%d-%d", &(UtcTime.Year), &(UtcTime.Month), &(UtcTime.Day));
			UtcTime.Hour = UtcTime.Minute = 0;
			UtcTime.Second = 0.0;
			Time = UtcToGalileoTime(UtcTime);
		}
	}
	if (Time.Week < 0)
		return 0;
	while (fgets(str, 255, fp))
	{
		if (strstr(str, "<svAlmanac>"))	// start of new almanac
		{
			if ((svid = GetAlmanacGalileo(fp, &Alm, Time.Week)) > 0)
			{
				AlmCount ++;
				if ((Almanac[svid-1].flag & 1) == 0 || Alm.week > Almanac[svid-1].week)		// not a valid almanac or has newer one
					Almanac[svid-1] = Alm;
			}
		}
	}

	return AlmCount;
}

int GetAlmanacGalileo(FILE *fp, PGPS_ALMANAC Alm, int RefWeek)
{
	int svid = 0, data;
	char str[256], *p;

	while (fgets(str, 255, fp))
	{
		if (strstr(str, "</svAlmanac>"))	// end of almanac
			break;
		if ((p = strstr(str, "<SVID>")) != 0)
			sscanf(p + 6, "%d", &svid);
		else if ((p = strstr(str, "<aSqRoot>")) != 0)
			sscanf(p + 9, "%lf", &(Alm->sqrtA));
		else if ((p = strstr(str, "<ecc>")) != 0)
			sscanf(p + 5, "%lf", &(Alm->ecc));
		else if ((p = strstr(str, "<deltai>")) != 0)
			sscanf(p + 8, "%lf", &(Alm->i0));
		else if ((p = strstr(str, "<omega0>")) != 0)
			sscanf(p + 8, "%lf", &(Alm->omega0));
		else if ((p = strstr(str, "<omegaDot>")) != 0)
			sscanf(p + 10, "%lf", &(Alm->omega_dot));
		else if ((p = strstr(str, "<w>")) != 0)
			sscanf(p + 3, "%lf", &(Alm->w));
		else if ((p = strstr(str, "<m0>")) != 0)
			sscanf(p + 4, "%lf", &(Alm->M0));
		else if ((p = strstr(str, "<af0>")) != 0)
			sscanf(p + 5, "%lf", &(Alm->af0));
		else if ((p = strstr(str, "<af1>")) != 0)
			sscanf(p + 5, "%lf", &(Alm->af1));
		else if ((p = strstr(str, "<iod>")) != 0)
			sscanf(p + 5, "%d", &data);
		else if ((p = strstr(str, "<t0a>")) != 0)
			sscanf(p + 5, "%d", &(Alm->toa));
		else if ((p = strstr(str, "<wna>")) != 0)
			sscanf(p + 5, "%d", &data);
	}

	if ((RefWeek & 3) != data)	// week does not match
		return 0;

	Alm->sqrtA += SQRT_A0;
	Alm->i0 = Alm->i0 * PI + NORMINAL_I0;
	Alm->omega0 *= PI;
	Alm->omega_dot *= PI;
	Alm->w *= PI;
	Alm->M0 *= PI;
	Alm->week = RefWeek;
	Alm->svid = svid;
	Alm->dummy = (unsigned char)data;
	Alm->health = 0;
	Alm->flag = 1;

	return svid;
}

int ReadAlmanacGlonass(FILE *fp, GLONASS_ALMANAC Almanac[])
{
	int svid;
	GLONASS_ALMANAC Alm;
	int AlmCount = 0;

	while ((svid = GetAlmanacGlonass(fp, &Alm)) > 0)
	{
		AlmCount ++;
		if ((Almanac[svid-1].flag & 1) == 0 || (Alm.leap_year * 1461 + Alm.day) > (Almanac[svid-1].leap_year * 1461 + Almanac[svid-1].day))		// not a valid almanac or has newer one
			Almanac[svid-1] = Alm;
	}

	return AlmCount;
}

int GetAlmanacGlonass(FILE *fp, PGLONASS_ALMANAC Alm)
{
	char str[256];
	int slot = 0, freq;
	double Period, Inclination;
	UTC_TIME UtcTime;
	GLONASS_TIME GlonassTime;

	fgets(str, 255, fp);
	sscanf(str, "%d %d.%d.%d %lf %lf %lf %lf %lf %lf %lf %d %lf", &slot, &(UtcTime.Day), &(UtcTime.Month), &(UtcTime.Year),
		&(Alm->t), &Period, &(Alm->ecc), &Inclination,
		&(Alm->lambda), &(Alm->w), &(Alm->clock_error), &freq, &(Alm->dt));
	if (slot <= 0 || slot > 24)
		return 0;
	// convert parameters
	UtcTime.Year += 2000;
	UtcTime.Second = UtcTime.Hour = UtcTime.Minute = 0;
	GlonassTime = UtcToGlonassTime(UtcTime);
	Alm->freq = (signed char)freq;
	Alm->flag = 1;
	Alm->leap_year = GlonassTime.LeapYear;
	Alm->day = GlonassTime.Day;
	Alm->di = (Inclination - 63) / 180;
	Alm->lambda /= 180;
	Alm->w /= 180;
	Alm->dt = Period - 43200;

	return slot;
}

GPS_ALMANAC GetAlmanacFromEphemeris(PGPS_EPHEMERIS Eph, int week, int toa)
{
	GPS_ALMANAC Alm;
	int dt;

	Alm.flag = Eph->valid & 1;
	Alm.health = (unsigned char)Eph->health;
	Alm.dummy = (unsigned char)Eph->iodc;
	Alm.svid = Eph->svid;

	if (Alm.flag == 0)
		return Alm;

	// calculate time difference between toe and toa
	Alm.toa = toa;
	Alm.week = week;
	dt = (week - Eph->week) * 604800 + (toa - Eph->toe);

	if (Eph->sqrtA > 6000 && Eph->i0 < 0.5)	// this is GEO satellite
	{
		Alm.flag = 0;	// temporarily disable
		return Alm;
	}
	// parameters of non-time-varying
	Alm.week = Eph->week;
	Alm.ecc = Eph->ecc;
	Alm.sqrtA = Eph->sqrtA;
	Alm.w = Eph->w;
	Alm.omega_dot = Eph->omega_dot;
	Alm.af1 = Eph->af1;
	// parameters adjusted with reference time change
	Alm.M0= Eph->M0 + Eph->n * dt;
	Alm.omega0 = Eph->omega0 + Eph->omega_dot * dt;
	Alm.i0 = Eph->i0 + Eph->idot * dt;
	Alm.af0 = Eph->af0 + Eph->af1 * dt;

	return Alm;
}

GLONASS_ALMANAC GetAlmanacFromEphemeris(PGLONASS_EPHEMERIS Eph)
{
	GLONASS_ALMANAC Alm;

	return Alm;
}
