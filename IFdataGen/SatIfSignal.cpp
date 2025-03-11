#include <math.h>

#include "SatIfSignal.h"

CSatIfSignal::CSatIfSignal(int MsSampleNumber, int SatIfFreq, GnssSystem SatSystem, int SatSignalIndex, unsigned char SatId) : SampleNumber(MsSampleNumber), IfFreq(SatIfFreq), System(SatSystem), SignalIndex(SatSignalIndex), Svid((int)SatId)
{
	SampleArray = new complex_number[SampleNumber];
	PrnSequence = new PrnCode(System, SignalIndex, Svid);
	SatParam = NULL;
	DataLength = PrnSequence->Attribute->DataPeriod * PrnSequence->Attribute->ChipRate;
	PilotLength = PrnSequence->Attribute->PilotPeriod * PrnSequence->Attribute->ChipRate;
	GlonassHalfCycle = ((IfFreq % 1000) != 0) ? 1 : 0;
}

CSatIfSignal::~CSatIfSignal()
{
	delete[] SampleArray;
	SampleArray = NULL;
	delete PrnSequence;
	PrnSequence = NULL;
}

void CSatIfSignal::InitState(GNSS_TIME CurTime, PSATELLITE_PARAM pSatParam, NavBit* pNavData)
{
	SatParam = pSatParam;
	if (!SatelliteSignal.SetSignalAttribute(System, SignalIndex, pNavData, Svid))
		SatelliteSignal.NavData = (NavBit*)0;	// if system/frequency and navigation data not match, set pointer to NULL
	StartCarrierPhase = GetCarrierPhase(SatParam, SignalIndex);
	SignalTime = StartTransmitTime = GetTransmitTime(CurTime, GetTravelTime(SatParam, SignalIndex));
	SatelliteSignal.GetSatelliteSignal(SignalTime, DataSignal, PilotSignal);
	HalfCycleFlag = 0;
}

void CSatIfSignal::GetIfSample(GNSS_TIME CurTime)
{
	int i, TransmitMsDiff;
	double CurPhase, PhaseStep, CurChip, CodeDiff, CodeStep;
	const PrnAttribute* CodeAttribute = PrnSequence->Attribute;
	complex_number IfSample;
	double Amp = pow(10, (SatParam->CN0 - 3000) / 1000.) / sqrt(SampleNumber);

	if (!SatParam)
		return;
	SignalTime = StartTransmitTime;
	SatelliteSignal.GetSatelliteSignal(SignalTime, DataSignal, PilotSignal);
	EndCarrierPhase = GetCarrierPhase(SatParam, SignalIndex);
	EndTransmitTime = GetTransmitTime(CurTime, GetTravelTime(SatParam, SignalIndex));

	// calculate start/end signal phase and phase step (actual local signal phase is negative ADR)
	PhaseStep = (StartCarrierPhase - EndCarrierPhase) / SampleNumber;
	PhaseStep += IfFreq / 1000. / SampleNumber;
	CurPhase = StartCarrierPhase - (int)StartCarrierPhase;
	CurPhase = 1 - CurPhase;	// carrier is fractional part of negative of travel time, equvalent to 1 minus positive fractional part
	StartCarrierPhase = EndCarrierPhase;
	if (GlonassHalfCycle)	// for GLONASS odd number FreqID, nominal IF result in half cycle toggle every 1ms
	{
		CurPhase += HalfCycleFlag ? 0.5 : 0.0;
		HalfCycleFlag = 1 - HalfCycleFlag;
	}

	// get PRN count for each sample
	TransmitMsDiff = EndTransmitTime.MilliSeconds - StartTransmitTime.MilliSeconds;
	if (TransmitMsDiff < 0)
		TransmitMsDiff += 86400000;
	CodeDiff = (TransmitMsDiff + EndTransmitTime.SubMilliSeconds - StartTransmitTime.SubMilliSeconds) * CodeAttribute->ChipRate;
	CodeStep = CodeDiff / SampleNumber;	// code increase between each sample
	CurChip = (StartTransmitTime.MilliSeconds % CodeAttribute->PilotPeriod + StartTransmitTime.SubMilliSeconds) * CodeAttribute->ChipRate;
	StartTransmitTime = EndTransmitTime;

	for (i = 0; i < SampleNumber; i ++)
	{
		SampleArray[i] = GetPrnValue(CurChip, CodeStep) * GetRotateValue(CurPhase, PhaseStep) * Amp;
	}
}

complex_number CSatIfSignal::GetPrnValue(double& CurChip, double CodeStep)
{
	int ChipCount = (int)CurChip;
	int DataChip, PilotChip;
	complex_number PrnValue;

	DataChip = ChipCount % DataLength;
	PrnValue = DataSignal * (PrnSequence->DataPrn[DataChip] ? -1 : 1);
	if (PrnSequence->PilotPrn)
	{
		PilotChip = ChipCount % PilotLength;
		PrnValue += PilotSignal * (PrnSequence->PilotPrn[PilotChip] ? -1 : 1);
	}
	CurChip += CodeStep;
	// check whether go beyond next code period (pilot code period multiple of data code period, so only check data period)
	if ((((int)CurChip) % DataLength) < DataChip)
	{
		SignalTime.MilliSeconds += PrnSequence->Attribute->DataPeriod;
		SatelliteSignal.GetSatelliteSignal(SignalTime, DataSignal, PilotSignal);
	}
	return PrnValue;
}

complex_number CSatIfSignal::GetRotateValue(double& CurPhase, double PhaseStep)
{
	complex_number Rotate = complex_number(cos(CurPhase * PI2), sin(CurPhase * PI2));
	CurPhase += PhaseStep;
	return Rotate;
}
