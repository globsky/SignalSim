//----------------------------------------------------------------------
// Coordinate.cpp:
//   Implementation of coordinate related functions
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <math.h>

#include "ConstVal.h"
#include "BasicTypes.h"
#include "Coordinate.h"

#define COARSE_STEP 30
#define COS_5 0.99619469809174553
#define SIN_5 0.087155742747658173559

static void RungeKutta(double h, double State[9]);
static void CalcAcceleration(double *State, double *Acc);
static void PredictState(double *State, double *State1, double *VelAcc, double Step);
static void CisToCts(double *State, double DeltaT, PKINEMATIC_INFO pCtsPos, double *Acc);

double GpsClockCorrection(PGPS_EPHEMERIS Eph, double TransmitTime)
{
	double TimeDiff = TransmitTime - Eph->toc;
	double ClockAdj;

	// protection for time ring back at week end
	if (TimeDiff > 302400.0)
		TimeDiff -= 604800;
	if (TimeDiff < -302400.0)
		TimeDiff += 604800;

	ClockAdj = Eph->af0 + (Eph->af1 + Eph->af2 * TimeDiff) * TimeDiff;
	ClockAdj *= (1 - Eph->af1);	// adjustment to time

	return ClockAdj;
}

double GlonassClockCorrection(PGLONASS_EPHEMERIS Eph, double TransmitTime)
{
	double TimeDiff = TransmitTime - (double)Eph->tb;

	if (TimeDiff > 43200.0)
		TimeDiff -= 86400.0;
	else if (TimeDiff < -43200.0)
		TimeDiff += 86400.0;

	return -Eph->tn + Eph->gamma * TimeDiff;
}

bool GpsSatPosSpeedEph(GnssSystem system, double TransmitTime, PGPS_EPHEMERIS pEph, PKINEMATIC_INFO pPosVel, double Acc[3])
{
	int i;
	double delta_t;
	double Mk, Ek, Ek1;
	double phi, phi_dot;
	double uk, rk, ik;
	double duk, drk, dik;
	double uk_dot, rk_dot, ik_dot;
	double duk_dot, drk_dot, dik_dot;
	double xp, yp, omega;
	double xp_dot, yp_dot;
	double xp_dot2, yp_dot2;
	double sin_temp, cos_temp;
	double Ek_dot2, phi_dot2;
	double uk_dot2, rk_dot2, ik_dot2;
	double alpha, beta;

	// calculate time difference
	delta_t = TransmitTime - pEph->toe;
	// protection for time ring back at week end
	if (delta_t > 302400.0)
		delta_t -= 604800;
	if (delta_t < -302400.0)
		delta_t += 604800;

	// get Ek from Mk with recursive algorithm
	alpha = pEph->delta_n_dot * delta_t;
	Ek1 = Ek = Mk = pEph->M0 + ((pEph->n + alpha / 2) * delta_t);
	for (i = 0; i < 10; i ++)
	{
		Ek = Mk + pEph->ecc * sin(Ek);
		if (fabs(Ek - Ek1) < 1e-14)
			break;
		Ek1 = Ek;
	}
	pEph->Ek = Ek;

	// assign Ek1 as 1-e*cos(Ek)
	Ek1 = 1.0 - (pEph->ecc * cos(Ek));

	// get phi(k) with atan2
	phi = atan2(pEph->root_ecc * sin(Ek), cos(Ek) - pEph->ecc) + pEph->w;
	sin_temp = sin(phi + phi);
	cos_temp = cos(phi + phi);

	// get u(k), r(k) and i(k)
	uk = phi;
	rk = (pEph->axis + pEph->axis_dot * delta_t) * Ek1;
	ik = pEph->i0 + (pEph->idot * delta_t);
	// apply 2nd order correction to u(k), r(k) and i(k)
	duk = (pEph->cuc * cos_temp) + (pEph->cus * sin_temp);
	drk = (pEph->crc * cos_temp) + (pEph->crs * sin_temp);
	dik = (pEph->cic * cos_temp) + (pEph->cis * sin_temp);
	uk += duk;
	rk += drk;
	ik += dik;
	// calculate derivatives of r(k) and u(k)
	pEph->Ek_dot = (pEph->n + alpha) / Ek1;
	uk_dot = phi_dot = pEph->Ek_dot * pEph->root_ecc / Ek1;
	phi_dot = phi_dot * 2.0;
	rk_dot = pEph->axis * pEph->ecc * sin(Ek) * pEph->Ek_dot + pEph->axis_dot * Ek1;
	drk_dot = ((pEph->crs * cos_temp) - (pEph->crc * sin_temp)) * phi_dot;
	duk_dot = ((pEph->cus * cos_temp) - (pEph->cuc * sin_temp)) * phi_dot;
	dik_dot = ((pEph->cis * cos_temp) - (pEph->cic * sin_temp)) * phi_dot;
	rk_dot += drk_dot;
	uk_dot += duk_dot;
	ik_dot = pEph->idot + dik_dot;
	// calculate intermediate variables for acceleration
	if (Acc)
	{
		Ek_dot2 = -pEph->Ek_dot * pEph->Ek_dot * pEph->ecc * sin(Ek) / Ek1;
		phi_dot2 = 2 * Ek_dot2 * pEph->root_ecc / Ek1;
		alpha = 2 * phi_dot2 / phi_dot;	// phi_dot2/phi_dot
		beta = phi_dot * phi_dot;	// 4*phi_dot^2
		rk_dot2 = pEph->axis * pEph->ecc * (sin(Ek) * Ek_dot2 + cos(Ek) * pEph->Ek_dot * pEph->Ek_dot);
		rk_dot2 += alpha * drk_dot - beta * drk;
		uk_dot2 = phi_dot2 + alpha * duk_dot - beta * duk;
		ik_dot2 = alpha * dik_dot - beta * dik;
	}

	// calculate Xp and Yp and corresponding derivatives
	sin_temp = sin(uk);
	cos_temp = cos(uk);
	xp = rk * cos_temp;
	yp = rk * sin_temp;
	xp_dot = rk_dot * cos_temp - yp * uk_dot;
	yp_dot = rk_dot * sin_temp + xp * uk_dot;
	// calculate intermediate variables for acceleration
	if (Acc)
	{
		xp_dot2 = rk_dot2 * cos(uk) - 2 * uk_dot * rk_dot * sin(uk) - uk_dot * uk_dot * xp - uk_dot2 * yp;
		yp_dot2 = rk_dot2 * sin(uk) + 2 * uk_dot * rk_dot * cos(uk) - uk_dot * uk_dot * yp + uk_dot2 * xp;
	}

	// get final position and speed in ECEF coordinate
	omega = pEph->omega_t + pEph->omega_delta * delta_t;
	sin_temp = sin(omega);
	cos_temp = cos(omega);
	phi = sin(ik);
	pPosVel->z = yp * phi;
	pPosVel->vz = yp_dot * phi;

	phi = cos(ik);
	pPosVel->x = xp * cos_temp - yp * phi * sin_temp;
	pPosVel->y = xp * sin_temp + yp * phi * cos_temp;
	// phi_dot assign as yp_dot * cos(ik) - z * ik_dot
	phi_dot = yp_dot * phi - pPosVel->z * ik_dot;
	pPosVel->vx = xp_dot * cos_temp - phi_dot * sin_temp;
	pPosVel->vy = xp_dot * sin_temp + phi_dot * cos_temp;
	pPosVel->vx -= pPosVel->y * pEph->omega_delta;
	pPosVel->vy += pPosVel->x * pEph->omega_delta;
	pPosVel->vz += yp * ik_dot * phi;

	// calculate acceleration if given valid array pointer
	if (Acc)
	{
		alpha = pPosVel->vz * ik_dot + pPosVel->z * ik_dot2 - xp_dot * pEph->omega_delta;
		alpha += yp_dot * ik_dot * sin(ik) - yp_dot2 * cos(ik);
		beta = xp_dot2 + pPosVel->z * ik_dot * pEph->omega_delta - yp_dot * pEph->omega_delta * cos(ik);
		Acc[0] = -pPosVel->vy * pEph->omega_delta + alpha * sin_temp + beta * cos_temp;
		Acc[1] =  pPosVel->vx * pEph->omega_delta - alpha * cos_temp + beta * sin_temp;
		Acc[2] = (yp_dot2 - yp * ik_dot * ik_dot) * sin(ik);
		Acc[2] += (yp * ik_dot2 + 2 * yp_dot * ik_dot) * cos(ik);
	}

	if (system == BdsSystem && pEph->svid <= 5)
	{
		// first rotate -5 degree
		yp = pPosVel->y * COS_5 - pPosVel->z * SIN_5; // rotated y
		pPosVel->z = pPosVel->z * COS_5 + pPosVel->y * SIN_5; // rotated z
		yp_dot = pPosVel->vy * COS_5 - pPosVel->vz * SIN_5; // rotated vy
		pPosVel->vz = pPosVel->vz * COS_5 + pPosVel->vy * SIN_5; // rotated vz
		// rotate delta_t * CGS2000_OMEGDOTE
		omega = CGCS2000_OMEGDOTE * delta_t;
		sin_temp = sin(omega);
		cos_temp = cos(omega);
		pPosVel->y = yp * cos_temp - pPosVel->x * sin_temp;
		pPosVel->x = pPosVel->x * cos_temp + yp * sin_temp;
		pPosVel->vy = yp_dot * cos_temp - pPosVel->vx * sin_temp;
		pPosVel->vx = pPosVel->vx * cos_temp + yp_dot * sin_temp;
		// earth rotate compensation on velocity
		pPosVel->vx += pPosVel->y * CGCS2000_OMEGDOTE;
		pPosVel->vy -= pPosVel->x * CGCS2000_OMEGDOTE;
		if (Acc)
		{
			// first rotate -5 degree
			yp = Acc[1] * COS_5 - Acc[2] * SIN_5; // rotated ay
			Acc[2] = Acc[2] * COS_5 + Acc[1] * SIN_5; // rotated az
			Acc[1] = yp * cos_temp - Acc[0] * sin_temp;
			Acc[0] = Acc[0] * cos_temp + yp * sin_temp;
			// earth rotate compensation on acceleration
			Acc[0] += pPosVel->vy * CGCS2000_OMEGDOTE;
			Acc[1] -= pPosVel->vx * CGCS2000_OMEGDOTE;
		}
	}

	// if ephemeris expire, return 0
	if (fabs(delta_t) > 7200.0)
		return false;
	else if ((fabs(delta_t) > 3600.0) && system == BdsSystem)
		return false;
	else
		return true;
}

bool GlonassSatPosSpeedEph(double TransmitTime, PGLONASS_EPHEMERIS pEph, PKINEMATIC_INFO pPosVel, double Acc[3])
{
	double DeltaT, DeltaT1;
	double State[9];
	int i, StepNumber;

	DeltaT = TransmitTime - (double)pEph->tb;
	if (DeltaT > 43200.0)
		DeltaT -= 86400.0;
	else if (DeltaT < -43200.0)
		DeltaT += 86400.0;

	// delta t correction according to satellite clock error and clock drift
//	DeltaT += (pEph->tn + pEph->gamma * DeltaT);	

	// if position and velocity at tc not yet calculated
	if ((pEph->flag & 0x2) == 0)
	{
		// satellite position and velocity in CIS coordinate
		State[0] = pEph->x;
		State[1] = pEph->y;
		State[2] = pEph->z;
		State[3] = pEph->vx - PZ90_OMEGDOTE * pEph->y;
		State[4] = pEph->vy + PZ90_OMEGDOTE * pEph->x;
		State[5] = pEph->vz;
		State[6] = pEph->ax;
		State[7] = pEph->ay;
		State[8] = pEph->az;

		StepNumber = (int)DeltaT / COARSE_STEP;
		if (StepNumber >= 0)
		{
			for (i = StepNumber; i > 0; i --)
			{
				RungeKutta(COARSE_STEP, State);
			}
		}
		else
		{
			for (i = StepNumber; i < 0; i ++)
			{
				RungeKutta(-COARSE_STEP, State);
			}
		}
		DeltaT1 = DeltaT - StepNumber * COARSE_STEP;
	}
	// prediction from tc
	else
	{
		State[0] = pEph->PosVelT.x;
		State[1] = pEph->PosVelT.y;
		State[2] = pEph->PosVelT.z;
		State[3] = pEph->PosVelT.vx;
		State[4] = pEph->PosVelT.vy;
		State[5] = pEph->PosVelT.vz;
		State[6] = pEph->ax;
		State[7] = pEph->ay;
		State[8] = pEph->az;
		DeltaT1 = TransmitTime - pEph->tc;
		if (DeltaT1 > 43200.0)
			DeltaT1 -= 86400.0;
		else if (DeltaT1 < -43200.0)
			DeltaT1 += 86400.0;

		// delta t correction according to satellite clock error and clock drift
//		DeltaT1 += pEph->gamma * DeltaT1;
	}
	RungeKutta(DeltaT1, State);
	
	pEph->tc = TransmitTime;
	pEph->flag |= 0x2;	// can predict from pEph->tc instead of pEph->tn
	pEph->PosVelT.x = State[0];
	pEph->PosVelT.y = State[1];
	pEph->PosVelT.z = State[2];
	pEph->PosVelT.vx = State[3];
	pEph->PosVelT.vy = State[4];
	pEph->PosVelT.vz = State[5];

	// CIS to CTS(PZ-90) convertion
	CisToCts(State, DeltaT, pPosVel, Acc);

	return true;
}

LLA_POSITION EcefToLla(KINEMATIC_INFO ecef_pos)
{
	double p;
	double theta;
	double n;
	LLA_POSITION lla_pos;
	
	p = sqrt(ecef_pos.x * ecef_pos.x + ecef_pos.y * ecef_pos.y);

	if( p < 1e-10)	// north or south pole
	{
		lla_pos.lon = 0;
		lla_pos.lat = PI / 2;
		lla_pos.alt = (ecef_pos.z > 0) ? ecef_pos.z - WGS_AXIS_B : -ecef_pos.z - WGS_AXIS_B;
		return lla_pos;
	}

	theta = atan(ecef_pos.z * WGS_AXIS_A / (p * WGS_AXIS_B));
	lla_pos.lat = atan((ecef_pos.z + WGS_E2_SQR * WGS_AXIS_B * pow(sin(theta), 3)) /
								  (p - WGS_E1_SQR * WGS_AXIS_A * pow(cos(theta), 3)));
	lla_pos.lon = atan2(ecef_pos.y, ecef_pos.x);
	
	n = WGS_AXIS_A / sqrt(1.0 - WGS_E1_SQR * sin(lla_pos.lat) * sin(lla_pos.lat));
	lla_pos.alt = p / cos(lla_pos.lat) - n;

	return lla_pos;
}

KINEMATIC_INFO LlaToEcef(LLA_POSITION lla_pos)
{
	KINEMATIC_INFO ecef_pos;
	double n = WGS_AXIS_A / sqrt(1.0L - WGS_E1_SQR * sin(lla_pos.lat) * sin(lla_pos.lat));

	ecef_pos.x = (n + lla_pos.alt) * cos (lla_pos.lat) * cos (lla_pos.lon);
	ecef_pos.y = (n + lla_pos.alt) * cos (lla_pos.lat) * sin (lla_pos.lon);
	ecef_pos.z = (n * (1.0 - WGS_E1_SQR) + lla_pos.alt) * sin (lla_pos.lat);

	return ecef_pos;
}

CONVERT_MATRIX CalcConvMatrix(KINEMATIC_INFO Position)
{
	CONVERT_MATRIX ConvertMatrix;
	LLA_POSITION PosLla = EcefToLla(Position);

	ConvertMatrix.x2e = -sin(PosLla.lon);
	ConvertMatrix.y2e = cos(PosLla.lon);
	ConvertMatrix.x2n = -sin(PosLla.lat) * cos(PosLla.lon);
	ConvertMatrix.y2n = -sin(PosLla.lat) * sin(PosLla.lon);
	ConvertMatrix.z2n = cos(PosLla.lat);
	ConvertMatrix.x2u = cos(PosLla.lat) * cos(PosLla.lon);
	ConvertMatrix.y2u = cos(PosLla.lat) * sin(PosLla.lon);
	ConvertMatrix.z2u = sin(PosLla.lat);

	return ConvertMatrix;
}

CONVERT_MATRIX CalcConvMatrix(LLA_POSITION Position)
{
	CONVERT_MATRIX ConvertMatrix;

	ConvertMatrix.x2e = -sin(Position.lon);
	ConvertMatrix.y2e = cos(Position.lon);
	ConvertMatrix.x2n = -sin(Position.lat) * cos(Position.lon);
	ConvertMatrix.y2n = -sin(Position.lat) * sin(Position.lon);
	ConvertMatrix.z2n = cos(Position.lat);
	ConvertMatrix.x2u = cos(Position.lat) * cos(Position.lon);
	ConvertMatrix.y2u = cos(Position.lat) * sin(Position.lon);
	ConvertMatrix.z2u = sin(Position.lat);

	return ConvertMatrix;
}

void SpeedEnuToCourse(LOCAL_SPEED &Speed)
{
	Speed.speed = sqrt(Speed.ve * Speed.ve + Speed.vn * Speed.vn);
	Speed.course = atan2(Speed.ve, Speed.vn);
	if (Speed.course < 0)
		Speed.course += PI2;
}

void SpeedCourseToEnu(LOCAL_SPEED &Speed)
{
	Speed.ve = Speed.speed * sin(Speed.course);
	Speed.vn = Speed.speed * cos(Speed.course);
}

void SpeedEcefToLocal(CONVERT_MATRIX ConvertMatrix, KINEMATIC_INFO PosVel, LOCAL_SPEED &Speed)
{
	Speed.ve = PosVel.vx * ConvertMatrix.x2e + PosVel.vy * ConvertMatrix.y2e;
	Speed.vn = PosVel.vx * ConvertMatrix.x2n + PosVel.vy * ConvertMatrix.y2n + PosVel.vz * ConvertMatrix.z2n;
	Speed.vu = PosVel.vx * ConvertMatrix.x2u + PosVel.vy * ConvertMatrix.y2u + PosVel.vz * ConvertMatrix.z2u;
	SpeedEnuToCourse(Speed);
}

void SpeedLocalToEcef(CONVERT_MATRIX ConvertMatrix, LOCAL_SPEED Speed, KINEMATIC_INFO &PosVel)
{
	PosVel.vx = Speed.ve * ConvertMatrix.x2e + Speed.vn * ConvertMatrix.x2n + Speed.vu * ConvertMatrix.x2u;
	PosVel.vy = Speed.ve * ConvertMatrix.y2e + Speed.vn * ConvertMatrix.y2n + Speed.vu * ConvertMatrix.y2u;
	PosVel.vz =                                Speed.vn * ConvertMatrix.z2n + Speed.vu * ConvertMatrix.z2u;
}

void SpeedLocalToEcef(LLA_POSITION lla_pos, LOCAL_SPEED Speed, KINEMATIC_INFO &PosVel)
{
	CONVERT_MATRIX ConvertMatrix = CalcConvMatrix(lla_pos);
	SpeedLocalToEcef(ConvertMatrix, Speed, PosVel);
}

void SatElAz(PLLA_POSITION PositionLla, double LosVector[3], double *Elevation, double *Azimuth)
{
	CONVERT_MATRIX ConvertMatrix = CalcConvMatrix(*PositionLla);
	double LocalLos[3];

	LocalLos[0] = LosVector[0] * ConvertMatrix.x2e + LosVector[1] * ConvertMatrix.y2e;
	LocalLos[1] = LosVector[0] * ConvertMatrix.x2n + LosVector[1] * ConvertMatrix.y2n + LosVector[2] * ConvertMatrix.z2n;
	LocalLos[2] = LosVector[0] * ConvertMatrix.x2u + LosVector[1] * ConvertMatrix.y2u + LosVector[2] * ConvertMatrix.z2u;

	*Azimuth = atan2(LocalLos[0], LocalLos[1]);
	if (*Azimuth < 0)
		*Azimuth += PI2;
	*Elevation = asin(LocalLos[2]);
}

void SatElAz(PKINEMATIC_INFO Receiver, PKINEMATIC_INFO Satellite, double *Elevation, double *Azimuth)
{
	double LosVector[3];
	LLA_POSITION Position = EcefToLla(*Receiver);

	GeometryDistance(Receiver, Satellite, LosVector);
	SatElAz(&Position, LosVector, Elevation, Azimuth);
}

double GeometryDistance(const double *Receiver, const double *Satellite, double LosVector[3])
{
	double dx, dy, dz;
	double r;

	dx = Satellite[0] - Receiver[0];
	dy = Satellite[1] - Receiver[1];
	dz = Satellite[2] - Receiver[2];
	r = (double)sqrt(dx * dx + dy * dy + dz * dz);
	// add earth rotate compensation
	r += (Satellite[0] * Receiver[1] - Satellite[1] * Receiver[0]) * (WGS_OMEGDOTE / LIGHT_SPEED);
	if (LosVector)
	{
		LosVector[0] = dx / r; LosVector[1] = dy / r; LosVector[2] = dz / r;
	}
	return r;
}

double GeometryDistance(const PKINEMATIC_INFO Receiver, const PKINEMATIC_INFO Satellite, double LosVector[3])
{
	return GeometryDistance(Receiver->PosVel, Satellite->PosVel, LosVector);
}

double SatRelativeSpeed(PKINEMATIC_INFO Receiver, PKINEMATIC_INFO Satellite)
{
	double dx, dy, dz;
	double dvx, dvy, dvz;
	double Distance;

	dx = Receiver->x - Satellite->x;
	dy = Receiver->y - Satellite->y;
	dz = Receiver->z - Satellite->z;
	dvx = Receiver->vx - Satellite->vx;
	dvy = Receiver->vy - Satellite->vy;
	dvz = Receiver->vz - Satellite->vz;
	Distance = sqrt(dx * dx + dy * dy + dz * dz);
	return (dx * dvx + dy * dvy + dz * dvz) / Distance;
}

double GpsIonoDelay(PIONO_PARAM IonoParam, double time, double Lat, double Lon, double Elevation, double Azimuth)
{
	double El = Elevation / PI;
	double psi, F, PER, x, AMP, x1;
	double ReturnValue = 0.;
	double T;

	Lat /= PI;
	Lon /= PI;
	psi = 0.0137 / (El + 0.11) - 0.022;
	Lat += psi * cos(Azimuth);
	if (Lat > 0.416f)
		Lat = 0.416f;
	else if (Lat < -0.416f)
		Lat = -0.416f;

	Lon += psi * sin(Azimuth) / cos(Lat * PI);
	Lat += 0.064 * cos((Lon - 1.617) * PI);
	F = 1.0 + 16.0 * pow(0.53 - El, 3);
	PER = IonoParam->b0 + (IonoParam->b1 + (IonoParam->b2 + IonoParam->b3 * Lat) * Lat) * Lat;
	if (PER < 72000.0)
		PER = 72000.0;

	T = (43200.0 * Lon) + time;
	while (T >= 86400.)
		T -= 86400.;
	while (T < 0.)
		T += 86400.;
	x = PI2 * (T - 50400.) / PER;

	F *= LIGHT_SPEED;
	if (x >= 1.57 || x <= -1.57)
	{
		ReturnValue = F * 5e-9;
	}
	else
	{
		AMP = IonoParam->a0 + (IonoParam->a1 + (IonoParam->a2 + IonoParam->a3 * Lat) * Lat) * Lat;
		if (AMP < 0.0)
			ReturnValue = F * 5e-9;
		else
		{
			x *= x;
			x1 = 1.0f - x / 2.0f;
			x *= x;
			x1 += x / 24.0f;
			ReturnValue = F * (5e-9 + AMP * x1);
		}
	}

	return ReturnValue;
}

#define REL_HUMI 0.7
double TropoDelay(double Lat, double Altitude, double Elevation)
{
	const double t0 = 273.16 + 15.0; // average temparature at sea level
	double Pressure, t, e, z,trph, trpw;
    
    if (Altitude < -100.0 || Altitude > 1e4 || Elevation <= 0)
		return 0.0;
    if (Altitude < 0)
		Altitude = 0;
    
    Pressure = 1013.25 * pow(1.0 - 2.2557E-5 * Altitude, 5.2568);
    t = t0 - 6.5e-3 * Altitude;
    e = 6.108 * REL_HUMI * exp((17.15 * t - 4684.0) / (t - 38.45));
    
    z = PI / 2.0 - Elevation;
    trph = 0.0022767 * Pressure / (1.0 - 0.00266 * cos(2.0 * Lat) - 0.00028 * Altitude / 1E3) / cos(z);
    trpw = 0.002277 * (1255.0 / t + 0.05) * e / cos(z);

	return (trph + trpw);
}

void RungeKutta(double h, double State[9])
{
	double State1[9], VelAcc1[6], VelAcc2[6], VelAcc3[6], VelAcc4[6];
	int i;

	State1[6] = State[6];
	State1[7] = State[7];
	State1[8] = State[8];
	CalcAcceleration(State, VelAcc1);
	PredictState(State, State1, VelAcc1, 0.5 * h);
	CalcAcceleration(State1, VelAcc2);
	PredictState(State, State1, VelAcc2, 0.5 * h);
	CalcAcceleration(State1, VelAcc3);
	PredictState(State, State1, VelAcc3, h);
	CalcAcceleration(State1, VelAcc4);
	for (i = 0; i < 6; i ++)
	{
		VelAcc1[i] = (VelAcc1[i] + VelAcc4[i] + 2.0 * (VelAcc2[i] + VelAcc3[i])) / 6.0;
	}
	PredictState(State, State, VelAcc1, h);
}

void CalcAcceleration(double *State, double *Acc)
{
	double r2, r3, Coef20, Scale20;

	Acc[0] = State[3];
	Acc[1] = State[4];
	Acc[2] = State[5];
	r2 = (State[0] * State[0] + State[1] * State[1] + State[2] * State[2]);
	r3 = r2 * sqrt(r2);
	Coef20 = PZ90_C20AE2 / r2;
	Scale20 = PZ90_GM * (1.5 * Coef20 * (5 * State[2] * State[2] / r2 - 1.0) - 1.0) / r3;
	Acc[3] = Scale20 * State[0] + State[6];		// x acceleration
	Acc[4] = Scale20 * State[1] + State[7];		// y acceleration
	Acc[5] = (Scale20 - 3.0 * PZ90_GM * Coef20 / r3) * State[2] + State[8];	// z acceleration
}

void PredictState(double *State, double *State1, double *VelAcc, double Step)
{
	int i;

	for (i = 0; i < 6; i ++)
	{
		State1[i] = State[i] + VelAcc[i] * Step;
	}
}

static void CisToCts(double *State, double DeltaT, PKINEMATIC_INFO pCtsPos, double *Acc)
{
	double Omega, SinValue, CosValue;

	// calculate rotate angle between CIS and CTS
	Omega = PZ90_OMEGDOTE * DeltaT;
	CosValue = cos(Omega);
	SinValue = sin(Omega);
	// calculate position
	pCtsPos->x = State[0] * CosValue + State[1] * SinValue;
	pCtsPos->y = State[1] * CosValue - State[0] * SinValue;
	pCtsPos->z = State[2];
	// calculate velocity
	pCtsPos->vx = State[3] * CosValue + State[4] * SinValue;
	pCtsPos->vy = State[4] * CosValue - State[3] * SinValue;
	pCtsPos->vz = State[5];
	// calculate acceleration
	if (Acc)
	{
		Acc[0] = State[6] * CosValue + State[7] * SinValue;
		Acc[1] = State[7] * CosValue - State[6] * SinValue;
		Acc[2] = State[8];
	}
	// additional compensation on velocity/acceleration
	CosValue *= PZ90_OMEGDOTE;
	SinValue *= PZ90_OMEGDOTE;
	pCtsPos->vx -= State[0] * SinValue - State[1] * CosValue;
	pCtsPos->vy -= State[1] * SinValue + State[0] * CosValue;
	if (Acc)
	{
		Acc[0] -= State[3] * SinValue - State[4] * CosValue;
		Acc[1] -= State[4] * SinValue + State[3] * CosValue;
	}
}
