//----------------------------------------------------------------------
// Coordinate.h:
//   Coordinate related functions (Ephemeris to position, Geometry distance etc.)
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__COORDINATE_H__)
#define __COORDINATE_H__

#include "BasicTypes.h"

double GpsClockCorrection(PGPS_EPHEMERIS Eph, double TransmitTime);
double GlonassClockCorrection(PGLONASS_EPHEMERIS Eph, double TransmitTime);
bool GpsSatPosSpeedEph(GnssSystem system, double TransmitTime, PGPS_EPHEMERIS pEph, PKINEMATIC_INFO pPosVel, double Acc[3]);
bool GlonassSatPosSpeedEph(double TransmitTime, PGLONASS_EPHEMERIS pEph, PKINEMATIC_INFO pPosVel, double Acc[3]);
LLA_POSITION EcefToLla(KINEMATIC_INFO ecef_pos);
KINEMATIC_INFO LlaToEcef(LLA_POSITION lla_pos);
CONVERT_MATRIX CalcConvMatrix(KINEMATIC_INFO Position);
CONVERT_MATRIX CalcConvMatrix(LLA_POSITION Position);
void SpeedEnuToCourse(LOCAL_SPEED &Speed);
void SpeedCourseToEnu(LOCAL_SPEED &Speed);
void SpeedEcefToLocal(CONVERT_MATRIX ConvertMatrix, KINEMATIC_INFO PosVel, LOCAL_SPEED &Speed);
void SpeedLocalToEcef(CONVERT_MATRIX ConvertMatrix, LOCAL_SPEED Speed, KINEMATIC_INFO &PosVel);
void SpeedLocalToEcef(LLA_POSITION lla_pos, LOCAL_SPEED Speed, KINEMATIC_INFO &PosVel);
void SatElAz(PLLA_POSITION PositionLla, double LosVector[3], double *Elevation, double *Azimuth);
void SatElAz(PKINEMATIC_INFO Receiver, PKINEMATIC_INFO Satellite, double *Elevation, double *Azimuth);
double GeometryDistance(const double *UserPos, const double *SatPos, double LosVector[3]);
double GeometryDistance(const PKINEMATIC_INFO UserPosVel, const PKINEMATIC_INFO SatPosVel, double LosVector[3]);
double SatRelativeSpeed(PKINEMATIC_INFO Receiver, PKINEMATIC_INFO Satellite);
double GpsIonoDelay(PIONO_PARAM IonoParam, double time, double Lat, double Lon, double Elevation, double Azimuth);
double TropoDelay(double Lat, double Altitude, double Elevation);

#endif //!defined(__COORDINATE_H__)
