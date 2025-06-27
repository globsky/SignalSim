//----------------------------------------------------------------------
// CNavBit.h:
//   Implementation of navigation bit synthesis class for CNAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "CNavBit.h"

#define INVALID_TOA 255	// valid range of toa is 0~147
#define A_REF 26559710.0
#define OMEGA_DOT_REF (-2.6e-9)
#define NORMINAL_I0 0.94247779607693797153879301498385

CNavBit::CNavBit()
{
	memset(EphMessage, 0, sizeof(EphMessage));
	memset(MidiAlm, 0, sizeof(MidiAlm));
	memset(ReducedAlm, 0, sizeof(ReducedAlm));
	memset(ClockMessage, 0, sizeof(ClockMessage));
	memset(DelayMessage, 0, sizeof(DelayMessage));
	memset(UTCMessage, 0, sizeof(UTCMessage));
	memset(ConvEncodeBitsL2, 0, sizeof(ConvEncodeBitsL2));
	memset(ConvEncodeBitsL5, 0, sizeof(ConvEncodeBitsL5));
	TOA = INVALID_TOA;
}

CNavBit::~CNavBit()
{
}

// each message is 12s (600bit after encode), according maximum interval, the message is arranged to 48s frame and 1200s super frame
// each frame contains 4 messages and each super frame contains 25 frames, the order of message with a super frame is:
// frame index   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
// msg index 0  10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
// msg index 1  11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// msg index 2  30 33 31 37 31 37 30 33 31 37 31 37 30 33 31 37 31 37 30 33 31 37 31 37 30
// msg index 3  37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 33
// each super frame has 8 message 31 each has 4 reduced amlamanc
// each super frame has 32 message 37, message index 3 contains SV01 to SV24, message index 2 contains SV25 to SV32
// Param is used to distinguish from Dc in L2C and D5 in L5 (0 for L2C)
int CNavBit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i, j, TOW, message, BitCount;
	unsigned int EncodeData[9], CrcResult, EncodeWord;	// 276bit to be encoded by CRC
	unsigned char EncodeMessage[75], ConvEncodeBits;	// EncodeMessage contains 8x75 bits

	// validate svid to prevent out-of-bounds array access
	if (svid < 1 || svid > 32)
	{
		// fill NavBits with zeros for invalid svid
		memset(NavBits, 0, sizeof(int) * 600);
		return -1;
	}

	// first determine the current TOW and subframe number
	StartTime.Week += StartTime.MilliSeconds / 604800000;
	StartTime.MilliSeconds %= 604800000;
	TOW = StartTime.MilliSeconds / (Param ? 6000 : 12000);
	message = TOW % 100;	// message index within super frame
	TOW ++;		// TOW is the time of NEXT message
	if (!Param)
		TOW *= 2;;
	if (TOW >= 100800)
		TOW = 0;

	GetMessageData(svid, message, TOW, EncodeData);
	CrcResult = Crc24qEncode(EncodeData, 276);

	// do convolution encode (EncodeData[0] bit22 through EncodeData[6] bit0)
	ConvEncodeBits = Param ? ConvEncodeBitsL5[svid-1] : ConvEncodeBitsL2[svid-1];
	EncodeWord = EncodeData[0] << 12;	// move to MSB
	for (i = 0, BitCount = 12; i < 276 / 2; i ++)
	{
		EncodeMessage[i/2] = (EncodeMessage[i/2] << 4) + ConvolutionEncodePair(ConvEncodeBits, EncodeWord);
		BitCount += 2;
		if ((BitCount % 32) == 0)
			EncodeWord = EncodeData[BitCount >> 5];
	}
	EncodeWord = CrcResult << 8;
	for (; i < 300 / 2; i ++)	// encode CRC
		EncodeMessage[i/2] = (EncodeMessage[i/2] << 4) + ConvolutionEncodePair(ConvEncodeBits, EncodeWord);
	if (Param)
		ConvEncodeBitsL5[svid-1] = ConvEncodeBits;
	else
		ConvEncodeBitsL2[svid-1] = ConvEncodeBits;

	// put into NavBits
	for (i = 0; i < 75; i ++)
		for (j = 0, ConvEncodeBits = 0x80; j < 8; j ++, ConvEncodeBits >>= 1)
			*(NavBits ++) = (EncodeMessage[i] & ConvEncodeBits) ? 1 : 0;

	return 0;
}

int CNavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	if (svid < 1 || svid > 32 || !Eph || !Eph->valid)
		return 0;
	ComposeEphWords(Eph, EphMessage[svid-1], ClockMessage[svid-1], DelayMessage[svid-1]);
	return svid;
}

int CNavBit::SetAlmanac(GPS_ALMANAC Alm[])
{
	int i;

	for (i = 0; i < 32; i ++)
	{
		ComposeAlmWords(&Alm[i], ReducedAlm[i], MidiAlm[i]);
		if ((Alm[i].valid & 1) && TOA == INVALID_TOA)
			TOA = (Alm[i].week << 8) + (Alm[i].toa >> 12);
	}
	return 0;
}

int CNavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	signed int IntValue;

	if (IonoParam->flag == 0 || (UtcParam->flag & 3) != 3)
		return 0;

	IntValue = UnscaleInt(IonoParam->a0, -30);
	IonoMessage[0] = COMPOSE_BITS(IntValue, 12, 8);
	IntValue = UnscaleInt(IonoParam->a1, -27);
	IonoMessage[0] |= COMPOSE_BITS(IntValue, 4, 8);
	IntValue = UnscaleInt(IonoParam->a2, -24);
	IonoMessage[0] |= COMPOSE_BITS(IntValue >> 4, 0, 4);
	IonoMessage[1] = COMPOSE_BITS(IntValue, 28, 4);
	IntValue = UnscaleInt(IonoParam->a3, -24);
	IonoMessage[1] |= COMPOSE_BITS(IntValue, 20, 8);
	IntValue = UnscaleInt(IonoParam->b0, 11);
	IonoMessage[1] |= COMPOSE_BITS(IntValue, 12, 8);
	IntValue = UnscaleInt(IonoParam->b1, 14);
	IonoMessage[1] |= COMPOSE_BITS(IntValue, 4, 8);
	IntValue = UnscaleInt(IonoParam->b2, 16);
	IonoMessage[1] |= COMPOSE_BITS(IntValue >> 4, 0, 4);
	IonoMessage[2] = COMPOSE_BITS(IntValue, 28, 4);
	IntValue = UnscaleInt(IonoParam->b3, 16);
	IonoMessage[2] |= COMPOSE_BITS(IntValue, 20, 8);
	IonoMessage[2] |= COMPOSE_BITS(UtcParam->WN, 12, 8);	// use WN to take the place of WNop

	IntValue = UnscaleInt(UtcParam->A0, -35);
	UTCMessage[0] = COMPOSE_BITS(IntValue, 5, 16);
	IntValue = UnscaleInt(UtcParam->A1, -51);
	UTCMessage[0] |= COMPOSE_BITS(IntValue >> 8, 0, 5);
	UTCMessage[1] = COMPOSE_BITS(IntValue, 24, 8);
	IntValue = UnscaleInt(UtcParam->A2, -68);
	UTCMessage[1] |= COMPOSE_BITS(IntValue, 17, 7);
	UTCMessage[1] |= COMPOSE_BITS(UtcParam->TLS, 9, 8);
	UTCMessage[1] |= COMPOSE_BITS(UtcParam->tot, 1, 8);	// UtcParam->tot has scale factor of 2^12, so leaving 8LSB as 0 for scale factor 2^4 
	UTCMessage[2] = COMPOSE_BITS(UtcParam->WN, 12, 13);
	UTCMessage[2] |= COMPOSE_BITS(UtcParam->WNLSF >> 1, 0, 12);
	UTCMessage[3] = COMPOSE_BITS(UtcParam->WNLSF, 31, 1);
	UTCMessage[3] |= COMPOSE_BITS(UtcParam->DN, 27, 4);
	UTCMessage[3] |= COMPOSE_BITS(UtcParam->TLSF, 19, 8);

	return 0;
}

int CNavBit::ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int EphData[2][9], unsigned int ClockData[4], unsigned int DelayData[3])
{
	signed int IntValue;
	unsigned int UintValue;
	long long int LongValue;
	unsigned long long int ULongValue;

	// Message Type 10
	EphData[0][0] = (0x8b << 12) | (Ephemeris->svid << 6) | 10;
	EphData[0][1] = COMPOSE_BITS(Ephemeris->week, 1, 13);
	EphData[0][1] |= COMPOSE_BITS(Ephemeris->health >> 8, 15, 3);
	EphData[0][2] = COMPOSE_BITS(Ephemeris->health >> 6, 30, 2);
	UintValue = Ephemeris->top / 300;
	EphData[0][2] |= COMPOSE_BITS(UintValue, 19, 11);
	EphData[0][2] |= COMPOSE_BITS(Ephemeris->ura, 14, 5);
	UintValue = Ephemeris->toe / 300;
	EphData[0][2] |= COMPOSE_BITS(UintValue, 3, 11);
	IntValue = UnscaleInt(Ephemeris->axis - A_REF, -9);
	EphData[0][2] |= COMPOSE_BITS(IntValue >> 23, 0, 3);
	EphData[0][3] = COMPOSE_BITS(IntValue, 9, 23);
	IntValue = UnscaleUint(Ephemeris->axis_dot, -21);
	EphData[0][3] |= COMPOSE_BITS(IntValue >> 16, 0, 9);
	EphData[0][4] = COMPOSE_BITS(IntValue, 16, 16);
	IntValue = UnscaleInt(Ephemeris->delta_n / PI, -44);
	EphData[0][4] |= COMPOSE_BITS(IntValue >> 1, 0, 16);
	EphData[0][5] = COMPOSE_BITS(IntValue, 31, 1);
	IntValue = UnscaleInt(Ephemeris->delta_n_dot / PI, -57);
	EphData[0][5] |= COMPOSE_BITS(IntValue, 8, 23);
	LongValue = UnscaleLong(Ephemeris->M0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	EphData[0][5] |= COMPOSE_BITS(IntValue, 7, 1);
	EphData[0][5] |= COMPOSE_BITS(UintValue >> 25, 0, 7);
	EphData[0][6] = COMPOSE_BITS(UintValue, 7, 25);
	ULongValue = UnscaleULong(Ephemeris->ecc, -34);
	IntValue = (ULongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)ULongValue;
	EphData[0][6] |= COMPOSE_BITS(IntValue, 6, 1);
	EphData[0][6] |= COMPOSE_BITS(UintValue >> 26, 0, 6);
	EphData[0][7] = COMPOSE_BITS(UintValue, 6, 26);
	LongValue = UnscaleLong(Ephemeris->w / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	EphData[0][7] |= COMPOSE_BITS(IntValue, 5, 1);
	EphData[0][7] |= COMPOSE_BITS(UintValue >> 27, 0, 5);
	EphData[0][8] = COMPOSE_BITS(UintValue, 5, 27);

	// Message Type 11
	EphData[1][0] = (0x8b << 12) | (Ephemeris->svid << 6) | 11;
	UintValue = Ephemeris->toe / 300;
	EphData[1][1] = COMPOSE_BITS(UintValue, 3, 11);
	LongValue = UnscaleLong(Ephemeris->omega0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	EphData[1][1] |= COMPOSE_BITS(IntValue, 2, 1);
	EphData[1][1] |= COMPOSE_BITS(UintValue >> 30, 0, 2);
	EphData[1][2] = COMPOSE_BITS(UintValue, 2, 30);
	LongValue = UnscaleLong(Ephemeris->i0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	EphData[1][2] |= COMPOSE_BITS(IntValue, 1, 1);
	EphData[1][2] |= COMPOSE_BITS(UintValue >> 31, 0, 1);
	EphData[1][3] = COMPOSE_BITS(UintValue, 1, 31);
	IntValue = UnscaleInt(Ephemeris->omega_dot / PI - OMEGA_DOT_REF, -44);
	EphData[1][3] |= COMPOSE_BITS(IntValue >> 16, 0, 1);
	EphData[1][4] = COMPOSE_BITS(IntValue, 16, 16);
	IntValue = UnscaleInt(Ephemeris->idot / PI, -44);
	EphData[1][4] |= COMPOSE_BITS(IntValue, 1, 15);
	IntValue = UnscaleInt(Ephemeris->cis, -30);
	EphData[1][4] |= COMPOSE_BITS(IntValue >> 15, 0, 1);
	EphData[1][5] = COMPOSE_BITS(IntValue, 17, 15);
	IntValue = UnscaleInt(Ephemeris->cic, -30);
	EphData[1][5] |= COMPOSE_BITS(IntValue, 1, 16);
	IntValue = UnscaleInt(Ephemeris->crs, -8);
	EphData[1][5] |= COMPOSE_BITS(IntValue >> 23, 0, 1);
	EphData[1][6] = COMPOSE_BITS(IntValue, 9, 23);
	IntValue = UnscaleInt(Ephemeris->crc, -8);
	EphData[1][6] |= COMPOSE_BITS(IntValue >> 15, 0, 9);
	EphData[1][7] = COMPOSE_BITS(IntValue, 17, 15);
	IntValue = UnscaleInt(Ephemeris->cus, -30);
	EphData[1][7] |= COMPOSE_BITS(IntValue >> 4, 0, 17);
	EphData[1][8] = COMPOSE_BITS(IntValue, 28, 4);
	IntValue = UnscaleInt(Ephemeris->cuc, -30);
	EphData[1][8] |= COMPOSE_BITS(IntValue, 7, 21);

	// Clock Message
	UintValue = Ephemeris->top / 300;
	ClockData[0] = COMPOSE_BITS(UintValue, 3, 11);
	UintValue = Ephemeris->toc / 300;
	ClockData[1] = COMPOSE_BITS(UintValue, 13, 11);
	IntValue = UnscaleInt(Ephemeris->af0, -35);
	ClockData[1] |= COMPOSE_BITS(IntValue >> 13, 0, 13);
	ClockData[2] = COMPOSE_BITS(IntValue, 19, 13);
	IntValue = UnscaleInt(Ephemeris->af1, -48);
	ClockData[2] |= COMPOSE_BITS(IntValue >> 1, 0, 19);
	ClockData[3] = COMPOSE_BITS(IntValue, 31, 1);
	IntValue = UnscaleInt(Ephemeris->af2, -60);
	ClockData[3] |= COMPOSE_BITS(IntValue, 21, 10);

	// Delay Message
	IntValue = UnscaleInt(Ephemeris->tgd_ext[4], -35);
	DelayData[0] = COMPOSE_BITS(IntValue, 8, 13);
	IntValue = UnscaleInt(Ephemeris->tgd_ext[4] - Ephemeris->tgd, -35);
	DelayData[0] |= COMPOSE_BITS(IntValue >> 5, 0, 8);
	DelayData[1] = COMPOSE_BITS(IntValue, 27, 5);
	IntValue = UnscaleInt(Ephemeris->tgd_ext[4] - Ephemeris->tgd2, -35);
	DelayData[1] |= COMPOSE_BITS(IntValue, 14, 13);
	IntValue = UnscaleInt(Ephemeris->tgd_ext[4] - Ephemeris->tgd_ext[2], -35);
	DelayData[1] |= COMPOSE_BITS(IntValue, 1, 13);
	IntValue = UnscaleInt(Ephemeris->tgd_ext[4] - Ephemeris->tgd_ext[3], -35);
	DelayData[1] |= COMPOSE_BITS(IntValue >> 12, 0, 1);
	DelayData[2] = COMPOSE_BITS(IntValue, 20, 12);

	return 0;
}

int CNavBit::ComposeAlmWords(GPS_ALMANAC Almanac[], unsigned int &ReducedAlmData, unsigned int MidiAlmData[6])
{
	signed int IntValue;
	unsigned int UintValue;
	double DoubleValue;

	MidiAlmData[0] = COMPOSE_BITS(Almanac->valid ? Almanac->svid : 0, 26, 6);
	MidiAlmData[0] |= COMPOSE_BITS(Almanac->valid ? 0 : 7, 23, 3);
	UintValue = UnscaleUint(Almanac->ecc, -16);
	MidiAlmData[0] |= COMPOSE_BITS(UintValue, 12, 11);
	IntValue = UnscaleInt((Almanac->i0 - NORMINAL_I0) / PI, -14);
	MidiAlmData[0] |= COMPOSE_BITS(IntValue, 1, 11);
	IntValue = UnscaleInt(Almanac->omega_dot / PI, -33);
	MidiAlmData[0] |= COMPOSE_BITS(IntValue >> 10, 0, 1);
	MidiAlmData[1] = COMPOSE_BITS(IntValue, 22, 10);
	UintValue = UnscaleUint(Almanac->sqrtA, -4);
	MidiAlmData[1] |= COMPOSE_BITS(IntValue, 5, 17);
	IntValue = UnscaleInt(Almanac->omega0 / PI, -15);
	MidiAlmData[1] |= COMPOSE_BITS(IntValue >> 11, 0, 5);
	MidiAlmData[2] = COMPOSE_BITS(IntValue, 21, 11);
	IntValue = UnscaleInt(Almanac->w / PI, -15);
	MidiAlmData[2] |= COMPOSE_BITS(IntValue, 5, 16);
	IntValue = UnscaleInt(Almanac->M0 / PI, -15);
	MidiAlmData[2] |= COMPOSE_BITS(IntValue >> 11, 0, 5);
	MidiAlmData[3] = COMPOSE_BITS(IntValue, 21, 11);
	IntValue = UnscaleInt(Almanac->af0, -20);
	MidiAlmData[3] |= COMPOSE_BITS(IntValue, 10, 11);
	IntValue = UnscaleInt(Almanac->af0, -37);
	MidiAlmData[3] |= COMPOSE_BITS(IntValue, 0, 10);

	ReducedAlmData = (Almanac->valid ? Almanac->svid : 0) << 25;
	DoubleValue = Almanac->sqrtA * Almanac->sqrtA - A_REF;
	IntValue = (int)(DoubleValue / 512 + 0.5);
	ReducedAlmData |= COMPOSE_BITS(IntValue, 17, 8);
	IntValue = UnscaleInt(Almanac->omega0 / PI, -6);
	ReducedAlmData |= COMPOSE_BITS(IntValue, 10, 7);
	IntValue = UnscaleInt((Almanac->M0 + Almanac->w) / PI, -6);
	ReducedAlmData |= COMPOSE_BITS(IntValue, 3, 7);
	ReducedAlmData |= Almanac->valid ? 0 : 7;
	return 0;
}

void CNavBit::GetMessageData(int svid, int message, int TOW, unsigned int Data[9])
{
	int message_order[6] = {30, 33, 31, 37, 31, 37}, message_id;
	int frame = message / 4, alm_index;

	// validate svid to prevent out-of-bounds array access
	if (svid < 1 || svid > 32)
	{
		// initialize Data with zeros for invalid svid
		memset(Data, 0, sizeof(unsigned int) * 9);
		return;
	}

// frame index   1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25
// msg index 0  10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10 10
// msg index 1  11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// msg index 2  30 33 31 37 31 37 30 33 31 37 31 37 30 33 31 37 31 37 30 33 31 37 31 37 30
// msg index 3  37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 37 33
	message %= 4;
	switch (message)
	{
	case 0:	// message 10
		memcpy(Data, EphMessage[svid-1][0], sizeof(unsigned int) * 9);
		break;
	case 1:	// message 11
		memcpy(Data, EphMessage[svid-1][1], sizeof(unsigned int) * 9);
		break;
	case 2:	// message index 2
		message_id = message_order[frame%6];
		Data[1] = ClockMessage[svid-1][0]; Data[2] = ClockMessage[svid-1][1]; Data[3] = ClockMessage[svid-1][2]; Data[4] = ClockMessage[svid-1][3];	// copy clock fields
		switch (message_id)
		{
		case 30:
			Data[4] |= DelayMessage[svid-1][0]; Data[5] = DelayMessage[svid-1][1]; Data[6] = DelayMessage[svid-1][2];	// copy group delay fields
			Data[6] |= IonoMessage[0]; Data[7] = IonoMessage[1]; Data[8] = IonoMessage[2];	// copy ionosphere delay fields
			break;
		case 31:
			Data[4] |= TOA;
			alm_index = (frame / 6) * 8 + (((frame % 6) == 2) ? 0 : 4);
			Data[5] = (ReducedAlm[alm_index] << 1) + (ReducedAlm[alm_index+1] >> 30);
			Data[6] = (ReducedAlm[alm_index+1] << 2) + (ReducedAlm[alm_index+2] >> 29);
			Data[7] = (ReducedAlm[alm_index+2] << 3) + (ReducedAlm[alm_index+3] >> 28);
			Data[8] = (ReducedAlm[alm_index+3] << 4);
		case 33:
			Data[4] |= UTCMessage[0]; Data[5] = UTCMessage[1]; Data[6] = UTCMessage[2]; Data[7] = UTCMessage[2];	// copy UTC fields
			Data[8] = 0;
			break;
		case 37:
			alm_index = 24 + (frame / 6) + (((frame % 6) == 3) ? 0 : 1);
			Data[4] |= TOA; Data[5] = MidiAlm[alm_index][0]; Data[6] = MidiAlm[alm_index][1]; Data[7] = MidiAlm[alm_index][2]; Data[8] = MidiAlm[alm_index][3]; // copy almanac
			break;
		}
		Data[0] = (0x8b << 12) | (svid << 6) | message_id;
		break;
	case 3:	// message 37
		Data[0] = (0x8b << 12) | (svid << 6) | 37;
		Data[1] = ClockMessage[svid-1][0]; Data[2] = ClockMessage[svid-1][1]; Data[3] = ClockMessage[svid-1][2]; Data[4] = ClockMessage[svid-1][3];	// copy clock fields
		Data[4] |= TOA; Data[5] = MidiAlm[frame][0]; Data[6] = MidiAlm[frame][1]; Data[7] = MidiAlm[frame][2]; Data[8] = MidiAlm[frame][3]; // copy almanac
		break;
	}
	// add TOW
	Data[1] |= TOW << 15;
}

// encode 2MSB of EncodeWord, left shift EncodeWord and update ConvEncodeBits
unsigned char CNavBit::ConvolutionEncodePair(unsigned char &ConvEncodeBits, unsigned int &EncodeWord)
{
	ConvEncodeBits = (ConvEncodeBits << 2) + (unsigned char)(EncodeWord >> 30);
	EncodeWord <<= 2;
	return (ConvolutionEncode(ConvEncodeBits) & 0xf);
}
