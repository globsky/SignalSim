//----------------------------------------------------------------------
// FNavBit.h:
//   Declaration of navigation bit synthesis class for F/NAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __FNAV_BIT_H__
#define __FNAV_BIT_H__

#include "NavBit.h"

class FNavBit : public NavBit
{
public:
	FNavBit();
	~FNavBit();

	int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits);
	int SetEphemeris(int svid, PGPS_EPHEMERIS Eph);
	int SetAlmanac(GPS_ALMANAC Alm[]);
	int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);

private:
};

#endif // __FNAV_BIT_H__
