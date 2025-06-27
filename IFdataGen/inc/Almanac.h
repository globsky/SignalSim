//----------------------------------------------------------------------
// Almanac.h:
//   Declaration of almanac file read/write/convert functions
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __ALMANAC_H__
#define __ALMANAC_H__

#include <stdio.h>
#include "BasicTypes.h"

enum AlmanacType { AlmanacUnknown = 0, AlmanacGps, AlmanacBds, AlmanacGalileo, AlmanacGlonass };

AlmanacType CheckAlmnanacType(FILE *fp);
int ReadAlmanacGps(FILE *fp, GPS_ALMANAC Almanac[]);
int ReadAlmanacBds(FILE *fp, GPS_ALMANAC Almanac[]);
int ReadAlmanacGalileo(FILE *fp, GPS_ALMANAC Almanac[]);
int ReadAlmanacGlonass(FILE *fp, GLONASS_ALMANAC Almanac[]);
GPS_ALMANAC GetAlmanacFromEphemeris(PGPS_EPHEMERIS Eph, int week, int toa);
GLONASS_ALMANAC GetAlmanacFromEphemeris(PGLONASS_EPHEMERIS Eph, int day, int leap_year);

#endif // __RINEX_H__
