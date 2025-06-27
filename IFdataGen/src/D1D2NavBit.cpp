//----------------------------------------------------------------------
// D1D2NavBit.h:
//   Implementation of navigation bit synthesis class for BDS2 D1/D2
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "D1D2NavBit.h"

// H matrix:
// 1 1 1 1 0 1 0 1 1 0 0 1 0 0 0
// 0 1 1 1 1 0 1 0 1 1 0 0 1 0 0
// 0 0 1 1 1 1 0 1 0 1 1 0 0 1 0
// 1 1 1 0 1 0 1 1 0 0 1 0 0 0 1
const unsigned int D1D2NavBit::BCHPoly[4] = { 0x7ac0, 0x3d60, 0x1eb0, 0x7590 };

D1D2NavBit::D1D2NavBit()
{
	int i;

	memset(BdsStream123, 0x0, sizeof(BdsStream123));
	memset(BdsStreamAlm, 0x0, sizeof(BdsStreamAlm));
	memset(BdsStreamInfo, 0x0, sizeof(BdsStreamInfo));
	memset(BdsStreamHealth, 0x0, sizeof(BdsStreamHealth));
	memset(BdsStreamD2, 0x0, sizeof(BdsStreamD2));
	memset(&IonoParamSave, 0, sizeof(IONO_PARAM));
	// fill page number and AlEpID/AmID
	for (i = 0; i < 30; i ++)	// subframe 4 page 1~24 and subframe 5 page1~6
	{
		BdsStreamAlm[i][0] = (i < 24) ? ((i + 1) << 2) : ((i - 23) << 2);
		BdsStreamAlm[i][8] = 3;	// AlEpID = 11
	}
	for (; i < 43; i ++)	// subframe 5 page 11~23 for svid 31~43
	{
		BdsStreamAlm[i][0] = ((i - 19) << 2);
		BdsStreamAlm[i][8] = 1;	// AmID = 01
	}
	for (; i < 56; i ++)	// subframe 5 page 11~23 for svid 44~56
	{
		BdsStreamAlm[i][0] = ((i - 32) << 2);
		BdsStreamAlm[i][8] = 2;	// AmID = 01
	}
	for (; i < 63; i ++)	// subframe 5 page 11~17 for svid 57~63
	{
		BdsStreamAlm[i][0] = ((i - 45) << 2);
		BdsStreamAlm[i][8] = 3;	// AmID = 01
	}
	for (i = 0; i < 4; i ++)	// subframe 5 page 7~10
		BdsStreamInfo[0][0] = (i + 7) << 2;
	for (i = 0; i < 3; i ++)	// subframe 5 page 24
	{
		BdsStreamHealth[i][0] = 24 << 2;
		BdsStreamHealth[i][6] = (i + 1) << 15;	// AmID
	}
}

D1D2NavBit::~D1D2NavBit()
{
}

int D1D2NavBit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i, SOW, subframe, page = 0, page_ext, D1Data;
	unsigned int CurWord, Stream[10];

	for (i = 0; i < 10; i++)
		Stream[i] = 0;

	if ((svid >= 1 && svid <= 5) || (svid >= 59 && svid <= 63))
		D1Data = 0;
	else if (svid >= 6 && svid <= 58)
		D1Data = 1;
	else
		return 1;

	// first determine the current TOW and subframe number
	StartTime.Week += StartTime.MilliSeconds / 604800000;
	StartTime.MilliSeconds %= 604800000;
	if (D1Data)	// D1 bit stream
	{
		SOW = StartTime.MilliSeconds / 1000;
		subframe = ((SOW / 6) % 5) + 1;
		if (subframe > 3)	// subframe 4/5, further determine page number
		{
			page = (SOW / 30) % 72;
			page_ext = page / 24;
			page %= 24;
			if (subframe == 4)
				memcpy(Stream + 1, BdsStreamAlm[page], sizeof(unsigned int) * 9);
			else if (page < 6)	// subframe 5 page 1~6
				memcpy(Stream + 1, BdsStreamAlm[page+24], sizeof(unsigned int) * 9);
			else if (page < 10)	// subframe 5 page 7~10
				memcpy(Stream + 1, BdsStreamInfo[page-6], sizeof(unsigned int) * 9);
			else if (page < 23)	// subframe 5 page 11~23
			{
				if (page_ext == 0)	// AmID = 1
					memcpy(Stream + 1, BdsStreamAlm[page+20], sizeof(unsigned int) * 9);
				else if (page_ext == 1)	// AmID = 2
					memcpy(Stream + 1, BdsStreamAlm[page+33], sizeof(unsigned int) * 9);
				else if (page < 17)	// AmID = 3
					memcpy(Stream + 1, BdsStreamAlm[page+46], sizeof(unsigned int) * 9);
				else	// subframe 5 undefined page
				{
					memset(Stream + 1, 0, sizeof(unsigned int) * 9);
					Stream[1] = (page + 1) << 24;
				}
			}
			else	// subframe 5 page 24
				memcpy(Stream + 1, BdsStreamHealth[page_ext], sizeof(unsigned int) * 9);
		}
		else
		{
			for (i = 0; i < 9; i++)
				Stream[i + 1] = BdsStream123[svid - 6][(subframe - 1) * 9 + i];
		}
	}
	else	// D2 bit Stream
	{
		SOW = StartTime.MilliSeconds / 3000 * 3;
		subframe = ((StartTime.MilliSeconds / 600) % 5) + 1;
		if (subframe == 1)	// subframe 1, further determine page number
		{
			page = (StartTime.MilliSeconds / 3000) % 10;
			for (i = 0; i < 4; i ++)
				Stream[i + 1] = BdsStreamD2[(svid <= 5) ? (svid - 1) : (svid - 54)][page * 4 + i];
		}
	}

	// add preamble and SOW
	Stream[0] = (0x712 << 19) | (subframe << 12) | ((SOW >> 8) & 0xff0);
	Stream[1] |= ((SOW & 0xfff) << 10);

	Stream[0] = GetBCH(Stream[0]);
	AssignBits(Stream[0], 30, NavBits);
	for (i = 1; i < 10; i++)
	{
		CurWord = GetBCH((Stream[i] >> 7) & 0x7ff0) << 16;
		CurWord |= GetBCH((Stream[i] << 4) & 0x7ff0);
		CurWord = Interleave(CurWord);
		AssignBits(CurWord, 30, NavBits + i * 30);
	}

	return 0;
}

int D1D2NavBit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	if (!Eph || !Eph->valid)
		return 0;
	
	if (svid < 1)
		return 0;
	else if (svid < 6)	// GEO 1~5
		ComposeBdsStreamD2(Eph, &IonoParamSave, BdsStreamD2[svid - 1]);
	else if (svid < 59)	// MEO/IGSO 6~58
		ComposeBdsStream123(Eph, &IonoParamSave, BdsStream123[svid - 6]);
	else if (svid < 64)	// GEO 59~63
		ComposeBdsStreamD2(Eph, &IonoParamSave, BdsStreamD2[svid - 54]);
	else
		return 0;

	return svid;
}

int D1D2NavBit::SetAlmanac(GPS_ALMANAC Alm[])
{
	int i, toa = 0, week = 0;

	// fill in almanac page
	for (i = 0; i < 63; i ++)
	{
		FillBdsAlmanacPage(&Alm[i], BdsStreamAlm[i]);
		if (Alm[i].valid & 1)
		{
			toa = Alm[i].toa >> 12;
			week = Alm[i].week & 0xff;
		}
	}
	memset(BdsStreamInfo, 0, sizeof(unsigned int) * 18);
	BdsStreamInfo[0][0] = 7 << 2;
	BdsStreamInfo[1][0] = 8 << 2;
	memset(BdsStreamHealth, 0x0, sizeof(BdsStreamHealth));
	for (i = 0; i < 3; i ++)	// subframe 5 page 24
	{
		BdsStreamHealth[i][0] = 24 << 2;
		BdsStreamHealth[i][6] = (i + 1) << 15;	// AmID
	}
	// fill in health page 7
	FillBdsHealthPage(Alm, 19, BdsStreamInfo[0]);
	// fill in health page 8
	FillBdsHealthPage(Alm + 19, 11, BdsStreamInfo[1]);
	BdsStreamInfo[1][6] |= COMPOSE_BITS(toa, 19, 3);
	BdsStreamInfo[1][5] |= COMPOSE_BITS((toa >> 3), 0, 5);
	BdsStreamInfo[1][5] |= COMPOSE_BITS(week, 5, 8);
	// fill in health page 24
	FillBdsHealthPage(Alm + 30, 13, BdsStreamHealth[0]);
	FillBdsHealthPage(Alm + 43, 13, BdsStreamHealth[1]);
	FillBdsHealthPage(Alm + 56, 7, BdsStreamHealth[2]);

	return 0;
}

int D1D2NavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	unsigned int *Stream = BdsStreamInfo[3];	// UTC parameter in page 10 (indexed at 3)
	double Value;
	signed int IntValue;

	if (IonoParam->flag == 0 || (UtcParam->flag & 3) != 3)
		return 0;
	memcpy(&IonoParamSave, IonoParam, sizeof(IONO_PARAM));	// save ionosphere parameters to compose subframe1

	Stream[0] = COMPOSE_BITS((UtcParam->TLS >> 6), 0, 2);
	Stream[0] |= COMPOSE_BITS(10, 2, 7);	// page number 10
	Stream[1] = COMPOSE_BITS(UtcParam->WNLSF, 0, 8);
	Stream[1] |= COMPOSE_BITS(UtcParam->TLSF, 8, 8);
	Stream[1] |= COMPOSE_BITS((UtcParam->TLS & 0x3f), 16, 6);
	Value = UnscaleDouble(UtcParam->A0, -30);
	IntValue = roundi(Value);
	Stream[2] = COMPOSE_BITS((IntValue >> 10), 0, 22);
	Stream[3] = COMPOSE_BITS((IntValue & 0x3ff), 12, 10);
	Value = UnscaleDouble(UtcParam->A1, -50);
	IntValue = roundi(Value);
	Stream[3] |= COMPOSE_BITS((IntValue >> 12), 0, 12);
	Stream[4] = COMPOSE_BITS((IntValue & 0xfff), 10, 12);
	Stream[4] |= COMPOSE_BITS(UtcParam->DN, 2, 8);

	return 0;
}

int D1D2NavBit::ComposeBdsStream123(PGPS_EPHEMERIS Ephemeris, PIONO_PARAM IonoParam, unsigned int Stream[3*9])
{
	double Value;
	signed int IntValue;
	unsigned int UintValue;

	// subframe 1, Stream[0]~Stream[8]
	// subframe 1, Stream[0]~Stream[8]
	Stream[0] = COMPOSE_BITS(Ephemeris->health, 9, 1);
	Stream[0] |= COMPOSE_BITS(Ephemeris->iodc, 4, 5);
	Stream[0] |= COMPOSE_BITS(Ephemeris->ura, 0, 4);
	Stream[1] = COMPOSE_BITS(Ephemeris->week, 9, 13);
	IntValue = Ephemeris->toc >> 3;
	Stream[1] |= COMPOSE_BITS(IntValue >> 8, 0, 9);
	Stream[2] = COMPOSE_BITS(IntValue, 14, 8);
	IntValue = roundi(Ephemeris->tgd * 1e10);
	Stream[2] |= COMPOSE_BITS(IntValue, 4, 10);
	IntValue = roundi(Ephemeris->tgd2 * 1e10);
	Stream[2] |= COMPOSE_BITS(IntValue >> 6, 0, 4);
	Stream[3] = COMPOSE_BITS(IntValue, 16, 6);
	Value = UnscaleDouble(IonoParam->a0, -30);
	IntValue = roundi(Value);
	Stream[3] |= COMPOSE_BITS(IntValue, 8, 8);
	Value = UnscaleDouble(IonoParam->a1, -27);
	IntValue = roundi(Value);
	Stream[3] |= COMPOSE_BITS(IntValue, 0, 8);
	Value = UnscaleDouble(IonoParam->a2, -24);
	IntValue = roundi(Value);
	Stream[4] = COMPOSE_BITS(IntValue, 14, 8);
	Value = UnscaleDouble(IonoParam->a3, -24);
	IntValue = roundi(Value);
	Stream[4] |= COMPOSE_BITS(IntValue, 6, 8);
	Value = UnscaleDouble(IonoParam->b0, 11);
	IntValue = roundi(Value);
	Stream[4] |= COMPOSE_BITS(IntValue >> 2, 0, 6);
	Stream[5] = COMPOSE_BITS(IntValue, 20, 2);
	Value = UnscaleDouble(IonoParam->b1, 14);
	IntValue = roundi(Value);
	Stream[5] |= COMPOSE_BITS(IntValue, 12, 8);
	Value = UnscaleDouble(IonoParam->b2, 16);
	IntValue = roundi(Value);
	Stream[5] |= COMPOSE_BITS(IntValue, 4, 8);
	Value = UnscaleDouble(IonoParam->b3, 16);
	IntValue = roundi(Value);
	Stream[5] |= COMPOSE_BITS(IntValue >> 4, 0, 4);
	Stream[6] = COMPOSE_BITS(IntValue, 18, 4);
	Value = UnscaleDouble(Ephemeris->af2, -66);
	IntValue = roundi(Value);
	Stream[6] |= COMPOSE_BITS(IntValue, 7, 11);
	Value = UnscaleDouble(Ephemeris->af0, -33);
	IntValue = roundi(Value);
	Stream[6] |= COMPOSE_BITS(IntValue >> 17, 0, 7);
	Stream[7] = COMPOSE_BITS(IntValue, 5, 17);
	Value = UnscaleDouble(Ephemeris->af1, -50);
	IntValue = roundi(Value);
	Stream[7] |= COMPOSE_BITS(IntValue >> 17, 0, 5);
	Stream[8] = COMPOSE_BITS(IntValue, 5, 17);
	Stream[8] |= COMPOSE_BITS(Ephemeris->iode, 0, 5);

	// subframe 2, Stream[9]~Stream[17]
	Value = UnscaleDouble(Ephemeris->delta_n / PI, -43);
	IntValue = roundi(Value);
	Stream[9] = COMPOSE_BITS(IntValue >> 6, 0, 10);
	Stream[10] = COMPOSE_BITS(IntValue, 16, 6);
	Value = UnscaleDouble(Ephemeris->cuc, -31);
	IntValue = roundi(Value);
	Stream[10] |= COMPOSE_BITS(IntValue >> 2, 0, 16);
	Stream[11] = COMPOSE_BITS(IntValue, 20, 2);
	Value = UnscaleDouble(Ephemeris->M0 / PI, -31);
	IntValue = roundi(Value);
	Stream[11] |= COMPOSE_BITS(IntValue >> 12, 0, 20);
	Stream[12] = COMPOSE_BITS(IntValue, 10, 12);
	Value = UnscaleDouble(Ephemeris->ecc, -33);
	UintValue = roundu(Value);
	Stream[12] |= COMPOSE_BITS(UintValue >> 22, 0, 10);
	Stream[13] = COMPOSE_BITS(UintValue, 0, 22);
	Value = UnscaleDouble(Ephemeris->cus, -31);
	IntValue = roundi(Value);
	Stream[14] = COMPOSE_BITS(IntValue, 4, 18);
	Value = UnscaleDouble(Ephemeris->crc, -6);
	IntValue = roundi(Value);
	Stream[14] |= COMPOSE_BITS(IntValue >> 14, 0, 4);
	Stream[15] = COMPOSE_BITS(IntValue, 8, 14);
	Value = UnscaleDouble(Ephemeris->crs, -6);
	IntValue = roundi(Value);
	Stream[15] |= COMPOSE_BITS(IntValue >> 10, 0, 8);
	Stream[16] = COMPOSE_BITS(IntValue, 12, 10);
	Value = UnscaleDouble(Ephemeris->sqrtA, -19);
	UintValue = roundu(Value);
	Stream[16] |= COMPOSE_BITS(UintValue >> 20, 0, 12);
	Stream[17] = COMPOSE_BITS(UintValue, 2, 20);
	Stream[17] |= COMPOSE_BITS(Ephemeris->toe >> 18, 0, 2);

	// subframe 3, Stream[18]~Stream[26]
	Stream[18] = COMPOSE_BITS(Ephemeris->toe >> 8, 0, 10);
	Stream[19] = COMPOSE_BITS(Ephemeris->toe >> 3, 17, 5);
	Value = UnscaleDouble(Ephemeris->i0 / PI, -31);
	IntValue = roundi(Value);
	Stream[19] |= COMPOSE_BITS(IntValue >> 15, 0, 17);
	Stream[20] = COMPOSE_BITS(IntValue, 7, 15);
	Value = UnscaleDouble(Ephemeris->cic, -31);
	IntValue = roundi(Value);
	Stream[20] |= COMPOSE_BITS(IntValue >> 11, 0, 7);
	Stream[21] = COMPOSE_BITS(IntValue, 11, 11);
	Value = UnscaleDouble(Ephemeris->omega_dot / PI, -43);
	IntValue = roundi(Value);
	Stream[21] |= COMPOSE_BITS(IntValue >> 13, 0, 11);
	Stream[22] = COMPOSE_BITS(IntValue, 9, 13);
	Value = UnscaleDouble(Ephemeris->cis, -31);
	IntValue = roundi(Value);
	Stream[22] |= COMPOSE_BITS(IntValue >> 9, 0, 9);
	Stream[23] = COMPOSE_BITS(IntValue, 13, 9);
	Value = UnscaleDouble(Ephemeris->idot / PI, -43);
	IntValue = roundi(Value);
	Stream[23] |= COMPOSE_BITS(IntValue >> 1, 0, 13);
	Stream[24] = COMPOSE_BITS(IntValue, 21, 1);
	Value = UnscaleDouble(Ephemeris->omega0 / PI, -31);
	IntValue = roundi(Value);
	Stream[24] |= COMPOSE_BITS(IntValue >> 11, 0, 21);
	Stream[25] = COMPOSE_BITS(IntValue, 11, 11);
	Value = UnscaleDouble(Ephemeris->w / PI, -31);
	IntValue = roundi(Value);
	Stream[25] |= COMPOSE_BITS(IntValue >> 21, 0, 11);
	Stream[26] = COMPOSE_BITS(IntValue, 1, 21);

	return 0;
}

int D1D2NavBit::ComposeBdsStreamD2(PGPS_EPHEMERIS Ephemeris, PIONO_PARAM IonoParam, unsigned int Stream[10*4])
{
	double Value;
	signed int IntValue;
	unsigned int UintValue;

	// page 1
	Stream[0*4+0] = COMPOSE_BITS(1, 6, 4);
	Stream[0*4+0] |= COMPOSE_BITS(Ephemeris->health, 5, 1);
	Stream[0*4+0] |= COMPOSE_BITS(Ephemeris->iodc, 0, 5);
	Stream[0*4+1] = COMPOSE_BITS(Ephemeris->ura, 18, 4);
	Stream[0*4+1] |= COMPOSE_BITS(Ephemeris->week, 5, 13);
	IntValue = Ephemeris->toc >> 3;
	Stream[0*4+1] |= COMPOSE_BITS(IntValue >> 12, 0, 5);
	Stream[0*4+2] = COMPOSE_BITS(IntValue, 10, 12);
	IntValue = roundi(Ephemeris->tgd * 1e10);
	Stream[0*4+2] |= COMPOSE_BITS(IntValue, 0, 10);
	IntValue = roundi(Ephemeris->tgd2 * 1e10);
	Stream[0*4+3] = COMPOSE_BITS(IntValue, 12, 10);
	// page 2
	Stream[1*4+0] = COMPOSE_BITS(2, 6, 4);
	Value = UnscaleDouble(IonoParam->a0, -30);
	IntValue = roundi(Value);
	Stream[1*4+0] |= COMPOSE_BITS(IntValue >> 2, 0, 6);
	Stream[1*4+1] = COMPOSE_BITS(IntValue, 20, 2);
	Value = UnscaleDouble(IonoParam->a1, -27);
	IntValue = roundi(Value);
	Stream[1*4+1] |= COMPOSE_BITS(IntValue, 12, 8);
	Value = UnscaleDouble(IonoParam->a2, -24);
	IntValue = roundi(Value);
	Stream[1*4+1] |= COMPOSE_BITS(IntValue, 4, 8);
	Value = UnscaleDouble(IonoParam->a3, -24);
	IntValue = roundi(Value);
	Stream[1*4+1] |= COMPOSE_BITS(IntValue >> 4, 0, 4);
	Stream[1*4+2] = COMPOSE_BITS(IntValue, 18, 4);
	Value = UnscaleDouble(IonoParam->b0, 11);
	IntValue = roundi(Value);
	Stream[1*4+2] |= COMPOSE_BITS(IntValue, 10, 8);
	Value = UnscaleDouble(IonoParam->b1, 14);
	IntValue = roundi(Value);
	Stream[1*4+2] |= COMPOSE_BITS(IntValue, 2, 8);
	Value = UnscaleDouble(IonoParam->b2, 16);
	IntValue = roundi(Value);
	Stream[1*4+2] |= COMPOSE_BITS(IntValue >> 6, 0, 2);
	Stream[1*4+3] = COMPOSE_BITS(IntValue, 16, 6);
	Value = UnscaleDouble(IonoParam->b3, 16);
	IntValue = roundi(Value);
	Stream[1*4+3] |= COMPOSE_BITS(IntValue, 8, 6);
	// page 3
	Stream[2*4+0] = COMPOSE_BITS(3, 6, 4);
	Value = UnscaleDouble(Ephemeris->af0, -33);
	IntValue = roundi(Value);
	Stream[2*4+2] = COMPOSE_BITS(IntValue >> 12, 0, 12);
	Stream[2*4+3] = COMPOSE_BITS(IntValue, 10, 12);
	Value = UnscaleDouble(Ephemeris->af1, -50);
	IntValue = roundi(Value);
	Stream[2*4+3] |= COMPOSE_BITS(IntValue >> 18, 6, 4);
	// page 4
	Stream[3*4+0] = COMPOSE_BITS(4, 6, 4);
	Stream[3*4+0] |= COMPOSE_BITS(IntValue >> 12, 0, 6);
	Stream[3*4+1] = COMPOSE_BITS(IntValue, 10, 12);
	Value = UnscaleDouble(Ephemeris->af2, -66);
	IntValue = roundi(Value);
	Stream[3*4+1] |= COMPOSE_BITS(IntValue >> 1, 0, 11);
	Stream[3*4+2] = COMPOSE_BITS(IntValue, 21, 1);
	Stream[3*4+2] |= COMPOSE_BITS(Ephemeris->iode, 16, 5);
	Value = UnscaleDouble(Ephemeris->delta_n / PI, -43);
	IntValue = roundi(Value);
	Stream[3*4+2] |= COMPOSE_BITS(IntValue, 0, 16);
	Value = UnscaleDouble(Ephemeris->cuc, -31);
	IntValue = roundi(Value);
	Stream[3*4+3] = COMPOSE_BITS(IntValue >> 4, 8, 14);
	// page 5
	Stream[4*4+0] = COMPOSE_BITS(5, 6, 4);
	Stream[4*4+0] |= COMPOSE_BITS(IntValue, 2, 4);
	Value = UnscaleDouble(Ephemeris->M0 / PI, -31);
	IntValue = roundi(Value);
	Stream[4*4+0] |= COMPOSE_BITS(IntValue >> 30, 0, 2);
	Stream[4*4+1] = COMPOSE_BITS(IntValue >> 8, 0, 22);
	Stream[4*4+2] = COMPOSE_BITS(IntValue, 14, 8);
	Value = UnscaleDouble(Ephemeris->cus, -31);
	IntValue = roundi(Value);
	Stream[4*4+2] |= COMPOSE_BITS(IntValue >> 4, 0, 14);
	Stream[4*4+3] = COMPOSE_BITS(IntValue, 18, 4);
	Value = UnscaleDouble(Ephemeris->ecc, -33);
	UintValue = roundu(Value);
	Stream[4*4+3] |= COMPOSE_BITS(UintValue >> 22, 8, 10);
	// page 6
	Stream[5*4+0] = COMPOSE_BITS(6, 6, 4);
	Stream[5*4+0] |= COMPOSE_BITS(UintValue >> 16, 0, 6);
	Stream[5*4+1] = COMPOSE_BITS(UintValue, 6, 16);
	Value = UnscaleDouble(Ephemeris->sqrtA, -19);
	UintValue = roundu(Value);
	Stream[5*4+1] |= COMPOSE_BITS(UintValue >> 26, 0, 6);
	Stream[5*4+2] = COMPOSE_BITS(UintValue >> 4, 0, 22);
	Stream[5*4+3] = COMPOSE_BITS(UintValue, 18, 4);
	Value = UnscaleDouble(Ephemeris->cic, -31);
	IntValue = roundi(Value);
	Stream[5*4+3] |= COMPOSE_BITS(IntValue >> 8, 8, 10);
	// page 7
	Stream[6*4+0] = COMPOSE_BITS(7, 6, 4);
	Stream[6*4+0] |= COMPOSE_BITS(IntValue >> 2, 0, 6);
	Stream[6*4+1] = COMPOSE_BITS(IntValue, 20, 2);
	Value = UnscaleDouble(Ephemeris->cis, -31);
	IntValue = roundi(Value);
	Stream[6*4+1] |= COMPOSE_BITS(IntValue, 2, 18);
	Stream[6*4+1] |= COMPOSE_BITS(Ephemeris->toe >> 18, 0, 2);
	Stream[6*4+2] = COMPOSE_BITS(Ephemeris->toe >> 3, 7, 15);
	Value = UnscaleDouble(Ephemeris->i0 / PI, -31);
	IntValue = roundi(Value);
	Stream[6*4+2] |= COMPOSE_BITS(IntValue >> 25, 0, 7);
	Stream[6*4+3] = COMPOSE_BITS(IntValue >> 11, 8, 14);
	// page 8
	Stream[7*4+0] = COMPOSE_BITS(8, 6, 4);
	Stream[7*4+0] |= COMPOSE_BITS(IntValue >> 5, 0, 6);
	Stream[7*4+1] = COMPOSE_BITS(IntValue, 17, 5);
	Value = UnscaleDouble(Ephemeris->crc, -6);
	IntValue = roundi(Value);
	Stream[7*4+1] |= COMPOSE_BITS(IntValue >> 1, 0, 17);
	Stream[7*4+2] = COMPOSE_BITS(IntValue, 21, 1);
	Value = UnscaleDouble(Ephemeris->crs, -6);
	IntValue = roundi(Value);
	Stream[7*4+2] |= COMPOSE_BITS(IntValue, 3, 18);
	Value = UnscaleDouble(Ephemeris->omega_dot / PI, -43);
	IntValue = roundi(Value);
	Stream[7*4+2] |= COMPOSE_BITS(IntValue >> 21, 0, 3);
	Stream[7*4+3] = COMPOSE_BITS(IntValue >> 5, 6, 16);
	// page 9
	Stream[8*4+0] = COMPOSE_BITS(9, 6, 4);
	Stream[8*4+0] |= COMPOSE_BITS(IntValue, 1, 5);
	Value = UnscaleDouble(Ephemeris->omega0 / PI, -31);
	IntValue = roundi(Value);
	Stream[8*4+0] |= COMPOSE_BITS(IntValue >> 31, 0, 1);
	Stream[8*4+1] = COMPOSE_BITS(IntValue >> 9, 0, 22);
	Stream[8*4+2] = COMPOSE_BITS(IntValue, 13, 9);
	Value = UnscaleDouble(Ephemeris->w / PI, -31);
	IntValue = roundi(Value);
	Stream[8*4+2] |= COMPOSE_BITS(IntValue >> 19, 0, 13);
	Stream[8*4+3] = COMPOSE_BITS(IntValue >> 5, 8, 14);
	// page 10
	Stream[9*4+0] = COMPOSE_BITS(10, 6, 4);
	Stream[9*4+0] |= COMPOSE_BITS(IntValue, 1, 5);
	Value = UnscaleDouble(Ephemeris->idot / PI, -43);
	IntValue = roundi(Value);
	Stream[9*4+0] |= COMPOSE_BITS(IntValue >> 13, 0, 1);
	Stream[9*4+1] = COMPOSE_BITS(IntValue, 9, 13);

	return 0;
}

int D1D2NavBit::FillBdsAlmanacPage(PGPS_ALMANAC Almanac, unsigned int Stream[9])
{
	double Value;
	signed int IntValue;
	unsigned int UintValue;

	if (Almanac->valid == 0)
		return 0;
	Value = UnscaleDouble(Almanac->sqrtA, -11);
	UintValue = roundu(Value);
	Stream[0] |= COMPOSE_BITS(UintValue >> 22, 0, 2);
	Stream[1] = COMPOSE_BITS(UintValue, 0, 22);
	Value = UnscaleDouble(Almanac->af1, -38);
	IntValue = roundi(Value);
	Stream[2] = COMPOSE_BITS(IntValue, 11, 11);
	Value = UnscaleDouble(Almanac->af0, -20);
	IntValue = roundi(Value);
	Stream[2] |= COMPOSE_BITS(IntValue, 0, 11);
	Value = UnscaleDouble(Almanac->omega0 / PI, -23);
	IntValue = roundi(Value);
	Stream[3] = COMPOSE_BITS(IntValue >> 2, 0, 22);
	Stream[4] = COMPOSE_BITS(IntValue, 20, 2);
	Value = UnscaleDouble(Almanac->ecc, -21);
	UintValue = roundu(Value);
	Stream[4] |= COMPOSE_BITS(UintValue, 3, 17);
	if (Almanac->i0 > 0.5)
		Value = UnscaleDouble(Almanac->i0 / PI - 0.3, -19);
	else
		Value = UnscaleDouble(Almanac->i0 / PI, -19);
	IntValue = roundi(Value);
	Stream[4] |= COMPOSE_BITS(IntValue >> 13, 0, 3);
	Stream[5] = COMPOSE_BITS(IntValue, 9, 13);
	Stream[5] |= COMPOSE_BITS(Almanac->toa >> 12, 1, 8);
	Value = UnscaleDouble(Almanac->omega_dot / PI, -38);
	IntValue = roundi(Value);
	Stream[5] |= COMPOSE_BITS(IntValue >> 16, 0, 1);
	Stream[6] = COMPOSE_BITS(IntValue, 6, 16);
	Value = UnscaleDouble(Almanac->w / PI, -23);
	IntValue = roundi(Value);
	Stream[6] |= COMPOSE_BITS(IntValue >> 18, 0, 6);
	Stream[7] = COMPOSE_BITS(IntValue, 4, 18);
	Value = UnscaleDouble(Almanac->M0 / PI, -23);
	IntValue = roundi(Value);
	Stream[7] |= COMPOSE_BITS(IntValue >> 20, 0, 4);
	Stream[8] = COMPOSE_BITS(IntValue, 2, 20);

	return 0;
}

int D1D2NavBit::FillBdsHealthPage(PGPS_ALMANAC Almanac, int Length, unsigned int Stream[9])
{
	int i, index0, index1;
	int health;

	for (i = 0; i < Length; i ++)
	{
		health = (Almanac[i].valid == 1) ? 0 : (0x1ff + Almanac[i].health);
		index0 = i * 9 + 20;	// first bit position
		index1 = index0 + 8;	// last bit position
		if ((index0 / 22) == (index1 / 22))	// in same WORD
			Stream[index0/22] |= COMPOSE_BITS(health, (21 - (index1 % 22)), 9);
		else
		{
			Stream[index1/22] |= COMPOSE_BITS(health, (21 - (index1 % 22)), ((index1 % 22) + 1));	// fill in LSB
			health >>= ((index1 % 22) + 1);
			Stream[index0/22] |= COMPOSE_BITS(health, 0, (8 - (index0 % 22)));	// fill in MSB
		}
	}

	return i;
}

// calculate BCH and XOR to lowest 4bit
// to check BCH, after calling this function, the lowest 4bit of return value should be 0
// to calculate BCH, set lowest 4bit as 0 and return value contains the full word
unsigned int D1D2NavBit::GetBCH(unsigned int word)
{
	unsigned int xor_value;
	int i;

	// calculate parity value
	for (i = 0; i < 4; i++)
	{
		xor_value = word & BCHPoly[i];
		// calculate bit 1 number
		xor_value = (xor_value & 0x5555) + ((xor_value & 0xaaaa) >> 1);
		xor_value = (xor_value & 0x3333) + ((xor_value & 0xcccc) >> 2);
		xor_value = (xor_value & 0x0f0f) + ((xor_value & 0xf0f0) >> 4);
		xor_value = (xor_value & 0x000f) + ((xor_value & 0x0f00) >> 8);
		if (xor_value & 1)
			word ^= (1 << (3 - i));
	}
	return word;
}

unsigned int D1D2NavBit::Interleave(unsigned int data)
{
	data = (data & 0xff0000ff) | ((data & 0x0000ff00) << 8) | ((data & 0x00ff0000) >> 8);
	data = (data & 0xf00ff00f) | ((data & 0x00f000f0) << 4) | ((data & 0x0f000f00) >> 4);
	data = (data & 0xc3c3c3c3) | ((data & 0x0c0c0c0c) << 2) | ((data & 0x30303030) >> 2);
	data = (data & 0x19999999) | ((data & 0x02222222) << 1) | ((data & 0x44444444) >> 1);

	return data;
}
