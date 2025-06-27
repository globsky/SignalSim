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
static void ReadUtcParam(char *str, PUTC_PARAM UtcParam);
static int ReadContentsTime(char *str, UTC_TIME *time, double *data);
static void ReadStoParam(FILE *fp_nav, PUTC_PARAM UtcParam);
static void ReadContentsData(char *str, double *data);
static NavDataType ReadRinex4Iono(char *str, FILE *fp_nav, void *NavData);
static BOOL DecodeEphParam(NavDataType DataType, char *str, FILE *fp_nav, PGPS_EPHEMERIS Eph);
static BOOL DecodeEphOrbit(NavDataType DataType, char *str, FILE *fp_nav, PGLONASS_EPHEMERIS Eph);
static signed short GetUraIndex(double data);
static unsigned char GetGalileoUra(double data);
static void SetField(char *dest, char *src, int length);
static void PrintTextField(FILE *fp, int enable, char *text, const char *description);
static void PrintObsType(FILE *fp, char system, unsigned int mask[3]);
static void SetObsField(char *s, char system, int freq, int channel, int type);
static void PrintSlotFreq(FILE *fp, int SlotFreq[], unsigned int SlotMask);
void PrintObservation(FILE *fp, SAT_OBSERVATION obs);

NavDataType LoadNavFileHeader(FILE *fp_nav, void *NavData)
{
	char str[256], TimeMark;
	PIONO_PARAM Iono = (PIONO_PARAM)NavData;
	PUTC_PARAM UtcParam = (PUTC_PARAM)NavData;
	int Svid;
	int data;

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
		if (strstr(str, "GAL") == str)
		{
			ConvertD2E(str);
			sscanf(str + 4, "%lf %lf %lf", &(Iono->a0), &(Iono->a1), &(Iono->a2));
			return NavDataGalileoIono;
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
			ReadUtcParam(str, UtcParam);
			return NavDataGpsUtc;
		}
		else if (strstr(str, "GAUT") == str)
		{
			ReadUtcParam(str, UtcParam);
			return NavDataGalileoUtc;
		}
		else if (strstr(str, "BDUT") == str)
		{
			ReadUtcParam(str, UtcParam);
			return NavDataBdsUtc;
		}
	}
	else if (strstr(str, "LEAP SECONDS") != 0)
	{
		sscanf(str + 4, "%d", &data);
		UtcParam->TLS = (signed char)data;
		return NavDataLeapSecond;
	}
	else if (strstr(str, "END OF HEADER") != 0)
		return NavDataEnd;

	return NavDataUnknown;
}

NavDataType LoadNavFileContents(FILE *fp_nav, void *NavData)
{
	char str[256];
	NavDataType DataType;

	if (!fgets(str, 255, fp_nav))
		return NavDataEnd;

	if (str[0] == '>')	// RINEX4 format initial line
	{
		if (str[2] == 'E' && str[3] == 'P' && str[4] == 'H')	// ephemeris
		{
			switch (str[6])	// system source
			{
			case 'G':	// GPS, "LNAV", "CNAV", "CNV2"
			case 'J':	// QZSS, "LNAV", "CNAV", "CNV2"
				DataType = (str[10] == 'L') ? NavDataGpsLnav : (str[13] == 'V') ? NavDataGpsCnav : NavDataGpsCnav2;
				break;
			case 'C':	// BDS, "D1", "D2", "CNV1", "CNV2", "CNV3"
				DataType = (str[10] == 'D') ? NavDataBdsD1D2 : (str[13] == '1') ? NavDataBdsCnav1 : (str[13] == '2') ? NavDataBdsCnav2 : NavDataBdsCnav3;
				break;
			case 'E':	// Galileo, "FNAV", "INAV"
				DataType = (str[10] == 'I') ? NavDataGalileoINav : NavDataGalileoFNav;
				break;
			case 'R':	// GLONASS, "FDMA"
				DataType = NavDataGlonassFdma;
				break;
			case 'I':	// NavIC, "LNAV"
				DataType = NavDataNavICLnav;
				break;
			default:
				DataType = NavDataUnknown;
			}
			fgets(str, 255, fp_nav);	// get first line of content
			if (DataType == NavDataGlonassFdma)
				DecodeEphOrbit(DataType, str, fp_nav, (PGLONASS_EPHEMERIS)NavData);
			else if (DataType != NavDataUnknown)
				DecodeEphParam(DataType, str, fp_nav, (PGPS_EPHEMERIS)NavData);
		}
		else if (str[2] == 'I' && str[3] == 'O' && str[4] == 'N')	// ionosphere parameters
		{
			DataType = ReadRinex4Iono(str, fp_nav, NavData);
		}
		else if (str[2] == 'S' && str[3] == 'T' && str[4] == 'O')	// system time parameters
		{
			ReadStoParam(fp_nav, (PUTC_PARAM)NavData);
			DataType = NavDataGpsUtc;
		}
		else if (str[2] == 'E' && str[3] == 'O' && str[4] == 'P')	// earth orientatin parameters
		{
			fgets(str, 128, fp_nav);
			fgets(str, 128, fp_nav);
			fgets(str, 128, fp_nav);
			DataType = NavDataUnknown;
		}
	}
	else if (str[0] == 'G' || str[0] == 'C' || str[0] == 'E')
	{
		DataType = (str[0] == 'G') ? NavDataGpsLnav : (str[0] == 'C') ? NavDataBdsD1D2 : NavDataGalileoINav;
		DecodeEphParam(DataType, str, fp_nav, (PGPS_EPHEMERIS)NavData);
	}
	else if (str[0] == 'R')	// GLONASS
	{
		DataType = NavDataGlonassFdma;
		DecodeEphOrbit(DataType, str, fp_nav, (PGLONASS_EPHEMERIS)NavData);
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
		sprintf(str + 40, "%4d%02d%02d %02d%02d%02d UTC ", Header->DateTime.Year,Header->DateTime.Month, Header->DateTime.Day, Header->DateTime.Hour, Header->DateTime.Minute, (int)Header->DateTime.Second);
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
	// GLONASS SLOT / FRQ #
	if (Header->HeaderFlag & RINEX_HEADER_SLOT_FREQ)
		PrintSlotFreq(fp, Header->GlonassFreqNumber, Header->GlonassSlotMask);
	
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

void ReadUtcParam(char *str, PUTC_PARAM UtcParam)
{
	char ch;
	int data1, data2;

	ConvertD2E(str);
	ch = str[22]; str[22] = 0; // put string terminator on first data
	sscanf(str + 4, "%lf", &(UtcParam->A0));
	str[22] = ch;	// restore char after first data
	sscanf(str + 22, "%lf %d %d", &(UtcParam->A1), &data1, &data2);
	UtcParam->A2 = 0.0;
	UtcParam->tot = (unsigned char)(data1 >> 12);
	UtcParam->WN = (short)data2;
	UtcParam->WNLSF = UtcParam->WN;
	UtcParam->DN = 0;
}

void ReadStoParam(FILE *fp_nav, PUTC_PARAM UtcParam)
{
	char str[256];
	UTC_TIME time;
	double data[4];
	GNSS_TIME tot_time;

	fgets(str, 128, fp_nav);
	ReadContentsTime(str, &time, &data[0]);
	fgets(str, 128, fp_nav);
	ReadContentsData(str, &data[0]);
	tot_time = UtcToGpsTime(time, FALSE);
	UtcParam->tot = (unsigned char)((tot_time.MilliSeconds / 1000) >> 12);
	UtcParam->WN = (short)(tot_time.Week);
	UtcParam->A0 = data[1];
	UtcParam->A1 = data[2];
	UtcParam->A2 = data[3];
	UtcParam->WNLSF = UtcParam->WN;
	UtcParam->DN = 0;
}

int ReadContentsTime(char *str, UTC_TIME *time, double *data)
{
	int Second, svid;
	int length = strlen(str);

	ConvertD2E(str);
	sscanf(str+4, "%d %d %d %d %d %d", &(time->Year), &(time->Month), &(time->Day), &(time->Hour), &(time->Minute), &Second);
	time->Second = (double)Second;
	if (str[1] == ' ') svid = 0; else sscanf(str+1, "%2d", &svid);
	if (length > 24 && str[24] != ' ') sscanf(str+23, "%lf", &data[0]); else data[0] = 0.0;
	if (length > 43 && str[43] != ' ') sscanf(str+42, "%lf", &data[1]); else data[1] = 0.0;
	if (length > 62 && str[62] != ' ') sscanf(str+61, "%lf", &data[2]); else data[2] = 0.0;

	return svid;
}

void ReadContentsData(char *str, double *data)
{
	int length = strlen(str);

	ConvertD2E(str);
	if (length >  5 && str[ 5] != ' ') sscanf(str+ 4, "%lf", &data[0]); else data[0] = 0.0;
	if (length > 24 && str[24] != ' ') sscanf(str+23, "%lf", &data[1]); else data[1] = 0.0;
	if (length > 43 && str[43] != ' ') sscanf(str+42, "%lf", &data[2]); else data[2] = 0.0;
	if (length > 62 && str[62] != ' ') sscanf(str+61, "%lf", &data[3]); else data[3] = 0.0;
}

NavDataType ReadRinex4Iono(char *str, FILE *fp_nav, void *NavData)
{
	PIONO_PARAM Iono = (PIONO_PARAM)NavData;
	PIONO_BDGIM IonoBds = (PIONO_BDGIM)NavData;
	PIONO_NEQUICK IonoGal = (PIONO_NEQUICK)NavData;
	NavDataType DataType;
	UTC_TIME time;
	double data[11];
	int prn;

	sscanf(str + 7, "%2d", &prn);
	if (str[6] == 'E')	// NEQUICK-G model
	{
		fgets(str, 128, fp_nav);
		ReadContentsTime(str, &time, &data[0]);
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[3]);
		IonoGal->ai0 = data[0]; IonoGal->ai1 = data[1]; IonoGal->ai2 = data[2];
		IonoGal->flag = (unsigned long)data[3];
		DataType = NavDataIonGalileo;
	}
	else if (str[6] == 'C' && str[13] == 'X')	// BDS BDGIM model
	{
		fgets(str, 128, fp_nav);
		ReadContentsTime(str, &time, &data[0]);
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[3]);
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[7]);
		IonoBds->alpha1 = data[0]; IonoBds->alpha2 = data[1]; IonoBds->alpha3 = data[2];
		IonoBds->alpha4 = data[3]; IonoBds->alpha5 = data[4]; IonoBds->alpha6 = data[5];
		IonoBds->alpha7 = data[6]; IonoBds->alpha8 = data[7]; IonoBds->alpha9 = data[8];
		IonoBds->flag = 1 | (prn << 8);
		DataType = NavDataIonBdgim;
	}
	else	// Klobuchar model
	{
		fgets(str, 128, fp_nav);
		ReadContentsTime(str, &time, &data[0]);
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[3]);
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[7]);
		Iono->a0 = data[0]; Iono->a1 = data[1]; Iono->a2 = data[2]; Iono->a3 = data[3];
		Iono->b0 = data[4]; Iono->b1 = data[5]; Iono->b2 = data[6]; Iono->b3 = data[7];
		DataType = (str[6] == 'C') ? NavDataIonBds : NavDataIonGps;
	}

	return DataType;
}

// definitions for different delay correction factor vs. frequency
#define TGD_GAMMA_L2 1.6469444444444444444444444444444	// (77/60)^2
#define TGD_GAMME_L5 1.7932703213610586011342155009452	// (154/115)^2, also used for E5a, B2a
#define TGD_GAMMA_E5b 1.7032461936225222637173226084458	// (77/59)^2, also used for B2b/B2I

BOOL DecodeEphParam(NavDataType DataType, char *str, FILE *fp_nav, PGPS_EPHEMERIS Eph)
{
	int svid, i, LineCount = 9;
	UTC_TIME time;
	GNSS_TIME toc_time;
	double data[39];

	switch (DataType)
	{
		case NavDataGpsLnav:
		case NavDataBdsD1D2:
		case NavDataGalileoINav:
		case NavDataGalileoFNav:
		case NavDataNavICLnav:
			LineCount --;
		case NavDataGpsCnav:
		case NavDataBdsCnav3:
			LineCount --;
		case NavDataGpsCnav2:
		case NavDataBdsCnav1:
		case NavDataBdsCnav2:
		    Eph->valid = 1;      // Ephemeris valid
			break;
		default:
			LineCount = 0;
		    Eph->valid = 0;      // unknown data type, Ephemeris invalid
			break;
	}
	svid = ReadContentsTime(str, &time, &data[0]);
	toc_time = UtcToGpsTime(time, FALSE);
	for (i = 0; i < LineCount; i ++)
	{
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[i*4+3]);
	}

	// common variables
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
	Eph->top = Eph->toe = (int)(data[11] + 0.5);      /* toe in week */

	// variables for different navigation format
	switch (DataType)
	{
		case NavDataGpsLnav:
			Eph->axis_dot = 0.0;
			Eph->delta_n_dot = 0.0;
			Eph->iodc = (unsigned short)data[26];      /* IODC */
			Eph->iode = (unsigned char)data[3];      /* IODE/AODE */
			Eph->week = (int)data[21];      /* week number */
			Eph->health = (unsigned short)data[24];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			if (Eph->ura < 0) Eph->ura = 0;	/* clipped to minimize 0 */
			Eph->flag = (unsigned short)data[20] | ((unsigned short)data[22] << 2) | ((data[28] > 4.0) ? 8 : 0);
			Eph->tgd = Eph->tgd_ext[4] = data[25];      /* TGD */
			Eph->tgd2 = Eph->tgd * TGD_GAMMA_L2;      /* TGD for L2 */
			Eph->tgd_ext[0] = Eph->tgd_ext[1] = Eph->tgd;	/* TGD for L1Cd and L1Cp */
			Eph->tgd_ext[2] = Eph->tgd_ext[3] = Eph->tgd * TGD_GAMME_L5;	/* TGD for L5I and L5Q */
			Eph->source = EPH_SOURCE_LNAV;
			break;
		case NavDataGpsCnav:
			Eph->toe = Eph->toc;	/* toe must be same as toc */
			Eph->axis_dot = data[3];
			Eph->delta_n_dot = data[20];
			Eph->iodc = 0;      /* IODC */
			Eph->iode = 0;      /* IODE/AODE */
			Eph->week = (int)data[32];      /* week number for WNop */
			Eph->health = (unsigned short)data[24];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = 0;	// put URA_NED later
			Eph->tgd = data[25] - data[27];      /* TGD - ISC_L1CA */
			Eph->tgd2 = data[25] - data[28];      /* TGD - ISC_L2C */
			Eph->tgd_ext[0] = Eph->tgd_ext[1] = Eph->tgd - data[27];	/* TGD for L1Cd and L1Cp */
			Eph->tgd_ext[2] = data[25] - data[29];	/* TGD for L5I */
			Eph->tgd_ext[3] = data[25] - data[30];	/* TGD for L5Q */
			Eph->tgd_ext[4] = data[25];	/* store TGD for navigation data recovery */
			Eph->source = EPH_SOURCE_CNAV;
			break;
		case NavDataGpsCnav2:
			Eph->toe = Eph->toc;	/* toe must be same as toc */
			Eph->axis_dot = data[3];
			Eph->delta_n_dot = data[20];
			Eph->iodc = 0;      /* IODC */
			Eph->iode = 0;      /* IODE/AODE */
			Eph->week = (int)data[36];      /* week number for WNop */
			Eph->health = (unsigned short)data[24];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = 0;	// put URA_NED later
			Eph->tgd = data[25] - data[27];      /* TGD - ISC_L1CA */
			Eph->tgd2 = data[25] - data[28];      /* TGD - ISC_L2C */
			Eph->tgd_ext[0] = data[25] - data[31];	/* TGD for L1Cd */
			Eph->tgd_ext[1] = data[25] - data[32];	/* TGD for L1Cp */
			Eph->tgd_ext[2] = data[25] - data[29];	/* TGD for L5I */
			Eph->tgd_ext[3] = data[25] - data[30];	/* TGD for L5Q */
			Eph->tgd_ext[4] = data[25];	/* store TGD for navigation data recovery */
			Eph->source = EPH_SOURCE_CNV2;
			break;
		case NavDataGalileoINav:
		case NavDataGalileoFNav:
			Eph->axis_dot = 0.0;
			Eph->delta_n_dot = 0.0;
			Eph->iodc = (unsigned short)data[3];      /* IOD */
			Eph->iode = (unsigned char)data[3];      /* IODE/AODE */
			Eph->week = (int)data[21];      /* week number */
			Eph->health = (unsigned short)data[24];      /* sv health */
			Eph->ura = GetGalileoUra(data[23]);
			Eph->flag = (unsigned short)data[20];
			Eph->tgd = data[25];      /* TGD */
			Eph->tgd2 = (Eph->flag & 0x2) ? data[25] : data[26];      /* TGD for E1/E5b */
			Eph->tgd_ext[2] = Eph->tgd * TGD_GAMME_L5;	/* TGD for E5a */
			Eph->tgd_ext[4] = Eph->tgd * TGD_GAMMA_E5b;	/* TGD for E5b */
			Eph->source = (Eph->flag & 0x2) ? EPH_SOURCE_FNAV : EPH_SOURCE_INAV;
			break;
		case NavDataBdsD1D2:
			Eph->axis_dot = 0.0;
			Eph->delta_n_dot = 0.0;
			Eph->iodc = (unsigned short)data[28];      /* AODC */
			Eph->iode = (unsigned char)data[3];      /* IODE/AODE */
			Eph->week = (int)data[21];      /* week number */
			Eph->health = (data[24] != 0) ? 0x80 : 0;      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = (Eph->sqrtA > 6000.0) ? ((Eph->i0 > 0.5) ? 2 : 1) : 3;
//			Eph->flag = (unsigned short)data[20] | ((unsigned short)data[22] << 2) | ((data[28] > 4.0) ? 8 : 0);
			Eph->tgd = data[25];      /* TGD */
			Eph->tgd2 = data[26];      /* TGD for B2I */
			Eph->tgd_ext[0] = Eph->tgd_ext[1] = Eph->tgd;	/* TGD for B1Cd and B1Cp */
			Eph->tgd_ext[2] = Eph->tgd_ext[3] = Eph->tgd * TGD_GAMME_L5;	/* TGD for B2ad and B2ap */
			Eph->tgd_ext[4] = Eph->tgd * TGD_GAMMA_E5b;	/* TGD for B2bI */
			Eph->source = EPH_SOURCE_D1D2;
			break;
		case NavDataBdsCnav1:
			Eph->axis_dot = data[3];
			Eph->delta_n_dot = data[20];
			Eph->iodc = (unsigned short)data[34];      /* IODC */
			Eph->iode = (unsigned char)Eph->iodc;      /* IODE/AODE */
			Eph->week = toc_time.Week - 1356;      /* week number from toc */
			Eph->health = (unsigned short)data[32];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = (unsigned short)data[21] | ((unsigned short)data[33] << 2) | ((unsigned short)data[31] << 11);
			Eph->tgd = data[29];      /* TGD for B1I */
			Eph->tgd2 = data[29] * TGD_GAMMA_E5b;      /* TGD for B2I */
			Eph->tgd_ext[0] = data[29] + data[27];	/* TGD for L1Cd */
			Eph->tgd_ext[1] = data[29];	/* TGD for L1Cp */
			Eph->tgd_ext[2] = Eph->tgd_ext[3] = data[30];	/* TGD for B2ad and B2ap */
			Eph->tgd_ext[4] = data[29] * TGD_GAMMA_E5b;      /* TGD for B2bI */
			Eph->source = EPH_SOURCE_CNV1;
			break;
		case NavDataBdsCnav2:
			Eph->axis_dot = data[3];
			Eph->delta_n_dot = data[20];
			Eph->iodc = (unsigned short)data[34];      /* IODC */
			Eph->iode = (unsigned char)Eph->iodc;      /* IODE/AODE */
			Eph->week = toc_time.Week - 1356;      /* week number from toc */
			Eph->health = (unsigned short)data[32];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = (unsigned short)data[21] | ((unsigned short)data[33] << 2) | ((unsigned short)data[31] << 11);
			Eph->tgd = data[29];      /* TGD for B1I */
			Eph->tgd2 = data[29] * TGD_GAMMA_E5b;      /* TGD for B2I */
			Eph->tgd_ext[0] = Eph->tgd_ext[1] = data[29];	/* TGD for B1Cd and B1Cp */
			Eph->tgd_ext[2] = data[30] + data[28];	/* TGD for B2ad */
			Eph->tgd_ext[3] = data[30];	/* TGD forB2ap */
			Eph->tgd_ext[4] = data[29] * TGD_GAMMA_E5b;      /* TGD for B2bI */
			Eph->source = EPH_SOURCE_CNV2;
			break;
		case NavDataBdsCnav3:
			Eph->axis_dot = data[3];
			Eph->delta_n_dot = data[20];
			Eph->iodc = 0;      /* IODC */
			Eph->iode = (unsigned char)Eph->iodc;      /* IODE/AODE */
			Eph->week = toc_time.Week - 1356;      /* week number from toc */
			Eph->health = (unsigned short)data[28];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = (unsigned short)data[21] | ((unsigned short)data[29] << 8) | ((unsigned short)data[27] << 11);;
			Eph->tgd = data[29] / TGD_GAMMA_E5b;      /* TGD for B1I */
			Eph->tgd2 = data[29];      /* TGD for B2I */
			Eph->tgd_ext[0] = Eph->tgd_ext[1] = Eph->tgd;	/* TGD for B1Cd and B1Cp */
			Eph->tgd_ext[2] = Eph->tgd2;	/* TGD for B2ad */
			Eph->tgd_ext[3] = Eph->tgd2;	/* TGD forB2ap */
			Eph->tgd_ext[4] = Eph->tgd2;      /* TGD for B2bI */
			Eph->source = EPH_SOURCE_CNV3;
			break;
		case NavDataNavICLnav:
			Eph->axis_dot = 0.0;
			Eph->delta_n_dot = 0.0;
			Eph->iodc = (unsigned short)data[26];      /* IODC */
			Eph->iode = (unsigned char)Eph->iodc;      /* IODE/AODE */
			Eph->week = (int)data[21];      /* week number */
			Eph->health = (unsigned short)data[24];      /* sv health */
			Eph->ura = GetUraIndex(data[23]);
			Eph->flag = (unsigned short)data[20] | ((unsigned short)data[22] << 2) | ((data[28] > 4.0) ? 8 : 0);
			Eph->tgd = data[25];      /* TGD */
			Eph->source = EPH_SOURCE_LNAV;
			break;
	}

	// calculate derived variables
	if (DataType == NavDataBdsD1D2 || DataType == NavDataBdsCnav1 || DataType == NavDataBdsCnav2 || DataType == NavDataBdsCnav3)
	{
		Eph->axis = Eph->sqrtA * Eph->sqrtA;
		Eph->n = CGCS2000_SQRT_GM / (Eph->sqrtA * Eph->axis) + Eph->delta_n;
		Eph->root_ecc = sqrt(1.0 - Eph->ecc * Eph->ecc);
		Eph->omega_t = Eph->omega0 - CGCS2000_OMEGDOTE * Eph->toe;
		Eph->omega_delta = (svid <= 5 || Eph->svid >= 59) ? Eph->omega_dot : (Eph->omega_dot - CGCS2000_OMEGDOTE);
		Eph->flag = (Eph->svid <= 5 || Eph->svid >= 59) ? 1 : (Eph->axis > 4e7) ? 2 : 3;	// SatType
	}
	else //if (DataType == NavDataGpsLnav || DataType == NavDataGalileoIFNav)	// for GPS, Galileo
	{
		Eph->axis = Eph->sqrtA * Eph->sqrtA;
		Eph->n = WGS_SQRT_GM / (Eph->sqrtA * Eph->axis) + Eph->delta_n;
		Eph->root_ecc = sqrt(1.0 - Eph->ecc * Eph->ecc);
		Eph->omega_t = Eph->omega0 - WGS_OMEGDOTE * Eph->toe;
		Eph->omega_delta = Eph->omega_dot - WGS_OMEGDOTE;
	}

	return TRUE;
}

BOOL DecodeEphOrbit(NavDataType DataType, char *str, FILE *fp_nav, PGLONASS_EPHEMERIS Eph)
{
	int svid, i, LineCount = 9;
	int FrameTime;
	UTC_TIME time;
	GLONASS_TIME eph_time;
	double data[19];

	svid = ReadContentsTime(str, &time, &data[0]);
	eph_time = UtcToGlonassTime(time);	// in RINEX, time is GPS time, so eph_time has bias of leap second
	for (i = 0; i < 3; i ++)
	{
		fgets(str, 128, fp_nav);
		ReadContentsData(str, &data[i*4+3]);
	}
	DataType = NavDataGlonassFdma;
	Eph->n = svid;
	Eph->freq = (signed char)data[10];
	FrameTime = (int)(data[2] + 3 * 3600 + 15) / 30;
	FrameTime = FrameTime % 2880;
	Eph->tk = (unsigned short)(((FrameTime / 120) << 7) + (FrameTime % 120));
	Eph->P = (Eph->tk & 1) ? 0xc4 : 0xc0;		// P=11, ln=0, P4=0, P3=0 (determined by frame number), P2=LSB of tb, P1=00
	Eph->M = 1;		// assume GLONASS-M satellite
	Eph->Ft = 0;	// no data
	Eph->day = (unsigned short)(eph_time.Day);
	Eph->tb = ((int)(eph_time.MilliSeconds) + 450000) / 900000 * 900;	// when align to 900 second, leap second bias eleminated
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

signed short GetUraIndex(double data)
{
	int i, value = (int)(data * 100 + 0.5);	// convert to cm
	int boundary[21] = { 1, 2, 3, 4, 6, 8, 11, 15, 21, 30, 43, 60, 85, 120, 170, 240, 340, 485, 685, 965, 1365 };

	if (value <= boundary[20])
	{
		for (i = 0; i < 21; i ++)
		{
			if (value <= boundary[i])
				return i - 15;
		}
	}
	for (i = 0; i < 9; i ++)
	{
		if (value <= (2400 << i))
			break;
	}
	return 15;
}

unsigned char GetGalileoUra(double data)
{
	int value = (int)(data * 100 + 0.5);	// convert to cm

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

void PrintTextField(FILE *fp, int enable, char *text, const char *description)
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

void PrintObsType(FILE *fp, char system, unsigned int mask[3])
{
	char str[100];
	int i, j, freq, channel, obs_number = 0;

	SET_FIELD_EMPTY(str);
	str[0] = system;
	for (i = 0; i < 3; i ++)
	{
		channel = (mask[i] >> 4) & 0xf;
		freq = (mask[i] >> 8) & 0xf;
		for (j = 0; j < 4; j ++)
			if (mask[i] & (1 << j))
				SetObsField(str + 7 + (obs_number ++) * 4, system, freq, channel, j);
	}
	sprintf(str + 1, "%5d", obs_number);
	str[6] = ' ';
	if (obs_number > 0)
	{
		strcpy(str + 60, "SYS / # / OBS TYPES \n");
		fputs(str, fp);
	}
}

static const char GpsFreqCode[] = {'1', '1', '2', '2', '5', };	// L1CA/L1C/L2C/L2P/L5
static const char BdsFreqCode[] = {'1', '2', '7', '6', '5', '7', };	// B1C/B1/B2/B3/B2a/B2b
static const char GalileoFreqCode[] = {'1', '5', '7', '8', '6', };	// E1/E5a/E5b/E5/E6
static const char GlonassFreqCode[] = {'1', '2', '3', '4', '6'};	// G1/G2/G3/G1a/G2a
static const char TypeCode[] = {'C', 'L', 'D', 'S', };
static const char GpsChannelCode[][16] = {
	{'C', 'S', 'L', 'X', 'P', 'W', 'Y', 'M', 'N', 'R', },
	{'C', 'S', 'L', 'X', 'P', 'W', 'Y', 'M', 'N', 'R', },
	{'C', 'D', 'S', 'L', 'X', 'P', 'W', 'Y', 'M', 'N', 'R', },
	{'C', 'D', 'S', 'L', 'X', 'P', 'W', 'Y', 'M', 'N', 'R', },
	{'I', 'Q', 'X', },
};
static const char ChannelCodeABC[] = {'A', 'B', 'C', 'X', 'Z', };
static const char ChannelCodeDPX[] = {'D', 'P', 'X', };
static const char ChannelCodeIQX[] = {'I', 'Q', 'X', };
void SetObsField(char *s, char system, int freq, int channel, int type)
{
	switch (system)
	{
	case 'G':
		s[0] = TypeCode[type];
		s[1] = GpsFreqCode[freq];
		s[2] = GpsChannelCode[freq][channel];
		break;
	case 'C':
		s[0] = TypeCode[type];
		s[1] = BdsFreqCode[freq];
		s[2] = (freq >= 1) && (freq <= 3) ? ChannelCodeIQX[channel] : ChannelCodeDPX[channel];
		break;
	case 'E':
		s[0] = TypeCode[type];
		s[1] = GalileoFreqCode[freq];
		s[2] = ((freq == 0) || (freq == 4)) ? ChannelCodeABC[channel] : ChannelCodeIQX[channel];
		break;
	case 'R':
		s[0] = TypeCode[type];
		s[1] = GlonassFreqCode[freq];
		s[2] = (channel == 0) ? 'C' : 'P';
		break;
	}
}

void PrintSlotFreq(FILE *fp, int SlotFreq[], unsigned int SlotMask)
{
	int i, Count, Number = 0;
	char str[100];

	// count the number of SLOT/FRQ pair
	for (i = 0; i < 24; i ++)
		Number += (SlotMask & (1 << i)) ? 1 : 0;
	if (Number == 0)
		return;

	for (i = 0, Count = 0; i < 24; i ++)
	{
		if ((SlotMask & (1 << i)) == 0)
			continue;
		if (Count == 0)	// first data
			sprintf(str, " %2d", Number);
		else if ((Count % 8) == 0)
			sprintf(str, "   ");
		sprintf(str + 3 + 7 * (Count & 7), " R%02d %2d", i + 1, SlotFreq[i]);
		if (((++Count) % 8) == 0)
		{
			strcat(str + 59, " GLONASS SLOT / FRQ #\n");
			fputs(str, fp);
		}
	}
	if ((Number % 8) != 0)	// less than 8 slots to fill a line
	{
		for (i = 3 + 7 * (Number & 7); i < 59; i ++)	// fill rest with space
			str[i] = ' ';
		strcat(str + 59, " GLONASS SLOT / FRQ #\n");
		fputs(str, fp);
	}
}

void PrintObservation(FILE *fp, SAT_OBSERVATION obs)
{
	char str[256];
	int i, obs_number = 0;

	sprintf(str, "%c%02d", "GCER"[obs.system], obs.svid);
	for (i = 0; i < MAX_OBS_NUMBER; i ++)
	{
		if ((obs.ValidMask & (1 << i)) == 0)
			continue;
		sprintf(str + 3 + obs_number * 64, "  %12.3f   %13.3f  %14.3f          %6.3f  ", obs.PseudoRange[i], obs.CarrierPhase[i], obs.Doppler[i], obs.CN0[i]);
		obs_number ++;
	}
	strcat(str, "\n");
	fputs(str, fp);
}
