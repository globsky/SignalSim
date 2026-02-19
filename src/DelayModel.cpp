//----------------------------------------------------------------------
// DelayModel.cpp:
//   Implementation of ionosphere and troposphere delay model
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include "DelayModel.h"
#include "Coordinate.h"

CIonoDelay::CIonoDelay()
{
}

CIonoDelay::~CIonoDelay()
{
}

CIonoKlobuchar8::CIonoKlobuchar8()
{
	IonoParam = {0.0};
};

CIonoKlobuchar8::CIonoKlobuchar8(PIONO_PARAM Parameters)
{
	IonoParam = *Parameters;
};

void CIonoKlobuchar8::SetIonoParam(PIONO_PARAM Parameters)
{
	IonoParam = *Parameters;
}

double CIonoKlobuchar8::GetDelay(double time, double Lat, double Lon, double Elevation, double Azimuth)
{
	return GpsIonoDelay(&IonoParam, time, Lat, Lon, Elevation, Azimuth);
}
