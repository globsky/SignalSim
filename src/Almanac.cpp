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
#include "Coordinate.h"
#include "Almanac.h"

#define SQRT_A0 5440.588203494177338011974948823
#define NORMINAL_I0 0.97738438111682456307726683035362

int GetAlmanacGps(FILE *fp, PGPS_ALMANAC Alm);
int GetAlmanacBds(FILE *fp, PGPS_ALMANAC Alm);
int GetAlmanacGalileo(FILE *fp, PGPS_ALMANAC Alm, int RefWeek);
int GetAlmanacGlonass(FILE *fp, PGLONASS_ALMANAC Alm);
double NormAngle(double angle);
bool ConvertAlmanacFromEphemeris(PGPS_ALMANAC Alm, PGPS_EPHEMERIS Eph, int week, int toa);
bool ConvertAlmanacFromEphemerisGeo(PGPS_ALMANAC Alm, PGPS_EPHEMERIS Eph, int week, int toa);

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
				if ((Almanac[svid-1].valid & 1) == 0 || Alm.week > Almanac[svid-1].week)		// not a valid almanac or has newer one
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
	Alm->valid = 1;

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
		if ((Almanac[svid-1].valid & 1) == 0 || Alm.week > Almanac[svid-1].week)		// not a valid almanac or has newer one
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
	Alm->flag = (Alm->sqrtA > 6000.0) ? ((Alm->i0 > 0.5) ? 2 : 1) : 3;
	Alm->valid = 1;

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
				if ((Almanac[svid-1].valid & 1) == 0 || Alm.week > Almanac[svid-1].week)		// not a valid almanac or has newer one
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
	Alm->flag = (unsigned char)data;
	Alm->health = 0;
	Alm->valid = 1;

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

	if (!fgets(str, 255, fp) || str[0] == '\0')	// end of file or empty string read
		return 0;
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

double NormAngle(double angle)
{
	while (angle < -PI) angle += PI2;
	while (angle >= PI) angle -= PI2;
	return angle;
}

GPS_ALMANAC GetAlmanacFromEphemeris(PGPS_EPHEMERIS Eph, int week, int toa)
{
	GPS_ALMANAC Alm;

	Alm.valid = Eph->valid & 1;
	Alm.health = (unsigned char)Eph->health;
	Alm.svid = Eph->svid;

	if (Alm.valid == 0)
		return Alm;
	if (Eph->sqrtA > 6000 && Eph->i0 < 0.5)	// this is GEO satellite
		ConvertAlmanacFromEphemerisGeo(&Alm, Eph, week, toa);
	else
		ConvertAlmanacFromEphemeris(&Alm, Eph, week, toa);
	Alm.flag = (Eph->sqrtA > 6000.0) ? ((Eph->i0 > 0.5) ? 2 : 1) : 3;

	return Alm;
}

#define INCLINATION_FACTOR 0.94651548789182584209669573761592
GLONASS_ALMANAC GetAlmanacFromEphemeris(PGLONASS_EPHEMERIS Eph, int day, int leap_year)
{
	GLONASS_ALMANAC Alm;
	KINEMATIC_INFO PosVel;
	double t = Eph->tb - Eph->z / Eph->vz;
	int iter = 5;
	double h[3], h2, p, r, v2, rv, a, E, root_ecc, c20;

	// first calculate time of ascending
	do
	{
		GlonassSatPosSpeedEph(t, Eph, &PosVel, NULL);
		t -= PosVel.z / PosVel.vz;
	} while (fabs(PosVel.z) > 1e-3 && (iter --) > 0);
	// time and longitude of first ascending node
	Alm.t = t;
	Alm.lambda = atan2(PosVel.y, PosVel.x) / PI;
	// do velocity compensation from ECEF coordinate to inertial coordinate
	PosVel.vx -= PosVel.y * PZ90_OMEGDOTE;
	PosVel.vy += PosVel.x * PZ90_OMEGDOTE;
	// calculate areal velocity vector
	h[0] = PosVel.y * PosVel.vz - PosVel.z * PosVel.vy;
	h[1] = PosVel.z * PosVel.vx - PosVel.x * PosVel.vz;
	h[2] = PosVel.x * PosVel.vy - PosVel.y * PosVel.vx;
	// inclination and longitude of ascending node
	p = sqrt(h[0] * h[0] + h[1] * h[1]);
	Alm.di = atan(p/h[2]) / PI - 0.35;
	// calculate major-axis and ccentricity
	h2 = h[0] * h[0] + h[1] * h[1] + h[2] * h[2];
	p = h2 / PZ90_GM;
	r = sqrt(PosVel.x * PosVel.x + PosVel.y * PosVel.y + PosVel.z * PosVel.z);
	v2 = PosVel.vx * PosVel.vx + PosVel.vy * PosVel.vy + PosVel.vz * PosVel.vz;
	rv = PosVel.x * PosVel.vx + PosVel.y * PosVel.vy + PosVel.z * PosVel.vz;
	a = 1 / (2 / r -  v2 / PZ90_GM);
	if (a <= 0.0)
	{
		Alm.flag = 0;
		return Alm;
	}
	t = PI2 / sqrt(PZ90_GM / (a * a * a));	// period from major-axis
	c20 = -PZ90_C20 * 1.5 * PZ90_AE2 / (p * p);
	t *= (1 + c20 * INCLINATION_FACTOR);	// correction to orbital period
	Alm.dt = t - 43200;
	Alm.ecc = (a > p) ? sqrt(1 - p / a) : 0.0;
	if (Alm.t >= t)	// seccond ascending node
	{
		Alm.t -= t;	// previous ascending node
		Alm.lambda += (PZ90_OMEGDOTE / PI) * t;// + c20 * 2;
		if (Alm.lambda > 1)
			Alm.lambda -= 2;
	}
	// calculate argument of perigee
	E = atan2(rv, (1 - r / a) * sqrt(a * PZ90_GM));	// n = sqrt(GM/a^3) -> na^2 = sqrt(GM*a)
	root_ecc = sqrt(p / a);
	Alm.w = -atan2(root_ecc * sin(E), cos(E) - Alm.ecc) / PI;
	Alm.clock_error = Eph->tn;

	Alm.flag = 1;
	Alm.freq = Eph->freq;
	Alm.leap_year = leap_year;
	Alm.day = day;
	return Alm;
}

//void SatPosSpeedAlm(int WeekNumber, int TransmitTime, PGPS_ALMANAC pAlm, PKINEMATIC_INFO pPosVel);

bool ConvertAlmanacFromEphemeris(PGPS_ALMANAC Alm, PGPS_EPHEMERIS Eph, int week, int toa)
{
	int dt;

	// calculate time difference between toe and toa
	Alm->toa = toa;
	Alm->week = week;
	dt = (week - Eph->week) * 604800 + (toa - Eph->toe);

	// parameters of non-time-varying
	Alm->ecc = Eph->ecc;
	Alm->sqrtA = Eph->sqrtA;
	Alm->w = Eph->w;
	Alm->omega_dot = Eph->omega_dot;
	Alm->af1 = Eph->af1;
	// parameters adjusted with reference time change
	Alm->M0 = NormAngle(Eph->M0 + Eph->n * dt);
	Alm->omega0 = NormAngle(Eph->omega0 + Eph->omega_dot * dt);
	Alm->i0 = Eph->i0 + Eph->idot * dt;
	Alm->af0 = Eph->af0 + Eph->af1 * dt;

	// check almanac
/*	KINEMATIC_INFO PosVelAlm;
	GpsSatPosSpeedEph(BdsSystem, Eph->toe, Eph, &PosVelAlm, NULL);
	SatPosSpeedAlm(Eph->week, Eph->toe, Alm, &PosVelAlm);*/

	return true;
}

bool ConvertAlmanacFromEphemerisGeo(PGPS_ALMANAC Alm, PGPS_EPHEMERIS Eph, int week, int toa)
{
	KINEMATIC_INFO PosVel;
	double h[3], h2, p, r, v2, rv, a, E, root_ecc, u, w;
	int dt;

	// first calculate satellite position and velocity at toe
	GpsSatPosSpeedEph(BdsSystem, Eph->toe, Eph, &PosVel, NULL);
	// do velocity compensation from ECEF coordinate to inertial coordinate
	PosVel.vx -= PosVel.y * CGCS2000_OMEGDOTE;
	PosVel.vy += PosVel.x * CGCS2000_OMEGDOTE;
	// calculate areal velocity vector
	h[0] = PosVel.y * PosVel.vz - PosVel.z * PosVel.vy;
	h[1] = PosVel.z * PosVel.vx - PosVel.x * PosVel.vz;
	h[2] = PosVel.x * PosVel.vy - PosVel.y * PosVel.vx;
	// inclination and longitude of ascending node
	p = sqrt(h[0] * h[0] + h[1] * h[1]);
	Alm->i0 = atan(p/h[2]);
	Alm->omega0 = atan2(h[0], -h[1]) + Eph->toe * CGCS2000_OMEGDOTE;
	// calculate major-axis and ccentricity
	h2 = h[0] * h[0] + h[1] * h[1] + h[2] * h[2];
	p = h2 / (CGCS2000_SQRT_GM * CGCS2000_SQRT_GM);
	r = sqrt(PosVel.x * PosVel.x + PosVel.y * PosVel.y + PosVel.z * PosVel.z);
	v2 = PosVel.vx * PosVel.vx + PosVel.vy * PosVel.vy + PosVel.vz * PosVel.vz;
	rv = PosVel.x * PosVel.vx + PosVel.y * PosVel.vy + PosVel.z * PosVel.vz;
	a = 1 / (2 / r -  v2 / (CGCS2000_SQRT_GM * CGCS2000_SQRT_GM));
	if (a <= 0.0)
	{
		Alm->valid = 0;
		return false;
	}
	Alm->sqrtA = sqrt(a);
	Alm->ecc = (a > p) ? sqrt(1 - p / a) : 0.0;
	// calculate mean anomaly at reference time
	E = atan2(rv, (1 - r / a) * Alm->sqrtA * CGCS2000_SQRT_GM);	// n = sqrt(GM/a^3) -> na^2 = sqrt(GM*a)
	Alm->M0 = E - Alm->ecc * sin(E);
	// calculate true anomaly
	root_ecc = sqrt(p / a);
	u = atan2(PosVel.z * sqrt(h2), PosVel.y * h[0] - PosVel.x * h[1]);
	w = u - atan2(root_ecc * sin(E), cos(E) - Alm->ecc);
	Alm->w = NormAngle(w);

	// here Alm holds the parameters using reference time at Eph toe and week
	Alm->toa = toa;
	Alm->week = week;
	Alm->omega_dot = Eph->omega_dot;
	Alm->af1 = Eph->af1;
	dt = (week - Eph->week) * 604800 + (toa - Eph->toe);
	// parameters adjusted with reference time change
	Alm->M0 = NormAngle(Alm->M0 + Eph->n * dt);
	Alm->omega0 = NormAngle(Alm->omega0 + Eph->omega_dot * dt);
	Alm->af0 = Eph->af0 + Eph->af1 * dt;

	// check almanac
/*	KINEMATIC_INFO PosVelAlm;
	SatPosSpeedAlm(Eph->week, Eph->toe, Alm, &PosVelAlm);*/

	return true;
}

/*void SatPosSpeedAlm(int WeekNumber, int TransmitTime, PGPS_ALMANAC pAlm, PKINEMATIC_INFO pPosVel)
{
	int i;
	int delta_t;
	double axis, n, root_ecc, omega_t, omega_delta;
	double Mk, Ek, Ek1;
	double phi;
	double uk, rk, ik;
	double uk_dot, rk_dot;
	double xp, yp, omega;
	double xp_dot, yp_dot;
	double sin_temp, cos_temp;

	// calculate derived variables
	axis = pAlm->sqrtA * pAlm->sqrtA;
	n = WGS_SQRT_GM / (pAlm->sqrtA * axis);
	root_ecc = sqrt(1.0 - pAlm->ecc * pAlm->ecc);
	omega_t = pAlm->omega0 - WGS_OMEGDOTE * (pAlm->toa);
	omega_delta = pAlm->omega_dot - WGS_OMEGDOTE;

	// calculate time difference with week number considered
	delta_t = TransmitTime - pAlm->toa;
	delta_t += (WeekNumber - pAlm->week) * 604800;

	// get Ek from Mk with recursive algorithm
	// here pAlm->M0 and pAlm->n with unit of cycle/second
	Mk = pAlm->M0 + (n * delta_t);
	Ek1 = Ek = Mk;
	for (i = 0; i < 10; i ++)
	{
		Ek = Mk + pAlm->ecc * sin(Ek);
		if (fabs(Ek - Ek1) < 1e-14)
			break;
		Ek1 = Ek;
	}

	// assign Ek1 as 1-e*cos(Ek)
	Ek1 = 1.0 - (pAlm->ecc * cos(Ek));

	// get u(k), r(k) and i(k)
	phi = atan2(root_ecc * sin(Ek), cos(Ek) - pAlm->ecc) + pAlm->w;
	uk = phi;
	rk = axis * Ek1;
	ik = pAlm->i0;
	rk_dot = axis * pAlm->ecc * sin(Ek) * n / Ek1;
	uk_dot = n * root_ecc / (Ek1 * Ek1);

	// calculate Xp and Yp and corresponding derivatives
	sin_temp = sin(uk);
	cos_temp = cos(uk);
	xp = rk * cos_temp;
	yp = rk * sin_temp;
	xp_dot = rk_dot * cos_temp - yp * uk_dot;
	yp_dot = rk_dot * sin_temp + xp * uk_dot;

	// get omega, pAlm->omega_t in cycle and pAlm->omega_delta in cycle/second
	omega = omega_t + omega_delta * delta_t;
	sin_temp = sin(omega);
	cos_temp = cos(omega);
	// get final position and speed in ECEF coordinate
	ik = cos(pAlm->i0);
	pPosVel->x = xp * cos_temp - yp * ik * sin_temp;
	pPosVel->y = xp * sin_temp + yp * ik * cos_temp;
	pPosVel->vx = xp_dot * cos_temp - ik * yp_dot * sin_temp;
	pPosVel->vy = xp_dot * sin_temp + ik * yp_dot * cos_temp;
	sin_temp *= omega_delta;
	cos_temp *= omega_delta;
	pPosVel->vx -= xp * sin_temp + ik * yp * cos_temp;
	pPosVel->vy += xp * cos_temp - ik * yp * sin_temp;
	ik = sin(pAlm->i0);
	pPosVel->z = yp * ik;
	pPosVel->vz = yp_dot * ik;
}*/
