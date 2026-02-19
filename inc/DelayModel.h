//----------------------------------------------------------------------
// DelayModel.h:
//   Declaration of ionosphere and troposphere delay model
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __DELAY_MODEL_H__
#define __DELAY_MODEL_H__

#include "BasicTypes.h"

class CIonoDelay
{
public:
	CIonoDelay();
	~CIonoDelay();

	virtual double GetDelay(double time, double Lat, double Lon, double Elevation, double Azimuth) = 0;
};

class CIonoKlobuchar8 : public CIonoDelay
{
public:
	CIonoKlobuchar8();
	CIonoKlobuchar8(PIONO_PARAM Parameters);
	void SetIonoParam(PIONO_PARAM Parameters);
	double GetDelay(double time, double Lat, double Lon, double Elevation, double Azimuth);
private:
    IONO_PARAM IonoParam;
};
#if 0
class CIonoKlobuchar14 : public CIonoDelay
{
public:
	CIonoKlobuchar14(double *Parameters);
	double GetDelay(double time, double Lat, double Lon, double Elevation, double Azimuth);
};

class CIonoNequick : public CIonoDelay
{
public:
	CIonoNequick(double *Parameters);
	double GetDelay(double time, double Lat, double Lon, double Elevation, double Azimuth);
};
#endif
#endif // __DELAY_MODEL_H__
