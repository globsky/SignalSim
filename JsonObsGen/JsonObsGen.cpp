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
#include "SatelliteParam.h"
#include "Rinex.h"
#include "JsonParser.h"
#include "JsonInterpreter.h"

#define TOTAL_GPS_SAT 32
#define TOTAL_BDS_SAT 63
#define TOTAL_GAL_SAT 36
#define TOTAL_GLO_SAT 24

void CalcObservation(PSAT_OBSERVATION Obs, PSATELLITE_PARAM SatParam, unsigned int FreqSelect);
void SetSysObsType(int system, unsigned int ObsType[], unsigned int FreqSelect);

int main()
{
	int i, index;
	GNSS_TIME time;
	UTC_TIME UtcTime;
	GNSS_TIME BdsTime;
	GLONASS_TIME GlonassTime;
	CTrajectory Trajectory;
	CNavData NavData;
	CPowerControl PowerControl;
	DELAY_CONFIG DelayConfig;
	LLA_POSITION StartPos, CurPos;
	LOCAL_SPEED StartVel;
	KINEMATIC_INFO PosVel;
	FILE *fp;
	int GpsSatNumber, BdsSatNumber, GalSatNumber, GloSatNumber;
	PGPS_EPHEMERIS GpsEph[TOTAL_GPS_SAT], GpsEphVisible[TOTAL_GPS_SAT];
	PGPS_EPHEMERIS BdsEph[TOTAL_BDS_SAT], BdsEphVisible[TOTAL_BDS_SAT];
	PGPS_EPHEMERIS GalEph[TOTAL_GAL_SAT], GalEphVisible[TOTAL_GAL_SAT];
	PGLONASS_EPHEMERIS GloEph[TOTAL_GLO_SAT], GloEphVisible[TOTAL_GLO_SAT];
	OUTPUT_PARAM OutputParam;
	SATELLITE_PARAM GpsSatelliteParam[TOTAL_GPS_SAT], BdsSatelliteParam[TOTAL_BDS_SAT], GalSatelliteParam[TOTAL_GAL_SAT], GloSatelliteParam[TOTAL_GLO_SAT];
	int ObservationNumber;
	SAT_OBSERVATION Observations[TOTAL_GPS_SAT+TOTAL_BDS_SAT+TOTAL_GAL_SAT], *Obs;
	RINEX_HEADER RinexHeader;
	int ListCount;
	PSIGNAL_POWER PowerList;

	JsonStream JsonTree;
	JsonObject *Object;

	memset(&DelayConfig, 0, sizeof(DelayConfig));

	JsonTree.ReadFile("test_obs2.json");
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

	memset(GpsSatelliteParam, 0, sizeof(GpsSatelliteParam));
	memset(BdsSatelliteParam, 0, sizeof(BdsSatelliteParam));
	memset(GalSatelliteParam, 0, sizeof(GalSatelliteParam));
	memset(GloSatelliteParam, 0, sizeof(GloSatelliteParam));
	for (i = 0; i < TOTAL_GPS_SAT; i ++)
	{
		GpsSatelliteParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
		GpsSatelliteParam[i].PosTimeTag = -1;
	}
	for (i = 0; i < TOTAL_BDS_SAT; i ++)
	{
		BdsSatelliteParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
		BdsSatelliteParam[i].PosTimeTag = -1;
	}
	for (i = 0; i < TOTAL_GAL_SAT; i ++)
	{
		GalSatelliteParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
		GalSatelliteParam[i].PosTimeTag = -1;
	}
	for (i = 0; i < TOTAL_GLO_SAT; i ++)
	{
		GloSatelliteParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
		GloSatelliteParam[i].PosTimeTag = -1;
	}

	for (i = 1; i <= TOTAL_GPS_SAT; i ++)
		GpsEph[i-1] = NavData.FindEphemeris(GpsSystem, time, i);
	for (i = 1; i <= TOTAL_BDS_SAT; i ++)
		BdsEph[i-1] = NavData.FindEphemeris(BdsSystem, BdsTime, i);
	for (i = 1; i <= TOTAL_GAL_SAT; i ++)
		GalEph[i-1] = NavData.FindEphemeris(GalileoSystem, time, i);
	for (i = 1; i <= TOTAL_GLO_SAT; i ++)
		GloEph[i-1] = NavData.FindGloEphemeris(GlonassTime, i);

	fp = fopen(OutputParam.filename, "w");
	if (fp == NULL)
		return -1;

	GpsSatNumber = (OutputParam.FreqSelect[GpsSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
	BdsSatNumber = (OutputParam.FreqSelect[BdsSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
	GalSatNumber = (OutputParam.FreqSelect[GalileoSystem]) ? GetVisibleSatellite(PosVel, time, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
	GloSatNumber = (OutputParam.FreqSelect[GlonassSystem]) ? GetGlonassVisibleSatellite(PosVel, GlonassTime, OutputParam, GloEph, TOTAL_GLO_SAT, GloEphVisible) : 0;
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
//		RinexHeader.SysObsTypeGps[0] = OBS_TYPE_MASK_ALL; RinexHeader.SysObsTypeGps[1] = RinexHeader.SysObsTypeGps[2] = 0x0;
//		RinexHeader.SysObsTypeGlonass[0] = OBS_TYPE_MASK_ALL; RinexHeader.SysObsTypeGlonass[1] = RinexHeader.SysObsTypeGlonass[2] = 0x0;
//		RinexHeader.SysObsTypeBds[0] = OBS_TYPE_MASK_ALL | OBS_CHANNEL_P; RinexHeader.SysObsTypeBds[1] = RinexHeader.SysObsTypeBds[2] = 0x0;
//		RinexHeader.SysObsTypeGalileo[0] = OBS_TYPE_MASK_ALL | OBS_CHANNEL_GAL_E1C; RinexHeader.SysObsTypeGalileo[1] = RinexHeader.SysObsTypeGalileo[2] = 0x0;
		SetSysObsType(0, RinexHeader.SysObsTypeGps, OutputParam.FreqSelect[0]);
		SetSysObsType(1, RinexHeader.SysObsTypeBds, OutputParam.FreqSelect[1]);
		SetSysObsType(2, RinexHeader.SysObsTypeGalileo, OutputParam.FreqSelect[2]);
		SetSysObsType(3, RinexHeader.SysObsTypeGlonass, OutputParam.FreqSelect[3]);
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
			GetSatelliteParam(PosVel, CurPos, time, GpsSystem, GpsEphVisible[i], NavData.GetGpsIono(), &GpsSatelliteParam[index]);
			GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GpsSatelliteParam[index]);
			CalcObservation(Obs, &GpsSatelliteParam[index], OutputParam.FreqSelect[0]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < BdsSatNumber; i ++)
		{
			index = BdsEphVisible[i]->svid - 1;
			GetSatelliteParam(PosVel, CurPos, time, BdsSystem, BdsEphVisible[i], NavData.GetGpsIono(), &BdsSatelliteParam[index]);
			GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &BdsSatelliteParam[index]);
			CalcObservation(Obs, &BdsSatelliteParam[index], OutputParam.FreqSelect[1]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < GalSatNumber; i ++)
		{
			index = GalEphVisible[i]->svid - 1;
			GetSatelliteParam(PosVel, CurPos, time, GalileoSystem, GalEphVisible[i], NavData.GetGpsIono(), &GalSatelliteParam[index]);
			GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GalSatelliteParam[index]);
			CalcObservation(Obs, &GalSatelliteParam[index], OutputParam.FreqSelect[2]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < GloSatNumber; i ++)
		{
			index = GloEphVisible[i]->n - 1;
			GetSatelliteParam(PosVel, CurPos, time, GlonassSystem, (PGPS_EPHEMERIS)GloEphVisible[i], NavData.GetGpsIono(), &GloSatelliteParam[index]);
			GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GloSatelliteParam[index]);
			CalcObservation(Obs, &GloSatelliteParam[index], OutputParam.FreqSelect[3]);
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
			GlonassTime = UtcToGlonassTime(UtcTime);
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
				GetSatelliteParam(PosVel, CurPos, time, GpsSystem, GpsEphVisible[i], NavData.GetGpsIono(), &GpsSatelliteParam[index]);
				GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GpsSatelliteParam[index]);
				CalcObservation(Obs, &GpsSatelliteParam[index], OutputParam.FreqSelect[0]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < BdsSatNumber; i ++)
			{
				index = BdsEphVisible[i]->svid - 1;
				GetSatelliteParam(PosVel, CurPos, time, BdsSystem, BdsEphVisible[i], NavData.GetGpsIono(), &BdsSatelliteParam[index]);
				GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &BdsSatelliteParam[index]);
				CalcObservation(Obs, &BdsSatelliteParam[index], OutputParam.FreqSelect[1]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < GalSatNumber; i ++)
			{
				index = GalEphVisible[i]->svid - 1;
				GetSatelliteParam(PosVel, CurPos, time, GalileoSystem, GalEphVisible[i], NavData.GetGpsIono(), &GalSatelliteParam[index]);
				GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GalSatelliteParam[index]);
				CalcObservation(Obs, &GalSatelliteParam[index], OutputParam.FreqSelect[2]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < GloSatNumber; i ++)
			{
				index = GloEphVisible[i]->n - 1;
				GetSatelliteParam(PosVel, CurPos, time, GlonassSystem, (PGPS_EPHEMERIS)GloEphVisible[i], NavData.GetGpsIono(), &GloSatelliteParam[index]);
				GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GloSatelliteParam[index]);
				CalcObservation(Obs, &GloSatelliteParam[index], OutputParam.FreqSelect[3]);
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

void CalcObservation(PSAT_OBSERVATION Obs, PSATELLITE_PARAM SatParam, unsigned int FreqSelect)
{
	int i;

	Obs->system = SatParam->system;
	Obs->svid = SatParam->svid;
	Obs->ValidMask = FreqSelect;
	for (i = 0; i < MAX_OBS_FREQ; i ++)
	{
		if ((FreqSelect & (1 << i)) == 0)
			continue;
		Obs->PseudoRange[i] = GetTravelTime(SatParam, i) * LIGHT_SPEED;
		Obs->CarrierPhase[i] = GetCarrierPhase(SatParam, i);
		Obs->Doppler[i] = GetDoppler(SatParam, i);
		Obs->CN0[i] = SatParam->CN0 / 100.;
	}
}

void SetSysObsType(int system, unsigned int ObsType[], unsigned int FreqSelect)
{
	int MaxFreqIndex[4] = { 3, 6, 5, 3 };
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
		case 0:	// GPS
			ObsType[ObsTypeIndex] |= (i == 0) ? OBS_CHANNEL_GPS_CA : (i == 1) ? OBS_CHANNEL_GPS_L2CL : OBS_CHANNEL_Q;
			break;
		case 1:	// BDS
			ObsType[ObsTypeIndex] |= ((i >= 1) && (i <= 3)) ? OBS_CHANNEL_I : OBS_CHANNEL_P;
			break;
		case 2:	// GAL
			ObsType[ObsTypeIndex] |= ((i == 0) || (i == 4)) ? OBS_CHANNEL_GAL_E1C : OBS_CHANNEL_Q;
			break;
		case 3:	// GLO
			ObsType[ObsTypeIndex] |= OBS_CHANNEL_GLO_CA;
			break;
		}
		ObsTypeIndex ++;
	}
}
