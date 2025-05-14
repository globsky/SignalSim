//----------------------------------------------------------------------
// NavBit.h:
//   Declaration of navigation bit synthesis base class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __NAV_BIT_H__
#define __NAV_BIT_H__

#include "BasicTypes.h"

#define COMPOSE_BITS(data, start, width) (((data) & ((1UL << (width)) - 1)) << (start))

typedef union
{
	double d_data;
	unsigned int i_data[2];
} DOUBLE_INT_UNION;

class NavBit
{
public:
	enum NavParamType { ParamTypeSTO, ParamTypeEOP, ParamTypeIonKModel, ParamTypeIonGModel, ParamTypeIonBModel };

	NavBit();
	~NavBit();

	virtual int GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits) = 0;	// Param reserved for same Navigation bit structure in different signal
	virtual int SetEphemeris(int svid, PGPS_EPHEMERIS Eph) = 0;
	virtual int SetAlmanac(GPS_ALMANAC Alm[]) = 0;
	virtual int SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam) = 0;
	virtual int SetNavParam(NavParamType ParamType, void *Param) { return 0; }
	int roundi(double data);
	int roundu(double data);
	double UnscaleDouble(double value, int scale);
	int UnscaleInt(double value, int scale);
	unsigned int UnscaleUint(double value, int scale);
	long long int UnscaleLong(double value, int scale);
	unsigned long long int UnscaleULong(double value, int scale);
	int AssignBits(unsigned int Data, int BitNumber, int BitStream[]);
	unsigned char ConvolutionEncode(unsigned char EncodeBits);
	unsigned int Crc24qEncode(unsigned int *BitStream, int Length);

	static const unsigned char ConvEncodeTable[256];
	static const unsigned int Crc24q[256];
};

#endif // __NAV_BIT_H__
