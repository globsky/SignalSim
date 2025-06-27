#include "BasicTypes.h"
#include "ComplexNumber.h"
#include "PrnGenerate.h"
#include "NavBit.h"
#include "SatelliteSignal.h"

class CSatIfSignal
{
public:
	CSatIfSignal(int MsSampleNumber, int SatIfFreq, GnssSystem SatSystem, int SatSignalIndex, unsigned char SatId);
	~CSatIfSignal();
	void InitState(GNSS_TIME CurTime, PSATELLITE_PARAM pSatParam, NavBit* pNavData);
	void GetIfSample(GNSS_TIME CurTime);
	complex_number *SampleArray;

private:
	int SampleNumber;	// sample number within 1ms
	int IfFreq;	// must be multiple of 500 (for GLONASS) or 1000 (other signal)
	GnssSystem System;
	int SignalIndex;
	int Svid;
	PrnGenerate* PrnSequence;
	int DataLength, PilotLength;
	CSatelliteSignal SatelliteSignal;
	PSATELLITE_PARAM SatParam;
	double StartCarrierPhase, EndCarrierPhase;
	GNSS_TIME StartTransmitTime, EndTransmitTime, SignalTime;
	complex_number DataSignal, PilotSignal;
	int GlonassHalfCycle, HalfCycleFlag;

	complex_number GetPrnValue(double &CurChip, double CodeStep);
	complex_number GetRotateValue(double & CurPhase, double PhaseStep);
	void GenerateSamplesVectorized(int SampleCount, double& CurChip, double CodeStep, double& CurPhase, double PhaseStep, double Amp);
};
