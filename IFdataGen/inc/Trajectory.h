//----------------------------------------------------------------------
// Trajectory.h:
//   Declaration of trajectory processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __TRAJECTORY_H__
#define __TRAJECTORY_H__

#include "BasicTypes.h"

#define TRAJECTORY_NO_ERR 0
#define TRAJECTORY_UNKNOWN_TYPE 1
#define TRAJECTORY_TYPE_MISMATCH 2
#define TRAJECTORY_INVALID_PARAM 3
#define TRAJECTORY_ZERO_SPEED 4
#define TRAJECTORY_ACC_JERK 1
#define TRAJECTORY_ZERO_ACC 2
#define TRAJECTORY_ZERO_ACCRATE 3
#define TRAJECTORY_ZERO_DEGREE 4
#define TRAJECTORY_NEGATIVE 5

enum TrajectoryType { TrajTypeUnknown = 0, TrajTypeConstSpeed, TrajTypeConstAcc, TrajTypeVerticalAcc, TrajTypeJerk, TrajTypeHorizontalCircular };
enum TrajectoryDataType { TrajDataTimeSpan = 0, TrajDataAcceleration, TrajDataSpeed, TrajDataAccRate, TrajDataAngle, TrajDataAngularRate, TrajDataRadius };

class CTrajectorySegment
{
public:
	CTrajectorySegment();
	~CTrajectorySegment();
#if 0
	static LLA_POSITION EcefToLla(KINEMATIC_INFO ecef_pos);
	static KINEMATIC_INFO LlaToEcef(LLA_POSITION lla_pos);
	static CONVERT_MATRIX CalcConvMatrix(KINEMATIC_INFO Position);
	static CONVERT_MATRIX CalcConvMatrix(LLA_POSITION Position);
	static void SpeedEnuToCourse(LOCAL_SPEED &Speed);
	static void SpeedCourseToEnu(LOCAL_SPEED &Speed);
	static void SpeedEcefToLocal(CONVERT_MATRIX ConvertMatrix, KINEMATIC_INFO PosVel, LOCAL_SPEED &Speed);
	static void SpeedLocalToEcef(CONVERT_MATRIX ConvertMatrix, LOCAL_SPEED Speed, KINEMATIC_INFO &PosVel);
#endif
	static TrajectoryType GetTrajectoryType(CTrajectorySegment *pTrajectory);
	void GetSpeedProjection(double projection[3]);
	void InitSegment(CTrajectorySegment *PrevSegment);
	virtual int SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2) = 0;
	virtual KINEMATIC_INFO GetPosVel(double RelativeTime) = 0;

	KINEMATIC_INFO m_StartPosVel;
	LOCAL_SPEED m_LocalSpeed;
	CONVERT_MATRIX m_ConvertMatrix;
//	double m_StartTime;
	double m_TimeSpan;
	CTrajectorySegment *m_pNextTrajectory;
};

class CTrajectoryConstSpeed : public CTrajectorySegment
{
public:
	int SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2);
	KINEMATIC_INFO GetPosVel(double RelativeTime);
};

class CTrajectoryConstAcc : public CTrajectorySegment
{
public:
	double m_ax, m_ay, m_az;
	int SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2);
	KINEMATIC_INFO GetPosVel(double RelativeTime);
};

class CTrajectoryVerticalAcc : public CTrajectorySegment
{
public:
	double m_ax, m_ay, m_az;
	int SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2);
	KINEMATIC_INFO GetPosVel(double RelativeTime);
};

class CTrajectoryJerk : public CTrajectorySegment
{
public:
	KINEMATIC_INFO m_Acc;		// initial acceleration in x, y, z and acceleration rate in vx, vy, vz
	int SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2);
	KINEMATIC_INFO GetPosVel(double RelativeTime);
};

class CTrajectoryHorizontalCircular : public CTrajectorySegment
{
public:
	double m_AngularRate;
	int SetSegmentParam(CTrajectorySegment *PrevSegment, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2);
	KINEMATIC_INFO GetPosVel(double RelativeTime);
};

class CTrajectory
{
public:
	CTrajectory();
	~CTrajectory();

	void SetInitPosVel(KINEMATIC_INFO InitPosVel);
	void SetInitPosVel(LLA_POSITION InitPosition, LOCAL_SPEED InitVelocity, bool IsEnu);

	void ClearTrajectoryList();
	int AppendTrajectory(TrajectoryType TrajType, TrajectoryDataType DataType1, double Data1, TrajectoryDataType DataType2, double Data2);
	void ResetTrajectoryTime();
	BOOL GetNextPosVelECEF(double TimeStep, KINEMATIC_INFO &PosVel);
	BOOL GetNextPosVelLLA(double TimeStep, LLA_POSITION &Position, LOCAL_SPEED &Velocity);
	double GetTimeLength();
	void SetTrajectoryName(char *Name);
	char *GetTrajectoryName() { return TrajectoryName; }

private:
	KINEMATIC_INFO	m_InitPosVel;
	LOCAL_SPEED m_InitLocalSpeed;
	CTrajectorySegment *m_pTrajectoryList;
	CTrajectorySegment *m_pCurrentTrajectory;
	double RelativeTime;
	char TrajectoryName[128];

	CTrajectorySegment *GetLastSegment();
};

#endif // __TRAJECTORY_H__
