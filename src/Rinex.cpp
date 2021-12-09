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

#define SET_FIELD_EMPTY(str) memset(str, 32, 60)

static void ConvertD2E(char *str);
static NavDataType ReadEphmeris(FILE *fp_nav, void *NavData);
static BOOL DecodeEphGps(NavDataType system, int svid, double *data, UTC_TIME time, PGPS_EPHEMERIS Eph);
static BOOL DecodeEphGlonass(int svid, double *data, UTC_TIME time, PGLONASS_EPHEMERIS Eph);
static unsigned char GetUraIndex(double data);
static unsigned char GetGalileoUra(double data);
static void SetField(char *dest, char *src, int length);
static void PrintTextField(FILE *fp, int enable, char *text, char *description);
static void PrintObsType(FILE *fp, char type, unsigned int mask);
static void SetObsField(char *s, int band, unsigned int mask, char attribute);
void PrintObservation(FILE *fp, SAT_OBSERVATION obs);

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

void OutputHeader(FILE *fp, PRINEX_HEADER Header)
{
	char str[100];

	// RINEX VERSION / TYPE
	fprintf(fp, "    %2d.%02d           OBSERVATION DATA    M (MIXED)           RINEX VERSION / TYPE\n", Header->MajorVersion, Header->MinorVersion);
	// PGM / RUN BY / DATE
	SET_FIELD_EMPTY(str);
	if (Header->HeaderFlag & RINEX_HEADER_PGM)
		SetField(str, Header->Program, 20);
	if (Header->HeaderFlag & RINEX_HEADER_AGENCY)
		SetField(str + 20, Header->Agency, 20);
	if (Header->HeaderFlag & RINEX_HEADER_DATE)
		sprintf(str + 40, "%4d%02d%02d %02d%02d%02d UTC ", Header->DateTime.Year,Header->DateTime.Month, Header->DateTime.Day, Header->DateTime.Hour, Header->DateTime.Minute, Header->DateTime.Second);
	strcpy(str + 60, "PGM / RUN BY / DATE \n");
	fputs(str, fp);
	// COMMENT
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_COMMENT, Header->Comment, "COMMENT             \n");
	// MARKER NAME
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_MK_NAME, Header->MakerName, "MARKER NAME         \n");
	// MARKER NUMBER
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_MK_NUMBER, Header->MakerNumber, "MARKER NUMBER       \n");
	// MARKER TYPE
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_MK_TYPE, Header->MakerType, "MARKER TYPE         \n");
	// OBSERVER / AGENCY
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_OBSERVER, Header->Observer, "OBSERVER / AGENCY   \n");
	// REC # / TYPE / VERS
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_OBSERVER, Header->ReceiverType, "REC # / TYPE / VERS \n");
	// ANT # / TYPE
	PrintTextField(fp, Header->HeaderFlag & RINEX_HEADER_OBSERVER, Header->AntennaType, "ANT # / TYPE        \n");
	// APPROX POSITION XYZ
	if (Header->HeaderFlag & RINEX_HEADER_APPROX_POS)
		fprintf(fp, "%14.4f%14.4f%14.4f                  APPROX POSITION XYZ \n", Header->ApproxPos[0], Header->ApproxPos[1], Header->ApproxPos[2]);
	// ANTENNA: DELTA H/E/N
	if (Header->HeaderFlag & RINEX_HEADER_ANT_DELTA)
		fprintf(fp, "%14.4f%14.4f%14.4f                  ANTENNA: DELTA H/E/N\n", Header->AntennaDelta[0], Header->AntennaDelta[1], Header->AntennaDelta[2]);
	// SYS / # / OBS TYPES
	PrintObsType(fp, 'G', Header->SysObsTypeGps);
	PrintObsType(fp, 'R', Header->SysObsTypeGlonass);
	PrintObsType(fp, 'C', Header->SysObsTypeBds);
	PrintObsType(fp, 'E', Header->SysObsTypeGalileo);
	
	fprintf(fp, "     %5.3f                                                  INTERVAL            \n", Header->Interval);
	fprintf(fp, "                                                            END OF HEADER       \n");
}

void OutputObservation(FILE *fp, UTC_TIME time, int TotalObsNumber, SAT_OBSERVATION Observations[])
{
	int i;

	fprintf(fp, "> %4d %02d %02d %02d %02d %10.7f  0 %2d                     \n", time.Year, time.Month, time.Day, time.Hour, time.Minute, time.Second, TotalObsNumber);
	for (i = 0; i < TotalObsNumber; i ++)
		PrintObservation(fp, Observations[i]);
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
	Eph->toc = toc_time.MilliSeconds / 1000;
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
	Eph->tb = ((int)(eph_time.MilliSeconds) + 450000) / 900000 * 900;
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

void SetField(char *dest, char *src, int length)
{
	int fill_length = 0;

	while (fill_length < length && src[fill_length])
	{
		dest[fill_length] = src[fill_length];
		fill_length ++;
	}
	while (fill_length < length)
		dest[fill_length++] = ' ';
}

void PrintTextField(FILE *fp, int enable, char *text, char *description)
{
	char str[100];

	SET_FIELD_EMPTY(str);
	if (enable)
	{
		SetField(str, text, 60);
		strcpy(str + 60, description);
		fputs(str, fp);
	}
}

void PrintObsType(FILE *fp, char type, unsigned int mask)
{
	char str[100];
	int i, band, obs_number = 0;
	char *s = str + 6;
	unsigned int band_mask;
	char attribute = (type == 'C') ? 'P' : 'C';	// BDS set attribute to 'P'

	SET_FIELD_EMPTY(str);
	str[0] = type;
	for (i = 0; i < 12; i ++)
		if (mask & (1 << i)) obs_number ++;
	sprintf(str + 1, "%5d", obs_number);
	for (band = 1; band <= 2; band ++)
	{
		band_mask = (mask >> ((band-1) * 4)) & 0xf;
		if (band_mask)
		{
			SetObsField(s, band, band_mask, attribute);
			s += 16;
		}
	}
	if (mask)
	{
		strcpy(str + 60, "SYS / # / OBS TYPES \n");
		fputs(str, fp);
	}
}

void SetObsField(char *s, int band, unsigned int mask, char attribute)
{
	if (band >= 3)
		band ++;
	if (mask & 1)
	{
		s[0] = ' '; s[1] = 'C'; s[2] = '0' + band; s[3] = attribute;
	}
	s += 4;
	if (mask & 2)
	{
		s[0] = ' '; s[1] = 'L'; s[2] = '0' + band; s[3] = attribute;
	}
	s += 4;
	if (mask & 4)
	{
		s[0] = ' '; s[1] = 'D'; s[2] = '0' + band; s[3] = attribute;
	}
	s += 4;
	if (mask & 8)
	{
		s[0] = ' '; s[1] = 'S'; s[2] = '0' + band; s[3] = attribute;
	}
}

void PrintObservation(FILE *fp, SAT_OBSERVATION obs)
{
	char str[128];
	char SystemId[4] = { 'G', 'C', 'E', 'R' };

	sprintf(str, "%c%02d  %12.3f   %13.3f  %14.3f          %6.3f\n", SystemId[obs.system], obs.svid, obs.PseudoRange, obs.CarrierPhase, obs.Doppler, obs.CN0);
	fputs(str, fp);
}
