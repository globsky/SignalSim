//----------------------------------------------------------------------
// SatelliteSignal.h:
//   Definition of functions to calculate satellite signal with data/pilot modulation
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__SATELLITE_SIGNAL_H__)
#define __SATELLITE_SIGNAL_H__

#include "BasicTypes.h"
#include "ComplexNumber.h"
#include "SatelliteParam.h"
#include "NavBit.h"

struct SignalAttribute
{
	int CodeLength;	// PRN length in unit of millisecond
	int NHLength;	// length of one data bit in unit of PRN period
	int NHCode;		// NH for data modulation
	int FrameLength;	// length of frame/subframe in unit of millisecond
//	int SecondaryLength;	// length of secondary code
//	int SecondarySize;	// size of secondary code array for one SV in DWORD (0 if all SV uses the same secondary code)
//	const unsigned int *SecondaryCode;	// array of secondary code
};

class CSatelliteSignal
{
public:
	CSatelliteSignal();
	~CSatelliteSignal();

	BOOL SetSignalAttribute(GnssSystem System, int SignalIndex, NavBit *pNavData, int svid);
	BOOL GetSatelliteSignal(GNSS_TIME TransmitTime, complex_number &DataSignal, complex_number &PilotSignal);

	// signal attributes
	GnssSystem SatSystem;
	int SatSignal;
	int Svid;
	NavBit *NavData;
	const SignalAttribute* Attribute;

	// variables used to calculate modulated signal
	int CurrentFrame;		// frame number of data stream filling in Bits
	int CurrentBitIndex;	// bit index used for current ms correlation result
	int DataBits[1800];		// maximum 1800 encoded data bit for one subframe/page

	// constant arrays for signal attributes and NH code
	static const SignalAttribute SignalAttributes[32];
};

#endif //!defined(__SATELLITE_SIGNAL_H__)
