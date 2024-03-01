//----------------------------------------------------------------------
// FNavBit.h:
//   Implementation of navigation bit synthesis class for F/NAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "FNavBit.h"

FNavBit::FNavBit()
{
}

FNavBit::~FNavBit()
{
}

// Param is used to distinguish from E1 and E5b (0 for E1)
int FNavBit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i;

	for (i = 0; i < 500; i ++)
		NavBits[i] = 0;

	return 0;
}

int FNavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	return svid;
}

int FNavBit::SetAlmanac(GPS_ALMANAC Alm[])
{
	return 0;
}

int FNavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	return 0;
}
