//----------------------------------------------------------------------
// Rinex.cpp:
//   RINEX file read/write functions
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
#include "Rinex.h"

static void ConvertD2E(char *str);
static NavDataType ReadEphmeris(FILE *fp_nav, void *NavData);
static BOOL DecodeEphGps(NavDataType system, int svid, double *data, UTC_TIME time, PGPS_EPHEMERIS Eph);
static BOOL DecodeEphGlonass(int svid, double *data, UTC_TIME time, PGLONASS_EPHEMERIS Eph);
static unsigned char GetUraIndex(double data);
static unsigned char GetGalileoUra(double data);

NavDataType LoadNavFileHeader(FILE *fp_nav, void *NavData)
{
	char str[256], ch;
	PIONO_PARAM Iono = (PIONO_PARAM)NavData;
	PUTC_PARAM UtcParam = (PUTC_PARAM)NavData;
	int TimeMark, Svid;
	int data1, data2;

	if (!fgets(str, 255, fp_nav))
		return NavDataEnd;
	if (strstr(str, "IONOSPHERIC CORR") != 0)
	{
		if (strstr(str, "GPSA") == str)
		{
			ConvertD2E(str);
			sscanf(str + 4, "%lf %lf %lf %lf", &(Iono->a0), &(Iono->a1), &(Iono->a2), &(Iono->a3));
			if (fgets(str, 255, fp_nav) && strstr(str, "GPSB") == str)
			{
				ConvertD2E(str);
				sscanf(str + 4, "%lf %lf %lf %lf", &(Iono->b0), &(Iono->b1), &(Iono->b2), &(Iono->b3));
				Iono->flag = 1;
				return NavDataGpsIono;
			}
			else
				return NavDataUnknown;
		}
		else if (strstr(str, "BDSA") == str)
		{
			ConvertD2E(str);
			sscanf(str + 4, "%lf %lf %lf %lf %c %d", &(Iono->a0), &(Iono->a1), &(Iono->a2), &(Iono->a3), &TimeMark, &Svid);
			Iono->flag = (Svid << 8) + TimeMark - 'A';
			return NavDataBdsIonoA;
		}
		else if (strstr(str, "BDSA") == str)
		{
			ConvertD2E(str);
			sscanf(str + 4, "%lf %lf %lf %lf %c %d", &(Iono->a0), &(Iono->a1), &(Iono->a2), &(Iono->a3), &TimeMark, &Svid);
			Iono->flag = (Svid << 8) + TimeMark - 'A';
			return NavDataBdsIonoB;
		}
		return NavDataUnknown;
	}
	else if (strstr(str, "TIME SYSTEM CORR") != 0)
	{
		if (strstr(str, "GPUT") == str)
		{
			ConvertD2E(str);
			ch = str[22]; str[22] = 0; // put string terminator on first data
			sscanf(str + 4, "%lf", &(UtcParam->A0));
			str[22] = ch;	// restore char after first data
			sscanf(str + 22, "%lf %d %d", &(UtcParam->A1), &data1, &data2);
			UtcParam->tot = (unsigned char)(data1 >> 12);
			UtcParam->WN = (short)data2;
			UtcParam->WNLSF = UtcParam->WN;
			UtcParam->DN = 0;
			return NavDataGpsUtc;
		}
	}
	else if (strstr(str, "LEAP SECONDS") != 0)
	{
		sscanf(str + 4, "%d", &data1);
		UtcParam->TLS = (signed char)data1;
		return NavDataLeapSecond;
	}
	else if (strstr(str, "END OF HEADER") != 0)
		return NavDataEnd;

	return NavDataUnknown;
}

NavDataType LoadNavFileEphemeris(FILE *fp_nav, void *NavData)
{
	char str[256], system;
	double data[31];
	int svid, i, Second;
	UTC_TIME time;
	PGPS_EPHEMERIS Eph = (PGPS_EPHEMERIS)NavData;
	PGLONASS_EPHEMERIS GloEph = (PGLONASS_EPHEMERIS)NavData;
	NavDataType DataType;

	if (!fgets(str, 255, fp_nav))
		return NavDataEnd;

	ConvertD2E(str);
	if (str[0] == 'G' || str[0] == 'C' || str[0] == 'E')
	{
		switch (str[0])
		{
		case 'G':
			DataType = NavDataGpsEph;
			break;
		case 'C':
			DataType = NavDataBdsEph;
			break;
		case 'E':
			DataType = NavDataGalileoEph;
			break;
		default:
			DataType = NavDataGpsEph;
		}
		sscanf(str+4, "%d %d %d %d %d %d", &(time.Year), &(time.Month), &(time.Day), &(time.Hour), &(time.Minute), &Second);
		time.Second = (double)Second;
		system = str[0];
		sscanf(str+1, "%2d", &svid);
		sscanf(str+23, "%lf", &data[0]);
		sscanf(str+42, "%lf", &data[1]);
		sscanf(str+61, "%lf", &data[2]);
		for (i = 0; i < 7; i ++)
		{
			fgets(str, 128, fp_nav);
			ConvertD2E(str);
			sscanf(str+4,  "%lf", &data[i*4+3]);
			sscanf(str+23, "%lf", &data[i*4+4]);
			sscanf(str+42, "%lf", &data[i*4+5]);
			sscanf(str+61, "%lf", &data[i*4+6]);
		}
		DecodeEphGps(DataType, svid, data, time, Eph);
	}
	else if (str[0] == 'R')	// GLONASS
	{
		sscanf(str+4, "%d %d %d %d %d %d", &(time.Year), &(time.Month), &(time.Day), &(time.Hour), &(time.Minute), &Second);
		time.Second = (double)Second;
		system = str[0];
		sscanf(str+1, "%2d", &svid);
		sscanf(str+23, "%lf", &data[0]);
		sscanf(str+42, "%lf", &data[1]);
		sscanf(str+61, "%lf", &data[2]);
		for (i = 0; i < 3; i ++)
		{
			fgets(str, 128, fp_nav);
			ConvertD2E(str);
			sscanf(str+4,  "%lf", &data[i*4+3]);
			sscanf(str+23, "%lf", &data[i*4+4]);
			sscanf(str+42, "%lf", &data[i*4+5]);
			sscanf(str+61, "%lf", &data[i*4+6]);
		}
		DataType = NavDataGlonassEph;
		DecodeEphGlonass(svid, data, time, GloEph);
	}
	// TODO: add J (8 lines), S (4 lines), I (8 lines) decode function
	else
		DataType = NavDataUnknown;

	return DataType;
}

// convert all D in string to E
static void ConvertD2E(char *str)
{
	while (*str)
	{
		if (*str == 'D')
			*str = 'E';
		str ++;
	}
}

BOOL DecodeEphGps(NavDataType system, int svid, double *data, UTC_TIME time, PGPS_EPHEMERIS Eph)
{
	unsigned int value;
	GNSS_TIME toc_time = UtcToGpsTime(time, FALSE);

	Eph->svid = svid;
	Eph->toc = (int)(toc_time.Seconds);
	Eph->af0 = data[0];
	Eph->af1 = data[1];
	Eph->af2 = data[2];
	Eph->sqrtA = data[10];
	Eph->ecc = data[8];
	Eph->i0 = data[15];
	Eph->omega0 = data[13];
	Eph->w = data[17];
	Eph->M0 = data[6];
	Eph->delta_n = data[5];
	Eph->omega_dot = data[18];
	Eph->idot = data[19];
	Eph->crc = data[16];
	Eph->crs = data[4];
	Eph->cuc = data[7];
	Eph->cus = data[9];
	Eph->cic = data[12];
	Eph->cis = data[14];
    Eph->flag = 1;      /* Ephemeris valid */

	Eph->iode2 = Eph->iode3 = (unsigned char)data[3];      /* IODE/AODE */
	Eph->toe = (int)(data[11] + 0.5);      /* toe in week */
	Eph->week = (int)data[21];      /* week number */
	Eph->health = (unsigned char)data[24];      /* sv health */
	Eph->ura = GetUraIndex(data[23]);
	Eph->tgd = data[25];      /* TGD */

	if (system == NavDataGpsEph)	// for GPS
	{
		Eph->ura |= ((unsigned char)data[20] << 4);
		Eph->ura |= ((unsigned char)data[22] << 6);
		Eph->iodc = (unsigned short)data[26];      /* IODC */
		if (data[28] > 4.0)
			Eph->ura |= 0x80;
	}
	else if (system == NavDataBdsEph)	// for BDS
	{
		Eph->iodc = (unsigned short)data[28];      /* AODC */
		Eph->tgd2 = data[26];      /* TGD for B2 */
	}
	else if (system == NavDataGalileoEph)	// for Galileo
	{
		Eph->iodc = (unsigned short)data[3];      /* IOD */
		value = (unsigned int)data[20];
		if (value & 0x5)	// data source, either from E1B or E5b
		{
			value = (unsigned int)data[24];
			Eph->health = (value & 0x1c0) >> 3;
			Eph->health |= (value & 0x7);
		}
		else	// data source, either from E5a
		{
			value = (unsigned int)data[24];
			Eph->health = (value & 0x38) >> 3;
		}
		Eph->ura = GetGalileoUra(data[23]);
		Eph->tgd2 = data[26];      /* TGD for E1B/E5b */
//		Eph->week -= 1024;
	}

	// calculate derived variables
	if (system == NavDataGpsEph || system == NavDataGalileoEph)	// for GPS
	{
		Eph->axis = Eph->sqrtA * Eph->sqrtA;
		Eph->n = WGS_SQRT_GM / (Eph->sqrtA * Eph->axis) + Eph->delta_n;
		Eph->root_ecc = sqrt(1.0 - Eph->ecc * Eph->ecc);
		Eph->omega_t = Eph->omega0 - WGS_OMEGDOTE * Eph->toe;
		Eph->omega_delta = Eph->omega_dot - WGS_OMEGDOTE;
	}
	else if (system == NavDataBdsEph)
	{
		Eph->axis = Eph->sqrtA * Eph->sqrtA;
		Eph->n = CGCS2000_SQRT_GM / (Eph->sqrtA * Eph->axis) + Eph->delta_n;
		Eph->root_ecc = sqrt(1.0 - Eph->ecc * Eph->ecc);
		Eph->omega_t = Eph->omega0 - CGCS2000_OMEGDOTE * Eph->toe;
		Eph->omega_delta = (svid <= 5) ? Eph->omega_dot : (Eph->omega_dot - CGCS2000_OMEGDOTE);
	}

	return TRUE;
}

BOOL DecodeEphGlonass(int svid, double *data, UTC_TIME time, PGLONASS_EPHEMERIS Eph)
{
	int FrameTime;
	GLONASS_TIME eph_time = UtcToGlonassTime(time);

	Eph->n = svid;
	Eph->freq = (signed char)data[10];
	FrameTime = (int)(data[2] + 3 * 3600 + 15) / 30;
	FrameTime = FrameTime % 2880;
	Eph->tk = (unsigned short)(((FrameTime / 120) << 7) + (FrameTime % 120));
	Eph->P = (Eph->tk & 1) ? 0xc4 : 0xc0;		// P=11, ln=0, P4=0, P3=0 (determined by frame number), P2=LSB of tb, P1=00
	Eph->M = 1;		// assume GLONASS-M satellite
	Eph->Ft = 0;	// no data
	Eph->day = (unsigned short)(eph_time.Day);
	Eph->tb = ((int)(eph_time.Seconds) + 450000) / 900000 * 900;
	Eph->Bn = (unsigned char)data[6];
	Eph->En = (unsigned char)data[14];
	Eph->tn = -data[0];
	Eph->gamma = data[1];
	Eph->dtn = 0;	// no data
	Eph->x  = data[3] * 1E3; Eph->y  = data[7] * 1E3; Eph->z  = data[11] * 1E3;
	Eph->vx = data[4] * 1E3; Eph->vy = data[8] * 1E3; Eph->vz = data[12] * 1E3;
	Eph->ax = data[5] * 1E3; Eph->ay = data[9] * 1E3; Eph->az = data[13] * 1E3;
    Eph->flag = 1;      /* Ephemeris valid */

	return -1;
}

unsigned char GetUraIndex(double data)
{
	if (data < 2.4)
		return 0;
	else if (data < 3.4)
		return 1;
	else if (data < 4.85)
		return 2;
	else if (data < 6.85)
		return 3;
	else if (data < 9.65)
		return 4;
	else if (data < 13.65)
		return 5;
	else if (data < 24.0)
		return 6;
	else if (data < 48.0)
		return 7;
	else if (data < 96.0)
		return 8;
	else if (data < 192.0)
		return 9;
	else if (data < 384.0)
		return 10;
	else if (data < 768.0)
		return 11;
	else if (data < 1536.0)
		return 12;
	else if (data < 3072.0)
		return 13;
	else if (data < 6144.0)
		return 14;
	else
		return 15;
}

unsigned char GetGalileoUra(double data)
{
	int value = (int)(data * 100);	// convert to cm

	if (value < 0 || value > 6000)
		return 255;
	if (value < 50)
		return (unsigned char)value;
	else if (value < 100)
		return (unsigned char)((value - 50) / 2 + 50);
	else if (value < 200)
		return (unsigned char)((value - 100) / 4 + 75);
	else
		return (unsigned char)((value - 200) / 16 + 100);
}
