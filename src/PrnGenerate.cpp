//----------------------------------------------------------------------
// PrnGenerate.h:
//   Implementation of PRN code generation class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include <string.h>
#include "PrnGenerate.h"

const unsigned int PrnGenerate::L1CAPrnInit[32] = {
0x0df, 0x06f, 0x037, 0x01b, 0x1a4, 0x0d2, 0x1a6, 0x0d3, 0x069, 0x0bb, 0x05d, 0x017, 0x00b, 0x005, 0x002, 0x001, 
0x191, 0x0c8, 0x064, 0x032, 0x019, 0x00c, 0x1cc, 0x039, 0x01c, 0x00e, 0x007, 0x003, 0x1a8, 0x0d4, 0x06a, 0x035, };

const unsigned int PrnGenerate::L5IPrnInit[32] = {
0x04ea, 0x1583, 0x0202, 0x0c8d, 0x1d77, 0x0be6, 0x1f25, 0x04bd, 0x1a9f, 0x0f7e, 0x0b90, 0x13e7, 0x0738, 0x1c82, 0x0b56, 0x1278, 
0x1e32, 0x0f0f, 0x1f13, 0x16d6, 0x0204, 0x1ef7, 0x0fe1, 0x05a3, 0x16cb, 0x0d35, 0x0f6a, 0x0d5e, 0x10fa, 0x1da1, 0x0f28, 0x13a0, };

const unsigned int PrnGenerate::L5QPrnInit[32] = {
0x0669, 0x0de2, 0x188f, 0x0adc, 0x09bc, 0x12aa, 0x103f, 0x02d6, 0x185d, 0x0c24, 0x1408, 0x146a, 0x14b2, 0x1f85, 0x1e3d, 0x1f4b, 
0x0267, 0x04ed, 0x1b4c, 0x11c3, 0x0136, 0x0e34, 0x17d1, 0x19f6, 0x1b22, 0x07aa, 0x0be1, 0x085f, 0x048a, 0x13c1, 0x14fa, 0x0a89, };

const unsigned int PrnGenerate::L2CMPrnInit[32] = {
0x15ef0f5, 0x50f811e, 0x10e553d, 0x16b0258, 0x416f3bc, 0x65bc21e, 0x0f5be58, 0x496777f,
0x4a5a8e2, 0x36e44d6, 0x5e84705, 0x345ea19, 0x6965b5b, 0x447fb02, 0x0043a6e, 0x35e5896,
0x3059ddd, 0x5c16d2a, 0x10c80db, 0x1c754b4, 0x650324e, 0x7fb4e14, 0x74e048f, 0x0663507,
0x1f887f9, 0x487c247, 0x5fd6d8c, 0x20818d1, 0x1ece400, 0x7aeb923, 0x656b597, 0x602e157, };

const unsigned int PrnGenerate::L2CLPrnInit[32] = {
0x29be220, 0x2012ed7, 0x3d7d64b, 0x12b1c4a, 0x5e3a308, 0x31c0719, 0x3b5179f, 0x74429a6,
0x1d5fc3b, 0x3bf943a, 0x587c624, 0x0be84ce, 0x57d8717, 0x6a8376f, 0x5a13f5d, 0x4a5f5df,
0x046b92b, 0x7a7c2ae, 0x45886a6, 0x5a9a643, 0x68872f2, 0x3e759f6, 0x6b6fdbd, 0x31b717b,
0x048fcb0, 0x1cbc9e3, 0x6b38d5b, 0x6f5b8fa, 0x121a76e, 0x5f23c35, 0x326fd21, 0x3cb4e3c, };

const unsigned int PrnGenerate::B1IPrnInit[63] = {
0x187, 0x639, 0x1e6, 0x609, 0x605, 0x1f8, 0x606, 0x1f9, 0x704, 0x7be, 0x061, 0x78e, 0x782, 0x07f, 0x781, 0x07e,
0x7df, 0x030, 0x03c, 0x7c1, 0x03f, 0x7c0, 0x7ef, 0x7e3, 0x01e, 0x7e0, 0x01f, 0x00c, 0x7f1, 0x00f, 0x7f0, 0x7fd,
0x003, 0x7fc, 0x7fe, 0x001, 0x7ff, 0x457, 0x4ed, 0x4dd, 0x4d1, 0x4d2, 0x32d, 0x48c, 0x492, 0x4bc, 0x4b0, 0x4b3,
0x34c, 0x4a2, 0x4ae, 0x4ad, 0x352, 0x5d0, 0x5b1, 0x5af, 0x50b, 0x515, 0x53b, 0x537, 0x534, 0x2cb, 0x525, };

const unsigned int PrnGenerate::B3IPrnInit[63] = {
0x1ff5, 0x1a8f, 0x0a3d, 0x1bff, 0x1f13, 0x04c9, 0x097f, 0x17f7, 0x0805, 0x1b04, 0x01d7, 0x0f34, 0x1526, 0x0c8e, 0x1231, 0x07c7, 
0x1464, 0x06e0, 0x1d51, 0x0f68, 0x1684, 0x0a34, 0x1e68, 0x08cc, 0x025c, 0x1292, 0x196d, 0x08f5, 0x15e8, 0x1ffe, 0x1e36, 0x1235, 
0x1aa9, 0x14b3, 0x174b, 0x05df, 0x1cd4, 0x0117, 0x013b, 0x0e6b, 0x0581, 0x137a, 0x07b6, 0x11cb, 0x089c, 0x146a, 0x0cf9, 0x025f, 
0x1250, 0x06a1, 0x064f, 0x1e32, 0x0300, 0x0401, 0x0cac, 0x0c4d, 0x03ce, 0x0a74, 0x0df3, 0x1449, 0x008e, 0x084c, 0x0e44, };

const unsigned int PrnGenerate::B2aDPrnInit[63] = {
0x1481, 0x0581, 0x16a1, 0x1e51, 0x1551, 0x0eb1, 0x0ef1, 0x1bf1, 0x1299, 0x0b79, 0x1585, 0x0445, 0x1545, 0x1b45, 0x0745, 0x18a5, 
0x1de5, 0x1015, 0x0f95, 0x1ab5, 0x11b5, 0x194d, 0x08cd, 0x032d, 0x0dad, 0x09ed, 0x1fed, 0x091d, 0x079d, 0x10bd, 0x027d, 0x057d, 
0x1afd, 0x19fd, 0x1143, 0x0523, 0x1da3, 0x1113, 0x1313, 0x1ab3, 0x11b3, 0x0973, 0x154b, 0x05cb, 0x1a6b, 0x1d5b, 0x0587, 0x1827, 
0x1a27, 0x18a7, 0x02a7, 0x1b97, 0x1d37, 0x024f, 0x052f, 0x132f, 0x0b6f, 0x03ef, 0x1fef, 0x15bf, 0x0804, 0x15fb, 0x0978, };

const unsigned int PrnGenerate::B2aPPrnInit[63] = {
0x1481, 0x0581, 0x16a1, 0x1e51, 0x1551, 0x0eb1, 0x0ef1, 0x1bf1, 0x1299, 0x0b79, 0x1585, 0x0445, 0x1545, 0x1b45, 0x0745, 0x18a5, 
0x1de5, 0x1015, 0x0f95, 0x1ab5, 0x11b5, 0x194d, 0x08cd, 0x032d, 0x0dad, 0x09ed, 0x1fed, 0x091d, 0x079d, 0x10bd, 0x027d, 0x057d, 
0x1afd, 0x19fd, 0x1143, 0x0523, 0x1da3, 0x1113, 0x1313, 0x1ab3, 0x11b3, 0x0973, 0x154b, 0x05cb, 0x1a6b, 0x1d5b, 0x0587, 0x1827, 
0x1a27, 0x18a7, 0x02a7, 0x1b97, 0x1d37, 0x024f, 0x052f, 0x132f, 0x0b6f, 0x03ef, 0x1fef, 0x15bf, 0x0c25, 0x03f4, 0x1558, };

const unsigned int PrnGenerate::B2bPrnInit[63] = {
0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0eb1, 0x0ef1, 0x1bf1, 0x1299, 0x0b79, 0x1585, 0x0445, 0x1545, 0x1b45, 0x0745, 0x18a5, 
0x1de5, 0x1015, 0x0f95, 0x1ab5, 0x11b5, 0x194d, 0x08cd, 0x032d, 0x0dad, 0x09ed, 0x1fed, 0x091d, 0x079d, 0x10bd, 0x027d, 0x057d, 
0x1afd, 0x19fd, 0x1143, 0x0523, 0x1da3, 0x1113, 0x1313, 0x1ab3, 0x11b3, 0x0973, 0x154b, 0x05cb, 0x1a6b, 0x1d5b, 0x0587, 0x1827, 
0x1a27, 0x18a7, 0x02a7, 0x1b97, 0x1d37, 0x024f, 0x052f, 0x132f, 0x0b6f, 0x03ef, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, };

const unsigned int PrnGenerate::E5aIPrnInit[50] = {
0x30c5, 0x189c, 0x2e8b, 0x217f, 0x26ca, 0x3733, 0x1b8c, 0x155f, 0x0357, 0x309e, 0x2ee4, 0x0eba, 0x3cff, 0x1e26, 0x0d1c, 0x1b05, 
0x28aa, 0x1399, 0x29fe, 0x0198, 0x1370, 0x1eba, 0x2f25, 0x33c2, 0x160a, 0x1901, 0x39d7, 0x2597, 0x3193, 0x2eae, 0x0350, 0x1889, 
0x3335, 0x2474, 0x374e, 0x05df, 0x22ce, 0x3b15, 0x3b9b, 0x29ad, 0x182c, 0x2e17, 0x0d84, 0x332d, 0x3935, 0x2abb, 0x21f3, 0x33d1, 
0x1eca, 0x16bf, };

const unsigned int PrnGenerate::E5aQPrnInit[50] = {
0x2baa, 0x0a62, 0x29d3, 0x33e9, 0x2ef6, 0x29b0, 0x37ad, 0x2f28, 0x0f96, 0x03c5, 0x15cf, 0x3452, 0x1c3d, 0x1da4, 0x3f6e, 0x053f, 
0x04b5, 0x0d18, 0x2a26, 0x15dd, 0x08b2, 0x1298, 0x001f, 0x0c5f, 0x08ca, 0x2186, 0x1272, 0x24aa, 0x315b, 0x298c, 0x0ff7, 0x35c5, 
0x0a2a, 0x2f6b, 0x07c9, 0x0421, 0x39fd, 0x0abc, 0x3eee, 0x1c85, 0x3cb8, 0x0d80, 0x2dfb, 0x1efd, 0x3ab7, 0x3cad, 0x1424, 0x2d22, 
0x2391, 0x2b09, };

const unsigned int PrnGenerate::E5bIPrnInit[50] = {
0x0e90, 0x2c27, 0x00aa, 0x1e76, 0x1871, 0x0560, 0x035f, 0x2c13, 0x03d5, 0x219f, 0x04f4, 0x2fd9, 0x31a0, 0x387c, 0x0d34, 0x0fbe, 
0x3499, 0x10eb, 0x01ed, 0x2c3f, 0x13a4, 0x135f, 0x3a4d, 0x212a, 0x39a5, 0x2bb4, 0x2303, 0x34ab, 0x04df, 0x31ff, 0x2e52, 0x24ff, 
0x3c7d, 0x363d, 0x3669, 0x165c, 0x0f1b, 0x108e, 0x3b36, 0x055b, 0x0ae9, 0x3051, 0x1808, 0x357e, 0x30d6, 0x3f1b, 0x2c12, 0x3bf8, 
0x0db8, 0x140f, };

const unsigned int PrnGenerate::E5bQPrnInit[50] = {
0x06d9, 0x0c63, 0x2ad2, 0x26f9, 0x010b, 0x3c9d, 0x1fe8, 0x09e5, 0x1605, 0x3e60, 0x306d, 0x209f, 0x0731, 0x33b2, 0x2e66, 0x0b67, 
0x052e, 0x300b, 0x00d2, 0x11f1, 0x2df7, 0x3c04, 0x31cb, 0x0fb2, 0x2388, 0x205c, 0x12b2, 0x11c6, 0x3863, 0x1229, 0x2b30, 0x1fb5, 
0x34ec, 0x2298, 0x2066, 0x12f2, 0x3ea6, 0x1ce4, 0x1a1c, 0x2b39, 0x2ba6, 0x246f, 0x08de, 0x1cee, 0x083d, 0x0596, 0x13c6, 0x3e09, 
0x2e21, 0x3214, };

const int PrnGenerate::B1CDataTruncation[63] = {
  699,  694, 7318, 2127,  715, 6682, 7850, 5495, 1162, 7682, 6792, 9973, 6596, 2092,   19,10151,
 6297, 5766, 2359, 7136, 1706, 2128, 6827,  693, 9729, 1620, 6805,  534,  712, 1929, 5355, 6139,
 6339, 1470, 6867, 7851, 1162, 7659, 1156, 2672, 6043, 2862,  180, 2663, 6940, 1645, 1582,  951,
 6878, 7701, 1823, 2391, 2606,  822, 6403,  239,  442, 6769, 2560, 2502, 5072, 7268,  341, };
 
 const int PrnGenerate::B1CDataPhaseDiff[63] = {
 2678, 4802,  958,  859, 3843, 2232,  124, 4352, 1816, 1126, 1860, 4800, 2267,  424, 4192, 4333,
 2656, 4148,  243, 1330, 1593, 1470,  882, 3202, 5095, 2546, 1733, 4795, 4577, 1627, 3638, 2553,
 3646, 1087, 1843,  216, 2245,  726, 1966,  670, 4130,   53, 4830,  182, 2181, 2006, 1080, 2288,
 2027,  271,  915,  497,  139, 3693, 2054, 4342, 3342, 2592, 1007,  310, 4203,  455, 4318, };

const int PrnGenerate::B1CPilotTruncation[63] = {
 7575, 2369, 5688,  539, 2270, 7306, 6457, 6254, 5644, 7119, 1402, 5557, 5764, 1073, 7001, 5910,
10060, 2710, 1546, 6887, 1883, 5613, 5062, 1038,10170, 6484, 1718, 2535, 1158, 526 , 7331, 5844,
 6423, 6968, 1280, 1838, 1989, 6468, 2091, 1581, 1453, 6252, 7122, 7711, 7216, 2113, 1095, 1628,
 1713, 6102, 6123, 6070, 1115, 8047, 6795, 2575,   53, 1729, 6388,  682, 5565, 7160, 2277, };

const int PrnGenerate::B1CPilotPhaseDiff[63] = {
  796,  156, 4198, 3941, 1374, 1338, 1833, 2521, 3175,  168, 2715, 4408, 3160, 2796,  459, 3594,
 4813,  586, 1428, 2371, 2285, 3377, 4965, 3779, 4547, 1646, 1430,  607, 2118, 4709, 1149, 3283,
 2473, 1006, 3670, 1817,  771, 2173,  740, 1433, 2458, 3459, 2155, 1205,  413,  874, 2463, 1106,
 1590, 3873, 4026, 4272, 3556,  128, 1200,  130, 4494, 1871, 3073, 4386, 4098, 1923, 1176, };

const int PrnGenerate::L1CDataInsertIndex[63] = {
  181,  359,   72, 1110, 1480, 5034, 4622,    1, 4547,  826, 6284, 4195,  368,    1, 4796,  523,
  151,  713, 9850, 5734,   34, 6142,  190,  644,  467, 5384,  801,  594, 4450, 9437, 4307, 5906,
  378, 9448, 9432, 5849, 5547, 9546, 9132,  403, 3766,    3,  684, 9711,  333, 6124,10216, 4251,
 9893, 9884, 4627, 4449, 9798,  985, 4272,  126,10024,  434, 1029,  561,  289,  638, 4353, };

const int PrnGenerate::L1CDataPhaseDiff[63] = {
 5097, 5110, 5079, 4403, 4121, 5043, 5042, 5104, 4940, 5035, 4372, 5064, 5084, 5048, 4950, 5019,
 5076, 3736, 4993, 5060, 5061, 5096, 4983, 4783, 4991, 4815, 4443, 4769, 4879, 4894, 4985, 5056,
 4921, 5036, 4812, 4838, 4855, 4904, 4753, 4483, 4942, 4813, 4957, 4618, 4669, 4969, 5031, 5038,
 4740, 4073, 4843, 4979, 4867, 4964, 5025, 4579, 4390, 4763, 4612, 4784, 3716, 4703, 4851, };

const int PrnGenerate::L1CPilotInsertIndex[63] = {
  412,  161,    1,  303,  207, 4971, 4496,    5, 4557,  485,  253, 4676,    1,   66, 4485,  282,
  193, 5211,  729, 4848,  982, 5955, 9805,  670,  464,   29,  429,  394,  616, 9457, 4429, 4771,
  365, 9705, 9489, 4193, 9947,  824,  864,  347,  677, 6544, 6312, 9804,  278, 9461,  444, 4839,
 4144, 9875,  197, 1156, 4674,10035, 4504,    5, 9937,  430,    5,  355,  909, 1622, 6284,};

const int PrnGenerate::L1CPilotPhaseDiff[63] = {
 5111, 5109, 5108, 5106, 5103, 5101, 5100, 5098, 5095, 5094, 5093, 5091, 5090, 5081, 5080, 5069,
 5068, 5054, 5044, 5027, 5026, 5014, 5004, 4980, 4915, 4909, 4893, 4885, 4832, 4824, 4591, 3706,
 5092, 4986, 4965, 4920, 4917, 4858, 4847, 4790, 4770, 4318, 4126, 3961, 3790, 4911, 4881, 4827,
 4795, 4789, 4725, 4675, 4539, 4535, 4458, 4197, 4096, 3484, 3481, 3393, 3175, 2360, 1852, };

// Galileo memory code
#include "MemoryCode.dat"

const PrnAttribute PrnGenerate::PrnAttributes[] = {
	// ChipRate DataPeriod PilotPeriod Attribute
	{  1023,       1,         1,                 0 },	// index  0 for L1CA
	{  2046,      10,        10, PRN_ATTRIBUTE_BOC },	// index  1 for L1C/B1C (BOC(1,1) treated as 2 chips per PRN code)
	{  1023,      20,      1500, PRN_ATTRIBUTE_TMD },	// index  2 for L2C
	{ 10230, 10230*2, 10230 * 2,                 0 },	// index  3 for L2P
	{ 10230,       1,         1,                 0 },	// index  4 for L5/B2a/E5a/E5b
	{  2046,       1,         1,                 0 },	// index  5 for B1I/B2I
	{ 10230,       1,         1,                 0 },	// index  6 for B3I
	{ 10230,       1,         1,                 0 },	// index  7 for B2b
	{  2046,       4,         4, PRN_ATTRIBUTE_BOC },	// index  8 for E1
	{  5115,       1,         1,                 0 },	// index  9 for E6
	{   511,       1,         1,                 0 },	// index 10 for G1/G2
};

LsfrSequence::LsfrSequence(unsigned int InitState, unsigned int Polynomial, int Length) : mInitState(InitState), mPolynomial(Polynomial), mOutputMask(1<<(Length-1))
{
	mCurrentState = mInitState;
}

LsfrSequence::~LsfrSequence()
{
}

void LsfrSequence::Initial()
{
	mCurrentState = mInitState;
}

int LsfrSequence::GetOutput()
{
	int Output = (mCurrentState & mOutputMask) ? 1 : 0;
	unsigned int Feedback = mCurrentState & mPolynomial;

	// count number of 1s
	Feedback = (Feedback & 0x55555555) + ((Feedback >> 1) & 0x55555555);
	Feedback = (Feedback & 0x33333333) + ((Feedback >> 2) & 0x33333333);
	Feedback = (Feedback & 0x0f0f0f0f) + ((Feedback >> 4) & 0x0f0f0f0f);
	Feedback = (Feedback & 0x00ff00ff) + ((Feedback >> 8) & 0x00ff00ff);
	Feedback = (Feedback & 0x0000ffff) + ((Feedback >> 16) & 0x0000ffff);
	mCurrentState = (mCurrentState << 1) | (Feedback & 1);

	return Output;
}

PrnGenerate::PrnGenerate(GnssSystem System, int SignalIndex, int Svid)
{
	// signal and navigation bit match
	switch (System)
	{
	case GpsSystem:
		// validate GPS SVID range
		if (Svid < 1 || Svid > 32)
		{
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
			break;
		}
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_L1CA:
			DataPrn  = GetGoldCode(L1CAPrnInit[Svid-1], 0x3a6, 0x3ff, 0x204, 1023, 10, 1023);
			PilotPrn = NULL;
			Attribute = &PrnAttributes[0];
			break;
		case SIGNAL_INDEX_L1C:
			DataPrn  = GetL1CWeil(L1CDataInsertIndex[Svid-1], L1CDataPhaseDiff[Svid-1]);
			PilotPrn = GetL1CWeil(L1CPilotInsertIndex[Svid-1], L1CPilotPhaseDiff[Svid-1]);
			Attribute = &PrnAttributes[1];
			break;
		case SIGNAL_INDEX_L2C:
			DataPrn  = GetGoldCode(L2CMPrnInit[Svid-1], 0x0494953c, 0x0, 0x0, 10230, 27, 10230);
			PilotPrn = GetGoldCode(L2CLPrnInit[Svid-1], 0x0494953c, 0x0, 0x0, 10230*75, 27, 0);
			Attribute = &PrnAttributes[2];
			break;
		case SIGNAL_INDEX_L2P:
			DataPrn  = new int[10230*2];
			PilotPrn = NULL;
			Attribute = &PrnAttributes[3];
			break;
		case SIGNAL_INDEX_L5:
			DataPrn  = GetGoldCode(L5IPrnInit[Svid-1], 0x18ed, 0x1fff, 0x1b00, 10230, 13, 8190);
			PilotPrn = GetGoldCode(L5QPrnInit[Svid-1], 0x18ed, 0x1fff, 0x1b00, 10230, 13, 8190);
			Attribute = &PrnAttributes[4];
			break;
		default:	// unknown SignalIndex
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
		}
		break;
	case BdsSystem:
		// validate BDS SVID range
		if (Svid < 1 || Svid > 63)
		{
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
			break;
		}
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_B1I:
		case SIGNAL_INDEX_B2I:
			DataPrn  = GetGoldCode(B1IPrnInit[Svid-1], 0x59f, 0x2aa, 0x7c1, 2046, 11, 2046);
			PilotPrn = NULL;
			Attribute = &PrnAttributes[5];
			break;
		case SIGNAL_INDEX_B3I:
			DataPrn  = GetGoldCode(B3IPrnInit[Svid-1], 0x1b71, 0x1fff, 0x100d, 10230, 13, 8190);
			PilotPrn = NULL;
			Attribute = &PrnAttributes[6];
			break;
		case SIGNAL_INDEX_B1C: 
			DataPrn  = GetB1CWeil(B1CDataTruncation[Svid-1], B1CDataPhaseDiff[Svid-1]);
			PilotPrn = GetB1CWeil(B1CPilotTruncation[Svid-1], B1CPilotPhaseDiff[Svid-1]);
			Attribute = &PrnAttributes[1];
			break;
		case SIGNAL_INDEX_B2a:
			DataPrn  = GetGoldCode(B2aDPrnInit[Svid-1], 0x1d14, 0x1fff, 0x1411, 10230, 13, 8190);
			PilotPrn = GetGoldCode(B2aPPrnInit[Svid-1], 0x18d1, 0x1fff, 0x1064, 10230, 13, 8190);
			Attribute = &PrnAttributes[4];
			break;
		case SIGNAL_INDEX_B2b:
			DataPrn  = GetGoldCode(B2bPrnInit[Svid-1], 0x192c, (Svid < 6) || (Svid > 58) ? 0 : 0x1fff, 0x1301, 10230, 13, 8190);
			PilotPrn = NULL;
			Attribute = &PrnAttributes[7];
			break;
		default:	// unknown SignalIndex
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
		}
		break;
	case GalileoSystem:
		// validate Galileo SVID range
		if (Svid < 1 || Svid > 36)
		{
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
			break;
		}
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_E1 :
			DataPrn  = GetMemorySequence(E1MemoryCode + (Svid - 1) * 128, 4);
			PilotPrn = GetMemorySequence(E1MemoryCode + (Svid + 49) * 128, 4);
			Attribute = &PrnAttributes[8];
			break;
		case SIGNAL_INDEX_E5a:
			DataPrn  = GetGoldCode(E5aIPrnInit[Svid-1], 0x28d8, 0x3fff, 0x20a1, 10230, 14, 10230);
			PilotPrn = GetGoldCode(E5aQPrnInit[Svid-1], 0x28d8, 0x3fff, 0x20a1, 10230, 14, 10230);
			Attribute = &PrnAttributes[4];
			break;
		case SIGNAL_INDEX_E5b:
			DataPrn  = GetGoldCode(E5bIPrnInit[Svid-1], 0x2992, 0x3fff, 0x3408, 10230, 14, 10230);
			PilotPrn = GetGoldCode(E5bQPrnInit[Svid-1], 0x2331, 0x3fff, 0x3408, 10230, 14, 10230);
			Attribute = &PrnAttributes[4];
			break;
		case SIGNAL_INDEX_E6 :
			DataPrn  = GetMemorySequence(E6MemoryCode + (Svid - 1) * 160, 5);
			PilotPrn = GetMemorySequence(E6MemoryCode + (Svid + 49) * 160, 5);
			Attribute = &PrnAttributes[9];
			break;
		default:	// unknown SignalIndex
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
		}
		break;
	case GlonassSystem:
		switch (SignalIndex)
		{
		case SIGNAL_INDEX_G1:
		case SIGNAL_INDEX_G2:
			DataPrn  = GetGoldCode(0x1fc, 0x110, 0x0, 0x0, 511, 9, 511);
			PilotPrn = NULL;
			Attribute = &PrnAttributes[10];
			break;
		default:	// unknown SignalIndex
			DataPrn = NULL; PilotPrn = NULL;
			Attribute = NULL;
		}
		break;
	default:	// unknown system
		DataPrn = NULL; PilotPrn = NULL;
		Attribute = NULL;
	}
}

PrnGenerate::~PrnGenerate()
{
	if (DataPrn)
		delete DataPrn;
	if (PilotPrn)
		delete PilotPrn;
}

int *PrnGenerate::GetGoldCode(unsigned int G1Init, unsigned int G1Poly, unsigned int G2Init, unsigned int G2Poly, int Length, int Depth, int ResetPos)
{
	int *PrnSequence = new int[Length];
	int i;
	LsfrSequence G1(G1Init, G1Poly, Depth), G2(G2Init, G2Poly, Depth);

	for (i = 0; i < Length; i ++)
	{
		if (i == ResetPos) G2.Initial();
		PrnSequence[i] = G1.GetOutput() ^ G2.GetOutput();
	}
	return PrnSequence;
}

void PrnGenerate::LegendreSequence(int *Data, int Length)
{
	int i;

	memset(Data, 0, sizeof(int) * Length);
	for (i = 1; i < Length; i ++)
		Data[(i*i) % Length] = 1;
}

int *PrnGenerate::GetL1CWeil(int InsertIndex, int PhaseDiff)
{
	int LegendreCode[10223], InsertSequence[7] = {0, 1, 1, 0, 1, 0, 0};
	int *PrnSequence = new int[10230];
	int i, Index1 = 0, Index2 = PhaseDiff;

	LegendreSequence(LegendreCode, 10223);
	for (i = 0; i < 10230; i ++)
	{
		if (Index2 >= 10223) Index2 -= 10223;
		if (i >= InsertIndex - 1 && i < InsertIndex + 6)
			PrnSequence[i] = InsertSequence[i - InsertIndex + 1];
		else
			PrnSequence[i] = LegendreCode[Index1++] ^ LegendreCode[Index2++];
	}
	return PrnSequence;
}

int *PrnGenerate::GetB1CWeil(int TruncationPoint, int PhaseDiff)
{
	int LegendreCode[10243];
	int *PrnSequence = new int[10230];
	int i, Index1 = TruncationPoint - 1, Index2 = TruncationPoint + PhaseDiff - 1;

	LegendreSequence(LegendreCode, 10243);
	for (i = 0; i < 10230; i ++)
	{
		if (Index1 >= 10243) Index1 -= 10243;
		if (Index2 >= 10243) Index2 -= 10243;
		PrnSequence[i] = LegendreCode[Index1++] ^ LegendreCode[Index2++];
	}
	return PrnSequence;
}

int *PrnGenerate::GetMemorySequence(const unsigned int *BinarySequence, int SectorLength)
{
	int *PrnSequence = new int[1023 * SectorLength], *pPrn;
	int i, j;

	pPrn = PrnSequence;
	for (i = 0; i < SectorLength; i ++)
	{
		for (j = 0; j < 1023; j ++)
			*pPrn++ = (BinarySequence[j>>5] & (1 << (31 - (j&0x1f)))) ? 1 : 0;
		BinarySequence += 32;
	}

	return PrnSequence;
}
