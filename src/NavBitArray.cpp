//----------------------------------------------------------------------
// NavBit.cpp:
//   Implementation of navigation bit synthesis base class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <math.h>
#include "NavBitArray.h"
#include "LNavBit.h"
#include "CNavBit.h"
#include "CNav2Bit.h"
#include "INavBit.h"
#include "FNavBit.h"
#include "D1D2NavBit.h"
#include "BCNav1Bit.h"
#include "BCNav2Bit.h"
#include "BCNav3Bit.h"
#include "GNavBit.h"

CNavBitArray::CNavBitArray()
{
	for (int i = 0; i < MAX_NAVBIT_TYPE; i ++)
		NavBitArray[i] = (NavBit*)0;
}

CNavBitArray::~CNavBitArray()
{
	for (int i = 0; i < MAX_NAVBIT_TYPE; i ++)
		if (NavBitArray[i])
			delete NavBitArray[i];
}

int CNavBitArray::CreateNavBitArray(unsigned int SignalSelect[])
{
	unsigned int TypeMask = 0;

	if (SignalSelect[GpsSystem] & (1 << SIGNAL_INDEX_L1CA))
		TypeMask |= NAVBIT_MASK_LNAV;
	if (SignalSelect[GpsSystem] & (1 << SIGNAL_INDEX_L1C))
		TypeMask |= NAVBIT_MASK_CNAV2;
	if (SignalSelect[GpsSystem] & (1 << SIGNAL_INDEX_L2C))
		TypeMask |= NAVBIT_MASK_CNAV;
	if (SignalSelect[GpsSystem] & (1 << SIGNAL_INDEX_L2P))
		TypeMask |= NAVBIT_MASK_LNAV;
	if (SignalSelect[GpsSystem] & (1 << SIGNAL_INDEX_L5))
		TypeMask |= NAVBIT_MASK_CNAV;
	if (SignalSelect[BdsSystem] & (1 << SIGNAL_INDEX_B1C))
		TypeMask |= NAVBIT_MASK_BCNAV1;
	if (SignalSelect[BdsSystem] & (1 << SIGNAL_INDEX_B1I))
		TypeMask |= NAVBIT_MASK_D1D2;
	if (SignalSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2I))
		TypeMask |= NAVBIT_MASK_D1D2;
	if (SignalSelect[BdsSystem] & (1 << SIGNAL_INDEX_B3I))
		TypeMask |= NAVBIT_MASK_D1D2;
	if (SignalSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2a))
		TypeMask |= NAVBIT_MASK_BCNAV2;
	if (SignalSelect[BdsSystem] & (1 << SIGNAL_INDEX_B2b))
		TypeMask |= NAVBIT_MASK_BCNAV3;
	if (SignalSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E1))
		TypeMask |= NAVBIT_MASK_INAV;
	if (SignalSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E5a))
		TypeMask |= NAVBIT_MASK_FNAV;
	if (SignalSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E5b))
		TypeMask |= NAVBIT_MASK_INAV;
	if (SignalSelect[GalileoSystem] & (1 << SIGNAL_INDEX_E6))
		TypeMask |= NAVBIT_MASK_ECNAV;
	if (SignalSelect[GlonassSystem] & (1 << SIGNAL_INDEX_G1))
		TypeMask |= NAVBIT_MASK_GNAV;
	if (SignalSelect[GlonassSystem] & (1 << SIGNAL_INDEX_G2))
		TypeMask |= NAVBIT_MASK_GNAV;

	return CreateNavBitArray(TypeMask);
}

int CNavBitArray::CreateNavBitArray(unsigned int TypeMask)
{
	int CreateNumber = 0;

	for (int i = 0; i < MAX_NAVBIT_TYPE; i ++)
	{
		if ((TypeMask & (1 << i)) == 0)
		{
			if (NavBitArray[i])
				delete NavBitArray[i];
			NavBitArray[i] = (NavBit*)0;
			continue;
		}
		switch (i)
		{
		case NAVBIT_TYPE_LNAV:   NavBitArray[i] = new LNavBit; CreateNumber ++; break;
		case NAVBIT_TYPE_CNAV:   NavBitArray[i] = new CNavBit; CreateNumber ++; break;
		case NAVBIT_TYPE_CNAV2:  NavBitArray[i] = new CNav2Bit; CreateNumber ++; break;
		case NAVBIT_TYPE_GNAV:   NavBitArray[i] = new GNavBit; CreateNumber ++; break;
		case NAVBIT_TYPE_GNAV2:  NavBitArray[i] = (NavBit*)0; break;
		case NAVBIT_TYPE_D1D2:   NavBitArray[i] = new D1D2NavBit; CreateNumber ++; break;
		case NAVBIT_TYPE_BCNAV1: NavBitArray[i] = new BCNav1Bit; CreateNumber ++; break;
		case NAVBIT_TYPE_BCNAV2: NavBitArray[i] = new BCNav2Bit; CreateNumber ++; break;
		case NAVBIT_TYPE_BCNAV3: NavBitArray[i] = new BCNav3Bit; CreateNumber ++; break;
		case NAVBIT_TYPE_INAV:   NavBitArray[i] = new INavBit; CreateNumber ++; break;
		case NAVBIT_TYPE_FNAV:   NavBitArray[i] = new FNavBit; CreateNumber ++; break;
		case NAVBIT_TYPE_ECNAV:  NavBitArray[i] = (NavBit*)0; break;
		case NAVBIT_TYPE_SBAS:   NavBitArray[i] = (NavBit*)0; break;
		default:                 NavBitArray[i] = (NavBit*)0; break;
		}
	}
	return CreateNumber;
}

int CNavBitArray::SetEphemeris(GnssSystem System, unsigned int long long SetEphMask, PGPS_EPHEMERIS pEphList[])
{
	switch (System)
	{
	case GpsSystem:
		SetEphemeris(NAVBIT_TYPE_LNAV, SetEphMask, pEphList);
		SetEphemeris(NAVBIT_TYPE_CNAV, SetEphMask, pEphList);
		SetEphemeris(NAVBIT_TYPE_CNAV2, SetEphMask, pEphList);
		break;
	case BdsSystem:
		SetEphemeris(NAVBIT_TYPE_D1D2, SetEphMask, pEphList);
		SetEphemeris(NAVBIT_TYPE_BCNAV1, SetEphMask, pEphList);
		SetEphemeris(NAVBIT_TYPE_BCNAV1, SetEphMask, pEphList);
		SetEphemeris(NAVBIT_TYPE_BCNAV1, SetEphMask, pEphList);
		break;
	case GalileoSystem:
		SetEphemeris(NAVBIT_TYPE_INAV, SetEphMask, pEphList);
		SetEphemeris(NAVBIT_TYPE_FNAV, SetEphMask, pEphList);
		break;
	case GlonassSystem:
		SetEphemeris(NAVBIT_TYPE_GNAV, SetEphMask, pEphList);
		break;
	}
	return 0;
}

int CNavBitArray::SetEphemeris(int NavBitType, unsigned int long long SetEphMask, PGPS_EPHEMERIS pEphList[])
{
	for (int i = 0; i < 63; i ++)
	{
		if ((SetEphMask & (1ULL << i)) == 0)
			continue;
		SetEphemeris(NavBitType, i + 1, pEphList[i]);
	}
	return 0;
}

int CNavBitArray::SetEphemeris(int NavBitType, int svid, PGPS_EPHEMERIS pEph)
{
	if (NavBitType < MAX_NAVBIT_TYPE && NavBitArray[NavBitType])
		return NavBitArray[NavBitType]->SetEphemeris(svid, pEph);
	else
		return -1;
}

int CNavBitArray::SetAlmanac(CNavData &NavData)
{
	SetAlmanac(NAVBIT_TYPE_LNAV, NavData.GetGpsAlmanac());
	SetAlmanac(NAVBIT_TYPE_CNAV, NavData.GetGpsAlmanac());
	SetAlmanac(NAVBIT_TYPE_CNAV2, NavData.GetGpsAlmanac());
	SetAlmanac(NAVBIT_TYPE_D1D2, NavData.GetBdsAlmanac());
	SetAlmanac(NAVBIT_TYPE_BCNAV1, NavData.GetBdsAlmanac());
	SetAlmanac(NAVBIT_TYPE_BCNAV2, NavData.GetBdsAlmanac());
	SetAlmanac(NAVBIT_TYPE_BCNAV3, NavData.GetBdsAlmanac());
	SetAlmanac(NAVBIT_TYPE_INAV, NavData.GetGalileoAlmanac());
	SetAlmanac(NAVBIT_TYPE_FNAV, NavData.GetGalileoAlmanac());
	SetAlmanac(NAVBIT_TYPE_GNAV, (PGPS_ALMANAC)NavData.GetGlonassAlmanac());
	return 0;
}

int CNavBitArray::SetAlmanac(int NavBitType, GPS_ALMANAC Alm[])
{
	if (NavBitType < MAX_NAVBIT_TYPE && NavBitArray[NavBitType])
		return NavBitArray[NavBitType]->SetAlmanac(Alm);
	else
		return -1;
}

int CNavBitArray::SetIonoUtc(CNavData &NavData)
{
	SetIonoUtc(NAVBIT_TYPE_LNAV, NavData.GetGpsIono(), NavData.GetGpsUtcParam());
	SetIonoUtc(NAVBIT_TYPE_CNAV, NavData.GetGpsIono(), NavData.GetGpsUtcParam());
	SetIonoUtc(NAVBIT_TYPE_CNAV2, NavData.GetGpsIono(), NavData.GetGpsUtcParam());
	SetIonoUtc(NAVBIT_TYPE_D1D2, NavData.GetBdsIono(), NavData.GetBdsUtcParam());
	SetIonoUtc(NAVBIT_TYPE_INAV, NavData.GetGalileoIono(), NavData.GetGalileoUtcParam());
	SetIonoUtc(NAVBIT_TYPE_FNAV, NavData.GetGalileoIono(), NavData.GetGalileoUtcParam());
	return 0;
}

int CNavBitArray::SetIonoUtc(int NavBitType, PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	if (NavBitType < MAX_NAVBIT_TYPE && NavBitArray[NavBitType])
		return NavBitArray[NavBitType]->SetIonoUtc(IonoParam, UtcParam);
	else
		return -1;
}

NavBit* CNavBitArray::GetNavBit(GnssSystem SatSystem, int SatSignalIndex)
{
	switch (SatSystem)
	{
	case GpsSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_L1CA: return NavBitArray[NAVBIT_TYPE_LNAV];
		case SIGNAL_INDEX_L1C:  return NavBitArray[NAVBIT_TYPE_CNAV2];
		case SIGNAL_INDEX_L2C:  return NavBitArray[NAVBIT_TYPE_CNAV];
		case SIGNAL_INDEX_L2P:  return NavBitArray[NAVBIT_TYPE_LNAV];
		case SIGNAL_INDEX_L5:   return NavBitArray[NAVBIT_TYPE_CNAV];
		default: return NavBitArray[NAVBIT_TYPE_LNAV];
		}
		break;
	case BdsSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_B1C: return NavBitArray[NAVBIT_TYPE_BCNAV1];
		case SIGNAL_INDEX_B1I: return NavBitArray[NAVBIT_TYPE_D1D2];
		case SIGNAL_INDEX_B2I: return NavBitArray[NAVBIT_TYPE_D1D2];
		case SIGNAL_INDEX_B3I: return NavBitArray[NAVBIT_TYPE_D1D2];
		case SIGNAL_INDEX_B2a: return NavBitArray[NAVBIT_TYPE_BCNAV2];
		case SIGNAL_INDEX_B2b: return NavBitArray[NAVBIT_TYPE_BCNAV3];
		default: return NavBitArray[NAVBIT_TYPE_D1D2];
		}
		break;
	case GalileoSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_E1:  return NavBitArray[NAVBIT_TYPE_INAV];
		case SIGNAL_INDEX_E5a: return NavBitArray[NAVBIT_TYPE_FNAV];
		case SIGNAL_INDEX_E5b: return NavBitArray[NAVBIT_TYPE_INAV];
		case SIGNAL_INDEX_E5:  return NavBitArray[NAVBIT_TYPE_FNAV];
		case SIGNAL_INDEX_E6:  return NavBitArray[NAVBIT_TYPE_ECNAV];
		default: return NavBitArray[NAVBIT_TYPE_INAV];
		}
		break;
	case GlonassSystem:
		switch (SatSignalIndex)
		{
		case SIGNAL_INDEX_G1: return NavBitArray[NAVBIT_TYPE_GNAV];
		case SIGNAL_INDEX_G2: return NavBitArray[NAVBIT_TYPE_GNAV];
		default: return NavBitArray[NAVBIT_TYPE_GNAV];
		}
		break;
	default: return NavBitArray[NAVBIT_TYPE_LNAV];
	}
}
