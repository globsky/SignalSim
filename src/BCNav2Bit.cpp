//----------------------------------------------------------------------
// BCNav2Bit.h:
//   Implementation of navigation bit synthesis class for B-CNAV2
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "BCNav2Bit.h"

const char BCNav2Bit::B2aMatrixGen[B2a_SYMBOL_LENGTH*B2a_SYMBOL_LENGTH+1] = {
"h[0G@Y0<JiVK0c0^KI40hKN0DNVh]i0JKN<F[0Jo0C0UFYo9"
"K`C0QKa0ggDVR0T0S70^VV0EW1>i^R20[;aG09YT>0c0GbQN"
"8KZ0>810<<jIZ0Q0nH0<110V5L`RlZW0C1]N0i[Q^0=0\\SIW"
"kg0_mX0EMEHc0e0WR9`0CRS03SHCFE0FcS1Yg0Md0<0XL`dN"
"O`T0VOC0aaDVQ0R0h]0g;;0EW1Ii`gN0[ZCG0hMEo0=0HbV\\"
"Z60`Yj0>PCMQ0K0jn2j0=ng0KgM=1C0Png9^j0PD0F0dijD["
"U8H0^UL0FF6`?0>0@N0F``0I_OlZ=?i0]LGW0RC>l0Y0Wf`5"
"RW0d1N0OIO:T070[TMD0RTX0`X:R_O0OTX@9W0IG050>9NGY"
"O`05Ud0A<A]40M0Ii@I0[ib0Mb][8A0<ib;oI0:e0g0Z1IeG"
"kKZ0>810<7jI>0Z0nH0<110V5L`Rl<W0C?]N0n[Q^0:0\\SIW"
"m:\\0lmN022VK40L09W0bKK02d8=1:be0GON50Z]^a0[05Jl_"
"jm0Q5U0CnPR[0>0UbZU0\\bG0>9c\\EP0nb9C]m0n80R0fJU8_"
"kKQ0>81077jI>0Z0nH0<110V5L`Rl<W0C?]N0n[VY0:0\\SIW"
"8l0YMi0T?hG50[0^6D^0a6O0[OGa:T0?6OIP^0?90J01`^92"
":cI0No^01?T\\I0G0J201^^0H@l4V51f0<^FS0JgGW0_0h8\\f"
"`>0h=@0MRMaN0S0FDkQ0PD10o1aPlM0RN1E5Q0cf0i0@VQf<"
"en10beN0>>a2H0?0:`0>NN0FUWS]fH[0;N1K0C6?S0@0Oc28"
"RW0d1N0OIO:T070[TFD0lhX0`X:R_O0OTX@>W0IG050N9DGY"
">]0WiC02jZ`U050=UKC0eUf05V`e\\Z0jU52h]0j[0`0ATC[R"
"o5F0No?011T\\20G0Ub0I\\\\01J4WV5Ik0<^?S0EgGi0d0S=Nf"
"Rj0IoD0Q_[<h0`0bhFO0lh`0``<l;[0_h`Q>j0_J060e9DJY"
"nS;0?nV0QQBF10Z0cI0SFF0<82bCS]l06GV`0[jZb0J0`W?K"
"NFi0RNj0AA_gR0[0^603aa0X`7<Y?3I0DCjV0^@Xk0S0;Hg1"
"8l0YFi0_?hG50[0^6D^0a6O0[OGa:T0?6OIP^0?>0J01`^>2"
"h260Zh70TTP<C0R0l;0T770gKG?[bC`0j76I0YDR?0S01\\<`"
"TO0mHL04G4eg0;0iKIY0hKA0NAeTJ40JgA:FO0G10k0LMY19"
"TO0mHL04G4eg0;0igdY0TKA0NAeTJ404gA:MO0G10k0FMY19"
"K`C0QK;0ggDVR0T0S70g;V0EW1>i^R@0[;aG09YT>0c0HbVN"
"Jk>0`JH0GGRLF0I0e\\0GLL01oSK<8F50QbH40gTIK0M04nLc"
"]803NK0Y259Z0I06ZiB0]ZT0hT9]m502ZTY[802`0U0b[K`j"
"gi01:90Vo37W0f02WGS0KWL0LL7KZ30oWLVIi0o@0R0_I9@M"
"2?j0R2A033_g[0i0\\E0?gg0X`<ZY?[>0DaAV0oJiZ0O0VHRI"
"3YW0c3h0SSI=50K0jn0O==0Si:o`YO90N8he01G4<0a0emcd"
"Z60`Yj0>PCFn0K0jn2j0=n40KgM=1C0PnK>^60PD0F0dijD["
"Pe0RKn0jlU[D0Z0nHCn01Ho0ZoO1XU0lHoj6n0lh0[045nh8"
"8l]0>870<<AIH0;0fG0ZII0Qn`^6lZh0a17N0f[QP0o0NO>W"
"Fo0^Sc0@[@AI0\\0Q_WR0U_70k7AU[@0[I7XKR0O40Y0cgR4T"
"kO<0Ik?07]i1<0V05>07??0;cbLgK<40T?Q\\053V`080^h14"
"5h09d30g>g1k0i02CY206C40i416eg0>C4GD20>M0Q0HN2M`"
"FZD0j?X0YMnij0P0HA0YXX09>gR_;YQ0UBmT0H8960L0E7iQ"
"kK0MmX0oFWHc030`R9`0CRS03SHCUE0FRS1Y`0Fd0@0?L`IN"
"LI30T`[0BBJET0i02C0X660AN;V9>XG0YR[702oAc0l0]?EH"
"fO<0Ik?07]i1I0V05>07??0;cbLgK740TFQ\\053V`080^h14"
"NF0Kn:0dTd6L0@0IYc<0DYH0@H6Dhd0TYHak<0f=030:7<=V"
"=l70H=I0ZZA>;0]0kF0ZII0Qn`\\64;D0aI<20jB]\\0c0NO>h"
"g40P?\\0S1SUE0]03Em90KWB0LBUgoS0SEBed401H0c0\\d9HM"
"T[0G@Y0<JiVK0N0^KI40hKb0NNVh]i0JKN<F[0Jo0V0UMYo9"
"@kI0L@>0HHgb`010_^0kbb0H:SO7kGc0V2>l0aE130m0lnL="
};

// assume message broadcase cycle is 60s (20 frames) with following MesType order
const int BCNav2Bit::MessageOrder[20] = {
	10, 11, 30, 34, 10, 11, 30, 34, 10, 11, 30, 34, 10, 11, 30, 34, 10, 11, 30, 34,
};

BCNav2Bit::BCNav2Bit()
{
}

BCNav2Bit::~BCNav2Bit()
{
}

int BCNav2Bit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i;
	int sow, MessageType;
	unsigned int FrameData[12];
	int Symbols[96];

	// data channel
	if (svid < 1 || svid > 63)
		return 1;

	sow = StartTime.MilliSeconds / 3000;	// SOW with scale factor 3s
	MessageType = MessageOrder[sow % 20];
	ComposeMessage(MessageType, StartTime.Week, sow, svid, FrameData);
	AppendCRC(FrameData, 12);
	// assign each 6bit into Symbols array
	for (i = 0; i < 12; i ++)
	{
		Symbols[i*4+0] = (FrameData[i] >> 18) & 0x3f;
		Symbols[i*4+1] = (FrameData[i] >> 12) & 0x3f;
		Symbols[i*4+2] = (FrameData[i] >> 6) & 0x3f;
		Symbols[i*4+3] = FrameData[i] & 0x3f;
	}
	LDPCEncode(Symbols, B2a_SYMBOL_LENGTH, B2aMatrixGen);		// do LDPC encode
	AssignBits(0xe24de8, 24, NavBits);	// Preamble
	for (i = 0; i < 96; i ++)
		AssignBits(Symbols[i], 6, NavBits + 24 + i * 6);	// 96 encoded symbols
	return 0;
}

// place data into Ephemeris1/Ephemeris2/ClockParam/IntegrityFlags
// data place with MSB first and least index first order
// to compatible with single integer variables, bits in array less than multiple of 32bit
// will fill zeros at first (MSBs in index 0)
int BCNav2Bit::SetEphemeris(int svid, PGPS_EPHEMERIS Eph)
{
	unsigned int *Data;
	signed int IntValue;
	unsigned int UintValue;
	long long int LongValue;
	unsigned long long int ULongValue;

	if (svid < 1 || svid > 63 || !Eph || !Eph->valid)
		return 0;

	// fill in Ephemeris1 (with 13 zeros)
	Data = Ephemeris1[svid-1];
	Data[0] = COMPOSE_BITS(Eph->iode, 11, 8);
	UintValue = Eph->toe / 300;	// toe
	Data[0] |= COMPOSE_BITS(UintValue, 0, 11);
	UintValue = Eph->flag;	// SatType
	Data[1] = COMPOSE_BITS(UintValue, 30, 2);
	IntValue = UnscaleInt(Eph->axis - ((UintValue == 3) ? 27906100.0 : 42162200.0), -9);	// deltaA
	Data[1] |= COMPOSE_BITS(IntValue, 4, 26);
	IntValue = UnscaleInt(Eph->axis_dot, -21);	// Adot
	Data[1] |= COMPOSE_BITS(IntValue >> 21, 0, 4);
	Data[2] = COMPOSE_BITS(IntValue, 11, 21);
	IntValue = UnscaleInt(Eph->delta_n / PI, -44);	// delta_n
	Data[2] |= COMPOSE_BITS(IntValue >> 6, 0, 11);
	Data[3] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(Eph->delta_n_dot, -57);	// delta n dot
	Data[3] |= COMPOSE_BITS(IntValue, 3, 23);
	LongValue = UnscaleLong(Eph->M0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[3] |= COMPOSE_BITS(IntValue, 2, 1);
	Data[3] |= COMPOSE_BITS(UintValue >> 30, 0, 2);
	Data[4] = COMPOSE_BITS(UintValue, 2, 30);
	ULongValue = UnscaleULong(Eph->ecc, -34);
	IntValue = (ULongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)ULongValue;
	Data[4] |= COMPOSE_BITS(IntValue, 1, 1);
	Data[4] |= COMPOSE_BITS(UintValue >> 31, 0, 1);
	Data[5] = COMPOSE_BITS(UintValue, 1, 31);
	LongValue = UnscaleLong(Eph->w / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[5] |= COMPOSE_BITS(IntValue, 0, 1);
	Data[6] = UintValue;

	// fill in Ephemeris2 (with 2 zeros)
	Data = Ephemeris2[svid-1];
	LongValue = UnscaleLong(Eph->omega0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[0] = COMPOSE_BITS(IntValue, 29, 1);
	Data[0] |= COMPOSE_BITS(UintValue >> 3, 0, 29);
	Data[1] = COMPOSE_BITS(UintValue, 29, 3);
	LongValue = UnscaleLong(Eph->i0 / PI, -32);
	IntValue = (LongValue & 0x100000000LL) ? 1 : 0;
	UintValue = (unsigned int)LongValue;
	Data[1] |= COMPOSE_BITS(IntValue, 28, 1);
	Data[1] |= COMPOSE_BITS(UintValue >> 4, 0, 28);
	Data[2] = COMPOSE_BITS(UintValue, 28, 4);
	IntValue = UnscaleInt(Eph->omega_dot / PI, -44);	// omega dot
	Data[2] |= COMPOSE_BITS(IntValue, 9, 19);
	IntValue = UnscaleInt(Eph->idot / PI, -44);	// i dot
	Data[2] |= COMPOSE_BITS(IntValue >> 6, 0, 9);
	Data[3] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(Eph->cis, -30);	// cis
	Data[3] |= COMPOSE_BITS(IntValue, 10, 16);
	IntValue = UnscaleInt(Eph->cic, -30);	// cic
	Data[3] |= COMPOSE_BITS(IntValue >> 6, 0, 10);
	Data[4] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(Eph->crs, -8);	// crs
	Data[4] |= COMPOSE_BITS(IntValue, 2, 24);
	IntValue = UnscaleInt(Eph->crc, -8);	// crc
	Data[4] |= COMPOSE_BITS(IntValue >> 22, 0, 2);
	Data[5] = COMPOSE_BITS(IntValue, 10, 22);
	IntValue = UnscaleInt(Eph->cus, -30);	// cus
	Data[5] |= COMPOSE_BITS(IntValue >> 11, 0, 10);
	Data[6] = COMPOSE_BITS(IntValue, 21, 11);
	IntValue = UnscaleInt(Eph->cuc, -30);	// cuc
	Data[6] |= COMPOSE_BITS(IntValue, 0, 21);

	// fill in ClockParam (with 17 zeros)
	Data = ClockParam[svid-1];
	UintValue = Eph->toc / 300;	// toc
	Data[0] |= COMPOSE_BITS(UintValue, 4, 11);
	IntValue = UnscaleInt(Eph->af0, -34);	// af0
	Data[0] |= COMPOSE_BITS(IntValue >> 21, 0, 4);
	Data[1] = COMPOSE_BITS(IntValue, 11, 20);
	IntValue = UnscaleInt(Eph->af1, -50);	// af1
	Data[1] |= COMPOSE_BITS(IntValue >> 11, 0, 11);
	Data[2] = COMPOSE_BITS(IntValue, 21, 11);
	IntValue = UnscaleInt(Eph->af2, -66);	// af2
	Data[2] |= COMPOSE_BITS(IntValue, 10, 11);
	Data[2] |= COMPOSE_BITS(Eph->iodc, 0, 10);

	// fill in IntegrityFlags
	UintValue = COMPOSE_BITS(Eph->flag >> 5, 7, 3);
	UintValue |= COMPOSE_BITS(Eph->flag >> 2, 0, 3);
	UintValue |= COMPOSE_BITS(Eph->flag >> 11, 3, 4);
	IntegrityFlags[svid-1] = UintValue;

	return svid;
}

// 288 bits subframe information divided int 12 WORDs
// each WORD has 24bits in 24LSB of unsigned int data in FrameData[]
// bit order is MSB first (from bit23) and least index first
// each 24bits divided into four 6bit symbols in LDPC encode
void BCNav2Bit::ComposeMessage(int MessageType, int week, int sow, int svid, unsigned int FrameData[])
{
	unsigned int *Data;

	// first fill in PRN/MesType/SOW
	FrameData[0] = COMPOSE_BITS(svid, 18, 6);
	FrameData[0] |= COMPOSE_BITS(MessageType, 12, 6);
	FrameData[0] |= COMPOSE_BITS(sow >> 6, 0, 12);
	FrameData[1] = COMPOSE_BITS(sow, 18, 6);
	switch (MessageType)
	{
	case 10:
		FrameData[1] |= COMPOSE_BITS(week, 5, 13);
		FrameData[1] |= COMPOSE_BITS(IntegrityFlags[svid-1] >> 5, 0, 5);
		FrameData[2] = COMPOSE_BITS(IntegrityFlags[svid-1], 19, 5);
		Data = Ephemeris1[svid-1];
		FrameData[2] |= COMPOSE_BITS(Data[0], 0, 19);
		FrameData[3] = COMPOSE_BITS(Data[1] >> 8, 0, 24);
		FrameData[4] = COMPOSE_BITS(Data[1], 16, 8);
		FrameData[4] |= COMPOSE_BITS(Data[2] >> 16, 0, 16);
		FrameData[5] = COMPOSE_BITS(Data[2], 8, 16);
		FrameData[5] |= COMPOSE_BITS(Data[3] >> 24, 0, 8);
		FrameData[6] = COMPOSE_BITS(Data[3], 0, 24);
		FrameData[7] = COMPOSE_BITS(Data[4] >> 8, 0, 24);
		FrameData[8] = COMPOSE_BITS(Data[4], 16, 8);
		FrameData[8] |= COMPOSE_BITS(Data[5] >> 16, 0, 16);
		FrameData[9] = COMPOSE_BITS(Data[5], 8, 16);
		FrameData[9] |= COMPOSE_BITS(Data[6] >> 24, 0, 8);
		FrameData[10] = COMPOSE_BITS(Data[6], 0, 24);
		break;
	case 11:
		// HS left with 2 zeros
		FrameData[1] |= COMPOSE_BITS(IntegrityFlags[svid-1], 6, 10);
		Data = Ephemeris2[svid-1];
		FrameData[2] = COMPOSE_BITS(Data[0] >> 24, 0, 6);
		FrameData[2] |= COMPOSE_BITS(Data[0], 0, 24);
		FrameData[3] = COMPOSE_BITS(Data[1] >> 8, 0, 24);
		FrameData[4] = COMPOSE_BITS(Data[1], 16, 8);
		FrameData[4] |= COMPOSE_BITS(Data[2] >> 16, 0, 16);
		FrameData[5] = COMPOSE_BITS(Data[2], 8, 16);
		FrameData[5] |= COMPOSE_BITS(Data[3] >> 24, 0, 8);
		FrameData[6] = COMPOSE_BITS(Data[3], 0, 24);
		FrameData[7] = COMPOSE_BITS(Data[4] >> 8, 0, 24);
		FrameData[8] = COMPOSE_BITS(Data[4], 16, 8);
		FrameData[8] |= COMPOSE_BITS(Data[5] >> 16, 0, 16);
		FrameData[9] = COMPOSE_BITS(Data[5], 8, 16);
		FrameData[9] |= COMPOSE_BITS(Data[6] >> 24, 0, 8);
		FrameData[10] = COMPOSE_BITS(Data[6], 0, 24);
		break;
	case 30:
		// HS left with 2 zeros
		FrameData[1] |= COMPOSE_BITS(IntegrityFlags[svid-1], 6, 10);
		Data = ClockParam[svid-1];
		FrameData[1] |= COMPOSE_BITS(Data[0] >> 9, 0, 6);
		FrameData[2] = COMPOSE_BITS(Data[0], 15, 9);
		FrameData[2] |= COMPOSE_BITS(Data[1] >> 17, 0, 15);
		FrameData[3] = COMPOSE_BITS(Data[1], 7, 17);
		FrameData[3] |= COMPOSE_BITS(Data[2] >> 25, 0, 7);
		FrameData[4] = COMPOSE_BITS(Data[2] >> 1, 0, 24);
		FrameData[5] = COMPOSE_BITS(Data[2], 23, 1);
		FrameData[6] = FrameData[7] = FrameData[8] = FrameData[9] = FrameData[10] = 0;	// fill rest of 143bits with 0
		break;
	case 34:
		// HS left with 2 zeros
		FrameData[1] |= COMPOSE_BITS(IntegrityFlags[svid-1], 6, 10);
		// SISAI left with 22 zeros
		Data = ClockParam[svid-1];
		FrameData[2] = COMPOSE_BITS(Data[0] >> 7, 0, 8);
		FrameData[3] = COMPOSE_BITS(Data[0], 17, 7);
		FrameData[3] |= COMPOSE_BITS(Data[1] >> 15, 0, 17);
		FrameData[4] = COMPOSE_BITS(Data[1], 9, 15);
		FrameData[4] |= COMPOSE_BITS(Data[2] >> 23, 0, 9);
		FrameData[5] = COMPOSE_BITS(Data[2], 1, 23);
		FrameData[6] = FrameData[7] = FrameData[8] = FrameData[9] = FrameData[10] = 0;	// fill rest of 121bits with 0
		break;
	}
}
