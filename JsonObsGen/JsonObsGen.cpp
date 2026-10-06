#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "BasicTypes.h"
#include "ScenarioData.h"
//#include "Trajectory.h"
//#include "GnssTime.h"
//#include "NavData.h"
//#include "Coordinate.h"
//#include "DelayModel.h"
//#include "SatelliteParam.h"
//#include "Rinex.h"
//#include "JsonParser.h"
//#include "JsonInterpreter.h"

#define TOTAL_GPS_SAT 32
#define TOTAL_BDS_SAT 63
#define TOTAL_GAL_SAT 36
#define TOTAL_GLO_SAT 24

void CalcObservation(PSAT_OBSERVATION Obs, CSatelliteParam *SatParam, unsigned int FreqSelect);
void SetSysObsType(GnssSystem system, unsigned int ObsType[], unsigned int FreqSelect);
void OutputContent(FILE* fp, OutputFormat Format, PSAT_OBSERVATION Obs, int ObsNumber, KINEMATIC_INFO CurPosVel, LLA_POSITION CurPosLla, UTC_TIME UtcTime);

CScenarioData ScenarioData;

void DumpObjects(FILE *fp, JsonObject *Object);

#define JSON_FILE "test_obs2.json"

int main()
{
	SAT_OBSERVATION Observations[TOTAL_GPS_SAT+TOTAL_BDS_SAT+TOTAL_GAL_SAT+TOTAL_GLO_SAT], *Obs;
	CSatelliteParam *SatParamList[TOTAL_GPS_SAT+TOTAL_BDS_SAT+TOTAL_GAL_SAT+TOTAL_GLO_SAT];
	RINEX_HEADER RinexHeader;
	FILE *fp;
	KINEMATIC_INFO CurPosVel;
	LLA_POSITION CurPosLla;
	GNSS_TIME CurGnssTime;
	UTC_TIME CurUtcTime;
	OutputFormat Format;
	int i, SatNumber, ObsNumber;

	ScenarioData.LoadScenarioFile(JSON_FILE);
#if 0
	FILE *fpObj = fopen("Objects.txt", "w");
	DumpObjects(fpObj, ScenarioData.JsonTree.GetRootObject());
	fclose(fpObj);
#endif
	fp = fopen(ScenarioData.OutputParam.filename, "w");
	if (fp == NULL)
		return -1;

	Format = ScenarioData.OutputParam.Format;
	ScenarioData.GetCurPosVel(CurPosVel, CurPosLla);
#if 1
	if (Format == OutputFormatRinex)
	{
		RinexHeader.HeaderFlag = 0;
		RinexHeader.MajorVersion = 3;
		RinexHeader.MinorVersion= 3;
		RinexHeader.HeaderFlag |= RINEX_HEADER_PGM | RINEX_HEADER_APPROX_POS | RINEX_HEADER_SLOT_FREQ;
		strncpy(RinexHeader.Program, "OBSGEN", 20);
		RinexHeader.ApproxPos[0] = CurPosVel.x;
		RinexHeader.ApproxPos[1] = CurPosVel.y;
		RinexHeader.ApproxPos[2] = CurPosVel.z;
		SetSysObsType(GpsSystem, RinexHeader.SysObsTypeGps, ScenarioData.OutputParam.FreqSelect[0]);
		SetSysObsType(BdsSystem, RinexHeader.SysObsTypeBds, ScenarioData.OutputParam.FreqSelect[1]);
		SetSysObsType(GalileoSystem, RinexHeader.SysObsTypeGalileo, ScenarioData.OutputParam.FreqSelect[2]);
		SetSysObsType(GlonassSystem, RinexHeader.SysObsTypeGlonass, ScenarioData.OutputParam.FreqSelect[3]);
		RinexHeader.Interval = ScenarioData.OutputParam.Interval / 1000.;
		for (i = 0; i < 24; i ++)
			RinexHeader.GlonassFreqNumber[i] = ScenarioData.NavData.GetGlonassSlotFreq(i + 1);
		RinexHeader.GlonassSlotMask = 0xffffff;
		OutputHeader(fp, &RinexHeader);
	}
	else if (Format == OutputFormatEcef)
		fprintf(fp, "%%  GPST                      x-ecef(m)      y-ecef(m)      z-ecef(m)   Q  ns\n");
	else if (Format == OutputFormatLla)
		fprintf(fp, "%%  GPST                  latitude(deg) longitude(deg)  height(m)   Q  ns\n");
	else if (Format == OutputFormatKml)
	{
		fprintf(fp, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		fprintf(fp, "<kml xmlns=\"http://www.opengis.net/kml/2.2\"> <Document>\n");
		fprintf(fp, "\t<name>Paths</name>\n");
		fprintf(fp, "\t<Style id=\"YellowLine\">\n");
		fprintf(fp, "\t\t<LineStyle>\n\t\t\t<color>7f00ffff</color>\n\t\t\t<width>4</width>\n\t\t</LineStyle>\n");
		fprintf(fp, "\t</Style>\n\t<Placemark>\n");
		fprintf(fp, "\t\t<name>Path Name</name>\n\t\t<styleUrl>#YellowLine</styleUrl>\n");
		fprintf(fp, "\t\t<LineString>\n\t\t\t<tessellate>1</tessellate>\n\t\t\t<altitudeMode>absolute</altitudeMode>\n");
		fprintf(fp, "\t\t\t<coordinates>\n");
	}

	do
	{
		ScenarioData.GetCurPosVel(CurPosVel, CurPosLla);
		ScenarioData.GetCurTime(CurGnssTime, CurUtcTime);
		CurUtcTime = GpsTimeToUtc(CurGnssTime, FALSE);	// recalculate UTC time without leap second adjustment
		ObsNumber = 0;
		Obs = Observations;
	
		SatNumber = ScenarioData.GetSatelliteParam(GpsSystem, SatParamList + ObsNumber);
		for (i = 0; i < SatNumber; i++)
			CalcObservation(Obs + i, SatParamList[ObsNumber + i], ScenarioData.OutputParam.FreqSelect[0]);
		ObsNumber += SatNumber; Obs += SatNumber;
		SatNumber = ScenarioData.GetSatelliteParam(BdsSystem, SatParamList + ObsNumber);
		for (i = 0; i < SatNumber; i++)
			CalcObservation(Obs + i, SatParamList[ObsNumber + i], ScenarioData.OutputParam.FreqSelect[1]);
		ObsNumber += SatNumber; Obs += SatNumber;
		SatNumber = ScenarioData.GetSatelliteParam(GalileoSystem, SatParamList + ObsNumber);
		for (i = 0; i < SatNumber; i++)
			CalcObservation(Obs + i, SatParamList[ObsNumber + i], ScenarioData.OutputParam.FreqSelect[2]);
		ObsNumber += SatNumber; Obs += SatNumber;
		SatNumber = ScenarioData.GetSatelliteParam(GlonassSystem, SatParamList + ObsNumber);
		for (i = 0; i < SatNumber; i++)
			CalcObservation(Obs + i, SatParamList[ObsNumber + i], ScenarioData.OutputParam.FreqSelect[3]);
		ObsNumber += SatNumber; Obs += SatNumber;

		OutputContent(fp, Format, Observations, ObsNumber, CurPosVel, CurPosLla, CurUtcTime);
	} while (ScenarioData.StepForward(CurUtcTime.Second + ScenarioData.OutputParam.Interval / 1000. >= 60., Format == OutputFormatRinex) > 0);

#endif
	if (Format == OutputFormatKml)
	{
		fprintf(fp, "\t\t\t</coordinates>\n\t\t</LineString>\n\t</Placemark>\n</Document> </kml>\n");
	}
	fclose(fp);
}

void CalcObservation(PSAT_OBSERVATION Obs, CSatelliteParam *SatParam, unsigned int FreqSelect)
{
	int i;

	Obs->system = SatParam->system;
	Obs->svid = SatParam->svid;
	Obs->ValidMask = FreqSelect;
	for (i = 0; i < MAX_OBS_NUMBER; i ++)
	{
		if ((FreqSelect & (1 << i)) == 0)
			continue;
		Obs->PseudoRange[i] = SatParam->GetTravelTime(i) * LIGHT_SPEED;
		Obs->CarrierPhase[i] = SatParam->GetCarrierPhase(i);
		Obs->Doppler[i] = SatParam->GetDoppler(i);
		Obs->CN0[i] = SatParam->CN0 / 100.;
	}
}

void SetSysObsType(GnssSystem system, unsigned int ObsType[], unsigned int FreqSelect)
{
	int MaxFreqIndex[4] = { 5, 6, 5, 3 };
	int MaxFreq = MaxFreqIndex[system];
	int i, ObsTypeIndex = 0;

	// clear all types
	for (i = 0; i < RINEX_MAX_FREQ; i ++)
		ObsType[i] = 0;

	// set ObsType according to FreqSelect bit mask
	for (i = 0; i < MaxFreq && ObsTypeIndex < RINEX_MAX_FREQ; i ++)
	{
		if ((FreqSelect & (1 << i)) == 0)
			continue;
		ObsType[ObsTypeIndex] = (i << 8) | OBS_TYPE_MASK_ALL;	// set frequency and type mask
		// set channel code
		switch (system)
		{
		case GpsSystem:	// GPS
			ObsType[ObsTypeIndex] |= (i == SIGNAL_INDEX_L1CA) ? OBS_CHANNEL_GPS_CA : (i == SIGNAL_INDEX_L2C) ? OBS_CHANNEL_GPS_L2CL : OBS_CHANNEL_Q;
			break;
		case BdsSystem:	// BDS
			ObsType[ObsTypeIndex] |= ((i >= SIGNAL_INDEX_B1I) && (i <= SIGNAL_INDEX_B3I)) ? OBS_CHANNEL_I : OBS_CHANNEL_P;
			break;
		case GalileoSystem:	// GAL
			ObsType[ObsTypeIndex] |= ((i == SIGNAL_INDEX_E1) || (i == SIGNAL_INDEX_E6)) ? OBS_CHANNEL_GAL_E1C : OBS_CHANNEL_Q;
			break;
		case GlonassSystem:	// GLO
			ObsType[ObsTypeIndex] |= OBS_CHANNEL_GLO_CA;
			break;
		}
		ObsTypeIndex ++;
	}
}

void OutputContent(FILE* fp, OutputFormat Format, PSAT_OBSERVATION Obs, int ObsNumber, KINEMATIC_INFO CurPosVel, LLA_POSITION CurPosLla, UTC_TIME UtcTime)
{
	if (Format == OutputFormatRinex)
		OutputObservation(fp, UtcTime, ObsNumber, Obs);
	else if (Format == OutputFormatEcef)
	{
		fprintf(fp, "%4d/%02d/%02d %02d:%02d:%06.3f", UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second);
		fprintf(fp, " %14.4f %14.4f %14.4f   5  12\n", CurPosVel.x, CurPosVel.y, CurPosVel.z);
	}
	else if (Format == OutputFormatLla)
	{
		fprintf(fp, "%4d/%02d/%02d %02d:%02d:%06.3f", UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second);
		fprintf(fp, " %14.9f %14.9f %10.4f   5  12\n", RAD2DEG(CurPosLla.lat), RAD2DEG(CurPosLla.lon), CurPosLla.alt);
	}
	else if (Format == OutputFormatKml)
	{
		fprintf(fp, "\t\t\t\t%.9f,%.9f,%.4f\n", RAD2DEG(CurPosLla.lon), RAD2DEG(CurPosLla.lat), CurPosLla.alt);
	}
}

#if 0
const char *TypeString[] = {
	"NULL ", "OBJ  ", "ARRAY", "STR  ", "INT  ", "FLOAT", "TRUE ", "FALSE"
};
void DumpObjects(FILE *fp, JsonObject *Object)
{
	Object = JsonStream::GetFirstObject(Object);

	while (Object)
	{
		fprintf(fp, "Object=%08x Type=%s Parent=%08x Content=%08x Next=%08x Key=%s\n", (unsigned int)Object, TypeString[Object->Type], (unsigned int)Object->pParent, (unsigned int)Object->pObjectContent, (unsigned int)Object->pNextObject, Object->Key);
		DumpObjects(fp, Object);
		Object = JsonStream::GetNextObject(Object);
	}
}
#endif
