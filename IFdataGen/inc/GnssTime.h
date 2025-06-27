//----------------------------------------------------------------------
// GnssTime.h:
//   Definition of functions to do convertion between GNSS time
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__GNSS_TIME_H__)
#define __GNSS_TIME_H__

#include "BasicTypes.h"

BOOL GetLeapSecond(unsigned int Seconds, int &LeapSecond);
UTC_TIME GpsTimeToUtc(GNSS_TIME GnssTime, BOOL UseLeapSecond = TRUE);
UTC_TIME GlonassTimeToUtc(GLONASS_TIME GlonassTime);
UTC_TIME BdsTimeToUtc(GNSS_TIME GnssTime);
UTC_TIME GalileoTimeToUtc(GNSS_TIME GnssTime);
GNSS_TIME UtcToGpsTime(UTC_TIME UtcTime, BOOL UseLeapSecond = TRUE);
GLONASS_TIME UtcToGlonassTime(UTC_TIME UtcTime);
GNSS_TIME UtcToBdsTime(UTC_TIME UtcTime);
GNSS_TIME UtcToGalileoTime(UTC_TIME UtcTime);

#endif //!defined(__GNSS_TIME_H__)
