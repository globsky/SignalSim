//----------------------------------------------------------------------
// PrnGenerate.h:
//   Declaration of PRN code generation class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __PRN_GENERATE_H__
#define __PRN_GENERATE_H__

#include "BasicTypes.h"

class LsfrSequence
{
public:
	LsfrSequence(unsigned int InitState, unsigned int Polynomial, int Length);
	~LsfrSequence();
	void Initial();
	int GetOutput();

private:
	unsigned int mInitState;
	unsigned int mCurrentState;
	unsigned int mPolynomial;
	unsigned int mOutputMask;
};

struct PrnAttribute
{
	int ChipRate;	// chips per millisecond
	int DataPeriod;	// PRN period for data channel in millisecond
	int PilotPeriod;// PRN period for pilot channel in millisecond
	unsigned int Attribute;	// defined as following
};
#define PRN_ATTRIBUTE_BOC 1
#define PRN_ATTRIBUTE_TMD 2

class PrnGenerate
{
public:
	PrnGenerate(GnssSystem System, int SignalIndex, int Svid);
	~PrnGenerate();

	int *DataPrn, *PilotPrn;
	const PrnAttribute* Attribute;

private:
	int *GetGoldCode(unsigned int G1Init, unsigned int G1Poly, unsigned int G2Init, unsigned int G2Poly, int Length, int Depth, int ResetPos);
	void LegendreSequence(int *Data, int Length);
	int *GetL1CWeil(int InsertPoint, int PhaseDiff);
	int *GetB1CWeil(int TruncationPoint, int PhaseDiff);
	int *GetMemorySequence(const unsigned int *BinarySequence, int SectorLength);

	static const unsigned int L1CAPrnInit[32];
	static const unsigned int L5IPrnInit[32];
	static const unsigned int L5QPrnInit[32];
	static const unsigned int L2CMPrnInit[32];
	static const unsigned int L2CLPrnInit[32];
	static const unsigned int B1IPrnInit[63];
	static const unsigned int B3IPrnInit[63];
	static const unsigned int B2aDPrnInit[63];
	static const unsigned int B2aPPrnInit[63];
	static const unsigned int B2bPrnInit[63];
	static const unsigned int E5aIPrnInit[50];
	static const unsigned int E5aQPrnInit[50];
	static const unsigned int E5bIPrnInit[50];
	static const unsigned int E5bQPrnInit[50];
	static const int B1CDataTruncation[63];
	static const int B1CDataPhaseDiff[63];
	static const int B1CPilotTruncation[63];
	static const int B1CPilotPhaseDiff[63];
	static const int L1CDataInsertIndex[63];
	static const int L1CDataPhaseDiff[63];
	static const int L1CPilotInsertIndex[63];
	static const int L1CPilotPhaseDiff[63];
	static const unsigned int E1MemoryCode[100*128];
	static const unsigned int E6MemoryCode[100*160];
	static const PrnAttribute PrnAttributes[];
};

#endif // __PRN_GENERATE_H__
