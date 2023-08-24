//----------------------------------------------------------------------
// INavBit.h:
//   Implementation of navigation bit synthesis class for I/NAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "INavBit.h"

const int INavBit::WordAllocationE1[15] = {
	2, 4, 6, 7, 8, 17, 19, 16, 0, 0, 1, 3, 5, 0, 16,
};

const int INavBit::WordAllocationE5[15] = {
	1, 3, 5, 7, 8, 0, 0, 0, 0, 0, 2, 4, 6, 0, 0,
};

const int INavBit::SyncPattern[10] = {
	0, 1, 0, 1, 1, 0, 0, 0, 0, 0
};

INavBit::INavBit()
{
	GalSpareData[0] = 0x02000000; GalSpareData[1] = GalSpareData[2] = GalSpareData[3] = 0;
	GalDummyData[0] = 0xfc000000; GalDummyData[1] = GalDummyData[2] = GalDummyData[3] = 0;
}

INavBit::~INavBit()
{
}

// Param is used to distinguish from E1 and E5b (0 for E1)
int INavBit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i, j, TOW, subframe, page;
	int Word, BitCount;
	unsigned int *Data;	// 128bit Data
	unsigned int EncodeData[7], CrcResult, EncodeWord;	// 196bit to be encoded by CRC
	unsigned int SSP[3] = { 0x04000000, 0x2b000000, 0x2f000000 };
	unsigned char EvenPart[30], OddPart[30], ConvEncodeBits;	// each part contains 8x30 bits

	// first determine the current TOW and subframe number
	StartTime.Week += StartTime.MilliSeconds / 604800000;
	StartTime.MilliSeconds %= 604800000;
	TOW = StartTime.MilliSeconds / 2000 * 2 - ((Param == 0) ? 1 : 0);	// Param == 0 for E1 I/NAV
	if (TOW < 0)
		TOW += 604800;
	subframe = (TOW % 720) / 30;
	page = (TOW % 30) / 2;
	Word = Param ? WordAllocationE5[page] : WordAllocationE1[page];
	if (Word > 6) Word = 63;	// temporarily put all word exceed 6 as dummy word
	if ((subframe & 1) && ((Word == 7) || (Word == 8)))	// Word 7/9 and Word 8/10 toggle for different subframe
		Word += 2;
	Data = GetWordData(svid, Word);
	// add WN/TOW for Word 0/5/6
	if (Word == 0)
		Data[3] = (StartTime.Week << 20) + TOW;
	else if (Word == 5)
	{
		Data[2] &= 0xff800000;	// clear 23LSB
		Data[2] |= ((StartTime.Week & 0xfff) << 11) + (TOW >> 9);
		Data[3] = TOW << 23;
	}
	else if (Word == 6)
	{
		Data[3] &= 0xff800000;	// clear 23LSB
		Data[3] |= TOW << 3;
	}

	// put into EncodeData to do CRC24Q encoding (totally 196 bits, leaving 28MSB of EncodeData[0] as 0s)
	EncodeData[0] = Data[0] >> 30;
	EncodeData[1] = (Data[0] << 2) | (Data[1] >> 30);
	EncodeData[2] = (Data[1] << 2) | (Data[2] >> 30);
	EncodeData[3] = (Data[2] << 2) | (Data[3] >> 30);
	EncodeData[4] = ((Data[3] << 2) & 0xfffc0000) | (Data[3] & 0xffff);
	EncodeData[5] = 0;	// 32MSB of Reserved 1
	EncodeData[6] = 0;	// 8LSB of Reserved 1, SAR and Spare bits are 0
	CrcResult = Crc24qEncode(EncodeData, 196);

	// do convolution encode on even part (EncodeData[0] bit3 through EncodeData[4] bit18)
	ConvEncodeBits = 0;
	EncodeWord = EncodeData[0] << 28;	// move to MSB
	for (i = 0, BitCount = 28; i < 114 / 2; i ++, BitCount += 2)
	{
		EvenPart[i/2] = (EvenPart[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
		if ((BitCount % 32) == 0)
			EncodeWord = EncodeData[BitCount >> 5];
	}
	EncodeWord = 0;	// append 6 zeros as tail
	EvenPart[28] = (EvenPart[28] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EvenPart[29] = GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EvenPart[29] = (EvenPart[29] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	// do convolution encode on odd part (EncodeData[4] bit17 through EncodeData[6] bit0)
	ConvEncodeBits = 0;
	EncodeWord = EncodeData[4] << 14;	// move to MSB
	for (i = 0, BitCount = 142; i < 82 / 2; i ++, BitCount += 2)
	{
		OddPart[i/2] = (OddPart[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
		if ((BitCount % 32) == 0)
			EncodeWord = EncodeData[BitCount >> 5];
	}
	EncodeWord = CrcResult << 8;
	for (; i < 106 / 2; i ++)	// encode CRC
		OddPart[i/2] = (OddPart[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EncodeWord = Param ? 0 : SSP[page % 3];
	for (; i < 114 / 2; i ++)	// encode SSP
		OddPart[i/2] = (OddPart[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EncodeWord = 0;	// append 6 zeros as tail
	OddPart[28] = (OddPart[28] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	OddPart[29] = GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	OddPart[29] = (OddPart[29] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);

	// do interleaving and put into NavBits
	for (i = 0; i < 10; i ++)
		*(NavBits ++) = SyncPattern[i];
	for (i = 0, ConvEncodeBits = 0x80; i < 8; i ++, ConvEncodeBits >>= 1)
		for (j = 0; j < 30; j ++)
			*(NavBits ++) = (EvenPart[j] & ConvEncodeBits) ? 1 : 0;
	for (i = 0; i < 10; i ++)
		*(NavBits ++) = SyncPattern[i];
	for (i = 0, ConvEncodeBits = 0x80; i < 8; i ++, ConvEncodeBits >>= 1)
		for (j = 0; j < 30; j ++)
			*(NavBits ++) = (OddPart[j] & ConvEncodeBits) ? 1 : 0;

	return 0;
}

int INavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	if (svid < 1 || svid > 36 || !Eph || !Eph->flag)
		return 0;
	ComposeEphWords(Eph, GalEphData[svid-1]);
	return svid;
}

int INavBit::SetAlmanac(int svid, PGPS_ALMANAC Alm)
{
	return svid;
}

int INavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	double Value;
	signed int IntValue;
	unsigned int UintValue;
	int i;
	unsigned int IonoWords[2];

	// IonoParam a0~a2 hold ai0~ai2
	Value = UnscaleDouble(IonoParam->a0, -2);
	UintValue = roundu(Value);
	IonoWords[0] = COMPOSE_BITS(UintValue, 15, 11);
	Value = UnscaleDouble(IonoParam->a1, -8);
	IntValue = roundi(Value);
	IonoWords[0] |= COMPOSE_BITS(IntValue, 4, 11);
	Value = UnscaleDouble(IonoParam->a2, -15);
	IntValue = roundi(Value);
	IonoWords[0] |= COMPOSE_BITS(IntValue >> 10, 0, 4);
	IonoWords[1] = COMPOSE_BITS(IntValue, 22, 10);

	// put ai0~ai2 into Word 5
	for (i = 0; i < 36; i ++)
	{
		GalEphData[i][16] &= 0xfc000000; GalEphData[i][16] |= IonoWords[0];
		GalEphData[i][17] &= 0x0001ffff; GalEphData[i][17] |= IonoWords[1];
	}

	// Word 6 for UTC parameters
	GalUtcData[0] = 0x18000000;	// put Type=6 to 6MSB
	Value = UnscaleDouble(UtcParam->A0, -30);
	IntValue = roundi(Value);
	GalUtcData[0] |= COMPOSE_BITS(IntValue >> 6, 0, 26);
	GalUtcData[1] = COMPOSE_BITS(IntValue, 26, 6);
	Value = UnscaleDouble(UtcParam->A1, -50);
	IntValue = roundi(Value);
	GalUtcData[1] |= COMPOSE_BITS(IntValue, 2, 24);
	GalUtcData[1] |= COMPOSE_BITS(UtcParam->TLS >> 6, 0, 2);
	GalUtcData[2] = COMPOSE_BITS(UtcParam->TLS, 26, 6);
	GalUtcData[2] |= COMPOSE_BITS(UtcParam->tot, 18, 8);
	GalUtcData[2] |= COMPOSE_BITS(UtcParam->WN, 10, 8);
	GalUtcData[2] |= COMPOSE_BITS(UtcParam->WNLSF, 2, 8);
	GalUtcData[2] |= COMPOSE_BITS(UtcParam->DN >> 1, 0, 2);
	GalUtcData[3] = COMPOSE_BITS(UtcParam->DN, 31, 1);
	GalUtcData[3] |= COMPOSE_BITS(UtcParam->TLSF, 23, 8);

	return 0;
}

int INavBit::ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int *EphData)
{
	double Value;
	signed int IntValue;
	unsigned int UintValue;

	// Word 1
	EphData[0] = 0x04000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10) | COMPOSE_BITS((Ephemeris->toe / 60), 2, 14);
	Value = UnscaleDouble(Ephemeris->M0 / PI, -31);
	IntValue = roundi(Value);
	EphData[0] |= COMPOSE_BITS(IntValue >> 30, 0, 2);
	EphData[1] = COMPOSE_BITS(IntValue, 2, 30);
	Value = UnscaleDouble(Ephemeris->ecc, -33);
	UintValue = roundu(Value);
	EphData[1] |= COMPOSE_BITS(UintValue >> 30, 0, 2);
	EphData[2] = COMPOSE_BITS(UintValue, 2, 30);
	Value = UnscaleDouble(Ephemeris->sqrtA, -19);
	UintValue = roundu(Value);
	EphData[2] |= COMPOSE_BITS(UintValue >> 30, 0, 2);
	EphData[3] = COMPOSE_BITS(UintValue, 2, 30);

	// Word 2
	EphData[4] = 0x08000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10);
	Value = UnscaleDouble(Ephemeris->omega0 / PI, -31);
	IntValue = roundi(Value);
	EphData[4] |= COMPOSE_BITS(IntValue >> 16, 0, 16);
	EphData[5] = COMPOSE_BITS(IntValue, 16, 16);
	Value = UnscaleDouble(Ephemeris->i0 / PI, -31);
	IntValue = roundi(Value);
	EphData[5] |= COMPOSE_BITS(IntValue >> 16, 0, 16);
	EphData[6] = COMPOSE_BITS(IntValue, 16, 16);
	Value = UnscaleDouble(Ephemeris->w / PI, -31);
	IntValue = roundi(Value);
	EphData[6] |= COMPOSE_BITS(IntValue >> 16, 0, 16);
	EphData[7] = COMPOSE_BITS(IntValue, 16, 16);
	Value = UnscaleDouble(Ephemeris->idot / PI, -43);
	IntValue = roundi(Value);
	EphData[7] |= COMPOSE_BITS(IntValue >> 16, 2, 14);

	// Word 3
	EphData[8] = 0x0c000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10);
	Value = UnscaleDouble(Ephemeris->omega_dot / PI, -43);
	IntValue = roundi(Value);
	EphData[8] |= COMPOSE_BITS(IntValue >> 8, 0, 16);
	EphData[9] = COMPOSE_BITS(IntValue, 24, 8);
	Value = UnscaleDouble(Ephemeris->delta_n / PI, -43);
	IntValue = roundi(Value);
	EphData[9] |= COMPOSE_BITS(IntValue >> 8, 8, 16);
	Value = UnscaleDouble(Ephemeris->cuc, -29);
	IntValue = roundi(Value);
	EphData[9] |= COMPOSE_BITS(IntValue >> 8, 0, 8);
	EphData[10] = COMPOSE_BITS(IntValue, 24, 8);
	Value = UnscaleDouble(Ephemeris->cus, -29);
	IntValue = roundi(Value);
	EphData[10] |= COMPOSE_BITS(IntValue, 8, 16);
	Value = UnscaleDouble(Ephemeris->crc, -5);
	IntValue = roundi(Value);
	EphData[10] |= COMPOSE_BITS(IntValue >> 8, 0, 8);
	EphData[11] = COMPOSE_BITS(IntValue, 24, 8);
	Value = UnscaleDouble(Ephemeris->crs, -5);
	IntValue = roundi(Value);
	EphData[11] |= COMPOSE_BITS(IntValue, 8, 16);
	EphData[11] |= COMPOSE_BITS(Ephemeris->ura, 0, 8);

	// Word 4
	EphData[12] = 0x10000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10) | COMPOSE_BITS(Ephemeris->svid, 10, 6);
	Value = UnscaleDouble(Ephemeris->cic, -29);
	IntValue = roundi(Value);
	EphData[12] |= COMPOSE_BITS(IntValue >> 6, 0, 10);
	EphData[13] = COMPOSE_BITS(IntValue, 26, 6);
	Value = UnscaleDouble(Ephemeris->cis, -29);
	IntValue = roundi(Value);
	EphData[13] |= COMPOSE_BITS(IntValue, 10, 16);
	UintValue = Ephemeris->toc / 60;
	EphData[13] |= COMPOSE_BITS(UintValue >> 4, 0, 10);
	EphData[14] = COMPOSE_BITS(UintValue, 28, 4);
	Value = UnscaleDouble(Ephemeris->af0, -34);
	IntValue = roundi(Value);
	EphData[14] |= COMPOSE_BITS(IntValue >> 3, 0, 28);
	EphData[15] = COMPOSE_BITS(IntValue, 29, 3);
	Value = UnscaleDouble(Ephemeris->af1, -46);
	IntValue = roundi(Value);
	EphData[15] |= COMPOSE_BITS(IntValue, 8, 21);
	Value = UnscaleDouble(Ephemeris->af2, -59);
	IntValue = roundi(Value);
	EphData[15] |= COMPOSE_BITS(IntValue, 2, 6);

	// Word 5
	EphData[16] &= 0x03ffffff; EphData[16] = 0x14000000;	// put Type=5 to 6MSB
	Value = UnscaleDouble(Ephemeris->tgd, -32);
	IntValue = roundi(Value);
	EphData[17] = COMPOSE_BITS(IntValue, 7, 10);
	Value = UnscaleDouble(Ephemeris->tgd2, -32);
	IntValue = roundi(Value);
	EphData[17] |= COMPOSE_BITS(IntValue >> 3, 0, 7);
	EphData[18] = COMPOSE_BITS(IntValue, 29, 3);

	return 0;
}

unsigned int *INavBit::GetWordData(int svid, int Word)
{
	switch (Word)
	{
	case 0:
		return GalSpareData;
	case 63:
		return GalDummyData;
	case 6:
		return GalUtcData;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		return GalEphData[svid-1] + 4 * (Word - 1);
	default:
		return GalDummyData;
	}
}

// encode 2MSB of EncodeWord, left shift EncodeWord and update ConvEncodeBits
unsigned char INavBit::GalConvolutionEncode(unsigned char &ConvEncodeBits, unsigned int EncodeWord)
{
	ConvEncodeBits = (ConvEncodeBits << 2) + (unsigned char)(EncodeWord >> 30);
	EncodeWord <<= 2;
	return ((ConvolutionEncode(ConvEncodeBits) & 0xf) ^ 0x5);	// invert G2
}
