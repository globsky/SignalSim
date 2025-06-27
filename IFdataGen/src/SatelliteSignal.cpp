//----------------------------------------------------------------------
// SatelliteSignal.cpp:
//   Implementation of functions to calculate satellite signal with data/pilot modulation
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <typeinfo>
#include <string.h>

#include "GnssTime.h"
#include "SatelliteSignal.h"
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
#include "PilotBit.h"

const SignalAttribute CSatelliteSignal::SignalAttributes[32] = {
 // CodeLength NHLength  NHCode FrameLength 
{        1,       20,     0x0,      6000,   },	// index  0 for LNAV
{       10,        1,     0x0,     18000,   },	// index  1 for CNAV2
{       20,        1,     0x0,     12000,   },	// index  2 for CNAV L2C
{        1,       10,   0x2b0,      6000,   },	// index  3 for CNAV L5
{       10,        1,     0x0,     18000,   },	// index  4 for BCNAV1
{        1,       20, 0x72b20,      6000,   },	// index  5 for D1
{        1,        2,     0x0,       600,   },	// index  6 for D2
{        1,        5,     0x8,      3000,   },	// index  7 for BCNAV2
{        1,        1,     0x0,      1000,   },	// index  8 for BCNAV3
{        4,        1,     0x0,      2000,   },	// index  9 for I/NAV E1
{        1,       20, 0x97421,     10000,   },	// index 10 for FNAV
{        1,        4,     0x7,      2000,   },	// index 11 for I/NAV E5b
{        1,        1,     0x0,      1000,   },	// index 12 for CNAV E6
{        1,       10,     0x0,      2000,   },	// index 13 for GNAV
};

CSatelliteSignal::CSatelliteSignal()
{
	CurrentFrame = Svid = -1;
	NavData = (NavBit *)0;
}

CSatelliteSignal::~CSatelliteSignal()
{
}

BOOL CSatelliteSignal::SetSignalAttribute(GnssSystem System, int SignalIndex, NavBit *pNavData, int svid)
{
	// member variable assignment
	SatSystem = System;
	SatSignal = SignalIndex;
	NavData = pNavData;
	Svid = svid;
	CurrentFrame = -1;	// reset current frame to force fill DataBits[] on next call to GetSatelliteSignal()
	memset(DataBits, 0, sizeof(DataBits));

	// signal and navigation bit match
	switch (SatSystem)
	{
	case GpsSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_L1CA: 
			Attribute = &SignalAttributes[0];
			return NavData ? ((typeid(*NavData) == typeid(LNavBit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_L1C: 
			Attribute = &SignalAttributes[1];
			return NavData ? ((typeid(*NavData) == typeid(CNav2Bit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_L2C:
			Attribute = &SignalAttributes[2];
			return NavData ? ((typeid(*NavData) == typeid(CNavBit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_L5:
			Attribute = &SignalAttributes[3];
			return NavData ? ((typeid(*NavData) == typeid(CNavBit)) ? TRUE : FALSE) : TRUE;
		default: return FALSE;	// unknown FreqIndex
		}
	case BdsSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_B1C: 
			Attribute = &SignalAttributes[4];
			return NavData ? ((typeid(*NavData) == typeid(BCNav1Bit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_B1I:
		case SIGNAL_INDEX_B2I:
		case SIGNAL_INDEX_B3I:
			Attribute = ((Svid < 6) || (Svid > 58)) ? &SignalAttributes[6] : &SignalAttributes[5];
			return NavData ? ((typeid(*NavData) == typeid(D1D2NavBit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_B2a:
			Attribute = &SignalAttributes[7];
			return NavData ? ((typeid(*NavData) == typeid(BCNav2Bit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_B2b:
			Attribute = &SignalAttributes[8];
			return NavData ? ((typeid(*NavData) == typeid(BCNav3Bit)) ? TRUE : FALSE) : TRUE;
		default: return FALSE;	// unknown FreqIndex
		}
	case GalileoSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_E1 :
			Attribute = &SignalAttributes[9];
			return NavData ? ((typeid(*NavData) == typeid(INavBit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_E5a:
			Attribute = &SignalAttributes[10];
			return NavData ? ((typeid(*NavData) == typeid(FNavBit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_E5b:
			Attribute = &SignalAttributes[11];
			return NavData ? ((typeid(*NavData) == typeid(INavBit)) ? TRUE : FALSE) : TRUE;
		case SIGNAL_INDEX_E6 :
			Attribute = &SignalAttributes[12];
			return TRUE;
//			return NavData ? ((typeid(*NavData) == typeid(ENavBit)) ? TRUE : FALSE) : TRUE;
		default: return FALSE;	// unknown FreqIndex
		}
	case GlonassSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_G1:
		case SIGNAL_INDEX_G2:
			Attribute = &SignalAttributes[13];
			return NavData ? ((typeid(*NavData) == typeid(GNavBit)) ? TRUE : FALSE) : TRUE;
		default: return FALSE;	// unknown FreqIndex
		}
	default: return FALSE;	// unknown system
	}
}

#define AMPLITUDE_1_2 0.7071067811865475244
#define AMPLITUDE_1_4 0.5
#define AMPLITUDE_29_44 0.811844140885988713377
#define AMPLITUDE_3_4 0.8660254037844386468
#define AMPLITUDE_5_11 0.6741998624632421

BOOL CSatelliteSignal::GetSatelliteSignal(GNSS_TIME TransmitTime, complex_number &DataSignal, complex_number &PilotSignal)
{
	int Milliseconds;
	int BitLength = (Attribute->CodeLength * Attribute->NHLength);
	int FrameNumber, BitNumber, BitPos, SecondaryPosition;
	int DataBit, PilotBit = 0;
	int SecondaryLength;
	const unsigned int *SecondaryCode = GetPilotBits(SatSystem, SatSignal, Svid, SecondaryLength);
	int Seconds, LeapSecond;
	int GalileoE1Signal = (SatSystem == GalileoSystem && SatSignal == SIGNAL_INDEX_E1) ? 1 : 0;
	int Param = ((SatSystem == GpsSystem && SatSignal == SIGNAL_INDEX_L5) ? 1 : 0) || GalileoE1Signal;	// set to 1 for E1 or L5

	if (Svid < 0)	// attribute not yet set
		return FALSE;
	if (SatSystem == BdsSystem)	// subtract leap second difference
		TransmitTime.MilliSeconds -= 14000;
	else if (SatSystem == GlonassSystem)	// subtract leap second, add 3 hours
	{
		Seconds = (unsigned int)(TransmitTime.Week * 604800 + TransmitTime.MilliSeconds / 1000);
		GetLeapSecond(Seconds, LeapSecond);
		TransmitTime.MilliSeconds = (TransmitTime.MilliSeconds + 10800000 - LeapSecond * 1000) % 86400000;
	}
	if (TransmitTime.MilliSeconds < 0)	// protection on negative millisecond
		TransmitTime.MilliSeconds += 604800000;

	Milliseconds = TransmitTime.MilliSeconds + (GalileoE1Signal ? 1000 : 0);	// E1 page has 1000ms bias to week boundary
	FrameNumber = Milliseconds / Attribute->FrameLength;	// subframe/page number
	Milliseconds %= Attribute->FrameLength;
	BitNumber = Milliseconds / BitLength;	// current bit position within current subframe/page
	Milliseconds %= BitLength;
	BitPos = Milliseconds / Attribute->CodeLength;	// coded round in data bit

	if (FrameNumber != CurrentFrame)
	{
		if (NavData && Svid > 0 && Svid <= 32)
			NavData->GetFrameData(TransmitTime, Svid, Param, DataBits);
		CurrentFrame = FrameNumber;
	}

	DataBit = (DataBits[BitNumber] ? -1 : 1) * ((Attribute->NHCode & (1 << BitPos)) ? -1 : 1);
	if (SecondaryCode)
	{
		SecondaryPosition = (TransmitTime.MilliSeconds / Attribute->CodeLength) % SecondaryLength;	// position in secondary code
		PilotBit = (SecondaryCode[SecondaryPosition / 32] & (1 << (SecondaryPosition & 0x1f))) ? -1 : 1;
	}

	// generate DataSignal and PilotSignal
	// the signal of data and pilot complex value will reflect their relative amplitude and phase
	// if the signal has only data channel, assume the data channel has 0 phase
	// if the signal has both data channel and pilot channel, the pilot channel has 0 phase
	// the overall data+pilot power is 1 (except for the case that need to deduct BOC(6,1) element)
	// signal and navigation bit match
	switch (SatSystem)
	{
	case GpsSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_L1CA:
			DataSignal = complex_number((double)DataBit, 0);
			PilotSignal = complex_number(0, 0);
			break;
		case SIGNAL_INDEX_L1C:
			DataSignal = complex_number(DataBit * AMPLITUDE_1_4, 0);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_29_44, 0);
			break;
		case SIGNAL_INDEX_L2C:
			DataSignal = complex_number(DataBit * AMPLITUDE_1_2, 0);
			PilotSignal = complex_number(AMPLITUDE_1_2, 0);
			break;
		case SIGNAL_INDEX_L5:
			DataSignal = complex_number(0, DataBit * AMPLITUDE_1_2);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_1_2, 0);
			break;
		}
		break;
	case BdsSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_B1C: 
			DataSignal = complex_number(0, -DataBit * AMPLITUDE_1_4);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_29_44, 0);
			break;
		case SIGNAL_INDEX_B1I:
		case SIGNAL_INDEX_B2I:
		case SIGNAL_INDEX_B3I:
			DataSignal = complex_number((double)DataBit, 0);
			PilotSignal = complex_number(0, 0);
			break;
		case SIGNAL_INDEX_B2a:
			DataSignal = complex_number(0, -DataBit * AMPLITUDE_1_2);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_1_2, 0);
			break;
		case SIGNAL_INDEX_B2b:
			DataSignal = complex_number(0, -DataBit * AMPLITUDE_1_2);	// B2b nominal power is 3dB lower than B2a, phase align with B2a data
			PilotSignal = complex_number(0, 0);
			break;
		}
		break;
	case GalileoSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_E1 :
			DataSignal = complex_number(-DataBit * AMPLITUDE_1_2, 0);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_1_2, 0);
			break;
		case SIGNAL_INDEX_E5a:
		case SIGNAL_INDEX_E5b:
			DataSignal = complex_number(0, -DataBit * AMPLITUDE_1_2);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_1_2, 0);
			break;
		case SIGNAL_INDEX_E6 :
			DataSignal = complex_number(0, 0);
			PilotSignal = complex_number(PilotBit * AMPLITUDE_1_2, 0);
			break;
		}
		break;
	case GlonassSystem:
		switch (SatSignal)
		{
		case SIGNAL_INDEX_G1 :
		case SIGNAL_INDEX_G2 :
			DataSignal = complex_number((double)DataBit, 0);
			PilotSignal = complex_number(0, 0);
			break;
		}
		break;
	}

	return TRUE;
}
