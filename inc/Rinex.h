//----------------------------------------------------------------------
// Trajectory.h:
//   Declaration of trajectory processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __RINEX_H__
#define __RINEX_H__

#include <stdio.h>
#include "BasicTypes.h"

enum NavDataType {
	NavDataEnd = 0, NavDataUnknown,
	NavDataGpsIono, NavDataBdsIonoA, NavDataBdsIonoB, NavDataGalileoIono,
	NavDataGpsUtc, NavDataBdsUtc, NavDataGalileoUtc,
	NavDataGpsEph, NavDataBdsEph, NavDataGalileoEph, NavDataGlonassEph
};

NavDataType LoadNavFileHeader(FILE *fp_nav, void *NavData);
NavDataType LoadNavFileEphemeris(FILE *fp_nav, void *NavData);

#endif // __RINEX_H__
