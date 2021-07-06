//----------------------------------------------------------------------
// Trajectory.cpp:
//   Definition of trajectory processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------
#include <math.h>
#include <string.h>

#include "ConstVal.h"
#include "Coordinate.h"
#include "Trajectory.h"

CTrajectorySegment::CTrajectorySegment()
{
}

CTrajectorySegment::~CTrajectorySegment()
{
}

TrajectoryType CTrajectorySegment::GetTrajectoryType(CTrajectorySegment *pTrajectory)
{
	if ((dynamic_cast<CTrajectoryConstSpeed*>(pTrajectory)) != nullptr)
		return TrajTypeConstSpeed;
	else if ((dynamic_cast<CTrajectoryConstAcc*>(pTrajectory)) != nullptr)
		return TrajTypeConstAcc;
	else if ((dynamic_cast<CTrajectoryVerticalAcc*>(pTrajectory)) != nullptr)
		return TrajTypeVerticalAcc;
	else if ((dynamic_cast<CTrajectoryJerk*>(pTrajectory)) != nullptr)
		return TrajTypeJerk;
	else if ((dynamic_cast<CTrajectoryHorizontalCircular*>(pTrajectory)) != nullptr)
		return TrajTypeHorizontalCircular;
	else
		return TrajTypeUnknown;
}

void CTrajectorySegment::GetSpeedProjection(double projection[3])
{
	double speed = sqrt(m_StartPosVel.vx * m_StartPosVel.vx + m_StartPosVel.vy * m_StartPosVel.vy + m_StartPosVel.vz * m_StartPosVel.vz);
	KINEMATIC_INFO PosVel = m_StartPosVel;

	if (speed < 1e-5)	// zero speed, direction derive from course
	{
		LOCAL_SPEED LocalSpeed = m_LocalSpeed;
		LocalSpeed.speed = 1.0;
		SpeedCourseToEnu(LocalSpeed);
		SpeedLocalToEcef(m_ConvertMatrix, LocalSpeed, PosVel);
		speed = sqrt(PosVel.vx * PosVel.vx + PosVel.vy * PosVel.vy + PosVel.vz * PosVel.vz);
	}
	projection[0] = PosVel.vx / speed;
	projection[1] = PosVel.vy / speed;
	projection[2] = PosVel.vz / speed;
}

void CTrajectorySegment::InitSegment(CTrajectorySegment *PrevSegment)
{
	m_StartPosVel = PrevSegment->GetPosVel(PrevSegment->m_TimeSpan);
	m_ConvertMatrix = CalcConvMatrix(m_StartPosVel);
	SpeedEcefToLocal(m_ConvertMatrix, m_StartPosVel, m_LocalSpeed);
	m_pNextTrajectory = (CTrajectorySegment *)NULL;
}

int CTrajectoryConstSpeed::SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2)
{
	if (DataType1 != TrajDataTimeSpan)
		return TRAJECTORY_TYPE_MISMATCH;
	else if (Data1 <= 0)
		return TRAJECTORY_INVALID_PARAM;

	m_TimeSpan = Data1;
	return TRAJECTORY_NO_ERR;
}

KINEMATIC_INFO CTrajectoryConstSpeed::GetPosVel(double RelativeTime)
{
	KINEMATIC_INFO PosVel = m_StartPosVel;

	PosVel.x += PosVel.vx * RelativeTime;
	PosVel.y += PosVel.vy * RelativeTime;
	PosVel.z += PosVel.vz * RelativeTime;

	return PosVel;
}

int CTrajectoryConstAcc::SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2)
{
	double time, acc, speed;
	double projection[3];

	speed = sqrt(m_StartPosVel.vx * m_StartPosVel.vx + m_StartPosVel.vy * m_StartPosVel.vy + m_StartPosVel.vz * m_StartPosVel.vz);
	switch (DataType1)
	{
	case TrajDataTimeSpan:
		time = Data1;
		if (DataType2 == TrajDataAcceleration)
			acc = Data2;
		else if (DataType2 == TrajDataSpeed)
			acc = (Data2 - speed) / time;
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataAcceleration:
		acc = Data1;
		if (DataType2 == TrajDataTimeSpan)
			time = Data2;
		else if (DataType2 == TrajDataSpeed)
		{
			time = (Data2 - speed) / acc;
			if (time < 0)
			{
				time = -time;
				acc = -acc;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataSpeed:
		if (DataType2 == TrajDataTimeSpan)
		{
			time = Data2;
			acc = (Data1 - speed) / time;
		}
		else if (DataType2 == TrajDataAcceleration)
		{
			acc = Data2;
			time = (Data1 - speed) / acc;
			if (time < 0)
			{
				time = -time;
				acc = -acc;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	}

	if (time <= 0)
		return TRAJECTORY_INVALID_PARAM;

	GetSpeedProjection(projection);
	m_ax = acc * projection[0];
	m_ay = acc * projection[1];
	m_az = acc * projection[2];
	m_TimeSpan = time;

	return TRAJECTORY_NO_ERR;
}

KINEMATIC_INFO CTrajectoryConstAcc::GetPosVel(double RelativeTime)
{
	KINEMATIC_INFO PosVel = m_StartPosVel;

	PosVel.x += (PosVel.vx + m_ax / 2 * RelativeTime) * RelativeTime;
	PosVel.y += (PosVel.vy + m_ay / 2 * RelativeTime) * RelativeTime;
	PosVel.z += (PosVel.vz + m_az / 2 * RelativeTime) * RelativeTime;
	PosVel.vx += m_ax * RelativeTime;
	PosVel.vy += m_ay * RelativeTime;
	PosVel.vz += m_az * RelativeTime;

	return PosVel;
}

int CTrajectoryVerticalAcc::SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2)
{
	double time, acc;

	switch (DataType1)
	{
	case TrajDataTimeSpan:
		time = Data1;
		if (DataType2 == TrajDataAcceleration)
			acc = Data2;
		else if (DataType2 == TrajDataSpeed)
			acc = (Data2 - m_LocalSpeed.vu) / time;
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataAcceleration:
		acc = Data1;
		if (DataType2 == TrajDataTimeSpan)
			time = Data2;
		else if (DataType2 == TrajDataSpeed)
		{
			time = (Data2 - m_LocalSpeed.vu) / acc;
			if (time < 0)
			{
				time = -time;
				acc = -acc;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataSpeed:
		if (DataType2 == TrajDataTimeSpan)
		{
			time = Data2;
			acc = (Data1 - m_LocalSpeed.vu) / time;
		}
		else if (DataType2 == TrajDataAcceleration)
		{
			acc = Data2;
			time = (Data1 - m_LocalSpeed.vu) / acc;
			if (time < 0)
			{
				time = -time;
				acc = -acc;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	}

	if (time <= 0)
		return TRAJECTORY_INVALID_PARAM;

	m_ax = acc * m_ConvertMatrix.x2u;
	m_ay = acc * m_ConvertMatrix.y2u;
	m_az = acc * m_ConvertMatrix.z2u;
	m_TimeSpan = time;

	return TRAJECTORY_NO_ERR;
}

KINEMATIC_INFO CTrajectoryVerticalAcc::GetPosVel(double RelativeTime)
{
	KINEMATIC_INFO PosVel = m_StartPosVel;

	PosVel.x += (PosVel.vx + m_ax / 2 * RelativeTime) * RelativeTime;
	PosVel.y += (PosVel.vy + m_ay / 2 * RelativeTime) * RelativeTime;
	PosVel.z += (PosVel.vz + m_az / 2 * RelativeTime) * RelativeTime;
	PosVel.vx += m_ax * RelativeTime;
	PosVel.vy += m_ay * RelativeTime;
	PosVel.vz += m_az * RelativeTime;

	return PosVel;
}

int CTrajectoryJerk::SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2)
{
	double time, acc, rate;
	double projection[3];

	TrajectoryType Type = GetTrajectoryType(PrevSegment);
	if (Type == TrajTypeJerk)
	{
		m_Acc.x = ((CTrajectoryJerk *)PrevSegment)->m_Acc.x + ((CTrajectoryJerk *)PrevSegment)->m_Acc.vx * PrevSegment->m_TimeSpan;
		m_Acc.y = ((CTrajectoryJerk *)PrevSegment)->m_Acc.y + ((CTrajectoryJerk *)PrevSegment)->m_Acc.vy * PrevSegment->m_TimeSpan;
		m_Acc.z = ((CTrajectoryJerk *)PrevSegment)->m_Acc.z + ((CTrajectoryJerk *)PrevSegment)->m_Acc.vz * PrevSegment->m_TimeSpan;
	}
	else if (Type == TrajTypeConstAcc)
	{
		m_Acc.x = ((CTrajectoryConstAcc *)PrevSegment)->m_ax;
		m_Acc.y = ((CTrajectoryConstAcc *)PrevSegment)->m_ay;
		m_Acc.z = ((CTrajectoryConstAcc *)PrevSegment)->m_az;
	}
	else if (Type == TrajTypeVerticalAcc)
	{
		m_Acc.x = ((CTrajectoryVerticalAcc *)PrevSegment)->m_ax;
		m_Acc.y = ((CTrajectoryVerticalAcc *)PrevSegment)->m_ay;
		m_Acc.z = ((CTrajectoryVerticalAcc *)PrevSegment)->m_az;
	}
	else
	{
		m_Acc.x = m_Acc.y = m_Acc.z = 0.0;
	}
	acc = sqrt(m_Acc.x * m_Acc.x + m_Acc.y * m_Acc.y + m_Acc.z * m_Acc.z);

	switch (DataType1)
	{
	case TrajDataTimeSpan:
		time = Data1;
		if (DataType2 == TrajDataAcceleration)
			rate = (Data2 - acc) / time;
		else if (DataType2 == TrajDataAccRate)
			acc = Data2;
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataAccRate:
		rate = Data1;
		if (DataType2 == TrajDataTimeSpan)
			time = Data2;
		else if (DataType2 == TrajDataAcceleration)
		{
			time = (Data2 - acc) / rate;
			if (time < 0)
			{
				time = -time;
				rate = -rate;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataAcceleration:
		if (DataType2 == TrajDataTimeSpan)
		{
			time = Data2;
			rate = (Data1 - acc) / time;
		}
		else if (DataType2 == TrajDataAccRate)
		{
			rate = Data2;
			time = (Data1 - acc) / rate;
			if (time < 0)
			{
				time = -time;
				rate = -rate;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	}

	if (time <= 0)
		return TRAJECTORY_INVALID_PARAM;

	GetSpeedProjection(projection);
	m_Acc.vx = rate * projection[0];
	m_Acc.vy = rate * projection[1];
	m_Acc.vz = rate * projection[2];
	m_TimeSpan = time;

	return TRAJECTORY_NO_ERR;
}

KINEMATIC_INFO CTrajectoryJerk::GetPosVel(double RelativeTime)
{
	KINEMATIC_INFO PosVel = m_StartPosVel;

	PosVel.x += (PosVel.vx + (m_Acc.x + m_Acc.vx / 3 * RelativeTime) / 2 * RelativeTime) * RelativeTime;
	PosVel.y += (PosVel.vy + (m_Acc.y + m_Acc.vy / 3 * RelativeTime) / 2 * RelativeTime) * RelativeTime;
	PosVel.z += (PosVel.vz + (m_Acc.z + m_Acc.vz / 3 * RelativeTime) / 2 * RelativeTime) * RelativeTime;
	PosVel.vx += (m_Acc.x + m_Acc.vx / 2 * RelativeTime) * RelativeTime;
	PosVel.vy += (m_Acc.y + m_Acc.vy / 2 * RelativeTime) * RelativeTime;
	PosVel.vz += (m_Acc.z + m_Acc.vz / 2 * RelativeTime) * RelativeTime;

	return PosVel;
}

int CTrajectoryHorizontalCircular::SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2)
{
	double time, rate;
	double speed = sqrt(m_StartPosVel.vx * m_StartPosVel.vx + m_StartPosVel.vy * m_StartPosVel.vy + m_StartPosVel.vz * m_StartPosVel.vz);

	if (speed < 1e-5)
		return TRAJECTORY_ZERO_SPEED;

	// convert type to angular rate
	if (DataType1 == TrajDataAcceleration)
	{
		DataType1 = TrajDataAngularRate;
		Data1 /= speed;
	}
	else if (DataType1 == TrajDataRadius)
	{
		if (fabs(Data1) < 1e-5)
			return TRAJECTORY_INVALID_PARAM;
		DataType1 = TrajDataAngularRate;
		Data1 = speed / Data1;
	}
	if (DataType2 == TrajDataAcceleration)
	{
		DataType2 = TrajDataAngularRate;
		Data2 /= speed;
	}
	else if (DataType2 == TrajDataRadius)
	{
		if (fabs(Data2) < 1e-5)
			return TRAJECTORY_INVALID_PARAM;
		DataType2 = TrajDataAngularRate;
		Data2 = speed / Data2;
	}

	switch (DataType1)
	{
	case TrajDataTimeSpan:
		time = Data1;
		if (DataType2 == TrajDataAngularRate)
			rate = Data2;
		else if (DataType2 == TrajDataAngle)
			rate = Data2 / time;
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataAngularRate:
		rate = Data1;
		if (DataType2 == TrajDataTimeSpan)
			time = Data2;
		else if (DataType2 == TrajDataAngle)
		{
			time = Data2 / rate;
			if (time < 0)
			{
				time = -time;
				rate = -rate;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	case TrajDataAngle:
		if (DataType2 == TrajDataTimeSpan)
		{
			time = Data2;
			rate = Data1 / time;
		}
		else if (DataType2 == TrajDataAngularRate)
		{
			rate = Data2;
			time = Data1 / rate;
			if (time < 0)
			{
				time = -time;
				rate = -rate;
			}
		}
		else
			return TRAJECTORY_TYPE_MISMATCH;
		break;
	}

	if (time <= 0)
		return TRAJECTORY_INVALID_PARAM;
	if (fabs(rate) <= 1e-8)
		return TRAJECTORY_INVALID_PARAM;

	m_TimeSpan = time;
	m_AngularRate = rate;

	return TRAJECTORY_NO_ERR;
}

KINEMATIC_INFO CTrajectoryHorizontalCircular::GetPosVel(double RelativeTime)
{
	KINEMATIC_INFO PosVel = m_StartPosVel;
	LOCAL_SPEED LocalSpeed = m_LocalSpeed;
	double Radius = m_LocalSpeed.speed / m_AngularRate;
	double e, n, u;
	double x, y, z;

	LocalSpeed.course += m_AngularRate * RelativeTime;
	SpeedCourseToEnu(LocalSpeed);
	SpeedLocalToEcef(m_ConvertMatrix, LocalSpeed, PosVel);
	e = Radius * (cos(m_LocalSpeed.course) - cos(LocalSpeed.course));
	n = Radius * (sin(LocalSpeed.course) - sin(m_LocalSpeed.course));
	u = m_LocalSpeed.vu * RelativeTime;
	x = e * m_ConvertMatrix.x2e + n * m_ConvertMatrix.x2n + u * m_ConvertMatrix.x2u;
	y = e * m_ConvertMatrix.y2e + n * m_ConvertMatrix.y2n + u * m_ConvertMatrix.y2u;
	z =                           n * m_ConvertMatrix.z2n + u * m_ConvertMatrix.z2u;
	PosVel.x += x;
	PosVel.y += y;
	PosVel.z += z;

	return PosVel;
}

CTrajectory::CTrajectory()
{
	m_pTrajectoryList = m_pCurrentTrajectory = (CTrajectorySegment *)NULL;
}

CTrajectory::~CTrajectory()
{
	ClearTrajectoryList();
}

void CTrajectory::SetInitPosVel(KINEMATIC_INFO InitPosVel)
{
	CONVERT_MATRIX ConvertMatrix;

	m_InitPosVel = InitPosVel;
	ConvertMatrix = CalcConvMatrix(m_InitPosVel);
	SpeedEcefToLocal(ConvertMatrix, m_InitPosVel, m_InitLocalSpeed);
}

void CTrajectory::SetInitPosVel(LLA_POSITION InitPosition, LOCAL_SPEED InitVelocity, bool IsEnu)
{
	CONVERT_MATRIX ConvertMatrix;

	m_InitPosVel = LlaToEcef(InitPosition);
	ConvertMatrix = CalcConvMatrix(InitPosition);
	if (IsEnu)
		SpeedEnuToCourse(InitVelocity);
	else
		SpeedCourseToEnu(InitVelocity);
	m_InitLocalSpeed = InitVelocity;
	SpeedLocalToEcef(ConvertMatrix, m_InitLocalSpeed, m_InitPosVel);
}

void CTrajectory::ClearTrajectoryList()
{
	CTrajectorySegment *pSegment = m_pTrajectoryList, *pCurSegment;

	while (pSegment)
	{
		pCurSegment = pSegment;
		pSegment = pCurSegment->m_pNextTrajectory;
		delete pCurSegment;
	}
}

int CTrajectory::AppendTrajectory(TrajectoryType TrajType, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2)
{
	CTrajectorySegment *TrajectorySegment, *PrevTrajectorySegment;
	BOOL ReturnValue;

	if (TrajType == TrajTypeConstSpeed)
		TrajectorySegment = new CTrajectoryConstSpeed;
	else if (TrajType == TrajTypeConstAcc)
		TrajectorySegment = new CTrajectoryConstAcc;
	else if (TrajType == TrajTypeVerticalAcc)
		TrajectorySegment = new CTrajectoryVerticalAcc;
	else if (TrajType == TrajTypeJerk)
		TrajectorySegment = new CTrajectoryJerk;
	else if (TrajType == TrajTypeHorizontalCircular)
		TrajectorySegment = new CTrajectoryHorizontalCircular;
	else
		return TRAJECTORY_UNKNOWN_TYPE;

	PrevTrajectorySegment = GetLastSegment();
	if (PrevTrajectorySegment == NULL)
	{
		m_pTrajectoryList = TrajectorySegment;
		// generate a constant speed segment with 0 time span 
		PrevTrajectorySegment = new CTrajectoryConstSpeed;
		PrevTrajectorySegment->m_StartPosVel = m_InitPosVel;
		PrevTrajectorySegment->m_LocalSpeed = m_InitLocalSpeed;
		PrevTrajectorySegment->m_ConvertMatrix = CalcConvMatrix(m_InitPosVel);
		PrevTrajectorySegment->m_TimeSpan = 0.0;
	}
	TrajectorySegment->InitSegment(PrevTrajectorySegment);
	ReturnValue = TrajectorySegment->SetSegmentParam(PrevTrajectorySegment, DataType1, Data1, DataType2, Data2);
	if (ReturnValue != TRAJECTORY_NO_ERR)
		delete TrajectorySegment;
	else
		PrevTrajectorySegment->m_pNextTrajectory = TrajectorySegment;
	if (m_pTrajectoryList == TrajectorySegment)
		delete PrevTrajectorySegment;

	return ReturnValue;
}

void CTrajectory::ResetTrajectoryTime()
{
	m_pCurrentTrajectory = m_pTrajectoryList;
	RelativeTime = 0.0;
}

BOOL CTrajectory::GetNextPosVelECEF(double TimeStep, KINEMATIC_INFO &PosVel)
{
	RelativeTime += TimeStep;
	while (m_pCurrentTrajectory && RelativeTime > m_pCurrentTrajectory->m_TimeSpan)
	{
		RelativeTime -= m_pCurrentTrajectory->m_TimeSpan;
		m_pCurrentTrajectory = m_pCurrentTrajectory->m_pNextTrajectory;
	}
	if (!m_pCurrentTrajectory)
		return FALSE;
	else
		PosVel = m_pCurrentTrajectory->GetPosVel(RelativeTime);

	return TRUE;
}

BOOL CTrajectory::GetNextPosVelLLA(double TimeStep, LLA_POSITION &Position, LOCAL_SPEED &Velocity)
{
	KINEMATIC_INFO PosVel;
	BOOL ReturnValue = GetNextPosVelECEF(TimeStep, PosVel);

	if (!ReturnValue)
		return FALSE;
	
	Position = EcefToLla(PosVel);
	SpeedEcefToLocal(m_pCurrentTrajectory->m_ConvertMatrix, PosVel, Velocity);

	return TRUE;
}

CTrajectorySegment *CTrajectory::GetLastSegment()
{
	CTrajectorySegment *pSegment = m_pTrajectoryList;

	while (pSegment)
	{
		if (!pSegment->m_pNextTrajectory)
			break;
		pSegment = pSegment->m_pNextTrajectory;
	}

	return pSegment;
}

void CTrajectory::SetTrajectoryName(char *Name)
{
	strncpy(TrajectoryName, Name, 127);
}
