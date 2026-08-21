//----------------------------------------------------------------------
// NavBit.h:
//   Declaration of navigation bit synthesis base class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __NAV_BIT_ARRAY_H__
#define __NAV_BIT_ARRAY_H__

#include "BasicTypes.h"
#include "NavBit.h"
#include "NavData.h"

#define NAVBIT_TYPES \
	X(LNAV,    0) \
	X(CNAV,    1) \
	X(CNAV2,   2) \
	X(GNAV,    3) \
	X(GNAV2,   4) \
	X(D1D2,    5) \
	X(BCNAV1,  6) \
	X(BCNAV2,  7) \
	X(BCNAV3,  8) \
	X(INAV,    9) \
	X(FNAV,   10) \
	X(ECNAV,  11) \
	X(SBAS,   12)

enum
{
#define X(name, val) NAVBIT_TYPE_##name = val,
	NAVBIT_TYPES
#undef X
};

enum
{
#define X(name, val) NAVBIT_MASK_##name = (1U << NAVBIT_TYPE_##name),
	NAVBIT_TYPES
#undef X
};

class CNavBitArray
{
public:
	static const int MAX_NAVBIT_TYPE = 13;
	NavBit *NavBitArray[MAX_NAVBIT_TYPE];

public:
	CNavBitArray();
	~CNavBitArray();
	int CreateNavBitArray(unsigned int SignalSelect[]);	// create navigation bit array on signal selection
	int CreateNavBitArray(unsigned int TypeMask);	// create navigation bit array on type mask
	int SetEphemeris(GnssSystem System, unsigned int long long SetEphMask, PGPS_EPHEMERIS pEphList[]);
	int SetEphemeris(int NavBitType, unsigned int long long SetEphMask, PGPS_EPHEMERIS pEphList[]);
	int SetEphemeris(int NavBitType, int svid, PGPS_EPHEMERIS pEph);
	int SetAlmanac(CNavData &NavData);
	int SetAlmanac(int NavBitType, GPS_ALMANAC Alm[]);
	int SetIonoUtc(CNavData &NavData);
	int SetIonoUtc(int NavBitType, PIONO_PARAM IonoParam, PUTC_PARAM UtcParam);
	NavBit* GetNavBit(GnssSystem SatSystem, int SatSignalIndex);
	BOOL NavBitTypeAvailable(int NavBitType) { return (NavBitType >= 0 && NavBitType < MAX_NAVBIT_TYPE && NavBitArray[NavBitType] != NULL); }
};

#endif // __NAV_BIT_ARRAY_H__
