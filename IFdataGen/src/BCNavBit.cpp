//----------------------------------------------------------------------
// BCNavBit.h:
//   Implementation of navigation bit synthesis class for BDS3 navigation bit
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "BCNavBit.h"
#include <cstdio>

const unsigned int BCNavBit::crc24q[256] = {
	0x00000000u, 0x01864CFBu, 0x028AD50Du, 0x030C99F6u, 0x0493E6E1u, 0x0515AA1Au, 0x061933ECu, 0x079F7F17u,
	0x08A18139u, 0x0927CDC2u, 0x0A2B5434u, 0x0BAD18CFu, 0x0C3267D8u, 0x0DB42B23u, 0x0EB8B2D5u, 0x0F3EFE2Eu,
	0x10C54E89u, 0x11430272u, 0x124F9B84u, 0x13C9D77Fu, 0x1456A868u, 0x15D0E493u, 0x16DC7D65u, 0x175A319Eu,
	0x1864CFB0u, 0x19E2834Bu, 0x1AEE1ABDu, 0x1B685646u, 0x1CF72951u, 0x1D7165AAu, 0x1E7DFC5Cu, 0x1FFBB0A7u,
	0x200CD1E9u, 0x218A9D12u, 0x228604E4u, 0x2300481Fu, 0x249F3708u, 0x25197BF3u, 0x2615E205u, 0x2793AEFEu,
	0x28AD50D0u, 0x292B1C2Bu, 0x2A2785DDu, 0x2BA1C926u, 0x2C3EB631u, 0x2DB8FACAu, 0x2EB4633Cu, 0x2F322FC7u,
	0x30C99F60u, 0x314FD39Bu, 0x32434A6Du, 0x33C50696u, 0x345A7981u, 0x35DC357Au, 0x36D0AC8Cu, 0x3756E077u,
	0x38681E59u, 0x39EE52A2u, 0x3AE2CB54u, 0x3B6487AFu, 0x3CFBF8B8u, 0x3D7DB443u, 0x3E712DB5u, 0x3FF7614Eu,
	0x4019A3D2u, 0x419FEF29u, 0x429376DFu, 0x43153A24u, 0x448A4533u, 0x450C09C8u, 0x4600903Eu, 0x4786DCC5u,
	0x48B822EBu, 0x493E6E10u, 0x4A32F7E6u, 0x4BB4BB1Du, 0x4C2BC40Au, 0x4DAD88F1u, 0x4EA11107u, 0x4F275DFCu,
	0x50DCED5Bu, 0x515AA1A0u, 0x52563856u, 0x53D074ADu, 0x544F0BBAu, 0x55C94741u, 0x56C5DEB7u, 0x5743924Cu,
	0x587D6C62u, 0x59FB2099u, 0x5AF7B96Fu, 0x5B71F594u, 0x5CEE8A83u, 0x5D68C678u, 0x5E645F8Eu, 0x5FE21375u,
	0x6015723Bu, 0x61933EC0u, 0x629FA736u, 0x6319EBCDu, 0x648694DAu, 0x6500D821u, 0x660C41D7u, 0x678A0D2Cu,
	0x68B4F302u, 0x6932BFF9u, 0x6A3E260Fu, 0x6BB86AF4u, 0x6C2715E3u, 0x6DA15918u, 0x6EADC0EEu, 0x6F2B8C15u,
	0x70D03CB2u, 0x71567049u, 0x725AE9BFu, 0x73DCA544u, 0x7443DA53u, 0x75C596A8u, 0x76C90F5Eu, 0x774F43A5u,
	0x7871BD8Bu, 0x79F7F170u, 0x7AFB6886u, 0x7B7D247Du, 0x7CE25B6Au, 0x7D641791u, 0x7E688E67u, 0x7FEEC29Cu,
	0x803347A4u, 0x81B50B5Fu, 0x82B992A9u, 0x833FDE52u, 0x84A0A145u, 0x8526EDBEu, 0x862A7448u, 0x87AC38B3u,
	0x8892C69Du, 0x89148A66u, 0x8A181390u, 0x8B9E5F6Bu, 0x8C01207Cu, 0x8D876C87u, 0x8E8BF571u, 0x8F0DB98Au,
	0x90F6092Du, 0x917045D6u, 0x927CDC20u, 0x93FA90DBu, 0x9465EFCCu, 0x95E3A337u, 0x96EF3AC1u, 0x9769763Au,
	0x98578814u, 0x99D1C4EFu, 0x9ADD5D19u, 0x9B5B11E2u, 0x9CC46EF5u, 0x9D42220Eu, 0x9E4EBBF8u, 0x9FC8F703u,
	0xA03F964Du, 0xA1B9DAB6u, 0xA2B54340u, 0xA3330FBBu, 0xA4AC70ACu, 0xA52A3C57u, 0xA626A5A1u, 0xA7A0E95Au,
	0xA89E1774u, 0xA9185B8Fu, 0xAA14C279u, 0xAB928E82u, 0xAC0DF195u, 0xAD8BBD6Eu, 0xAE872498u, 0xAF016863u,
	0xB0FAD8C4u, 0xB17C943Fu, 0xB2700DC9u, 0xB3F64132u, 0xB4693E25u, 0xB5EF72DEu, 0xB6E3EB28u, 0xB765A7D3u,
	0xB85B59FDu, 0xB9DD1506u, 0xBAD18CF0u, 0xBB57C00Bu, 0xBCC8BF1Cu, 0xBD4EF3E7u, 0xBE426A11u, 0xBFC426EAu,
	0xC02AE476u, 0xC1ACA88Du, 0xC2A0317Bu, 0xC3267D80u, 0xC4B90297u, 0xC53F4E6Cu, 0xC633D79Au, 0xC7B59B61u,
	0xC88B654Fu, 0xC90D29B4u, 0xCA01B042u, 0xCB87FCB9u, 0xCC1883AEu, 0xCD9ECF55u, 0xCE9256A3u, 0xCF141A58u,
	0xD0EFAAFFu, 0xD169E604u, 0xD2657FF2u, 0xD3E33309u, 0xD47C4C1Eu, 0xD5FA00E5u, 0xD6F69913u, 0xD770D5E8u,
	0xD84E2BC6u, 0xD9C8673Du, 0xDAC4FECBu, 0xDB42B230u, 0xDCDDCD27u, 0xDD5B81DCu, 0xDE57182Au, 0xDFD154D1u,
	0xE026359Fu, 0xE1A07964u, 0xE2ACE092u, 0xE32AAC69u, 0xE4B5D37Eu, 0xE5339F85u, 0xE63F0673u, 0xE7B94A88u,
	0xE887B4A6u, 0xE901F85Du, 0xEA0D61ABu, 0xEB8B2D50u, 0xEC145247u, 0xED921EBCu, 0xEE9E874Au, 0xEF18CBB1u,
	0xF0E37B16u, 0xF16537EDu, 0xF269AE1Bu, 0xF3EFE2E0u, 0xF4709DF7u, 0xF5F6D10Cu, 0xF6FA48FAu, 0xF77C0401u,
	0xF842FA2Fu, 0xF9C4B6D4u, 0xFAC82F22u, 0xFB4E63D9u, 0xFCD11CCEu, 0xFD575035u, 0xFE5BC9C3u, 0xFFDD8538u,
};

const unsigned int BCNavBit::e2v_table[128] = {
 1,  2,  4,  8, 16, 32,  3,  6, 12, 24, 48, 35,  5, 10, 20, 40,
19, 38, 15, 30, 60, 59, 53, 41, 17, 34,  7, 14, 28, 56, 51, 37, 
 9, 18, 36, 11, 22, 44, 27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 
13, 26, 52, 43, 21, 42, 23, 46, 31, 62, 63, 61, 57, 49, 33, 
 1,  2,  4,  8, 16, 32,  3,  6, 12, 24, 48, 35,  5, 10, 20, 40,
19, 38, 15, 30, 60, 59, 53, 41, 17, 34,  7, 14, 28, 56, 51, 37, 
 9, 18, 36, 11, 22, 44, 27, 54, 47, 29, 58, 55, 45, 25, 50, 39, 
13, 26, 52, 43, 21, 42, 23, 46, 31, 62, 63, 61, 57, 49, 33, 
};

const unsigned int BCNavBit::v2e_table[64] = {
 0,  0,  1,  6,  2, 12,  7, 26,  3, 32, 13, 35,  8, 48, 27, 18, 
 4, 24, 33, 16, 14, 52, 36, 54,  9, 45, 49, 38, 28, 41, 19, 56, 
 5, 62, 25, 11, 34, 31, 17, 47, 15, 23, 53, 51, 37, 44, 55, 40, 
10, 61, 46, 30, 50, 22, 39, 43, 29, 60, 42, 21, 20, 59, 57, 58, 
};

BCNavBit::BCNavBit()
{
	memset(Ephemeris1, 0, sizeof(Ephemeris1));
	memset(Ephemeris2, 0, sizeof(Ephemeris2));
	memset(ClockParam, 0, sizeof(ClockParam));
	memset(IntegrityFlags, 0, sizeof(IntegrityFlags));
	memset(ReducedAlmanac, 0, sizeof(ReducedAlmanac));
	memset(MidiAlmanac, 0, sizeof(MidiAlmanac));
	memset(BdGimIono, 0, sizeof(BdGimIono));
	memset(BdtUtcParam, 0, sizeof(BdtUtcParam));
	memset(EopParam, 0, sizeof(EopParam));
	memset(BgtoParam, 0, sizeof(BgtoParam));
}

BCNavBit::~BCNavBit()
{
}

int BCNavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	unsigned int *Data;
	signed int IntValue;
	unsigned int UintValue;
	long long int LongValue;
	unsigned long long int ULongValue;

	if (svid < 1 || svid > 63 || !Eph || !Eph->valid)
		return 0;
	if ((Eph->toe % 300) != 0)	// BCNAV ephemeris requires toe be multiple of 300
		return 0;

	// fill in Ephemeris1
	Data = Ephemeris1[svid-1];
	Data[0] = COMPOSE_BITS(Eph->iode, 16, 8);
	UintValue = Eph->toe / 300;	// toe
	Data[0] |= COMPOSE_BITS(UintValue, 5, 11);
	UintValue = Eph->flag;	// SatType
	Data[0] |= COMPOSE_BITS(UintValue, 3, 2);
	IntValue = UnscaleInt(Eph->axis - ((UintValue == 3) ? 27906100.0 : 42162200.0), -9);	// deltaA
	Data[0] |= COMPOSE_BITS(IntValue >> 23, 0, 3);
	Data[1] = COMPOSE_BITS(IntValue, 1, 23);
	IntValue = UnscaleInt(Eph->axis_dot, -21);	// Adot
	Data[1] |= COMPOSE_BITS(IntValue >> 24, 0, 1);
	Data[2] = COMPOSE_BITS(IntValue, 0, 24);
	IntValue = UnscaleInt(Eph->delta_n / PI, -44);	// delta_n
	Data[3] = COMPOSE_BITS(IntValue, 7, 17);
	IntValue = UnscaleInt(Eph->delta_n_dot, -57);	// delta n dot
	Data[3] |= COMPOSE_BITS(IntValue >> 16, 0, 7);
	Data[4] = COMPOSE_BITS(IntValue, 8, 16);
	LongValue = UnscaleLong(Eph->M0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[4] |= COMPOSE_BITS(IntValue, 7, 1);
	Data[4] |= COMPOSE_BITS(UintValue >> 25, 0, 7);
	Data[5] = COMPOSE_BITS(UintValue >> 1, 0, 24);
	Data[6] = COMPOSE_BITS(UintValue, 23, 1);
	ULongValue = UnscaleULong(Eph->ecc, -34);
	IntValue = (ULongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)ULongValue;
	Data[6] |= COMPOSE_BITS(IntValue, 22, 1);
	Data[6] |= COMPOSE_BITS(UintValue >> 10, 0, 22);
	Data[7] = COMPOSE_BITS(UintValue, 14, 10);
	LongValue = UnscaleLong(Eph->w / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[7] |= COMPOSE_BITS(IntValue, 13, 1);
	Data[7] |= COMPOSE_BITS(UintValue >> 19, 0, 13);
	Data[8] = COMPOSE_BITS(UintValue, 5, 19);

	// fill in Ephemeris2
	Data = Ephemeris2[svid-1];
	LongValue = UnscaleLong(Eph->omega0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[0] = COMPOSE_BITS(IntValue, 23, 1);
	Data[0] |= COMPOSE_BITS(UintValue >> 9, 0, 23);
	Data[1] = COMPOSE_BITS(UintValue, 15, 9);
	LongValue = UnscaleLong(Eph->i0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[1] |= COMPOSE_BITS(IntValue, 14, 1);
	Data[1] |= COMPOSE_BITS(UintValue >> 18, 0, 14);
	Data[2] = COMPOSE_BITS(UintValue, 6, 18);
	IntValue = UnscaleInt(Eph->omega_dot / PI, -44);	// omega dot
	Data[2] |= COMPOSE_BITS(IntValue >> 13, 0, 6);
	Data[3] = COMPOSE_BITS(IntValue, 11, 13);
	IntValue = UnscaleInt(Eph->idot / PI, -44);	// i dot
	Data[3] |= COMPOSE_BITS(IntValue >> 4, 0, 11);
	Data[4] = COMPOSE_BITS(IntValue, 20, 4);
	IntValue = UnscaleInt(Eph->cis, -30);	// cis
	Data[4] |= COMPOSE_BITS(IntValue, 4, 16);
	IntValue = UnscaleInt(Eph->cic, -30);	// cic
	Data[4] |= COMPOSE_BITS(IntValue >> 12, 0, 4);
	Data[5] = COMPOSE_BITS(IntValue, 12, 12);
	IntValue = UnscaleInt(Eph->crs, -8);	// crs
	Data[5] |= COMPOSE_BITS(IntValue >> 12, 0, 12);
	Data[6] = COMPOSE_BITS(IntValue, 12, 12);
	IntValue = UnscaleInt(Eph->crc, -8);	// crc
	Data[6] |= COMPOSE_BITS(IntValue >> 12, 0, 12);
	Data[7] = COMPOSE_BITS(IntValue, 12, 12);
	IntValue = UnscaleInt(Eph->cus, -30);	// cus
	Data[7] |= COMPOSE_BITS(IntValue >> 9, 0, 12);
	Data[8] = COMPOSE_BITS(IntValue, 15, 9);
	IntValue = UnscaleInt(Eph->cuc, -30);	// cuc
	Data[8] |= COMPOSE_BITS(IntValue >> 6, 0, 15);
	Data[9] = COMPOSE_BITS(IntValue, 18, 6);

	// fill in ClockParam (with 17 zeros)
	Data = ClockParam[svid-1];
	UintValue = Eph->toc / 300;	// toc
	Data[0] = COMPOSE_BITS(UintValue, 13, 11);
	IntValue = UnscaleInt(Eph->af0, -34);	// af0
	Data[0] |= COMPOSE_BITS(IntValue >> 12, 0, 13);
	Data[1] = COMPOSE_BITS(IntValue, 12, 12);
	IntValue = UnscaleInt(Eph->af1, -50);	// af1
	Data[1] |= COMPOSE_BITS(IntValue >> 10, 0, 12);
	Data[2] = COMPOSE_BITS(IntValue, 14, 10);
	IntValue = UnscaleInt(Eph->af2, -66);	// af2
	Data[2] |= COMPOSE_BITS(IntValue, 3, 11);
	Data[3] |= COMPOSE_BITS(Eph->iodc, 0, 10);

	// fill in IntegrityFlags
	IntegrityFlags[svid-1] = Eph->flag >> 2;

	// fill in TGD and ISC
	Data = TgsIscParam[svid-1];
	IntValue = UnscaleInt(Eph->tgd_ext[0] - Eph->tgd_ext[1], -34);	// ISC_B1C
	Data[0] = COMPOSE_BITS(IntValue, 12, 12);
	IntValue = UnscaleInt(Eph->tgd_ext[1], -34);	// TGD_B1C
	Data[0] |= COMPOSE_BITS(IntValue, 0, 12);
	IntValue = UnscaleInt(Eph->tgd_ext[2] - Eph->tgd_ext[3], -34);	// ISC_B2a
	Data[1] = COMPOSE_BITS(IntValue, 12, 12);
	IntValue = UnscaleInt(Eph->tgd_ext[1], -34);	// TGD_B2a
	Data[1] |= COMPOSE_BITS(IntValue, 0, 12);
	IntValue = UnscaleInt(Eph->tgd_ext[4], -34);	// TGD_B2b
	Data[2] = COMPOSE_BITS(IntValue, 0, 12);

	return svid;
}

int BCNavBit::SetAlmanac(GPS_ALMANAC Alm[])
{
	int i, toa = 0, week = 0;

	// fill in almanac page
	for (i = 0; i < 63; i ++)
	{
		FillBdsAlmanacPage(&Alm[i], MidiAlmanac[i], ReducedAlmanac[i]);
		if (Alm[i].valid & 1)
		{
			toa = Alm[i].toa >> 12;
			week = Alm[i].week & 0xff;
		}
	}

	return 0;
}

// put Length bits in 24bit WORD Src into 24bit WORD Dest starting from StartBit with MSB to LSB order
int BCNavBit::AppendWord(unsigned int *Dest, int StartBit, unsigned int *Src, int Length)
{
	int RemainBits = 24;
	int FillBits;

	while (Length > 0)
	{
		FillBits = 24 - StartBit;
		FillBits = (FillBits <= RemainBits) ? FillBits : RemainBits;
		FillBits = (FillBits <= Length) ? FillBits : Length;
		*Dest = ((StartBit == 0) ? 0 : (*Dest)) | COMPOSE_BITS(*Src >> (RemainBits - FillBits), (24 - StartBit - FillBits), FillBits);
		if ((StartBit += FillBits) >= 24)
		{
			StartBit = 0;
			Dest ++;
		}
		if ((RemainBits -= FillBits) <= 0)
		{
			RemainBits = 24;
			Src ++;
		}
		Length -= FillBits;
	}

	return StartBit;
}

// put bit in Data from MSB ot LSB into BitStream, bit order from bit(BitNumber-1) to bit(0) of Data
int BCNavBit::AssignBits(unsigned int Data, int BitNumber, int BitStream[])
{
	int i;

	Data <<= (32 - BitNumber);
	for (i = 0; i < BitNumber; i ++)
	{
		BitStream[i] = (Data & 0x80000000) ? 1 : 0;
		Data <<= 1;
	}

	return BitNumber;
}

// Append CRC to the end of data stream, Length is the size of DataStream (24bit data in each DWORD) including CRC bits
int BCNavBit::AppendCRC(unsigned int DataStream[], int Length)
{
	int i;
	unsigned int Data, crc_result = 0;

	for (i = 0; i < Length - 1; i ++)
	{
		Data = DataStream[i] << 8;	// move data to MSB
		crc_result = (crc_result << 8) ^ crc24q[(Data >> 24) ^ (unsigned char)(crc_result >> 16)];
		Data <<= 8;
		crc_result = (crc_result << 8) ^ crc24q[(Data >> 24) ^ (unsigned char)(crc_result >> 16)];
		Data <<= 8;
		crc_result = (crc_result << 8) ^ crc24q[(Data >> 24) ^ (unsigned char)(crc_result >> 16)];
		Data <<= 8;
	}
	DataStream[i] = (crc_result & 0xffffff);

	return 0;
}

int BCNavBit::LDPCEncode(int SymbolStream[], int SymbolLength, const char *MatrixGen)
{
	int i, j;
	int *Parity;
	const char *p1 = MatrixGen;
	int *p2, sum;

	// Check for NULL pointer
	if (!MatrixGen || !SymbolStream) {
		return -1;
	}

	Parity = SymbolStream + SymbolLength;
	for (i = 0; i < SymbolLength; i ++)
	{
		sum = 0;
		p2 = SymbolStream;
		for (j = 0; j < SymbolLength; j ++)
		{
			// Check if we're still within the matrix bounds
			if (*p1 == '\0') {
				return -1;
			}
			sum ^= GF6IntMul((int)(*p1)-'0', *p2);
			p1 ++; p2 ++;
		}
		*Parity ++ = sum;
	}

	return 0;
}

int BCNavBit::GF6IntMul(int a, int b)
{
	if (a && b)
		return e2v_table[v2e_table[a] + v2e_table[b]];
	else
		return 0;
}

int BCNavBit::FillBdsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int MidiAlm[8], unsigned int ReducedAlm[2])
{
	return 0;
}
