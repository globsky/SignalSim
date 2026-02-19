#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "BasicTypes.h"
#include "Trajectory.h"
#include "GnssTime.h"
#include "NavData.h"
#include "Coordinate.h"
#include "DelayModel.h"
#include "SatelliteParam.h"
#include "Rinex.h"
#include "JsonParser.h"
#include "JsonInterpreter.h"

#define TOTAL_GPS_SAT 32
#define TOTAL_BDS_SAT 63
#define TOTAL_GAL_SAT 36
#define TOTAL_GLO_SAT 24

void CalcObservation(PSAT_OBSERVATION Obs, CSatelliteParam *SatParam, unsigned int FreqSelect);
void SetSysObsType(GnssSystem system, unsigned int ObsType[], unsigned int FreqSelect);

CTrajectory Trajectory;
CNavData NavData;
CPowerControl PowerControl;
//DELAY_CONFIG DelayConfig;
PGPS_EPHEMERIS GpsEph[TOTAL_GPS_SAT], GpsEphVisible[TOTAL_GPS_SAT];
PGPS_EPHEMERIS BdsEph[TOTAL_BDS_SAT], BdsEphVisible[TOTAL_BDS_SAT];
PGPS_EPHEMERIS GalEph[TOTAL_GAL_SAT], GalEphVisible[TOTAL_GAL_SAT];
PGLONASS_EPHEMERIS GloEph[TOTAL_GLO_SAT], GloEphVisible[TOTAL_GLO_SAT];
OUTPUT_PARAM OutputParam;
SAT_OBSERVATION Observations[TOTAL_GPS_SAT+TOTAL_BDS_SAT+TOTAL_GAL_SAT], *Obs;
CSatelliteParam GpsSatParam[TOTAL_GPS_SAT], BdsSatParam[TOTAL_BDS_SAT], GalSatParam[TOTAL_GAL_SAT], GloSatParam[TOTAL_GLO_SAT];

int main()
{
	int i, index;
	GNSS_TIME time;
	UTC_TIME UtcTime;
	GNSS_TIME BdsTime;
	GLONASS_TIME GlonassTime;
	LLA_POSITION StartPos, CurPos;
	LOCAL_SPEED StartVel;
	KINEMATIC_INFO PosVel;
	FILE *fp;
	int GpsSatNumber, BdsSatNumber, GalSatNumber, GloSatNumber;
	int ObservationNumber;
	RINEX_HEADER RinexHeader;
	int ListCount;
	PSIGNAL_POWER PowerList;

	JsonStream JsonTree;
	JsonObject *Object;

//	memset(&DelayConfig, 0, sizeof(DelayConfig));

	JsonTree.ReadFile("test_obs3.json");
	Object = JsonTree.GetRootObject();
	AssignParameters(Object, &UtcTime, &StartPos, &StartVel, &Trajectory, &NavData, &OutputParam, &PowerControl, NULL);

	Trajectory.ResetTrajectoryTime();
	PosVel = LlaToEcef(StartPos);
	CurPos = StartPos;
	SpeedLocalToEcef(StartPos, StartVel, PosVel);
	time = UtcToGpsTime(UtcTime);
	BdsTime = UtcToBdsTime(UtcTime);
	GlonassTime = UtcToGlonassTime(UtcTime);
	UtcTime = GpsTimeToUtc(time, FALSE);	// convert back to UTC represented GPS time
	PowerControl.ResetTime();

	fp = fopen(OutputParam.filename, "w");
	if (fp == NULL)
		return -1;

	for (i = 1; i <= TOTAL_GPS_SAT; i ++)
		GpsEph[i-1] = NavData.FindEphemeris(GpsSystem, time, i);
	for (i = 1; i <= TOTAL_BDS_SAT; i ++)
		BdsEph[i-1] = NavData.FindEphemeris(BdsSystem, BdsTime, i);
	for (i = 1; i <= TOTAL_GAL_SAT; i ++)
		GalEph[i-1] = NavData.FindEphemeris(GalileoSystem, time, i);
	for (i = 1; i <= TOTAL_GLO_SAT; i ++)
		GloEph[i-1] = NavData.FindGloEphemeris(GlonassTime, i);

	GpsSatNumber = (OutputParam.FreqSelect[GpsSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
	BdsSatNumber = (OutputParam.FreqSelect[BdsSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
	GalSatNumber = (OutputParam.FreqSelect[GalileoSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
	GloSatNumber = (OutputParam.FreqSelect[GlonassSystem]) ? GetGlonassVisibleSatellite(PosVel, GlonassTime, OutputParam, GloEph, TOTAL_GLO_SAT, GloEphVisible) : 0;

	CIonoKlobuchar8 IonoModel(NavData.GetGpsIono());
	for (i = 0; i < TOTAL_GPS_SAT; i ++)
		GpsSatParam[i].Initialize(GpsSystem, GpsEph[i], &IonoModel, PowerControl.InitCN0, PowerControl.Adjust);
	for (i = 0; i < TOTAL_BDS_SAT; i ++)
		BdsSatParam[i].Initialize(BdsSystem, BdsEph[i], &IonoModel, PowerControl.InitCN0, PowerControl.Adjust);
	for (i = 0; i < TOTAL_GAL_SAT; i ++)
		GalSatParam[i].Initialize(GalileoSystem, GalEph[i], &IonoModel, PowerControl.InitCN0, PowerControl.Adjust);
	for (i = 0; i < TOTAL_GLO_SAT; i ++)
		GloSatParam[i].Initialize(GlonassSystem, (PGPS_EPHEMERIS)GloEph[i], &IonoModel, PowerControl.InitCN0, PowerControl.Adjust);

#if 1
	if (OutputParam.Format == OutputFormatRinex)
	{
		RinexHeader.HeaderFlag = 0;
		RinexHeader.MajorVersion = 3;
		RinexHeader.MinorVersion= 3;
		RinexHeader.HeaderFlag |= RINEX_HEADER_PGM | RINEX_HEADER_APPROX_POS | RINEX_HEADER_SLOT_FREQ;
		strncpy(RinexHeader.Program, "OBSGEN", 20);
		RinexHeader.ApproxPos[0] = PosVel.x;
		RinexHeader.ApproxPos[1] = PosVel.y;
		RinexHeader.ApproxPos[2] = PosVel.z;
		SetSysObsType(GpsSystem, RinexHeader.SysObsTypeGps, OutputParam.FreqSelect[0]);
		SetSysObsType(BdsSystem, RinexHeader.SysObsTypeBds, OutputParam.FreqSelect[1]);
		SetSysObsType(GalileoSystem, RinexHeader.SysObsTypeGalileo, OutputParam.FreqSelect[2]);
		SetSysObsType(GlonassSystem, RinexHeader.SysObsTypeGlonass, OutputParam.FreqSelect[3]);
		RinexHeader.Interval = OutputParam.Interval / 1000.;
		for (i = 0; i < 24; i ++)
			RinexHeader.GlonassFreqNumber[i] = NavData.GetGlonassSlotFreq(i + 1);
		RinexHeader.GlonassSlotMask = 0xffffff;
		OutputHeader(fp, &RinexHeader);
	}
	else if (OutputParam.Format == OutputFormatEcef)
		fprintf(fp, "%%  GPST                      x-ecef(m)      y-ecef(m)      z-ecef(m)   Q  ns\n");
	else if (OutputParam.Format == OutputFormatLla)
		fprintf(fp, "%%  GPST                  latitude(deg) longitude(deg)  height(m)   Q  ns\n");
	else if (OutputParam.Format == OutputFormatKml)
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
	if (OutputParam.Format == OutputFormatRinex)
	{
		ObservationNumber = 0;
		Obs = Observations;
		ListCount = PowerControl.GetPowerControlList(0, PowerList);
		for (i = 0; i < GpsSatNumber; i ++)
		{
			index = GpsEphVisible[i]->svid - 1;
			GpsSatParam[index].CalculateParam(PosVel, CurPos, time);
			GpsSatParam[index].UpdateCN0(ListCount, PowerList);
			CalcObservation(Obs, &GpsSatParam[index], OutputParam.FreqSelect[0]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < BdsSatNumber; i ++)
		{
			index = BdsEphVisible[i]->svid - 1;
			BdsSatParam[index].CalculateParam(PosVel, CurPos, time);
			BdsSatParam[index].UpdateCN0(ListCount, PowerList);
			CalcObservation(Obs, &BdsSatParam[index], OutputParam.FreqSelect[1]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < GalSatNumber; i ++)
		{
			index = GalEphVisible[i]->svid - 1;
			GalSatParam[index].CalculateParam(PosVel, CurPos, time);
			GalSatParam[index].UpdateCN0(ListCount, PowerList);
			CalcObservation(Obs, &GalSatParam[index], OutputParam.FreqSelect[2]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < GloSatNumber; i ++)
		{
			index = GloEphVisible[i]->n - 1;
			GloSatParam[index].CalculateParam(PosVel, CurPos, time);
			GloSatParam[index].UpdateCN0(ListCount, PowerList);
			CalcObservation(Obs, &GloSatParam[index], OutputParam.FreqSelect[3]);
			Obs ++;
			ObservationNumber ++;
		}
		OutputObservation(fp, UtcTime, ObservationNumber, Observations);
	}
	else if (OutputParam.Format == OutputFormatEcef)
	{
		fprintf(fp, "%4d/%02d/%02d %02d:%02d:%06.3f", UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second);
		fprintf(fp, " %14.4f %14.4f %14.4f   5  12\n", PosVel.x, PosVel.y, PosVel.z);
	}
	else if (OutputParam.Format == OutputFormatLla)
	{
		fprintf(fp, "%4d/%02d/%02d %02d:%02d:%06.3f", UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second);
		fprintf(fp, " %14.9f %14.9f %10.4f   5  12\n", RAD2DEG(CurPos.lat), RAD2DEG(CurPos.lon), CurPos.alt);
	}
	else if (OutputParam.Format == OutputFormatKml)
	{
		fprintf(fp, "\t\t\t\t%.9f,%.9f,%.4f\n", RAD2DEG(CurPos.lon), RAD2DEG(CurPos.lat), CurPos.alt);
	}
	
 	while (Trajectory.GetNextPosVelECEF(OutputParam.Interval / 1000., PosVel))
	{
		time.MilliSeconds += OutputParam.Interval;
		UtcTime = GpsTimeToUtc(time, FALSE);
		CurPos = EcefToLla(PosVel);
		if ((time.MilliSeconds % 60000) == 0)	// recalculate visible satellite at minute boundary
		{
			BdsTime = UtcToBdsTime(UtcTime);
			GlonassTime = UtcToGlonassTime(UtcTime);

			for (i = 1; i <= TOTAL_GPS_SAT; i ++)
				GpsSatParam[i-1].UpdateEphemeris(GpsEph[i-1] = NavData.FindEphemeris(GpsSystem, time, i));
			for (i = 1; i <= TOTAL_BDS_SAT; i ++)
				BdsSatParam[i-1].UpdateEphemeris(BdsEph[i-1] = NavData.FindEphemeris(BdsSystem, BdsTime, i));
			for (i = 1; i <= TOTAL_GAL_SAT; i ++)
				GalSatParam[i-1].UpdateEphemeris(GalEph[i-1] = NavData.FindEphemeris(GalileoSystem, time, i));
			for (i = 1; i <= TOTAL_GLO_SAT; i ++)
				GloSatParam[i-1].UpdateEphemeris((PGPS_EPHEMERIS)(GloEph[i-1] = NavData.FindGloEphemeris(GlonassTime, i)));

			GpsSatNumber = (OutputParam.FreqSelect[GpsSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
			BdsSatNumber = (OutputParam.FreqSelect[BdsSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
			GalSatNumber = (OutputParam.FreqSelect[GalileoSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
			GloSatNumber = (OutputParam.FreqSelect[GlonassSystem]) ? GetGlonassVisibleSatellite(PosVel, GlonassTime, OutputParam, GloEph, TOTAL_GLO_SAT, GloEphVisible) : 0;
		}
		if (OutputParam.Format == OutputFormatRinex)
		{
			ObservationNumber = 0;
			Obs = Observations;
			ListCount = PowerControl.GetPowerControlList(OutputParam.Interval, PowerList);
			for (i = 0; i < GpsSatNumber; i ++)
			{
				index = GpsEphVisible[i]->svid - 1;
				GpsSatParam[index].CalculateParam(PosVel, CurPos, time);
				GpsSatParam[index].UpdateCN0(ListCount, PowerList);
				CalcObservation(Obs, &GpsSatParam[index], OutputParam.FreqSelect[0]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < BdsSatNumber; i ++)
			{
				index = BdsEphVisible[i]->svid - 1;
				BdsSatParam[index].CalculateParam(PosVel, CurPos, time);
				BdsSatParam[index].UpdateCN0(ListCount, PowerList);
				CalcObservation(Obs, &BdsSatParam[index], OutputParam.FreqSelect[1]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < GalSatNumber; i ++)
			{
				index = GalEphVisible[i]->svid - 1;
				GalSatParam[index].CalculateParam(PosVel, CurPos, time);
				GalSatParam[index].UpdateCN0(ListCount, PowerList);
				CalcObservation(Obs, &GalSatParam[index], OutputParam.FreqSelect[2]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < GloSatNumber; i ++)
			{
				index = GloEphVisible[i]->n - 1;
				GloSatParam[index].CalculateParam(PosVel, CurPos, time);
				GloSatParam[index].UpdateCN0(ListCount, PowerList);
				CalcObservation(Obs, &GloSatParam[index], OutputParam.FreqSelect[3]);
				Obs ++;
				ObservationNumber ++;
			}
			OutputObservation(fp, UtcTime, ObservationNumber, Observations);
		}
		else if (OutputParam.Format == OutputFormatEcef)
		{
			fprintf(fp, "%4d/%02d/%02d %02d:%02d:%06.3f", UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second);
			fprintf(fp, " %14.4f %14.4f %14.4f   5  12\n", PosVel.x, PosVel.y, PosVel.z);
		}
		else if (OutputParam.Format == OutputFormatLla)
		{
			fprintf(fp, "%4d/%02d/%02d %02d:%02d:%06.3f", UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second);
			fprintf(fp, " %14.9f %14.9f %10.4f   5  12\n", RAD2DEG(CurPos.lat), RAD2DEG(CurPos.lon), CurPos.alt);
		}
		else if (OutputParam.Format == OutputFormatKml)
		{
			fprintf(fp, "\t\t\t\t%.9f,%.9f,%.4f\n", RAD2DEG(CurPos.lon), RAD2DEG(CurPos.lat), CurPos.alt);
		}
	}
#endif
	if (OutputParam.Format == OutputFormatKml)
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
