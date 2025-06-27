//----------------------------------------------------------------------
// FNavBit.h:
//   Implementation of navigation bit synthesis class for F/NAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "FNavBit.h"

#define SQRT_A0 5440.588203494177338011974948823
#define NORMINAL_I0 0.97738438111682456307726683035362

const int FNavBit::SyncPattern[12] = {
	1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 0
};

FNavBit::FNavBit()
{
	memset(GalEphData, 0, sizeof(GalEphData));
	memset(GalAlmData, 0, sizeof(GalAlmData));
	memset(GalUtcData, 0, sizeof(GalUtcData));
	memset(GalIonoData, 0, sizeof(GalIonoData));
}

FNavBit::~FNavBit()
{
}

int FNavBit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i, j, TOW, subframe, page, BitCount;
	unsigned int EncodeData[7], GST, CrcResult, EncodeWord;	// 214bit to be encoded by CRC
	unsigned char EncodeMessage[61], ConvEncodeBits;	// EncodeMessage contains 8x61 bits

	Param;	// not used
	// first determine the current TOW and subframe number
	StartTime.Week += StartTime.MilliSeconds / 604800000;
	StartTime.MilliSeconds %= 604800000;
	TOW = StartTime.MilliSeconds / 1000;
	GST = (((StartTime.Week - 1024) & 0xfff) << 20) + TOW;
	subframe = (TOW % 1200) / 50;	// two round of 600s frame (24 subframes) to hold 36 almanacs
	page = (TOW % 50) / 10;
	GetPageData(svid, page, subframe, GST, EncodeData);
	CrcResult = Crc24qEncode(EncodeData, 214);

	// do convolution encode (EncodeData[0] bit22 through EncodeData[6] bit0)
	ConvEncodeBits = 0;
	EncodeWord = EncodeData[0] << 10;	// move to MSB
	for (i = 0, BitCount = 10; i < 214 / 2; i ++)
	{
		EncodeMessage[i/2] = (EncodeMessage[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
		BitCount += 2;
		if ((BitCount % 32) == 0)
			EncodeWord = EncodeData[BitCount >> 5];
	}
	EncodeWord = CrcResult << 8;
	for (; i < 238 / 2; i ++)	// encode CRC
		EncodeMessage[i/2] = (EncodeMessage[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EncodeWord = 0;	// append 6 zeros as tail
	EncodeMessage[59] = (EncodeMessage[59] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EncodeMessage[60] = GalConvolutionEncode(ConvEncodeBits, EncodeWord);
	EncodeMessage[60] = (EncodeMessage[60] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);

	// do interleaving and put into NavBits
	for (i = 0; i < 12; i ++)
		*(NavBits ++) = SyncPattern[i];
	for (i = 0, ConvEncodeBits = 0x80; i < 8; i ++, ConvEncodeBits >>= 1)
		for (j = 0; j < 61; j ++)
			*(NavBits ++) = (EncodeMessage[j] & ConvEncodeBits) ? 1 : 0;

	return 0;
}

int FNavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	if (svid < 1 || svid > 36 || !Eph || !Eph->valid)
		return 0;
	ComposeEphWords(Eph, GalEphData[svid-1]);
	return svid;
}

int FNavBit::SetAlmanac(GPS_ALMANAC Alm[])
{
	int i, week = 0;

	for (i = 0; i < 36; i ++)
		if (Alm[i].valid & 1)
		{
			week = Alm[i].week;
			break;
		}
	for (i = 0; i < 12; i ++)
		ComposeAlmWords(Alm + i * 3, GalAlmData[i], week);
	return 0;
}

int FNavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	PIONO_NEQUICK IonoGal = (PIONO_NEQUICK)IonoParam;
	signed int IntValue;
	unsigned int UintValue;

	// put ionosphere parameters
	UintValue = UnscaleUint(IonoGal->ai0, -2);
	GalIonoData[0] = COMPOSE_BITS(UintValue, 5, 11);
	IntValue = UnscaleInt(IonoGal->ai1, -8);
	GalIonoData[0] |= COMPOSE_BITS(IntValue >> 6, 0, 5);
	GalIonoData[1] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(IonoGal->ai2, -15);
	GalIonoData[1] |= COMPOSE_BITS(IntValue, 12, 14);
	GalIonoData[1] |= COMPOSE_BITS(IonoGal->flag, 7, 5);

	// put UTC parameters
	IntValue = UnscaleInt(UtcParam->A0, -30);
	GalUtcData[0] = COMPOSE_BITS(IntValue >> 26, 0, 6);
	GalUtcData[1] = COMPOSE_BITS(IntValue, 6, 26);
	IntValue = UnscaleInt(UtcParam->A1, -50);
	GalUtcData[1] |= COMPOSE_BITS(IntValue >> 18, 0, 6);
	GalUtcData[2] = COMPOSE_BITS(IntValue, 14, 18);
	GalUtcData[2] |= COMPOSE_BITS(UtcParam->TLS, 6, 8);
	GalUtcData[2] |= COMPOSE_BITS(UtcParam->tot >> 2, 0, 6);
	GalUtcData[3] = COMPOSE_BITS(UtcParam->tot, 30, 2);
	GalUtcData[3] |= COMPOSE_BITS(UtcParam->WN, 22, 8);
	GalUtcData[3] |= COMPOSE_BITS(UtcParam->WNLSF, 14, 8);
	GalUtcData[3] |= COMPOSE_BITS(UtcParam->DN, 11, 3);
	GalUtcData[3] |= COMPOSE_BITS(UtcParam->TLSF, 3, 8);

	return 0;
}

int FNavBit::ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int EphData[4][7])
{
	signed int IntValue;
	unsigned int UintValue;

	// PageType 1
	EphData[0][0] = (1 << 16) | COMPOSE_BITS(Ephemeris->svid, 10, 6) | COMPOSE_BITS(Ephemeris->iodc, 0, 10);
	UintValue = Ephemeris->toc / 60;
	EphData[0][1] = COMPOSE_BITS(UintValue, 18, 14);
	IntValue = UnscaleInt(Ephemeris->af0, -34);
	EphData[0][1] |= COMPOSE_BITS(IntValue >> 13, 0, 18);
	EphData[0][2] = COMPOSE_BITS(IntValue, 19, 13);
	IntValue = UnscaleInt(Ephemeris->af1, -46);
	EphData[0][2] |= COMPOSE_BITS(IntValue >> 2, 0, 19);
	EphData[0][3] = COMPOSE_BITS(IntValue, 30, 2);
	IntValue = UnscaleInt(Ephemeris->af2, -59);
	EphData[0][3] |= COMPOSE_BITS(IntValue, 24, 6);
	EphData[0][3] |= COMPOSE_BITS(Ephemeris->ura, 16, 8);
	// <--- 41 bits gap for iono correction --->
	IntValue = UnscaleInt(Ephemeris->tgd, -32);
	EphData[0][4] = COMPOSE_BITS(IntValue >> 3, 0, 7);
	EphData[0][5] = COMPOSE_BITS(IntValue, 29, 3);
	EphData[0][5] |= COMPOSE_BITS(Ephemeris->health >> 4, 27, 2);
	// <--- 32 bits gap for GST --->
	EphData[0][6] = COMPOSE_BITS(Ephemeris->health >> 3, 26, 1);

	// PageType 2
	EphData[1][0] = (2 << 16) | COMPOSE_BITS(Ephemeris->iodc, 6, 10);
	IntValue = UnscaleInt(Ephemeris->M0 / PI, -31);
	EphData[1][0] |= COMPOSE_BITS(IntValue >> 26, 0, 6);
	EphData[1][1] = COMPOSE_BITS(IntValue, 6, 26);
	IntValue = UnscaleInt(Ephemeris->omega_dot / PI, -43);
	EphData[1][1] |= COMPOSE_BITS(IntValue >> 18, 0, 6);
	EphData[1][2] = COMPOSE_BITS(IntValue, 14, 18);
	UintValue = UnscaleUint(Ephemeris->ecc, -33);
	EphData[1][2] |= COMPOSE_BITS(UintValue >> 18, 0, 14);
	EphData[1][3] = COMPOSE_BITS(UintValue, 14, 18);
	UintValue = UnscaleUint(Ephemeris->sqrtA, -19);
	EphData[1][3] |= COMPOSE_BITS(UintValue >> 18, 0, 14);
	EphData[1][4] = COMPOSE_BITS(UintValue, 14, 18);
	IntValue = UnscaleInt(Ephemeris->omega0 / PI, -31);
	EphData[1][4] |= COMPOSE_BITS(IntValue >> 18, 0, 14);
	EphData[1][5] = COMPOSE_BITS(IntValue, 14, 18);
	IntValue = UnscaleInt(Ephemeris->idot / PI, -43);
	EphData[1][5] |= COMPOSE_BITS(IntValue, 0, 14);
	EphData[1][6] = 0;	// for GST

	// PageType 3
	EphData[2][0] = (3 << 16) | COMPOSE_BITS(Ephemeris->iodc, 6, 10);
	IntValue = UnscaleInt(Ephemeris->i0 / PI, -31);
	EphData[2][0] |= COMPOSE_BITS(IntValue >> 26, 0, 6);
	EphData[2][1] = COMPOSE_BITS(IntValue, 6, 26);
	IntValue = UnscaleInt(Ephemeris->w / PI, -31);
	EphData[2][1] |= COMPOSE_BITS(IntValue >> 26, 0, 6);
	EphData[2][2] = COMPOSE_BITS(IntValue, 6, 26);
	IntValue = UnscaleInt(Ephemeris->delta_n / PI, -43);
	EphData[2][2] |= COMPOSE_BITS(IntValue >> 10, 0, 6);
	EphData[2][3] = COMPOSE_BITS(IntValue, 22, 10);
	IntValue = UnscaleInt(Ephemeris->cuc, -29);
	EphData[2][3] |= COMPOSE_BITS(IntValue, 6, 16);
	IntValue = UnscaleInt(Ephemeris->cus, -29);
	EphData[2][3] |= COMPOSE_BITS(IntValue >> 10, 0, 6);
	EphData[2][4] = COMPOSE_BITS(IntValue, 22, 10);
	IntValue = UnscaleInt(Ephemeris->crc, -5);
	EphData[2][4] |= COMPOSE_BITS(IntValue, 6, 16);
	IntValue = UnscaleInt(Ephemeris->crs, -5);
	EphData[2][4] |= COMPOSE_BITS(IntValue >> 10, 0, 6);
	EphData[2][5] = COMPOSE_BITS(IntValue, 22, 10);
	EphData[2][5] |= COMPOSE_BITS((Ephemeris->toe / 60), 8, 14);
	EphData[2][6] = 0;	// for GST and 8 spare bits

	// PageType 4
	EphData[3][0] = (4 << 16) | COMPOSE_BITS(Ephemeris->iodc, 6, 10);
	IntValue = UnscaleInt(Ephemeris->cic, -29);
	EphData[3][0] |= COMPOSE_BITS(IntValue >> 10, 0, 6);
	EphData[3][1] = COMPOSE_BITS(IntValue, 22, 10);
	IntValue = UnscaleInt(Ephemeris->cis, -29);
	EphData[3][1] |= COMPOSE_BITS(IntValue, 6, 16);
	EphData[3][2] = EphData[3][3] = EphData[3][4] = EphData[3][5] = EphData[3][6] = 0;	// for GST-UTC, GST-GPS, TOW and 5 spare bits

	return 0;
}

int FNavBit::ComposeAlmWords(GPS_ALMANAC Almanac[], unsigned int AlmData[2][7], int week)
{
	signed int IntValue;
	unsigned int UintValue;
	int toa = (Almanac[0].valid & 1) ? Almanac[0].toa : (Almanac[1].valid & 1) ? Almanac[1].toa : (Almanac[2].valid & 1) ? Almanac[2].toa : 0;

	// PageType 5
	AlmData[0][0] = (5 << 16) | (4 << 12) | COMPOSE_BITS(week, 10, 2) | COMPOSE_BITS((toa / 600), 0, 10);	// IODa=4
	AlmData[0][1] = COMPOSE_BITS(Almanac[0].svid, 26, 6);	// SVID1 starts here
	IntValue = UnscaleInt(Almanac[0].sqrtA - SQRT_A0, -9);
	AlmData[0][1] |= COMPOSE_BITS(IntValue, 13, 13);
	UintValue = UnscaleUint(Almanac[0].ecc, -16);
	AlmData[0][1] |= COMPOSE_BITS(UintValue, 2, 11);
	IntValue = UnscaleInt(Almanac[0].w / PI, -15);
	AlmData[0][1] |= COMPOSE_BITS(IntValue >> 14, 0, 2);
	AlmData[0][2] = COMPOSE_BITS(IntValue, 18, 14);
	IntValue = UnscaleInt((Almanac[0].i0 - NORMINAL_I0) / PI, -14);
	AlmData[0][2] |= COMPOSE_BITS(IntValue, 7, 11);
	IntValue = UnscaleInt(Almanac[0].omega0 / PI, -15);
	AlmData[0][2] |= COMPOSE_BITS(IntValue >> 9, 0, 7);
	AlmData[0][3] = COMPOSE_BITS(IntValue, 23, 9);
	IntValue = UnscaleInt(Almanac[0].omega_dot / PI, -33);
	AlmData[0][3] |= COMPOSE_BITS(IntValue, 12, 11);
	IntValue = UnscaleInt(Almanac[0].M0 / PI, -15);
	AlmData[0][3] |= COMPOSE_BITS(IntValue >> 4, 0, 12);
	AlmData[0][4] = COMPOSE_BITS(IntValue, 28, 4);
	IntValue = UnscaleInt(Almanac[0].af0, -19);
	AlmData[0][4] |= COMPOSE_BITS(IntValue, 12, 16);
	IntValue = UnscaleInt(Almanac[0].af1, -38);
	AlmData[0][4] |= COMPOSE_BITS(IntValue >> 1, 0, 12);
	AlmData[0][5] = COMPOSE_BITS(IntValue, 31, 1);
	AlmData[0][5] |= COMPOSE_BITS((Almanac[1].valid & 1) ? 0 : 1, 29, 2);
	AlmData[0][5] = COMPOSE_BITS(Almanac[1].svid, 23, 6);	// SVID2 starts here
	IntValue = UnscaleInt(Almanac[1].sqrtA - SQRT_A0, -9);
	AlmData[0][5] |= COMPOSE_BITS(IntValue, 10, 13);
	UintValue = UnscaleUint(Almanac[1].ecc, -16);
	AlmData[0][5] |= COMPOSE_BITS(UintValue >> 1, 0, 10);
	AlmData[0][6] = COMPOSE_BITS(IntValue, 31, 1);
	IntValue = UnscaleInt(Almanac[1].w / PI, -15);
	AlmData[0][6] |= COMPOSE_BITS(IntValue, 15, 16);
	IntValue = UnscaleInt((Almanac[1].i0 - NORMINAL_I0) / PI, -14);
	AlmData[0][6] |= COMPOSE_BITS(IntValue, 4, 11);
	IntValue = UnscaleInt(Almanac[1].omega0 / PI, -15);
	AlmData[0][6] |= COMPOSE_BITS(IntValue >> 12, 0, 4);

	// PageType 6
	AlmData[1][0] = (6 << 16) | (4 << 12);	// IODa=4
	AlmData[1][0] |= COMPOSE_BITS(IntValue, 0, 12);
	IntValue = UnscaleInt(Almanac[1].omega_dot / PI, -33);
	AlmData[1][1] = COMPOSE_BITS(IntValue, 21, 11);
	IntValue = UnscaleInt(Almanac[1].M0 / PI, -15);
	AlmData[1][1] |= COMPOSE_BITS(IntValue, 5, 16);
	IntValue = UnscaleInt(Almanac[1].af0, -19);
	AlmData[1][1] |= COMPOSE_BITS(IntValue >> 11, 0, 5);
	AlmData[1][2] = COMPOSE_BITS(IntValue, 21, 11);
	IntValue = UnscaleInt(Almanac[1].af1, -38);
	AlmData[1][2] |= COMPOSE_BITS(IntValue, 8, 13);
	AlmData[1][2] |= COMPOSE_BITS((Almanac[1].valid & 1) ? 0 : 1, 6, 2);
	AlmData[1][2] |= COMPOSE_BITS(Almanac[2].svid, 0, 6);	// SVID3 starts here
	IntValue = UnscaleInt(Almanac[2].sqrtA - SQRT_A0, -9);
	AlmData[1][3] = COMPOSE_BITS(IntValue, 19, 13);
	UintValue = UnscaleUint(Almanac[2].ecc, -16);
	AlmData[1][3] |= COMPOSE_BITS(UintValue, 8, 11);
	IntValue = UnscaleInt(Almanac[2].w / PI, -15);
	AlmData[1][3] |= COMPOSE_BITS(IntValue >> 8, 0, 8);
	AlmData[1][4] = COMPOSE_BITS(IntValue, 24, 8);
	IntValue = UnscaleInt((Almanac[2].i0 - NORMINAL_I0) / PI, -14);
	AlmData[1][4] |= COMPOSE_BITS(IntValue, 13, 11);
	IntValue = UnscaleInt(Almanac[2].omega0 / PI, -15);
	AlmData[1][4] |= COMPOSE_BITS(IntValue >> 3, 0, 13);
	AlmData[1][5] = COMPOSE_BITS(IntValue, 29, 3);
	IntValue = UnscaleInt(Almanac[2].omega_dot / PI, -33);
	AlmData[1][5] |= COMPOSE_BITS(IntValue, 18, 11);
	IntValue = UnscaleInt(Almanac[2].M0 / PI, -15);
	AlmData[1][5] |= COMPOSE_BITS(IntValue, 2, 16);
	IntValue = UnscaleInt(Almanac[2].af0, -19);
	AlmData[1][5] |= COMPOSE_BITS(IntValue >> 14, 0, 2);
	AlmData[1][6] = COMPOSE_BITS(IntValue, 18, 14);
	IntValue = UnscaleInt(Almanac[2].af1, -38);
	AlmData[1][6] |= COMPOSE_BITS(IntValue, 5, 13);
	AlmData[1][6] |= COMPOSE_BITS((Almanac[2].valid & 1) ? 0 : 1, 3, 2);

	return 0;
}

void FNavBit::GetPageData(int svid, int page, int subframe, unsigned int GST, unsigned int Data[7])
{
	switch (page)
	{
	case 0:	// page 1
		memcpy(Data, GalEphData[svid-1][0], sizeof(unsigned int) * 7);
		// add iono correction
		Data[3] |= GalIonoData[0];
		Data[4] |= GalIonoData[1];
		// add GST
		Data[5] |= COMPOSE_BITS(GST >> 5, 0, 27);
		Data[6] |= COMPOSE_BITS(GST, 27, 5);
		break;
	case 1:	// page 2
		memcpy(Data, GalEphData[svid-1][1], sizeof(unsigned int) * 7);
		Data[6] = GST;
		break;
	case 2:	// page 3
		memcpy(Data, GalEphData[svid-1][2], sizeof(unsigned int) * 7);
		// add GST
		Data[5] |= COMPOSE_BITS(GST >> 24, 0, 8);
		Data[6] |= COMPOSE_BITS(GST, 8, 24);
		break;
	case 3:	// page 4
		memcpy(Data, GalEphData[svid-1][3], sizeof(unsigned int) * 7);
		// add GST-UTC
		Data[1] |= GalUtcData[0];
		Data[2] = GalUtcData[1];
		Data[3] = GalUtcData[2];
		Data[4] = GalUtcData[3];
		// add TOW
		Data[6] |= COMPOSE_BITS(GST, 5, 20);
		break;
	case 4:	// page 5/6
		memcpy(Data, GalAlmData[subframe/2][subframe&1], sizeof(unsigned int) * 7);
		break;
	}
}

// encode 2MSB of EncodeWord, left shift EncodeWord and update ConvEncodeBits
unsigned char FNavBit::GalConvolutionEncode(unsigned char &ConvEncodeBits, unsigned int &EncodeWord)
{
	ConvEncodeBits = (ConvEncodeBits << 2) + (unsigned char)(EncodeWord >> 30);
	EncodeWord <<= 2;
	return ((ConvolutionEncode(ConvEncodeBits) & 0xf) ^ 0x5);	// invert G2
}
