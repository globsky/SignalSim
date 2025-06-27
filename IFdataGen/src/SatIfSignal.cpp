#include <math.h>
#include <stdio.h>
#include <cstring>

#include "SatIfSignal.h"
#include "../inc/FastMath.h"

CSatIfSignal::CSatIfSignal(int MsSampleNumber, int SatIfFreq, GnssSystem SatSystem, int SatSignalIndex, unsigned char SatId) : SampleNumber(MsSampleNumber), IfFreq(SatIfFreq), System(SatSystem), SignalIndex(SatSignalIndex), Svid((int)SatId)
{
	SampleArray = new complex_number[SampleNumber];
	PrnSequence = new PrnGenerate(System, SignalIndex, Svid);
	
	// Validate PrnSequence creation
	if (!PrnSequence || !PrnSequence->Attribute || !PrnSequence->DataPrn) {
		// Handle invalid PRN generation
		if (PrnSequence) {
			delete PrnSequence;
			PrnSequence = NULL;
		}
		DataLength = 1;  // Set to safe default
		PilotLength = 1;
		SatParam = NULL;
		GlonassHalfCycle = ((IfFreq % 1000) != 0) ? 1 : 0;
		return;
	}
	
	SatParam = NULL;
	DataLength = PrnSequence->Attribute->DataPeriod * PrnSequence->Attribute->ChipRate;
	PilotLength = PrnSequence->Attribute->PilotPeriod * PrnSequence->Attribute->ChipRate;
	
	// Fix array sizes for specific signals
	if (System == GpsSystem && SignalIndex == SIGNAL_INDEX_L2C) {
		// GPS L2C actual array sizes from PrnGenerate.cpp
		DataLength = 10230;      // GetGoldCode(..., 10230, ...)
		PilotLength = 10230 * 75; // GetGoldCode(..., 10230*75, ...)
	}
	
	// Ensure positive lengths
	if (DataLength <= 0) DataLength = 1;
	if (PilotLength <= 0) PilotLength = 1;
	
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

	// Оптимизированная генерация с сохранением корректности
	for (i = 0; i < SampleNumber; i ++)
	{
		SampleArray[i] = GetPrnValue(CurChip, CodeStep) * GetRotateValue(CurPhase, PhaseStep) * Amp;
	}
}

complex_number CSatIfSignal::GetPrnValue(double& CurChip, double CodeStep)
{
	// Add comprehensive null checks
	if (!PrnSequence || !PrnSequence->Attribute) {
		CurChip += CodeStep;
		return complex_number(0, 0);  // Return zero signal for invalid sequence
	}
	
	const int ChipCount = (int)CurChip;
	int DataChip, PilotChip;
	complex_number PrnValue;
	const int IsBoc = (PrnSequence->Attribute->Attribute) & PRN_ATTRIBUTE_BOC;

	// Validate DataLength to prevent division by zero
	if (DataLength <= 0) {
		CurChip += CodeStep;
		return complex_number(0, 0);
	}
	
	// Cache frequently used values
	const int* DataPrn = PrnSequence->DataPrn;
	const int* PilotPrn = PrnSequence->PilotPrn;
	
	DataChip = ChipCount % DataLength;
	if (IsBoc)
		DataChip >>= 1;  // Faster division by 2
		
	// Add bounds checking for DataPrn
	if (!DataPrn || DataChip < 0 || DataChip >= DataLength) {
		PrnValue = complex_number(0, 0);
	} else {
		PrnValue = DataSignal * (DataPrn[DataChip] ? -1.0 : 1.0);
	}
	
	// Enhanced pilot signal processing with bounds checking
	if (PilotPrn && PilotLength > 0) {
		PilotChip = ChipCount % PilotLength;
		if (IsBoc)
			PilotChip >>= 1;  // Faster division by 2
			
		// Add explicit bounds check for PilotChip after BOC adjustment
		if (PilotChip >= 0 && PilotChip < PilotLength) {
			PrnValue += PilotSignal * (PilotPrn[PilotChip] ? -1.0 : 1.0);
		}
	}
	
	if (IsBoc && (ChipCount & 1))	// second half of BOC code
		PrnValue *= -1.0;
	CurChip += CodeStep;
	// check whether go beyond next code period (pilot code period multiple of data code period, so only check data period)
	if (DataLength > 0 && (((int)CurChip) % DataLength) < DataChip)
	{
		SignalTime.MilliSeconds += PrnSequence->Attribute->DataPeriod;
		SatelliteSignal.GetSatelliteSignal(SignalTime, DataSignal, PilotSignal);
	}
	return PrnValue;
}

complex_number CSatIfSignal::GetRotateValue(double& CurPhase, double PhaseStep)
{
	complex_number Rotate = FastMath::FastRotate(CurPhase * PI2);
	CurPhase += PhaseStep;
	return Rotate;
}

void CSatIfSignal::GenerateSamplesVectorized(int SampleCount, double& CurChip, double CodeStep, double& CurPhase, double PhaseStep, double Amp)
{
	// МЕГАОПТИМИЗИРОВАННАЯ генерация для максимальной скорости
	if (!PrnSequence || !PrnSequence->DataPrn) {
		// Быстрое заполнение нулями
		memset(SampleArray, 0, SampleCount * sizeof(complex_number));
		CurChip += CodeStep * SampleCount;
		CurPhase += PhaseStep * SampleCount;
		return;
	}
	
	// Предвычисленные константы для ускорения
	const int* __restrict DataPrn = PrnSequence->DataPrn;
	const int* __restrict PilotPrn = PrnSequence->PilotPrn;
	const double DataReal = DataSignal.real;
	const double DataImag = DataSignal.imag;
	const double PilotReal = PilotSignal.real;
	const double PilotImag = PilotSignal.imag;
	const double PI2_local = PI2;
	
	// СУПЕРСКОРОСТНАЯ генерация без лишних проверок
	#pragma omp simd 
	for (int i = 0; i < SampleCount; i++) {
		// PRN индекс с минимальными вычислениями
		const int ChipCount = (int)(CurChip + i * CodeStep);
		const int DataChip = ChipCount % DataLength;
		
		// Быстрое получение PRN значения
		const double dataSign = DataPrn[DataChip] ? -1.0 : 1.0;
		double prnReal = DataReal * dataSign;
		double prnImag = DataImag * dataSign;
		
		// Pilot PRN только если есть
		if (PilotPrn && PilotLength > 0) {
			const int PilotChip = ChipCount % PilotLength;
			const double pilotSign = PilotPrn[PilotChip] ? -1.0 : 1.0;
			prnReal += PilotReal * pilotSign;
			prnImag += PilotImag * pilotSign;
		}
		
		// Быстрое фазовое вращение
		const double phase = (CurPhase + i * PhaseStep) * PI2_local;
		const complex_number rotation = FastMath::FastRotate(phase);
		
		// Финальный сигнал с inline умножением
		SampleArray[i].real = (prnReal * rotation.real - prnImag * rotation.imag) * Amp;
		SampleArray[i].imag = (prnReal * rotation.imag + prnImag * rotation.real) * Amp;
	}
	
	// Обновляем состояние
	CurChip += CodeStep * SampleCount;
	CurPhase += PhaseStep * SampleCount;
}
