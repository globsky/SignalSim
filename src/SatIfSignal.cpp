#include <math.h>
#include <stdio.h>
#include <memory.h>

#include "SatIfSignal.h"
#include "FastMath.h"

CSatIfSignal::CSatIfSignal(int MsSampleNumber, int SatIfFreq, GnssSystem SatSystem, int SatSignalIndex, unsigned char SatId) : SampleNumber(MsSampleNumber), IfFreq(SatIfFreq), System(SatSystem), SignalIndex(SatSignalIndex), Svid((int)SatId)
{
	SampleArray = new complex_number[SampleNumber];
	PrnSequence = new PrnGenerate(System, SignalIndex, Svid);
	SatParam = NULL;

	if (!PrnSequence->Attribute || !PrnSequence->DataPrn)
		DataLength = PilotLength = 0;
	else
	{
	DataLength = PrnSequence->Attribute->DataPeriod * PrnSequence->Attribute->ChipRate;
	PilotLength = PrnSequence->Attribute->PilotPeriod * PrnSequence->Attribute->ChipRate;
	}
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
	int IsBoc = (PrnSequence->Attribute->Attribute) & PRN_ATTRIBUTE_BOC;
	int IsL2C = (PrnSequence->Attribute->Attribute) & PRN_ATTRIBUTE_TMD;

	if (DataLength == 0)
		return complex_number(0, 0);

	DataChip = ChipCount % DataLength;
	if (IsBoc || IsL2C)
		DataChip >>= 1;
	PrnValue = DataSignal * (PrnSequence->DataPrn[DataChip] ? -1 : 1);
	if (PrnSequence->PilotPrn && PilotLength > 0)
	{
		PilotChip = ChipCount % PilotLength;
		if (IsBoc || IsL2C)
			PilotChip >>= 1;
		if (IsL2C)
		{
			if (ChipCount & 1)	// L2CL slot
				PrnValue = PilotSignal * (PrnSequence->PilotPrn[PilotChip] ? -1 : 1);
		}
		else
		PrnValue += PilotSignal * (PrnSequence->PilotPrn[PilotChip] ? -1 : 1);
	}
	if (IsBoc && (ChipCount & 1))	// second half of BOC code
		PrnValue *= -1;
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
//	complex_number Rotate = complex_number(cos(CurPhase * PI2), sin(CurPhase * PI2));
	complex_number Rotate = FastMath::FastRotate(CurPhase * PI2);
	CurPhase += PhaseStep;
	return Rotate;
}

// Cannot correctly deal with data/secondary code modulation yet, just put here for future optimize
void CSatIfSignal::GenerateSamplesVectorized(int SampleCount, double& CurChip, double CodeStep, double& CurPhase, double PhaseStep, double Amp)
{
	// MEGA-OPTIMIZED generation for maximum speed
	if (!PrnSequence || !PrnSequence->DataPrn)
	{
		// Fast zero-padding
		memset(SampleArray, 0, SampleCount * sizeof(complex_number));
		CurChip += CodeStep * SampleCount;
		CurPhase += PhaseStep * SampleCount;
		return;
	}

	// Precomputed constants for speed
	const int* __restrict DataPrn = PrnSequence->DataPrn;
	const int* __restrict PilotPrn = PrnSequence->PilotPrn;
	const double DataReal = DataSignal.real;
	const double DataImag = DataSignal.imag;
	const double PilotReal = PilotSignal.real;
	const double PilotImag = PilotSignal.imag;
	const double PI2_local = PI2;

	// SUPER-FAST generation without unnecessary checks
//	#pragma omp simd
	for (int i = 0; i < SampleCount; i++)
	{
		// PRN index with minimal calculations
		const int ChipCount = (int)(CurChip + i * CodeStep);
		const int DataChip = ChipCount % DataLength;

		// Fast getting of PRN value
		const double dataSign = DataPrn[DataChip] ? -1.0 : 1.0;
		double prnReal = DataReal * dataSign;
		double prnImag = DataImag * dataSign;

		// Pilot PRN only if present
		if (PilotPrn && PilotLength > 0)
		{
			const int PilotChip = ChipCount % PilotLength;
			const double pilotSign = PilotPrn[PilotChip] ? -1.0 : 1.0;
			prnReal += PilotReal * pilotSign;
			prnImag += PilotImag * pilotSign;
		}

		// Fast phase rotation
		const double phase = (CurPhase + i * PhaseStep) * PI2_local;
		const complex_number rotation = FastMath::FastRotate(phase);

		// Final signal with inline multiplication
		SampleArray[i].real = (prnReal * rotation.real - prnImag * rotation.imag) * Amp;
		SampleArray[i].imag = (prnReal * rotation.imag + prnImag * rotation.real) * Amp;
	}

	// Update state
	CurChip += CodeStep * SampleCount;
	CurPhase += PhaseStep * SampleCount;
}
