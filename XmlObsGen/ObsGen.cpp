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

#define WAVELENGTH_GPSL1 0.19029367279836488047631742646405

extern INTERPRETE_PARAM RootInterpretParam;

void main(void)
{
	int i;
	GNSS_TIME time;
	UTC_TIME UtcTime;
	CTrajectory Trajectory;
	CNavData NavData;
	CPowerControl PowerControl;
	LLA_POSITION StartPos, CurPos;
	LOCAL_SPEED StartVel;
	KINEMATIC_INFO PosVel;
	FILE *fp;
	int GpsSatNumber;
	PGPS_EPHEMERIS Eph[32], EphVisible[32];
	OUTPUT_PARAM OutputParam;
	SATELLITE_PARAM SatelliteParam[32];
	SAT_OBSERVATION GpsObservation[32];
	int GpsCN0;
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
//	ProcessElement(RootElement, &RootInterpretParam);

	Trajectory.ResetTrajectoryTime();
	PosVel = LlaToEcef(StartPos);
	CurPos = StartPos;
	SpeedLocalToEcef(StartPos, StartVel, PosVel);
	time = UtcToGpsTime(UtcTime);
	UtcTime = GpsTimeToUtc(time, FALSE);
	PowerControl.ResetTime();
	for (i = 0; i < 32; i ++)
		SatelliteParam[i].CN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);

	for (i = 1; i <= 32; i ++)
		Eph[i-1] = NavData.FindEphemeris(CNavData::SystemGps, time, i);


	fp = fopen(OutputParam.filename, "w");
	if (fp == NULL)
		return;

	GpsSatNumber = GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, Eph, 32, EphVisible);
#if 1
	if (OutputParam.Format == OutputFormatRinex)
	{
		RinexHeader.HeaderFlag = 0;
		RinexHeader.MajorVersion = 3;
		RinexHeader.MinorVersion= 3;
		RinexHeader.HeaderFlag |= RINEX_HEADER_PGM;
		strncpy(RinexHeader.Program, "OBSGEN", 20);
		RinexHeader.SysObsTypeGps = 0xf;
		RinexHeader.SysObsTypeGlonass = 0x0;
		RinexHeader.SysObsTypeBds = 0x0;
		RinexHeader.SysObsTypeGalileo = 0x0;
		RinexHeader.Interval = 1.0;
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
		ListCount = PowerControl.GetPowerControlList(0, PowerList);
		for (i = 0; i < GpsSatNumber; i ++)
		{
			GpsCN0 = (int)(PowerControl.InitCN0 * 100 + 0.5);
			SatelliteParam[i] = GetSatelliteParam(PosVel, CurPos, time, GpsSystem, EphVisible[i], NavData.GetGpsIono());
			SatelliteParam[i].CN0 = GetSatelliteCN0(GpsCN0, ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, SatelliteParam[i].svid, SatelliteParam[i].Elevation);
			GpsObservation[i].system = 0;
			GpsObservation[i].svid = SatelliteParam[i].svid;
			GpsObservation[i].PseudoRange = SatelliteParam[i].TravelTime * LIGHT_SPEED + SatelliteParam[i].IonoDelay;
			GpsObservation[i].CarrierPhase = SatelliteParam[i].TravelTime * 1575.42e6 - SatelliteParam[i].IonoDelay / WAVELENGTH_GPSL1;
			GpsObservation[i].Doppler = -SatelliteParam[i].RelativeSpeed / WAVELENGTH_GPSL1;
			GpsObservation[i].CN0 = SatelliteParam[i].CN0 / 100.;
		}
		OutputObservation(fp, UtcTime, GpsSatNumber, GpsObservation);
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
		if (OutputParam.Format == OutputFormatRinex)
		{
			ListCount = PowerControl.GetPowerControlList(OutputParam.Interval, PowerList);
			for (i = 0; i < GpsSatNumber; i ++)
			{
				GpsCN0 = SatelliteParam[i].CN0;
				SatelliteParam[i] = GetSatelliteParam(PosVel, CurPos, time, GpsSystem, EphVisible[i], NavData.GetGpsIono());
				SatelliteParam[i].CN0 = GetSatelliteCN0(GpsCN0, ListCount, PowerList, PowerControl.InitCN0, PowerControl.Adjust, SatelliteParam[i].svid, SatelliteParam[i].Elevation);
				GpsObservation[i].system = 0;
				GpsObservation[i].svid = SatelliteParam[i].svid;
				GpsObservation[i].PseudoRange = SatelliteParam[i].TravelTime * LIGHT_SPEED + SatelliteParam[i].IonoDelay;
				GpsObservation[i].CarrierPhase = SatelliteParam[i].TravelTime * 1575.42e6 - SatelliteParam[i].IonoDelay / WAVELENGTH_GPSL1;
				GpsObservation[i].Doppler = -SatelliteParam[i].RelativeSpeed / WAVELENGTH_GPSL1;
				GpsObservation[i].CN0 = SatelliteParam[i].CN0 / 100.;
			}
			OutputObservation(fp, UtcTime, GpsSatNumber, GpsObservation);
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
