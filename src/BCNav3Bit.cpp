//----------------------------------------------------------------------
// BCNav3Bit.h:
//   Implementation of navigation bit synthesis class for B-CNAV3
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <memory.h>
#include "ConstVal.h"
#include "BCNav3Bit.h"

const char BCNav3Bit::B2bMatrixGen[B2b_SYMBOL_LENGTH*B2b_SYMBOL_LENGTH+1] = {
"nXGb3[3O=iTXeNSoUElQT5ORV7NkfXXf4cY?je6GhghdCLgg`UViO>0DegkheCFEYO68^:@A\\kimGdkZZ"
"ITMHn67]^=W_G6lbP_idWU]H7e6H>E8d?lS@@`eje2VA@`Uh9NL=LQhD`M9V`ZAc4]oGh2M5RTkgj:JXM"
">N:P]^LE_79\\]JPUZSjD9TfnS=_g5\\9;AE4E246o644HKd>D95:f:JDF87g>oKoXIWhfDJ7b^NN?DaT58"
"XF5@>HmD3?VFg_ZCG`Ml^2DbKW_jieFl[[]\\1g^5^2dC\\J2QUEK?a=0Wg24dg3n`]DLA:T1IojGV5bj=S"
"adnR`Yjl<21P5YFQkQ:81W6RjKYSgP18^l>l97KoKBB_;?WI=;]2]VIcTn=o73Hb?iS6IVnKYd\\Joh4UT"
"FLeDZ2\\aCLMbZdDmQl[YMg=OlnJE6bb6BaGa`ZCeCSF?WmSYMkeLeo0I5SEF<W@AGOK=Yh;^2EL>e6Ek5"
"1Em>DR]e`846DRK2Y2XW4@H>]_R:I64Pne;eTL_i_;;j\\liWd\\b8bNW9FmdiL<L=lQ:HWNm_REE[WU@BF"
":PQGcmDKOP?Yc>GFfiL1jl2BiT7hWYYW`KU7dcOQO[:oAF[[?aQPQ;0eE[h:cA]2UBXR1359IhP@QohaE"
"[H=X1I9djGZHC:7E>b8OL\\dXSl:XBoHOA7V`FCL=L\\YE`C\\;@RSGTk0lC\\KYCmcbVd2iJ6F?U_>J=NXkW"
"3o4]lU@LC4<:lU]6h6?Z<`XBdbCN[:<S1LML_Mb8bMM=g78Z<gEXERZKj4O8TDTG7Y3XZR4eUoonZ\\`[j"
"H2od`^`iE2KN7Yd9;OA8KCihkceRgNNg3T>eb7SoEWHGl9WWD;o2iU0?=WRH7l_O>hSHmV<LMR21oGRnn"
"7WYZK[6RFhLkG[2>4kMUL5RZJ8[Z<BWUQ2^==G8Y85a>=G5`j1HhHg0oG5:aGTPkbRf;jIeO3X4AYncme"
"UYVH=m9O]cFM=ZHGnXb?FKgaXE]WCMFfLOJOP3SQS33:jGU?FCVYVZ?_63WUQjQNJ[Bg?ZcdmYYe?4KC6"
"_a\\hETEfUgVaP4nMC;8IVof<1H4@eaae=:3QRPZ\\575BQS77OD1gY`0iP7>5PQN;3fZJKmd6l@gj\\B@FF"
"7XeZK[6RFhLk;[2>J>MUL5RZ68[Z<BWUQ=^==G8Y8IaP=G5`:1HhHI`oGe:YGTPSbRf;`IeO3X4AYncme"
"7?5@gG>[3?UFg_@JE`M:UBKb`WfjKFFTY[][1235327<\\J2:US5?5=:;42j7a\\nD]bLK:=RIGj?V5iBS4"
"J6bKagc=_E[6DSAdR]5G7;=VH?S:UC6GneX<TDQb7;kd<l;B4YHE9N03D;FkD_L]X=QoWPmCh:R[bV:II"
"Ul>ogO5ZMTBKIO39G9bFBV_o5<O=:KEFRZiZZm<L<88`P[VX?PDTDdX1m>?LmfjC[c]_Xd>aOl6NLQ;2>"
"9Zhk]7]UY;HZinJB<>o\\HdU1^2nmDQZ\\_PaGViIhH?@RG8??=3^;X40Ei?N@iGS>aUIMc[jQ5m<ThRmLL"
"h?ROMG`Lf9UAKG@cEc]TUXLO`3GbSFdTY[o[[n37322<[nX:C8595=:anRC7n\\<DJLjK:=RmG?P>7iBQR"
"YXH2_[A=FeLB_I2bJ6K`Lc;Z6]Ff;BBUl=M=ia8H8aaPTbY`L<HXHI`9QafYGTGSMgR;`Iej[fXD`nc<Q"
"`4m[SOSaZ4Xl>A[BFnR@fVacnUY7]ll]6;LYW>5mZ=`^kB==XFm4m90NJ=7`>kMnLc5`i8Hhj742m^7oo"
"Hc3FkTZ<N5Sc\\TW`=@ADSo<F9MTFGCcDIWKUU\\J3Jo]`U\\oORb^5^j0M\\om]\\V[@h<e?RLnfEa=63_:jP"
"EM=>8DdSQ^Gm8D>Z5Z2FGOaCBhD4TmGnbSASYAhJhAAU7HJFG7;^;<FkR=fJViV\\H3EaF<=hDMM_FlOTR"
"YXH2_[A=8eLB_\\2bJ6K`Lc;Z6]Ff;BBUl=M=ia8H8aYPTba`L<HXHI`9QafYGTGSMgR;`Iej[fXD`nc<Q"
"c[>NjAShkB6[o^PU3RO;6=haZQ^n5i[;K8mEXog>6C4UE2CDb@ZB:10doCV4oEHRmhgfLJ_i`n39>Yn77"
"k`V=Bb4gUAYC;bme]eF7Y6l=4abK8Lj7XgdggJa?aOO1oM6P<o@A@_PZJV<?Jh13MlTcP_V[b`iN?ERGV"
"Ggd?PCE:2nOaPCh^D^8KOo1?;UC@FaOm=:7:RNU[U77BLS[KOL\\n\\`KY>dM[NQNfSZG1K`dUCggjKeoA>"
"PiGbe[3O=i`XeNbLUElQT5ORE7?kfXXf4cY?je=G=gPdCLgg`UGiG>0D]gkPeCFOYR6P^a@A\\kimGdkZ]"
"ZSdV4WTE1O^S?XLGh=Po^fEV:eXV;iSo7LN5K?cdcfRG5?f\\3>FOFY0e?f_R?a9=NE8Q3HKlBjh[dkVYJ"
"X@Sjh_m\\R?Veh_ZCKCclVNAjm^_LieVlH\\]\\Jg^5^ddnE;5Q4Ea?aTQW[S45gYg1;DLAQTS^_@G85bN=["
"7?5@gH>[3?UFg_@JE`M:UBDb`WfjiFFiY[][1g35327<\\J22US5?5=0;42j7g\\nD]bLK:NRIGj?V5ijS4"
"T^UabLXBR\\F^QmH;`hifSlBan8maCM^f661ONQSUSl3;OQl?d<n\\V@08Qlk3Qj:h1BWg9ZN2Da`FU4a@c"
"7WeZK[6RFhLk;[2>4kMUL5RZ68[Z<BWUQ2^==G8Y8Ia>=G5`:1HhHg`oGe:aGTPkbRf;`IeO3X4AYncme"
"nJO;SeoN6OQ@SC;alaH]Q23AMX6`j@Q4GN9NU9\\R\\99fTVn]Qjg3gC]8BO`nRPRIVDm3]CO:eJJ=]L2jB"
"e9<1YPY`4D[9fFLI=3\\C[h`jTgFKO99ONW_;mfA<bie@BIii>=TD`]0UfiKbfBZ3_jAlHckMGKD:<@K66"
":4H[>KS;YHXl>9[DFDRiXVe_nUY7olX?6;=;W=Z`Z==^dB`iXdmem9iNJH7`MkMaBc:ei9HhK442i]VoJ"
"H2od`^`iE2KN7YX9;OA8KCihkcYRgNNg3T>eb7SoEWHGl9WWD;o2iU0?7WRB7l_O>hSHmV<LMR21oGRnn"
"7WYZK[6RFhLk;[2>4kMUL5RZ68[Z<BWUQ2^==G8Y85a>=G5`j1HhHg`oGe:aGTPkbRf;jIeO3X4AYncme"
"@R2OCa=8o63R9hjPgQcH]Z8;>FhUJTRH__B7E9V2]ZfP7KZ[lM>6d\\0A9ZGf9obQB8V:4YDTWUg32;U\\1"
"nbZk3NKC@im2<N9oVoHQmg8kKTNSf2mQ[CYCCeTGThhFUDgM]U_i_:M7eZ]Ge4PjDO68M:ZANbBIGRa>c"
"fURVO@_2NU]JOZVQ4YG7]b[mYiE^[JJ`F2n2:DNRNDfkdQD7]ARURa7=XD^fSd6>nmM[7aKo@^UcR`^AX"
"UMVH9D9NSYiM=I\\knXbRiKN[gEIW4MM4LOJ]P=BV^3U:jG33FngYNZ0_=3W^=jQXJNBh?fcd1WYeV:WCC"
"E`=C2DB[Q^GWaD>Z5W@nGK[CBhDCTm`nR>?SSVhJh<AZSVKFf7;^;3FkV=fAViU\\H[4aF<=N9MLdJlOe="
"2O1j?h47D13S?YjP>=C[3<:U=eDVJS3Ha7f7Kf]9]ffbMA2[3Jd:dY[`_1V2959ENk8:[Y1lhOO^[;<J_"
"i?nAHGMm[1Q?Re;aF^Y4\\bmLlheAj@?4BB7NIR\\n\\b9aNRb]:Tl1gc0hRbW9R[f^7m`3oCI>_AFQnLAcO"
"A:l[KJPbT>7UKJQRWRfh7`B[P2J\\3U7h?bZboE2k2mm8a]k<Sa6>6g<OilSkE9;H]Y\\B<gl2J:15k^Lni"
"I\\UYQLb6R\\d^QmYP<hi9dEn4h8oaC^^Cj616NQRURlI>OPlldcU\\U@0GklaIVO:B14Wn9K]2`a\\FUCack"
"HaPFYT6UN5SCYTW`9`AOS:?FZJTeGCSD8U]Ug\\J3J]][bh3OSb^5^LOMIPm3\\V\\nh<e?OLPJTaaiO_:jI"
"AC`4M6R5@ChTMK4Wd<fSZU5]<I\\_oTTokJj\\aM@`@QA3;WQQhd`C`b0B1Q_AM;^5j]7FSVYgO_CX`3_H1"
"8A1cXiW4:3PAULM@ja?<gE4c7;Lc=`A<fM9R[Ug1gEK@RUEdN_J3JF0;UEZKUSIa94C52e[Bb^j21TcFQ"
"X@SZh_:\\R?Veh_ZCKC>QVNAjm^RLieVlH\\d\\Jd^5^ddnE;5QVEa?aTQW[S45gYg1;9XAQTSU_@@8QbNi["
"LlD3hShC<lBKIj3[G5gen;C=_JM]QKKQRZbM4IcD<8L`f[88BGDlDd0\\@8]LIfm5b=cLXF>Wi]lND`]::"
";O9QWiCTIK\\O1i`FScd_\\kTQo5iQVjO_<`2nn1595k6Fn1kN[H?K?P0@1ke61gDc^T=][GU4XASB98fPU"
";O9QWiCTIK\\c]i`FScd_\\kTQo5iQVjO_<`2nn1595k6Fn1kN[H?K?EN@1ke61gDc^T=][GU4XASB98fPU"
"9kL6bncGjLTfb[6B^B]CTFMm:HjIDfT\\7G@G8@HhH@@S3EhCT3XMX[CKPLNhi_iVEJ9MC[L=nkklC1FDP"
"?`@mN2N3a`WL;9AM]4B5WR3KcfUTELLEXgFU^;l@aO?1hMOOY]@`3_0>;OT?;hJ4FKl?P7V\\dT`H@1T88"
"_97?nPM4ZfIdnQ?]8A^>IWE6AaZORdI=\\4o4DXl<lXXe[1_>IR797Q>JCfO_<[<LoijE>QfmP99U>SWRC"
"8A1^XiX4n3gAULl@_ah<gk4T7;Lc=AA=Sf9R[UC1KEKmR\\EEN_73JF0YUEcKURIa94C52e:Bbc3P1mcQQ"
"EM=C2DB[Q^GWaD>Z5W@nGK[CBhDCTm`nRS?SSVhJh<AUSVKFf7;^;<FkV=fAViU\\H[4aF<=N9MLdJlOe="
"FLeDZ2\\aC;MbZdDmQl[YMg=OlnJE=bb]BaGa`SCeCSF?WmSYMkeLeoYI5SEF@W@AGOK=Yo;^2EL>e6gk5"
"ODZIPd\\^WmBDk?Un9[NaBj^ACR?lKYDa2heXMk3ZBjLnX>j_HcCmf70:kjgLkX;[e^34G58YFl9oZAlEE"
"_hF@E4kQdgj;G4XM1M^Ij7J@kV4neWaITQ3QQPV\\V55NDi7A>DYgYmAHPF>\\P=NRifZJAmF64hCL\\<]`F"
"MQKcZ;Z_9]?Q38CjVF@b?m_H2`8PY<QbJD6173GK?>:j15>>nX2]AS0a3>L:31lF6_Gdfi[<kPVgKTP\\\\"
"FR91=aV;bfKUdaS\\m\\[MKe;1V:aZ?gnMco4oo2:B:<<X`2elY`NfN^lP29YB2]XkO;JIl^98aR5TB@_39"
"JM;>8DdSQ=Gm81>H5B2FGOaCB6Q4ammnbS@SYAh;hAJUiHJFGT;M;<FoRA4JViV\\@3[aF<=PD4M_FlOTR"
"l53a\\8k<15Rc\\Tagb@;DJ><_@MBFGccGVIKBn\\e31ol4UgooRb353j0hmoFl\\U[<K_el6:NfEF5S34FPP"
"C\\:gL`iX7N?\\]MGV^SUk?4Xgf=MgaY\\kEGIKK]O:O4[VK]419ZQNQJ0=]48[]AoSIXhR9;2bmP^D:WlJ5"
"4GahC>1A5P@HC>_8R8DJ@3Shbj>XWH@6mA\\AF\\jQj\\\\]feQJ@f=P=kJTKaEQc`cde24SJkaj>GGZJB3WK"
";O9QWiCTIK\\c]i`FScd_\\kTQC5iQVjO_<`2nn1595G6Fn1kN[H?K?EN@1Ue61gDc^T=]NGU4XASB98fPU"
"Y5Na\\=kIBNRc\\jaAbA;6R>9d@MBFPcRLVIoIno1[1oo4igl6Ri393j6hmNFl[U[<g_e96jNf=55S6G>Pm"
";>k:F?FYX>2IEJMoaNdh2jY^WOJ[3II39iZ=HE\\km`;Vbo``_ak>Yn0]E`[mEb8NZ^\\BegTG@[>7kV[ll"
"fL;8[2lKJjMT=2D_Q_G]MRK8lC28kbi]BaPaa@CFCSS?a@RY7:ejeoY<@;7F@W?AmKE=Yo;3NLX\\F6gH;"
"JM;>8DdSQ=Gm8<>H5B2FGOaCB6Q4TmmnbS@SYAh;hAAUiHJFGT;M;<FoRA4JViV\\@3[aF<=PDMM_FlOTR"
"WP]FmMmbKdAP5H2=Yj`EA4bX6CHShPPE^l:395i]ABN_3?BB1k6dcV0@5BTN537j:biOI8nD>SdJ]_Sgg"
"i?nAHGMm[1Q?Re;aF^Y4\\bmAlheAj@?4B;7NIR\\n\\b9aNRb]:Tl1gc0hRbW9RPf^7m`3oCI>_JFQnLAcO"
"7XeZ_[6=FhLB_[2>J>MULc;Z68[f<BLUl=a=iG8Y8aaP1bY`:1HhHI`oQe:YGTGSbRf;`Ie8[XXDYncmQ"
"fURVOe_2NU]JOZVQ4YG7]b>mYiE^`JJ`F2nE:ONRNDfkdQDD]ARURa0=XD^fOd6>nmM[79Ko<^UcR`^AX"
"Gb_9<N^CTbm2<P9DVK3Mma8SK5@6822R[CHCL<T_ThGF4DhMmf_b_:01ch6G74ejHSO8M:Z`N6bI_R6fc"
"egNSTC8k:RAgdWiYaV=>Q<kZIGWJ@Eg>oo[]6dQNQ<nY]O<3KmIRP^0Gd<Hnd:2V[k;UlM9E4JaANZJ^?"
":4H_RKn;YCXl>K[DFDL?XQe_nZKcolX?6;j;WMZ`Z==^dBQi1dmCm9iIJH1`MkAaB57ei9HgK4E2`]VbJ"
};

// assume message broadcase cycle is 6s (6 frames) with following MesType order
const int BCNav3Bit::MessageOrder[6] = {
	10, 0, 30, 0, 40, 0,
};


BCNav3Bit::BCNav3Bit()
{
}

BCNav3Bit::~BCNav3Bit()
{
}

int BCNav3Bit::GetFrameData(GNSS_TIME StartTime, int svid, int Param, int *NavBits)
{
	int i;
	int sow;
	unsigned int FrameData[21];
	int Symbols[162];

	// data channel
	if (svid < 1 || svid > 63)
		return 1;

	sow = StartTime.MilliSeconds / 1000;
	ComposeMessage(MessageOrder[sow % 6], StartTime.Week, sow, svid, FrameData);
	AppendCRC(FrameData, 21);
	// assign each 6bit into Symbols array
	Symbols[0] = FrameData[0] & 0x3f;
	for (i = 0; i < 20; i ++)
	{
		Symbols[i*4+1] = (FrameData[i+1] >> 18) & 0x3f;
		Symbols[i*4+2] = (FrameData[i+1] >> 12) & 0x3f;
		Symbols[i*4+3] = (FrameData[i+1] >> 6) & 0x3f;
		Symbols[i*4+4] = FrameData[i+1] & 0x3f;
	}
	LDPCEncode(Symbols, B2b_SYMBOL_LENGTH, B2bMatrixGen);		// do LDPC encode
	AssignBits(0xeb90, 16, NavBits);	// Preamble
	AssignBits(svid, 6, NavBits + 16);	// PRN
	AssignBits(0, 6, NavBits + 22);	// reserved
	for (i = 0; i < 162; i ++)
		AssignBits(Symbols[i], 6, NavBits + 28 + i * 6);	// 162 encoded symbols
	return 0;
}

// 486 bits subframe information divided int 21 WORDs
// each WORD has 24bits in 24LSB of unsigned int data in FrameData[]
// bit order is MSB first (from bit23) and least index first
// each 24bits divided into four 6bit symbols in LDPC encode
// FrameData[0] only has 6 valid bits in LSB 
void BCNav3Bit::ComposeMessage(int MessageType, int week, int sow, int svid, unsigned int FrameData[])
{
	// first fill in MessageType
	FrameData[0] = MessageType & 0x3f;

	switch (MessageType)
	{
	case 0:
		FrameData[1] = COMPOSE_BITS(sow, 4, 20);
		memset(&FrameData[2], 0x5a, sizeof(unsigned int) * 18);
		break;
	case 10:
		AppendWord(&FrameData[1], 16, Ephemeris1[svid-1], 211);	// with extra IODE put in FrameData[1]
		AppendWord(&FrameData[10], 11, Ephemeris2[svid-1], 222);
		FrameData[19] |= COMPOSE_BITS(IntegrityFlags[svid-1] >> 8, 4, 3);	// B2a DIF/SIF/AIF
		FrameData[19] |= COMPOSE_BITS(IntegrityFlags[svid-1] >> 11, 0, 4);	// SISMAI
		FrameData[1] = COMPOSE_BITS(sow, 4, 20);
		break;
	case 30:
		FrameData[1] = COMPOSE_BITS(sow, 4, 20);
		FrameData[1] |= COMPOSE_BITS(week >> 9, 0, 4);
		FrameData[2] = COMPOSE_BITS(week, 15, 9);
		AppendWord(&FrameData[2], 13, ClockParam[svid-1], 69);
		FrameData[5] |= COMPOSE_BITS(TgsIscParam[svid-1][2], 2, 12);
		AppendWord(&FrameData[5], 22, BdGimIono, 74);
		AppendWord(&FrameData[9], 0, BdtUtcParam, 97);
		AppendWord(&FrameData[13], 1, EopParam, 138);
		FrameData[19] = 0;	// SISAI_oc, SISAI_oe and HS filled with 0
		break;
	case 40:
		FrameData[1] = COMPOSE_BITS(sow, 4, 20);
		memset(&FrameData[2], 0, sizeof(unsigned int) * 18);
		break;
	}
}
