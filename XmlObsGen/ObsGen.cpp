#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ConstVal.h"
#include "BasicTypes.h"
#include "Trajectory.h"
#include "XmlElement.h"
#include "GnssTime.h"
#include "XmlInterpreter.h"
#include "NavData.h"
#include "Coordinate.h"
#include "SatelliteParam.h"
#include "Rinex.h"

#define TOTAL_GPS_SAT 32
#define TOTAL_BDS_SAT 63
#define TOTAL_GAL_SAT 50

void CalcObservation(PSAT_OBSERVATION Obs, PSATELLITE_PARAM SatParam);

void main(void)
{
	int i, index;
	GNSS_TIME time;
	UTC_TIME UtcTime;
	CTrajectory Trajectory;
	CNavData NavData;
	CPowerControl PowerControl;
	LLA_POSITION StartPos, CurPos;
	LOCAL_SPEED StartVel;
	KINEMATIC_INFO PosVel;
	FILE *fp;
	int GpsSatNumber, BdsSatNumber, GalSatNumber;
	PGPS_EPHEMERIS GpsEph[TOTAL_GPS_SAT], GpsEphVisible[TOTAL_GPS_SAT];
	PGPS_EPHEMERIS BdsEph[TOTAL_BDS_SAT], BdsEphVisible[TOTAL_BDS_SAT];
	PGPS_EPHEMERIS GalEph[TOTAL_GAL_SAT], GalEphVisible[TOTAL_GAL_SAT];
	OUTPUT_PARAM OutputParam;
	SATELLITE_PARAM GpsSatelliteParam[TOTAL_GPS_SAT], BdsSatelliteParam[TOTAL_BDS_SAT], GalSatelliteParam[TOTAL_GAL_SAT];
	int ObservationNumber;
	SAT_OBSERVATION Observations[TOTAL_GPS_SAT+TOTAL_BDS_SAT+TOTAL_GAL_SAT], *Obs;
	RINEX_HEADER RinexHeader;
	int ListCount;
	PSIGNAL_POWER PowerList;

	CXmlElementTree XmlTree;
	CXmlElement *RootElement, *Element;

	XmlTree.parse("test_obs2.xml");
	RootElement = XmlTree.getroot();
	i = 0;
	while ((Element = RootElement->GetElement(i ++)) != NULL)
	{
		if (strcmp(Element->GetTag(), "Time") == 0)
			AssignStartTime(Element, UtcTime);
		else if (strcmp(Element->GetTag(), "Trajectory") == 0)
			SetTrajectory(Element, StartPos, StartVel, Trajectory);
		else if (strcmp(Element->GetTag(), "Ephemeris") == 0)
			NavData.ReadNavFile(Element->GetText());
		else if (strcmp(Element->GetTag(), "Output") == 0)
			SetOutputParam(Element, OutputParam);
		else if (strcmp(Element->GetTag(), "PowerControl") == 0)
			SetPowerControl(Element, PowerControl);
	}

	Trajectory.ResetTrajectoryTime();
	PosVel = LlaToEcef(StartPos);
	CurPos = StartPos;
	SpeedLocalToEcef(StartPos, StartVel, PosVel);
	time = UtcToGpsTime(UtcTime);
	UtcTime = GpsTimeToUtc(time, FALSE);
	PowerControl.ResetTime();

	memset(GpsSatelliteParam, 0, sizeof(GpsSatelliteParam));
	memset(BdsSatelliteParam, 0, sizeof(BdsSatelliteParam));
	memset(GalSatelliteParam, 0, sizeof(GalSatelliteParam));
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

	for (i = 1; i <= TOTAL_GPS_SAT; i ++)
		GpsEph[i-1] = NavData.FindEphemeris(GpsSystem, time, i);
	for (i = 1; i <= TOTAL_BDS_SAT; i ++)
		BdsEph[i-1] = NavData.FindEphemeris(BdsSystem, time, i);
	for (i = 1; i <= TOTAL_GAL_SAT; i ++)
		GalEph[i-1] = NavData.FindEphemeris(GalileoSystem, time, i);


	fp = fopen(OutputParam.filename, "w");
	if (fp == NULL)
		return;

	GpsSatNumber = (OutputParam.SystemSelect & (1 << GpsSystem)) ? GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
	BdsSatNumber = (OutputParam.SystemSelect & (1 << BdsSystem)) ? GetVisibleSatellite(PosVel, time, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
	GalSatNumber = (OutputParam.SystemSelect & (1 << GalileoSystem)) ? GetVisibleSatellite(PosVel, time, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
#if 1
	if (OutputParam.Format == OutputFormatRinex)
	{
		RinexHeader.HeaderFlag = 0;
		RinexHeader.MajorVersion = 3;
		RinexHeader.MinorVersion= 3;
		RinexHeader.HeaderFlag |= RINEX_HEADER_PGM | RINEX_HEADER_APPROX_POS;
		strncpy(RinexHeader.Program, "OBSGEN", 20);
		RinexHeader.ApproxPos[0] = PosVel.x;
		RinexHeader.ApproxPos[1] = PosVel.y;
		RinexHeader.ApproxPos[2] = PosVel.z;
		RinexHeader.SysObsTypeGps = 0xf;
		RinexHeader.SysObsTypeGlonass = 0x0;
		RinexHeader.SysObsTypeBds = 0xf;
		RinexHeader.SysObsTypeGalileo = 0xf;
		RinexHeader.Interval = OutputParam.Interval / 1000.;
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
			CalcObservation(Obs, &GpsSatelliteParam[index]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < BdsSatNumber; i ++)
		{
			index = BdsEphVisible[i]->svid - 1;
			GetSatelliteParam(PosVel, CurPos, time, BdsSystem, BdsEphVisible[i], NavData.GetGpsIono(), &BdsSatelliteParam[index]);
			GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &BdsSatelliteParam[index]);
			CalcObservation(Obs, &BdsSatelliteParam[index]);
			Obs ++;
			ObservationNumber ++;
		}
		for (i = 0; i < GalSatNumber; i ++)
		{
			index = GalEphVisible[i]->svid - 1;
			GetSatelliteParam(PosVel, CurPos, time, GalileoSystem, GalEphVisible[i], NavData.GetGpsIono(), &GalSatelliteParam[index]);
			GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GalSatelliteParam[index]);
			CalcObservation(Obs, &GalSatelliteParam[index]);
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
		if (UtcTime.Minute == 5 && UtcTime.Second == 55.3)
			UtcTime.Minute = UtcTime.Minute;
		CurPos = EcefToLla(PosVel);
		if ((time.MilliSeconds % 60000) == 0)	// recalculate visible satellite at minute boundary
		{
			GpsSatNumber = (OutputParam.SystemSelect & (1 << GpsSystem)) ? GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, GpsEph, TOTAL_GPS_SAT, GpsEphVisible) : 0;
			BdsSatNumber = (OutputParam.SystemSelect & (1 << BdsSystem)) ? GetVisibleSatellite(PosVel, time, OutputParam, BdsSystem, BdsEph, TOTAL_BDS_SAT, BdsEphVisible) : 0;
			GalSatNumber = (OutputParam.SystemSelect & (1 << GalileoSystem)) ? GetVisibleSatellite(PosVel, time, OutputParam, GalileoSystem, GalEph, TOTAL_GAL_SAT, GalEphVisible) : 0;
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
				CalcObservation(Obs, &GpsSatelliteParam[index]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < BdsSatNumber; i ++)
			{
				index = BdsEphVisible[i]->svid - 1;
				GetSatelliteParam(PosVel, CurPos, time, BdsSystem, BdsEphVisible[i], NavData.GetGpsIono(), &BdsSatelliteParam[index]);
				GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &BdsSatelliteParam[index]);
				CalcObservation(Obs, &BdsSatelliteParam[index]);
				Obs ++;
				ObservationNumber ++;
			}
			for (i = 0; i < GalSatNumber; i ++)
			{
				index = GalEphVisible[i]->svid - 1;
				GetSatelliteParam(PosVel, CurPos, time, GalileoSystem, GalEphVisible[i], NavData.GetGpsIono(), &GalSatelliteParam[index]);
				GetSatelliteCN0(ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, &GalSatelliteParam[index]);
				CalcObservation(Obs, &GalSatelliteParam[index]);
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

void CalcObservation(PSAT_OBSERVATION Obs, PSATELLITE_PARAM SatParam)
{
	Obs->system = SatParam->system;
	Obs->svid = SatParam->svid;
	Obs->PseudoRange = GetTravelTime(SatParam, 0) * LIGHT_SPEED;
	Obs->CarrierPhase = GetCarrierPhase(SatParam, 0);
//	Obs->PseudoRange = (SatParam->TravelTime + SatParam->GroupDelay[0]) * LIGHT_SPEED + SatParam->IonoDelay;
//	Obs->CarrierPhase = (SatParam->TravelTime + SatParam->GroupDelay[0]) * 1575.42e6 - SatParam->IonoDelay / WAVELENGTH_GPSL1;
	Obs->Doppler = GetDoppler(SatParam, 0);
	Obs->CN0 = SatParam->CN0 / 100.;
}
