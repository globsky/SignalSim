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

#define WAVELENGTH_GPSL1 0.19029367279836488047631742646405

typedef struct
{
	double TravelTime;
	double IonoDelay;
	double RelativeSpeed;
	double Elevation;
	double Azimuth;
} SATELLITE_INFO, *PSATELLITE_INFO;

static int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[]);
static SATELLITE_INFO GetTravelTime(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam);
extern INTERPRETE_PARAM RootInterpretParam;

void main(void)
{
	int i;
	GNSS_TIME time;
	UTC_TIME UtcTime;
	CTrajectory Trajectory;
	CNavData NavData;
	LLA_POSITION StartPos, CurPos;
	LOCAL_SPEED StartVel;
	KINEMATIC_INFO PosVel;
	FILE *fp;
	int GpsSatNumber;
	PGPS_EPHEMERIS Eph[32], EphVisible[32];
	OUTPUT_PARAM OutputParam;
	SATELLITE_INFO SatelliteInfo;

	CXmlElementTree XmlTree;
	CXmlElement *RootElement, *Element;

	XmlTree.parse("test_obs.xml");
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
	}
//	ProcessElement(RootElement, &RootInterpretParam);

	Trajectory.ResetTrajectoryTime();
	PosVel = LlaToEcef(StartPos);
	CurPos = StartPos;
	SpeedLocalToEcef(StartPos, StartVel, PosVel);
	time = UtcToGpsTime(UtcTime);
	UtcTime = GpsTimeToUtc(time, FALSE);

	for (i = 1; i <= 32; i ++)
		Eph[i-1] = NavData.FindEphemeris(CNavData::SystemGps, time, i);


	fp = fopen(OutputParam.filename, "w");
	if (fp == NULL)
		return;

	GpsSatNumber = GetVisibleSatellite(PosVel, time, OutputParam, GpsSystem, Eph, 32, EphVisible);
#if 1
	if (OutputParam.Format == OutputFormatRinex)
	{
		fprintf(fp, "     3.03           OBSERVATION DATA    M (MIXED)           RINEX VERSION / TYPE\n");
		fprintf(fp, "G    4 C1C L1C D1C S1C                                      SYS / # / OBS TYPES \n");
		fprintf(fp, "                                                            END OF HEADER       \n");
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
		fprintf(fp, "> %4d %02d %02d %02d %02d %10.7f  0 %2d      -0.000000000000\n",
			UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second, GpsSatNumber);
		for (i = 0; i < GpsSatNumber; i ++)
		{
			SatelliteInfo = GetTravelTime(PosVel, CurPos, time, GpsSystem, EphVisible[i], NavData.GetGpsIono());
			fprintf(fp, "G%02d %13.3f 8 %13.3f 8 %13.3f          48.000\n", EphVisible[i]->svid,
				SatelliteInfo.TravelTime * LIGHT_SPEED + SatelliteInfo.IonoDelay,
				SatelliteInfo.TravelTime * 1575.42e6 - SatelliteInfo.IonoDelay / WAVELENGTH_GPSL1,
				(-SatelliteInfo.RelativeSpeed + LIGHT_SPEED * EphVisible[i]->af1) / WAVELENGTH_GPSL1);
		}
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
	while (Trajectory.GetNextPosVelECEF(OutputParam.Interval, PosVel))
	{
		time.Seconds += OutputParam.Interval;
		UtcTime = GpsTimeToUtc(time, FALSE);
		CurPos = EcefToLla(PosVel);
		if (OutputParam.Format == OutputFormatRinex)
		{
			fprintf(fp, "> %4d %02d %02d %02d %02d %10.7f  0 %2d      -0.000000000000\n",
				UtcTime.Year, UtcTime.Month, UtcTime.Day, UtcTime.Hour, UtcTime.Minute, UtcTime.Second, GpsSatNumber);
			for (i = 0; i < GpsSatNumber; i ++)
			{
				SatelliteInfo = GetTravelTime(PosVel, CurPos, time, GpsSystem, EphVisible[i], NavData.GetGpsIono());
				fprintf(fp, "G%02d %13.3f 8 %13.3f 8 %13.3f          48.000\n", EphVisible[i]->svid,
					SatelliteInfo.TravelTime * LIGHT_SPEED + SatelliteInfo.IonoDelay,
					SatelliteInfo.TravelTime * 1575.42e6 - SatelliteInfo.IonoDelay / WAVELENGTH_GPSL1,
					(-SatelliteInfo.RelativeSpeed + LIGHT_SPEED * EphVisible[i]->af1) / WAVELENGTH_GPSL1);
			}
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

int GetVisibleSatellite(KINEMATIC_INFO Position, GNSS_TIME time, OUTPUT_PARAM OutputParam, GnssSystem system, PGPS_EPHEMERIS Eph[], int Number, PGPS_EPHEMERIS EphVisible[])
{
	int i;
	int SatNumber = 0;
	KINEMATIC_INFO SatPosition;
	double Elevation, Azimuth;

	for (i = 0; i < Number; i ++)
	{
		if (Eph[i] == NULL || Eph[i]->flag == 0 || Eph[i]->health != 0)
			continue;
		if (OutputParam.GpsMaskOut & (1 << (i-1)))
			continue;
		if (!GpsSatPosSpeedEph(system, time.Seconds, Eph[i], &SatPosition))
			continue;
		SatElAz(&Position, &SatPosition, &Elevation, &Azimuth);
		if (Elevation < OutputParam.ElevationMask)
			continue;
		EphVisible[SatNumber ++] = Eph[i];
	}

	return SatNumber;
}

SATELLITE_INFO GetTravelTime(KINEMATIC_INFO PositionEcef, LLA_POSITION PositionLla, GNSS_TIME time, GnssSystem system, PGPS_EPHEMERIS Eph, PIONO_PARAM IonoParam)
{
	KINEMATIC_INFO SatPosition;
	double Distance, TravelTime, SatelliteTime = time.Seconds;
	double Elevation, Azimuth;
	double LosVector[3];
	SATELLITE_INFO SatelliteInfo;

	// first estimate the travel time, ignore tgd, ionosphere and troposphere delay
	GpsSatPosSpeedEph(system, time.Seconds, Eph, &SatPosition);
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, NULL) / LIGHT_SPEED;
	SatPosition.x -= TravelTime * SatPosition.vx; SatPosition.y -= TravelTime * SatPosition.vy; SatPosition.z -= TravelTime * SatPosition.vz;
	TravelTime = GeometryDistance(&PositionEcef, &SatPosition, NULL) / LIGHT_SPEED;
	SatelliteTime -= TravelTime;

	// calculate accurate transmit time
	// tt = rt - (d + diono + dtrop)/c + tgd + dts
	GpsSatPosSpeedEph(system, SatelliteTime, Eph, &SatPosition);
	Distance = GeometryDistance(&PositionEcef, &SatPosition, LosVector);
	SatElAz(&PositionLla, LosVector, &Elevation, &Azimuth);
//	Distance += GpsIonoDelay(IonoParam, SatelliteTime, PositionLla.lat, PositionLla.lon, Elevation, Azimuth);
	SatelliteInfo.IonoDelay = GpsIonoDelay(IonoParam, SatelliteTime, PositionLla.lat, PositionLla.lon, Elevation, Azimuth);
	Distance += TropoDelay(PositionLla.lat, PositionLla.alt, Elevation);
	TravelTime = Distance / LIGHT_SPEED + Eph->tgd - GpsClockCorrection(Eph, SatelliteTime);
	TravelTime -= WGS_F_GTR * Eph->ecc * Eph->sqrtA * sin(Eph->Ek);		// relativity correction
	SatelliteInfo.TravelTime = TravelTime;
	SatelliteInfo.Elevation = Elevation;
	SatelliteInfo.Azimuth = Azimuth;
	SatelliteInfo.RelativeSpeed = SatRelativeSpeed(&PositionEcef, &SatPosition);

	return SatelliteInfo;
}
