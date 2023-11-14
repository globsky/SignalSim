//----------------------------------------------------------------------
// LNavBit.h:
//   Implementation of navigation bit synthesis class for LNAV
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "LNavBit.h"

const unsigned char LNavBit::ParityTable[6][16] = {
	{ 0x00, 0x13, 0x25, 0x36, 0x0B, 0x18, 0x2E, 0x3D, 0x16, 0x05, 0x33, 0x20, 0x1D, 0x0E, 0x38, 0x2B, }, 
	{ 0x00, 0x2C, 0x19, 0x35, 0x32, 0x1E, 0x2B, 0x07, 0x26, 0x0A, 0x3F, 0x13, 0x14, 0x38, 0x0D, 0x21, },
	{ 0x00, 0x0E, 0x1F, 0x11, 0x3E, 0x30, 0x21, 0x2F, 0x3D, 0x33, 0x22, 0x2C, 0x03, 0x0D, 0x1C, 0x12, },
	{ 0x00, 0x38, 0x31, 0x09, 0x23, 0x1B, 0x12, 0x2A, 0x07, 0x3F, 0x36, 0x0E, 0x24, 0x1C, 0x15, 0x2D, },
	{ 0x00, 0x0D, 0x1A, 0x17, 0x37, 0x3A, 0x2D, 0x20, 0x2F, 0x22, 0x35, 0x38, 0x18, 0x15, 0x02, 0x0F, },
	{ 0x00, 0x1C, 0x3B, 0x27, 0x34, 0x28, 0x0F, 0x13, 0x2A, 0x36, 0x11, 0x0D, 0x1E, 0x02, 0x25, 0x39,}};
 
const unsigned int LNavBit::ParityAdjust[4] = { 0x00, 0xa5, 0xf6, 0x53};

LNavBit::LNavBit()
{
	memset(GpsStream123, 0xaa, sizeof(GpsStream123));
	memset(GpsStream45, 0xaa, sizeof(GpsStream45));
}

LNavBit::~LNavBit()
{
}

int LNavBit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i, TOW, subframe, page;
	unsigned int TlmWord, HowWord, CurWord, *Stream;

	// first determine the current TOW and subframe number
	StartTime.Week += StartTime.MilliSeconds / 604800000;
	StartTime.MilliSeconds %= 604800000;
	TOW = StartTime.MilliSeconds / 6000;
	subframe = (TOW % 5) + 1;
	if (subframe > 3)	// subframe 4/5, further determine page number
	{
		page = (TOW / 5) % 25;
		Stream = GpsStream45[subframe-4][page];
	}
	else if (svid >= 1 && svid <= 32)
		Stream = GpsStream123[svid-1] + (subframe - 1) * 8;
	else
		return 1;

	TlmWord = (0x8b << 22);		// set all TLM message to 0 and let ISF=0
	TOW ++;		// TOW is the count of NEXT subframe
	if (TOW >= 100800)
		TOW = 0;
	HowWord = (TOW << 13) + (subframe << 8);	// set HOW word

	// generate bit stream of TLM, this is WORD 1
	CurWord = TlmWord | GpsGetParity(TlmWord);	// D29* and D30* are 00 for TLM
	AssignBits(CurWord, 30, NavBits);

	// generate bit stream of HOW, this is WORD 2
	CurWord <<= 30;		// put D29* and D30* into bit31 and bit30
	CurWord |= HowWord;	// fill in d1 to d24 into bit29 to bit6
	if (CurWord & 0x40000000)	// convert d1 to d24 into D1 to D24
		CurWord ^= 0x3fffffc0;
	CurWord |= GpsGetParity(CurWord);	// add parity check
	CurWord ^= ParityAdjust[CurWord & 3];	// adjust d23 and d24 to ensure last two bit of parity to be 00
	AssignBits(CurWord, 30, NavBits + 30);

	// WORD 3 to DOWRD 10
	for (i = 0; i < 8; i ++)
	{
		CurWord <<= 30;		// put D29* and D30* into bit31 and bit30
		CurWord |= ((Stream[i] & 0xffffff) << 6);	// fill in d1 to d24 into bit29 to bit6
		if (subframe == 1 && i == 0)	// WORD3 of subframe 1, put in week number from bit 29~20
			CurWord |= ((StartTime.Week & 0x3ff) << 20);
		if (CurWord & 0x40000000)	// convert d1 to d24 into D1 to D24
			CurWord ^= 0x3fffffc0;
		CurWord |= GpsGetParity(CurWord);	// add parity check
		if (i == 7)
			CurWord ^= ParityAdjust[CurWord & 3];	// adjust d23 and d24 to ensure last two bit of parity to be 00
		AssignBits(CurWord, 30, NavBits + 60 + i * 30);
	}

	return 0;
}

int LNavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	if (svid < 1 || svid > 32 || !Eph || !Eph->valid)
		return 0;
	ComposeGpsStream123(Eph, GpsStream123[svid-1]);
	return svid;
}

int LNavBit::SetAlmanac(int svid, PGPS_ALMANAC Alm)
{
	unsigned int *Stream;

	if (svid < 1)
		return 0;
	else if (svid <= 24)
		Stream = GpsStream45[1][svid-1];	// SV01 to SV24 in subframe 5 page 1 to 24
	else if (svid <= 28)
		Stream = GpsStream45[0][svid-24];	// SV25 to SV28 in subframe 4 page 2 to 5
	else if (svid <= 32)
		Stream = GpsStream45[0][svid-23];	// SV29 to SV32 in subframe 4 page 7 to 10
	else
		return 0;
	
	FillGpsAlmanacPage(Alm, Stream);
	return svid;
}

int LNavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	unsigned int *Stream = GpsStream45[0][17];	// Ionosphere and UTC parameter in page 18 (indexed at 17)
	signed int IntValue;

	if (IonoParam->flag == 0 || (UtcParam->flag & 3) != 3)
		return 0;
	Stream[0] = COMPOSE_BITS(56 + 0x40, 16, 8);
	IntValue = UnscaleInt(IonoParam->a0, -30);
	Stream[0] |= COMPOSE_BITS(IntValue, 8, 8);
	IntValue = UnscaleInt(IonoParam->a1, -27);
	Stream[0] |= COMPOSE_BITS(IntValue, 0, 8);
	IntValue = UnscaleInt(IonoParam->a2, -24);
	Stream[1] = COMPOSE_BITS(IntValue, 16, 8);
	IntValue = UnscaleInt(IonoParam->a3, -24);
	Stream[1] |= COMPOSE_BITS(IntValue, 8, 8);
	IntValue = UnscaleInt(IonoParam->b0, 11);
	Stream[1] |= COMPOSE_BITS(IntValue, 0, 8);
	IntValue = UnscaleInt(IonoParam->b1, 14);
	Stream[2] = COMPOSE_BITS(IntValue, 16, 8);
	IntValue = UnscaleInt(IonoParam->b2, 16);
	Stream[2] |= COMPOSE_BITS(IntValue, 8, 8);
	IntValue = UnscaleInt(IonoParam->b3, 16);
	Stream[2] |= COMPOSE_BITS(IntValue, 0, 8);
	IntValue = UnscaleInt(UtcParam->A1, -50);
	Stream[3] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(UtcParam->A0, -30);
	Stream[4] = COMPOSE_BITS(IntValue >> 8, 0, 24);
	Stream[5] = COMPOSE_BITS(IntValue & 0xff, 16, 8);
	Stream[5] |= COMPOSE_BITS(UtcParam->tot, 8, 8);
	Stream[5] |= COMPOSE_BITS(UtcParam->WN, 0, 8);
	Stream[6] = COMPOSE_BITS(UtcParam->TLS, 16, 8);
	Stream[6] |= COMPOSE_BITS(UtcParam->WNLSF, 8, 8);
	Stream[6] |= COMPOSE_BITS(UtcParam->DN, 0, 8);
	Stream[7] = COMPOSE_BITS(UtcParam->TLSF, 16, 8);

	return 0;
}

// fill ephemeris data into subframe 1/2/3
// Word1/2 removed, parity not included
// 24 information bits occupies bit0~23 of each DWORD in Stream[]
int LNavBit::ComposeGpsStream123(PGPS_EPHEMERIS Ephemeris, unsigned int Stream[3*8])
{
	signed int IntValue;
	unsigned int UintValue;

	// subframe 1, Stream[0]~Stream[7]
	IntValue = Ephemeris->flag & 3;
	Stream[0] = COMPOSE_BITS(IntValue, 12, 2);
	IntValue = Ephemeris->ura & 0xf;
	Stream[0] |= COMPOSE_BITS(IntValue, 8, 4);
	Stream[0] |= COMPOSE_BITS(Ephemeris->health, 2, 6);
	IntValue = Ephemeris->iodc >> 8;
	Stream[0] |= COMPOSE_BITS(IntValue, 0, 2);
	IntValue = Ephemeris->flag >> 2;
	Stream[1] = COMPOSE_BITS(IntValue, 23, 1);
	IntValue = UnscaleInt(Ephemeris->tgd, -31);
	Stream[4] = COMPOSE_BITS(IntValue, 0, 8);
	Stream[5] = COMPOSE_BITS(Ephemeris->iodc, 16, 8);
	Stream[5] |= COMPOSE_BITS(Ephemeris->toc >> 4, 0, 16);
	IntValue = UnscaleInt(Ephemeris->af2, -55);
	Stream[6] = COMPOSE_BITS(IntValue, 16, 8);
	IntValue = UnscaleInt(Ephemeris->af1, -43);
	Stream[6] |= COMPOSE_BITS(IntValue, 0, 16);
	IntValue = UnscaleInt(Ephemeris->af0, -31);
	Stream[7] = COMPOSE_BITS(IntValue, 2, 22);

	// subframe 2, Stream[8]~Stream[15]
	Stream[8] = COMPOSE_BITS(Ephemeris->iode2, 16, 8);
	IntValue = UnscaleInt(Ephemeris->crs, -5);
	Stream[8] |= COMPOSE_BITS(IntValue, 0, 16);
	IntValue = UnscaleInt(Ephemeris->delta_n / PI, -43);
	Stream[9] = COMPOSE_BITS(IntValue, 8, 16);
	IntValue = UnscaleInt(Ephemeris->M0 / PI, -31);
	Stream[9] |= COMPOSE_BITS(IntValue >> 24, 0, 8);
	Stream[10] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Ephemeris->cuc, -29);
	Stream[11] = COMPOSE_BITS(IntValue, 8, 16);
	UintValue = UnscaleUint(Ephemeris->ecc, -33);
	Stream[11] |= COMPOSE_BITS(UintValue >> 24, 0, 8);
	Stream[12] = COMPOSE_BITS(UintValue, 0, 24);
	IntValue = UnscaleInt(Ephemeris->cus, -29);
	Stream[13] = COMPOSE_BITS(IntValue, 8, 16);
	UintValue = UnscaleUint(Ephemeris->sqrtA, -19);
	Stream[13] |= COMPOSE_BITS(UintValue >> 24, 0, 8);
	Stream[14] = COMPOSE_BITS(UintValue, 0, 24);
	Stream[15] = COMPOSE_BITS(Ephemeris->toe >> 4, 8, 16);
	Stream[15] |= COMPOSE_BITS(Ephemeris->flag >> 3, 7, 1);

	// subframe 3, Stream[16]~Stream[23]
	IntValue = UnscaleInt(Ephemeris->cic, -29);
	Stream[16] = COMPOSE_BITS(IntValue, 8, 16);
	IntValue = UnscaleInt(Ephemeris->omega0 / PI, -31);
	Stream[16] |= COMPOSE_BITS(IntValue >> 24, 0, 8);
	Stream[17] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Ephemeris->cis, -29);
	Stream[18] = COMPOSE_BITS(IntValue, 8, 16);
	IntValue = UnscaleInt(Ephemeris->i0 / PI, -31);
	Stream[18] |= COMPOSE_BITS(IntValue >> 24, 0, 8);
	Stream[19] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Ephemeris->crc, -5);
	Stream[20] = COMPOSE_BITS(IntValue, 8, 16);
	IntValue = UnscaleInt(Ephemeris->w / PI, -31);
	Stream[20] |= COMPOSE_BITS(IntValue >> 24, 0, 8);
	Stream[21] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Ephemeris->omega_dot / PI, -43);
	Stream[22] = COMPOSE_BITS(IntValue, 0, 24);
	Stream[23] = COMPOSE_BITS(Ephemeris->iode3, 16, 8);
	IntValue = UnscaleInt(Ephemeris->idot / PI, -43);
	Stream[23] |= COMPOSE_BITS(IntValue, 2, 14);

	return 0;
}

int LNavBit::FillGpsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int Stream[8])
{
	signed int IntValue;
	unsigned int UintValue;

	if (Almanac->flag == 0)
		return 0;
	Stream[0] = COMPOSE_BITS(Almanac->svid + 0x40, 16, 8);
	UintValue = UnscaleUint(Almanac->ecc, -21);
	Stream[0] |= COMPOSE_BITS(UintValue, 0, 16);
	Stream[1] = COMPOSE_BITS(Almanac->toa >> 12, 16, 8);
	IntValue = UnscaleInt(Almanac->i0 / PI - 0.3, -19);
	Stream[1] |= COMPOSE_BITS(IntValue, 0, 16);
	IntValue = UnscaleInt(Almanac->omega_dot / PI, -38);
	Stream[2] = COMPOSE_BITS(IntValue, 8, 16);
	Stream[2] |= COMPOSE_BITS(Almanac->health, 0, 8);
	UintValue = UnscaleUint(Almanac->sqrtA, -11);
	Stream[3] = COMPOSE_BITS(UintValue, 0, 24);
	IntValue = UnscaleInt(Almanac->omega0 / PI, -23);
	Stream[4] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Almanac->w / PI, -23);
	Stream[5] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Almanac->M0 / PI, -23);
	Stream[6] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Almanac->af0, -20);
	Stream[7] = COMPOSE_BITS(IntValue >> 3, 16, 8);
	Stream[7] |= COMPOSE_BITS(IntValue & 0x7, 2, 3);
	IntValue = UnscaleInt(Almanac->af1, -38);
	Stream[7] |= COMPOSE_BITS(IntValue, 5, 11);

	return 0;
}

// d29* at bit31, d30* at bit30, current word without parity at bit29 to bit6, bit5~0 ignored
// generate 6 parity bits
unsigned int LNavBit::GpsGetParity(unsigned int word)
{
	int i;
	unsigned int parity = 0;

	word >>= 6;	// remove bit5~0
	for (i = 0; i < 6; i ++)
	{
		parity ^= (unsigned int)ParityTable[i][word&0xf];
		word >>= 4;
	}
	// add d29* and d30*
	if (word & 1)
		parity ^= 0x15;
	if (word & 2)
		parity ^= 0x29;

	return parity;
}
