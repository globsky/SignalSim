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

#define SQRT_A0 5440.588203494177338011974948823
#define NORMINAL_I0 0.97738438111682456307726683035362

const int INavBit::WordAllocationE1[15] = {
	2, 4, 6, 7, 8, 17, 19, 16, 0, 0, 1, 3, 5, 0, 16,
};

const int INavBit::WordAllocationE5[15] = {
	1, 3, 5, 7, 8, 0, 0, 0, 0, 0, 2, 4, 6, 0, 0,
};

const int INavBit::SyncPattern[10] = {
	0, 1, 0, 1, 1, 0, 0, 0, 0, 0
};

const unsigned char INavBit::RsGenerateMatrix[60][58] = {
{193,146,156,183,62,35,90,25,89,208,43,148,117,199,92,112,4,70,74,183,37,100,118,44,238,107,186,200,179,63,140,194,176,131,38,110,33,178,43,173,98,96,105,115,90,76,133,194,133,182,99,20,183,90,84,131,26,139},
{91,221,65,59,102,79,66,118,29,43,164,253,13,228,108,106,252,230,45,237,12,149,152,235,143,178,102,199,227,65,230,178,140,133,93,213,89,41,198,197,215,94,122,58,18,251,179,187,61,1,239,229,179,214,244,97,202,42},
{190,173,125,234,225,164,122,176,130,152,32,205,41,44,78,230,128,101,92,134,176,227,128,184,180,130,142,165,134,141,110,153,50,65,25,14,98,172,34,94,181,210,38,49,15,55,169,204,233,34,197,138,78,134,181,57,82,206},
{154,48,226,109,5,179,159,79,237,32,199,98,33,127,219,8,110,36,68,76,24,100,105,49,245,131,53,59,61,75,216,146,103,132,185,132,24,216,243,97,70,233,161,6,10,226,77,85,182,219,24,87,154,117,124,3,106,233},
{101,12,238,213,120,140,44,51,196,68,201,121,36,108,200,161,56,97,123,115,237,146,54,164,141,59,232,60,60,36,85,169,129,61,49,75,21,19,49,124,195,252,63,207,153,125,144,60,39,141,117,232,96,5,162,38,187,80},
{58,136,130,2,173,144,135,249,194,161,38,205,85,247,142,247,228,245,99,151,114,158,68,235,185,203,78,14,149,18,163,149,250,51,198,100,15,234,113,242,230,245,22,250,196,114,38,80,103,157,65,57,4,107,109,16,9,131},
{231,109,52,176,87,22,48,253,54,106,47,175,119,108,137,133,99,143,63,81,242,135,75,49,22,130,125,185,168,229,196,43,92,226,57,76,149,140,100,86,217,135,132,114,90,208,180,174,150,112,147,191,11,164,32,117,5,36},
{197,18,114,255,50,100,185,124,218,88,190,215,41,191,206,138,120,5,243,244,111,223,137,234,221,169,43,39,143,82,41,137,149,2,84,3,143,219,88,107,118,9,200,15,221,43,131,249,253,37,40,189,116,164,225,126,177,188},
{152,246,146,92,78,104,171,40,30,39,246,202,204,246,55,63,240,204,63,221,233,17,105,240,166,128,153,215,115,174,39,28,112,195,178,227,100,75,117,74,82,114,105,148,192,160,58,182,232,67,140,167,147,187,216,183,66,19},
{88,233,3,20,252,51,135,113,130,146,84,13,81,30,147,99,56,10,63,185,42,166,207,88,220,165,199,7,86,27,71,58,216,213,113,126,19,45,56,108,143,26,252,120,123,185,143,39,153,166,78,151,33,124,208,125,40,233},
{73,120,195,119,81,197,85,70,175,109,204,25,136,117,21,195,222,74,162,75,237,9,155,148,242,157,99,129,206,123,255,182,75,94,57,229,151,178,115,53,34,17,139,20,30,190,82,126,204,133,128,61,227,71,16,86,4,201},
{62,195,117,83,179,169,59,213,201,116,240,164,198,84,105,89,127,51,126,50,81,215,167,203,139,96,251,23,173,37,188,176,235,36,53,56,67,148,47,18,237,164,221,182,234,200,51,29,243,115,114,246,173,29,62,127,167,14},
{169,67,240,110,142,45,20,35,20,206,92,117,106,151,17,35,2,94,255,101,84,49,101,168,17,122,73,59,87,175,103,105,140,56,62,64,5,220,188,80,138,214,246,235,11,222,227,230,54,55,52,88,237,16,238,237,27,194},
{88,216,182,118,206,243,194,206,137,152,189,167,238,184,242,69,36,248,173,121,146,27,239,84,132,18,61,215,186,63,70,122,173,41,138,242,176,76,175,165,149,194,88,231,4,114,241,254,201,120,58,47,222,2,123,75,114,176},
{188,19,183,35,242,4,50,148,5,83,98,226,132,70,109,207,93,173,89,56,167,215,50,7,10,227,169,183,90,56,48,240,253,128,253,243,52,59,182,102,244,200,169,111,38,177,75,71,199,90,47,66,186,31,231,86,23,111},
{23,55,128,7,36,174,140,216,161,135,112,142,248,244,169,173,126,229,219,233,3,40,112,29,130,246,246,170,182,54,85,154,76,227,6,219,121,94,24,81,142,202,226,212,231,55,200,225,62,28,217,96,242,50,143,249,1,118},
{36,111,216,59,50,180,160,75,113,134,211,91,182,143,169,101,207,67,126,96,42,56,183,225,15,184,125,128,115,21,76,214,228,55,135,200,122,54,10,195,57,215,193,137,218,47,31,75,201,19,226,46,219,252,171,244,153,190},
{202,21,206,182,17,61,151,229,64,62,73,178,215,44,252,93,5,209,253,16,63,35,156,48,132,142,110,111,142,65,41,174,240,80,64,126,247,108,249,9,154,80,102,29,170,52,203,253,175,191,82,31,64,248,79,31,153,237},
{63,144,241,65,93,237,185,69,251,58,85,58,152,193,129,101,152,224,102,114,197,80,147,168,161,35,37,176,37,19,150,140,126,239,31,84,110,203,7,62,79,48,27,101,153,52,2,110,203,86,143,164,144,196,194,80,87,17},
{20,217,37,96,55,13,235,165,222,135,30,148,34,171,224,35,95,56,14,247,5,181,213,16,162,2,124,197,27,211,78,31,186,240,5,6,25,69,239,23,105,5,142,36,99,80,157,139,199,218,129,93,53,150,145,76,209,173},
{102,239,33,51,4,178,28,177,128,137,105,141,208,44,210,248,152,72,255,24,26,112,197,180,35,51,35,150,104,152,253,28,134,112,41,200,80,83,171,151,10,167,71,162,53,204,242,207,41,54,32,236,75,36,132,91,34,112},
{230,225,186,12,73,187,104,52,239,137,241,251,194,35,227,12,21,251,1,210,178,186,187,212,199,45,72,126,86,110,94,159,88,238,94,182,143,236,43,42,34,71,81,29,120,105,177,144,178,120,255,94,193,145,44,236,247,141},
{131,247,106,160,99,127,254,236,225,79,24,221,242,124,91,71,209,121,237,27,217,152,35,103,27,7,61,28,185,73,182,77,200,106,117,51,96,56,125,37,61,213,103,101,88,83,102,162,159,216,31,113,68,132,78,30,248,98},
{141,134,43,202,72,109,204,33,132,193,51,182,43,212,100,221,126,205,46,77,190,130,181,189,175,208,165,139,133,24,113,224,15,96,20,206,175,176,68,217,213,95,140,58,214,164,80,48,161,118,97,194,209,247,238,230,26,34},
{214,148,137,44,243,55,191,63,77,214,201,75,217,156,103,212,104,128,241,41,83,85,83,182,214,56,127,110,57,214,249,25,236,146,192,92,101,119,184,14,83,139,28,130,232,139,88,56,204,204,150,58,197,3,51,115,171,240},
{45,176,137,213,83,71,180,128,251,170,151,33,125,122,145,152,32,29,244,173,19,175,36,161,46,108,88,154,198,123,60,147,246,64,239,179,146,240,54,156,124,91,75,164,1,18,169,50,26,173,131,149,102,70,251,159,11,242},
{101,187,110,190,192,218,216,24,11,82,67,41,103,48,205,235,168,47,66,195,12,153,253,233,29,224,7,81,157,223,101,77,128,172,245,29,34,153,25,185,62,198,141,37,59,118,96,216,64,33,3,115,162,249,145,161,39,49},
{94,206,2,170,252,165,52,243,243,102,47,226,182,51,212,93,231,98,241,134,172,160,3,137,198,160,51,66,153,220,86,62,128,24,198,71,126,233,228,243,31,158,125,24,203,239,228,59,74,135,132,63,183,76,5,9,143,167},
{62,212,195,146,110,4,91,180,124,40,251,71,61,106,47,152,225,10,86,97,156,150,14,83,150,84,198,71,110,114,27,25,99,239,115,199,225,125,116,133,43,153,82,64,230,29,98,171,182,245,112,242,175,73,53,106,248,133},
{65,208,229,79,178,119,149,48,150,242,149,110,57,7,128,153,202,211,206,218,3,45,91,82,253,98,180,185,153,212,22,233,116,66,62,79,247,165,192,32,79,200,68,87,209,89,71,144,241,160,165,119,126,62,7,20,178,125},
{66,194,199,72,30,167,104,8,188,230,72,45,18,64,35,84,64,224,143,99,244,77,194,171,115,86,200,40,205,185,128,199,14,197,255,61,184,242,31,99,85,216,129,3,72,182,211,150,26,45,164,63,218,97,181,182,26,172},
{46,73,249,186,70,166,170,161,98,165,160,141,25,242,131,190,43,45,12,242,21,247,30,156,188,150,124,206,42,21,99,201,162,50,212,179,52,108,180,204,174,41,140,123,14,106,68,154,100,177,54,58,66,215,92,137,24,216},
{131,63,194,227,213,112,227,46,116,194,52,140,132,167,138,39,99,71,59,22,249,63,110,194,83,124,134,40,9,53,205,112,158,144,169,185,101,131,253,186,219,89,9,184,62,37,101,87,149,14,214,184,32,7,8,110,157,141},
{42,239,135,195,81,125,136,127,58,211,123,157,75,107,154,100,161,46,157,58,123,179,20,189,215,98,69,18,175,20,175,11,27,127,17,177,82,77,58,140,53,23,43,11,64,33,1,163,115,59,46,114,185,216,43,233,30,109},
{187,107,37,79,138,135,199,125,100,160,209,101,79,175,142,182,14,16,33,85,179,136,192,221,110,59,128,190,176,114,107,181,8,123,87,3,82,100,79,166,32,172,206,166,177,16,30,27,156,47,3,241,186,3,233,75,189,170},
{9,165,153,194,5,174,10,63,16,164,181,18,107,155,255,189,32,70,255,198,172,116,102,163,87,238,208,19,207,81,226,130,36,223,98,234,108,136,113,176,188,113,103,29,43,131,26,247,17,208,158,247,22,55,98,62,83,91},
{122,218,243,230,181,240,78,230,125,90,141,63,181,72,34,163,211,215,3,128,106,59,113,131,158,208,182,227,2,45,162,58,49,199,209,30,40,232,161,64,126,69,193,153,253,55,57,194,77,251,139,85,136,246,82,129,112,108},
{3,23,252,210,151,185,126,184,72,131,93,160,61,246,134,64,40,203,79,34,204,149,25,221,12,193,154,85,73,152,29,129,176,161,171,238,217,105,239,86,255,10,209,131,23,37,231,26,18,135,175,22,116,6,51,194,87,235},
{19,239,7,48,105,219,196,193,243,116,95,221,73,66,231,53,8,176,139,131,2,10,74,187,90,36,199,11,209,224,181,56,51,115,183,217,134,79,181,170,252,113,6,84,254,59,138,194,181,204,63,218,218,9,135,240,75,4},
{118,105,33,252,158,172,57,215,1,102,65,97,114,123,228,46,77,159,175,112,2,78,135,37,128,188,73,95,136,97,211,225,153,170,208,55,32,27,122,127,162,200,171,237,182,165,230,222,31,80,218,186,33,56,95,30,193,34},
{6,172,37,99,30,134,173,200,150,224,104,27,101,244,250,32,37,125,178,237,232,225,10,194,38,62,40,146,22,161,1,149,232,164,114,174,164,162,21,137,28,74,253,47,236,27,24,160,99,150,248,253,248,32,175,98,175,234},
{154,136,227,53,140,76,189,152,149,52,191,42,247,51,3,188,168,129,92,162,115,60,107,187,143,17,137,157,10,219,244,253,107,94,92,239,184,30,253,86,145,64,57,221,20,1,97,228,218,81,172,106,237,195,218,25,49,20},
{14,142,223,89,178,180,186,230,37,21,217,61,213,111,83,33,88,216,192,230,12,2,136,41,188,132,163,107,195,180,59,177,232,241,187,138,165,223,243,87,177,4,57,80,43,195,13,36,232,89,143,255,208,27,118,64,9,126},
{193,156,18,104,103,145,238,163,191,245,62,77,72,18,51,35,37,30,146,119,195,104,116,164,199,215,62,107,216,252,56,249,1,107,215,213,171,23,244,94,53,209,109,74,10,103,70,207,161,94,58,155,72,138,79,245,90,130},
{79,59,250,36,207,70,246,222,60,177,165,41,204,234,152,167,149,103,155,164,187,3,17,106,13,56,133,163,235,220,120,143,129,186,95,61,175,136,71,10,115,67,117,1,45,98,49,241,153,52,208,110,173,47,113,244,61,144},
{251,48,41,161,98,4,23,58,118,137,248,53,112,216,84,215,230,30,47,192,128,172,163,224,153,32,247,171,185,250,215,122,108,176,103,100,88,226,193,41,242,37,192,195,80,138,188,51,47,149,155,136,53,252,19,64,193,142},
{124,242,42,20,52,143,229,10,57,221,174,40,150,213,224,204,178,33,118,18,244,244,237,3,222,160,236,31,243,114,105,73,192,178,102,101,53,94,197,3,132,227,160,186,34,225,141,34,52,76,102,119,181,212,14,205,191,213},
{18,64,11,249,128,124,136,8,208,164,247,53,52,35,117,134,253,34,226,165,253,103,53,43,10,195,107,215,253,44,79,166,118,115,128,1,187,156,116,38,55,38,104,27,189,70,245,66,54,172,51,147,164,178,22,189,253,110},
{186,131,99,220,159,82,188,47,130,235,80,242,18,26,119,140,149,68,174,53,227,91,10,152,118,211,236,58,19,86,67,191,40,102,127,135,84,178,104,78,20,130,120,219,219,165,194,5,198,44,83,108,68,100,192,6,80,203},
{244,1,47,157,50,104,143,121,171,22,185,89,161,84,117,207,80,102,206,80,112,208,81,221,226,101,99,138,24,202,173,238,196,243,122,135,30,65,224,10,207,251,255,153,6,227,204,111,108,15,154,216,146,153,174,27,154,46},
{166,77,140,145,232,144,89,158,148,141,37,165,157,162,192,200,9,9,104,112,87,52,200,120,142,45,51,235,242,210,120,215,74,253,40,204,70,217,114,147,235,77,87,158,168,173,127,182,243,109,81,99,102,163,156,151,214,50},
{235,88,105,58,29,99,50,220,211,227,31,24,23,181,195,94,74,2,153,222,56,121,105,55,131,212,174,111,223,208,196,124,12,40,90,94,6,220,75,174,126,71,86,38,60,251,12,123,23,235,46,225,213,196,219,254,253,173},
{167,130,60,200,96,187,40,113,160,84,38,24,41,222,144,171,225,27,143,56,22,41,127,36,234,121,79,140,113,99,114,84,85,69,215,249,41,254,25,158,209,208,108,9,109,223,220,156,92,80,114,87,64,158,130,146,138,215},
{108,47,174,236,151,205,169,124,151,201,168,91,80,176,57,217,13,166,171,95,96,91,149,209,18,164,249,213,250,137,237,234,214,216,233,152,7,248,2,105,38,58,205,209,27,40,167,68,228,43,79,118,135,82,40,15,50,86},
{41,109,177,142,47,51,187,195,198,206,119,44,158,252,67,181,212,88,228,139,126,213,82,234,75,124,215,142,248,125,35,8,11,167,53,206,180,110,70,132,89,158,220,141,167,220,220,66,176,128,95,118,86,241,187,89,169,89},
{19,197,125,125,53,99,78,4,136,250,18,247,197,225,237,240,253,76,24,40,171,184,10,240,109,99,122,70,10,81,80,6,186,200,177,71,166,34,178,3,46,215,146,89,240,139,115,249,237,110,56,42,186,43,112,120,208,250},
{76,132,133,106,171,24,138,136,53,120,173,40,116,36,165,27,205,167,81,15,168,148,227,184,214,205,123,4,108,148,229,196,244,145,144,100,250,196,117,187,145,44,231,188,176,64,13,231,127,178,180,241,61,83,21,91,102,129},
{48,210,222,175,41,107,16,81,186,110,8,71,8,48,88,203,67,80,210,123,187,227,142,241,160,79,25,237,118,57,179,239,140,1,31,71,136,158,180,190,132,130,153,179,180,177,193,7,102,67,155,145,219,53,159,224,186,35},
{42,92,106,223,29,129,147,140,69,29,215,161,128,231,13,182,77,14,138,211,22,241,200,93,228,145,118,141,106,107,163,117,132,109,128,7,172,160,39,197,49,72,240,155,75,171,149,7,35,200,99,63,144,35,25,126,144,74},
{208,233,31,47,4,122,82,145,161,246,27,245,202,177,213,121,8,131,31,207,85,30,100,142,53,205,170,102,118,16,234,141,112,36,21,182,63,246,166,158,44,135,62,122,72,187,234,187,70,199,128,31,122,67,112,185,130,81},
};

const unsigned char INavBit::Oct2Power[256] = {
   0,   0,   1,  25,   2,  50,  26, 198,   3, 223,  51, 238,  27, 104, 199,  75,
   4, 100, 224,  14,  52, 141, 239, 129,  28, 193, 105, 248, 200,   8,  76, 113,
   5, 138, 101,  47, 225,  36,  15,  33,  53, 147, 142, 218, 240,  18, 130,  69,
  29, 181, 194, 125, 106,  39, 249, 185, 201, 154,   9, 120,  77, 228, 114, 166,
   6, 191, 139,  98, 102, 221,  48, 253, 226, 152,  37, 179,  16, 145,  34, 136,
  54, 208, 148, 206, 143, 150, 219, 189, 241, 210,  19,  92, 131,  56,  70,  64,
  30,  66, 182, 163, 195,  72, 126, 110, 107,  58,  40,  84, 250, 133, 186,  61,
 202,  94, 155, 159,  10,  21, 121,  43,  78, 212, 229, 172, 115, 243, 167,  87,
   7, 112, 192, 247, 140, 128,  99,  13, 103,  74, 222, 237,  49, 197, 254,  24,
 227, 165, 153, 119,  38, 184, 180, 124,  17,  68, 146, 217,  35,  32, 137,  46,
  55,  63, 209,  91, 149, 188, 207, 205, 144, 135, 151, 178, 220, 252, 190,  97,
 242,  86, 211, 171,  20,  42,  93, 158, 132,  60,  57,  83,  71, 109,  65, 162,
  31,  45,  67, 216, 183, 123, 164, 118, 196,  23,  73, 236, 127,  12, 111, 246,
 108, 161,  59,  82,  41, 157,  85, 170, 251,  96, 134, 177, 187, 204,  62,  90,
 203,  89,  95, 176, 156, 169, 160,  81,  11, 245,  22, 235, 122, 117,  44, 215,
 79, 174, 213, 233, 230, 231, 173, 232, 116, 214, 244, 234, 168,  80,  88, 175 };

const unsigned char INavBit::Power2Oct[510] = {
   1,   2,   4,   8,  16,  32,  64, 128,  29,  58, 116, 232, 205, 135,  19,  38,
  76, 152,  45,  90, 180, 117, 234, 201, 143,   3,   6,  12,  24,  48,  96, 192,
 157,  39,  78, 156,  37,  74, 148,  53, 106, 212, 181, 119, 238, 193, 159,  35,
  70, 140,   5,  10,  20,  40,  80, 160,  93, 186, 105, 210, 185, 111, 222, 161,
  95, 190,  97, 194, 153,  47,  94, 188, 101, 202, 137,  15,  30,  60, 120, 240,
 253, 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163,  91, 182, 113, 226,
 217, 175,  67, 134,  17,  34,  68, 136,  13,  26,  52, 104, 208, 189, 103, 206,
 129,  31,  62, 124, 248, 237, 199, 147,  59, 118, 236, 197, 151,  51, 102, 204,
 133,  23,  46,  92, 184, 109, 218, 169,  79, 158,  33,  66, 132,  21,  42,  84,
 168,  77, 154,  41,  82, 164,  85, 170,  73, 146,  57, 114, 228, 213, 183, 115,
 230, 209, 191,  99, 198, 145,  63, 126, 252, 229, 215, 179, 123, 246, 241, 255,
 227, 219, 171,  75, 150,  49,  98, 196, 149,  55, 110, 220, 165,  87, 174,  65,
 130,  25,  50, 100, 200, 141,   7,  14,  28,  56, 112, 224, 221, 167,  83, 166,
  81, 162,  89, 178, 121, 242, 249, 239, 195, 155,  43,  86, 172,  69, 138,   9,
  18,  36,  72, 144,  61, 122, 244, 245, 247, 243, 251, 235, 203, 139,  11,  22,
  44,  88, 176, 125, 250, 233, 207, 131,  27,  54, 108, 216, 173,  71, 142,   1,
   2,   4,   8,  16,  32,  64, 128,  29,  58, 116, 232, 205, 135,  19,  38,  76,
 152,  45,  90, 180, 117, 234, 201, 143,   3,   6,  12,  24,  48,  96, 192, 157,
  39,  78, 156,  37,  74, 148,  53, 106, 212, 181, 119, 238, 193, 159,  35,  70,
 140,   5,  10,  20,  40,  80, 160,  93, 186, 105, 210, 185, 111, 222, 161,  95,
 190,  97, 194, 153,  47,  94, 188, 101, 202, 137,  15,  30,  60, 120, 240, 253,
 231, 211, 187, 107, 214, 177, 127, 254, 225, 223, 163,  91, 182, 113, 226, 217,
 175,  67, 134,  17,  34,  68, 136,  13,  26,  52, 104, 208, 189, 103, 206, 129,
  31,  62, 124, 248, 237, 199, 147,  59, 118, 236, 197, 151,  51, 102, 204, 133,
  23,  46,  92, 184, 109, 218, 169,  79, 158,  33,  66, 132,  21,  42,  84, 168,
  77, 154,  41,  82, 164,  85, 170,  73, 146,  57, 114, 228, 213, 183, 115, 230,
 209, 191,  99, 198, 145,  63, 126, 252, 229, 215, 179, 123, 246, 241, 255, 227,
 219, 171,  75, 150,  49,  98, 196, 149,  55, 110, 220, 165,  87, 174,  65, 130,
  25,  50, 100, 200, 141,   7,  14,  28,  56, 112, 224, 221, 167,  83, 166,  81,
 162,  89, 178, 121, 242, 249, 239, 195, 155,  43,  86, 172,  69, 138,   9,  18,
  36,  72, 144,  61, 122, 244, 245, 247, 243, 251, 235, 203, 139,  11,  22,  44,
  88, 176, 125, 250, 233, 207, 131,  27,  54, 108, 216, 173,  71, 142 };

INavBit::INavBit()
{
	GalSpareData[0] = 0x02000000; GalSpareData[1] = GalSpareData[2] = GalSpareData[3] = 0;
	GalDummyData[0] = 0xfc000000; GalDummyData[1] = GalDummyData[2] = GalDummyData[3] = 0;
	memset(GalEphData, 0, sizeof(GalEphData));
	memset(GalAlmData, 0, sizeof(GalAlmData));
	memset(GalUtcData, 0, sizeof(GalUtcData));
}

INavBit::~INavBit()
{
}

// Param is used to distinguish from E1 and E5b (1 for E1)
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
	TOW = StartTime.MilliSeconds / 1000;
	if ((TOW & 1) == 0 && Param == 1)	// Param == 1 for E1 I/NAV, TOW must be odd number
		TOW --;
	if (TOW < 0)
		TOW += 604800;
	subframe = ((TOW + ((Param == 1) ? 360 : 0)) % 720) / 30;
	page = (TOW % 30) / 2;
	Word = (Param == 1) ? WordAllocationE1[page] : WordAllocationE5[page];
	if (Word > 10) Word = 63;	// temporarily put all word exceed 10 as dummy word
	if ((subframe & 1) && ((Word == 7) || (Word == 8)))	// Word 7/9 and Word 8/10 toggle for different subframe
		Word += 2;
	if ((subframe & 1) && ((Word == 17) || (Word == 19)))	// Word 17/18 and Word 19/20 toggle for different subframe
		Word ++;
	Data = GetWordData(svid, Word, subframe);
	// add WN/TOW for Word 0/5/6
	if (Word == 0)
		Data[3] = ((StartTime.Week - 1024) << 20) + TOW;
	else if (Word == 5)
	{
		Data[2] &= 0xff800000;	// clear 23LSB
		Data[2] |= (((StartTime.Week - 1024) & 0xfff) << 11) + (TOW >> 9);
		Data[3] = TOW << 23;
	}
	else if (Word == 6)
	{
		Data[3] &= 0xff800000;	// clear 23LSB
		Data[3] |= TOW << 3;
	}

	// put into EncodeData to do CRC24Q encoding (totally 196 bits, leaving 28MSB of EncodeData[0] as 0s)
	EncodeData[0] = Data[0] >> 30;	// even/odd=0, page type=0, 2bit Data
	EncodeData[1] = (Data[0] << 2) | (Data[1] >> 30);	// 32bit Data
	EncodeData[2] = (Data[1] << 2) | (Data[2] >> 30);	// 32bit Data
	EncodeData[3] = (Data[2] << 2) | (Data[3] >> 30);	// 32bit Data
	EncodeData[4] = ((Data[3] << 2) & 0xfffc0000) | (Data[3] & 0xffff) | 0x20000;	// 14bit Data, even/odd=1, page type=1, 16bit Data
	EncodeData[5] = 0;	// 32MSB of Reserved 1
	EncodeData[6] = 0;	// 8LSB of Reserved 1, SAR and Spare bits are 0
	CrcResult = Crc24qEncode(EncodeData, 196);

// test encode and interleaving using sample stream in ICD
//	EncodeData[0] = 0xf; EncodeData[1] = 0xff0ccaa0; EncodeData[2] = 0x00f3355e;
//	EncodeData[3] = 0x3ecdf8a1; EncodeData[4] = 0xc1340000;

	// do convolution encode on even part (EncodeData[0] bit3 through EncodeData[4] bit18)
	ConvEncodeBits = 0;
	EncodeWord = EncodeData[0] << 28;	// move to MSB
	for (i = 0, BitCount = 28; i < 114 / 2; i ++)
	{
		EvenPart[i/2] = (EvenPart[i/2] << 4) + GalConvolutionEncode(ConvEncodeBits, EncodeWord);
		BitCount += 2;
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
	if (svid < 1 || svid > 36 || !Eph || !Eph->valid)
		return 0;
	ComposeEphWords(Eph, GalEphData[svid-1]);
	ComposeParityWords(GalEphData[svid-1], GalRsVector[svid-1]);
	return svid;
}

int INavBit::SetAlmanac(GPS_ALMANAC Alm[])
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

int INavBit::SetIonoUtc(PIONO_PARAM IonoParam, PUTC_PARAM UtcParam)
{
	PIONO_NEQUICK IonoGal = (PIONO_NEQUICK)IonoParam;
	signed int IntValue;
	unsigned int UintValue;
	int i;
	unsigned int IonoWords[2];

	UintValue = UnscaleUint(IonoGal->ai0, -2);
	IonoWords[0] = COMPOSE_BITS(UintValue, 15, 11);
	IntValue = UnscaleInt(IonoGal->ai1, -8);
	IonoWords[0] |= COMPOSE_BITS(IntValue, 4, 11);
	IntValue = UnscaleInt(IonoGal->ai2, -15);
	IonoWords[0] |= COMPOSE_BITS(IntValue >> 10, 0, 4);
	IonoWords[1] = COMPOSE_BITS(IntValue, 22, 10);
	IonoWords[1] |= COMPOSE_BITS(IonoGal->flag, 17, 5);

	// put ai0~ai2 into Word 5
	for (i = 0; i < 36; i ++)
	{
		GalEphData[i][16] &= 0xfc000000; GalEphData[i][16] |= IonoWords[0];
		GalEphData[i][17] &= 0x0001ffff; GalEphData[i][17] |= IonoWords[1];
	}

	// Word 6 for UTC parameters
	GalUtcData[0] = 0x18000000;	// put Type=6 to 6MSB
	IntValue = UnscaleInt(UtcParam->A0, -30);
	GalUtcData[0] |= COMPOSE_BITS(IntValue >> 6, 0, 26);
	GalUtcData[1] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(UtcParam->A1, -50);
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

// Word Type 1~5 each has 128 content bits fill EphData[] sequencially
// each 128bit has the order MSB first and least index first
int INavBit::ComposeEphWords(PGPS_EPHEMERIS Ephemeris, unsigned int *EphData)
{
	signed int IntValue;
	unsigned int UintValue;

	// Word 1
	EphData[0] = 0x04000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10) | COMPOSE_BITS((Ephemeris->toe / 60), 2, 14);
	IntValue = UnscaleInt(Ephemeris->M0 / PI, -31);
	EphData[0] |= COMPOSE_BITS(IntValue >> 30, 0, 2);
	EphData[1] = COMPOSE_BITS(IntValue, 2, 30);
	UintValue = UnscaleUint(Ephemeris->ecc, -33);
	EphData[1] |= COMPOSE_BITS(UintValue >> 30, 0, 2);
	EphData[2] = COMPOSE_BITS(UintValue, 2, 30);
	UintValue = UnscaleUint(Ephemeris->sqrtA, -19);
	EphData[2] |= COMPOSE_BITS(UintValue >> 30, 0, 2);
	EphData[3] = COMPOSE_BITS(UintValue, 2, 30);

	// Word 2
	EphData[4] = 0x08000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10);
	IntValue = UnscaleInt(Ephemeris->omega0 / PI, -31);
	EphData[4] |= COMPOSE_BITS(IntValue >> 16, 0, 16);
	EphData[5] = COMPOSE_BITS(IntValue, 16, 16);
	IntValue = UnscaleInt(Ephemeris->i0 / PI, -31);
	EphData[5] |= COMPOSE_BITS(IntValue >> 16, 0, 16);
	EphData[6] = COMPOSE_BITS(IntValue, 16, 16);
	IntValue = UnscaleInt(Ephemeris->w / PI, -31);
	EphData[6] |= COMPOSE_BITS(IntValue >> 16, 0, 16);
	EphData[7] = COMPOSE_BITS(IntValue, 16, 16);
	IntValue = UnscaleInt(Ephemeris->idot / PI, -43);
	EphData[7] |= COMPOSE_BITS(IntValue, 2, 14);

	// Word 3
	EphData[8] = 0x0c000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10);
	IntValue = UnscaleInt(Ephemeris->omega_dot / PI, -43);
	EphData[8] |= COMPOSE_BITS(IntValue >> 8, 0, 16);
	EphData[9] = COMPOSE_BITS(IntValue, 24, 8);
	IntValue = UnscaleInt(Ephemeris->delta_n / PI, -43);
	EphData[9] |= COMPOSE_BITS(IntValue, 8, 16);
	IntValue = UnscaleInt(Ephemeris->cuc, -29);
	EphData[9] |= COMPOSE_BITS(IntValue >> 8, 0, 8);
	EphData[10] = COMPOSE_BITS(IntValue, 24, 8);
	IntValue = UnscaleInt(Ephemeris->cus, -29);
	EphData[10] |= COMPOSE_BITS(IntValue, 8, 16);
	IntValue = UnscaleInt(Ephemeris->crc, -5);
	EphData[10] |= COMPOSE_BITS(IntValue >> 8, 0, 8);
	EphData[11] = COMPOSE_BITS(IntValue, 24, 8);
	IntValue = UnscaleInt(Ephemeris->crs, -5);
	EphData[11] |= COMPOSE_BITS(IntValue, 8, 16);
	EphData[11] |= COMPOSE_BITS(Ephemeris->ura, 0, 8);

	// Word 4
	EphData[12] = 0x10000000 | COMPOSE_BITS(Ephemeris->iodc, 16, 10) | COMPOSE_BITS(Ephemeris->svid, 10, 6);
	IntValue = UnscaleInt(Ephemeris->cic, -29);
	EphData[12] |= COMPOSE_BITS(IntValue >> 6, 0, 10);
	EphData[13] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(Ephemeris->cis, -29);
	EphData[13] |= COMPOSE_BITS(IntValue, 10, 16);
	UintValue = Ephemeris->toc / 60;
	EphData[13] |= COMPOSE_BITS(UintValue >> 4, 0, 10);
	EphData[14] = COMPOSE_BITS(UintValue, 28, 4);
	IntValue = UnscaleInt(Ephemeris->af0, -34);
	EphData[14] |= COMPOSE_BITS(IntValue >> 3, 0, 28);
	EphData[15] = COMPOSE_BITS(IntValue, 29, 3);
	IntValue = UnscaleInt(Ephemeris->af1, -46);
	EphData[15] |= COMPOSE_BITS(IntValue, 8, 21);
	IntValue = UnscaleInt(Ephemeris->af2, -59);
	EphData[15] |= COMPOSE_BITS(IntValue, 2, 6);

	// Word 5
	EphData[16] &= 0x03ffffff; EphData[16] = 0x14000000;	// put Type=5 to 6MSB
	IntValue = UnscaleInt(Ephemeris->tgd, -32);
	EphData[17] = COMPOSE_BITS(IntValue, 7, 10);
	IntValue = UnscaleInt(Ephemeris->tgd2, -32);
	EphData[17] |= COMPOSE_BITS(IntValue >> 3, 0, 7);
	EphData[18] = COMPOSE_BITS(IntValue, 29, 3);
	IntValue = Ephemeris->health;
	EphData[18] |= COMPOSE_BITS(IntValue >> 7, 27, 2);	// E5b HS
	EphData[18] |= COMPOSE_BITS(IntValue >> 1, 25, 2);	// E1B HS
	EphData[18] |= COMPOSE_BITS(IntValue >> 5, 24, 1);	// E5b DVS
	EphData[18] |= COMPOSE_BITS(IntValue, 23, 1);		// E1B DVS

	return 0;
}

int INavBit::ComposeAlmWords(GPS_ALMANAC Almanac[], unsigned int *AlmData, int week)
{
	signed int IntValue;
	unsigned int UintValue;
	int toa = (Almanac[0].valid & 1) ? Almanac[0].toa : (Almanac[1].valid & 1) ? Almanac[1].toa : (Almanac[2].valid & 1) ? Almanac[2].toa : 0;

	// Word 7
	AlmData[0] = 0x1d000000 | COMPOSE_BITS(week, 20, 2) | COMPOSE_BITS((toa / 600), 10, 10) | COMPOSE_BITS(Almanac[0].svid, 4, 6);	// Type=8, IODa=4
	IntValue = UnscaleInt(Almanac[0].sqrtA - SQRT_A0, -9);	// SVID1 starts here
	AlmData[0] |= COMPOSE_BITS(IntValue >> 9, 0, 4);
	AlmData[1] = COMPOSE_BITS(IntValue, 23, 9);
	UintValue = UnscaleUint(Almanac[0].ecc, -16);
	AlmData[1] |= COMPOSE_BITS(UintValue, 12, 11);
	IntValue = UnscaleInt(Almanac[0].w / PI, -15);
	AlmData[1] |= COMPOSE_BITS(IntValue >> 4, 0, 12);
	AlmData[2] = COMPOSE_BITS(IntValue, 28, 4);
	IntValue = UnscaleInt((Almanac[0].i0 - NORMINAL_I0) / PI, -14);
	AlmData[2] |= COMPOSE_BITS(IntValue, 17, 11);
	IntValue = UnscaleInt(Almanac[0].omega0 / PI, -15);
	AlmData[2] |= COMPOSE_BITS(IntValue, 1, 16);
	IntValue = UnscaleInt(Almanac[0].omega_dot / PI, -33);
	AlmData[2] |= COMPOSE_BITS(IntValue >> 10, 0, 1);
	AlmData[3] = COMPOSE_BITS(IntValue, 22, 10);
	IntValue = UnscaleInt(Almanac[0].M0 / PI, -15);
	AlmData[3] |= COMPOSE_BITS(IntValue, 6, 16);

	// Word 8
	AlmData[4] = 0x21000000;	// Type=8, IODa=4
	IntValue = UnscaleInt(Almanac[0].af0, -19);
	AlmData[4] |= COMPOSE_BITS(IntValue, 6, 16);
	IntValue = UnscaleInt(Almanac[0].af1, -38);
	AlmData[4] |= COMPOSE_BITS(IntValue >> 7, 0, 6);
	AlmData[5] = COMPOSE_BITS(IntValue, 25, 7);
	AlmData[5] |= COMPOSE_BITS((Almanac[0].valid & 1) ? 0 : 5, 21, 4);
	AlmData[5] |= COMPOSE_BITS(Almanac[1].svid, 15, 6);	// SVID2 starts here
	IntValue = UnscaleInt(Almanac[1].sqrtA - SQRT_A0, -9);
	AlmData[5] |= COMPOSE_BITS(IntValue, 2, 13);
	UintValue = UnscaleUint(Almanac[1].ecc, -16);
	AlmData[5] |= COMPOSE_BITS(UintValue >> 9, 0, 2);
	AlmData[6] = COMPOSE_BITS(UintValue, 23, 9);
	IntValue = UnscaleInt(Almanac[1].w / PI, -15);
	AlmData[6] |= COMPOSE_BITS(IntValue, 7, 16);
	IntValue = UnscaleInt((Almanac[1].i0 - NORMINAL_I0) / PI, -14);
	AlmData[6] |= COMPOSE_BITS(IntValue >> 4, 0, 7);
	AlmData[7] = COMPOSE_BITS(IntValue, 28, 4);
	IntValue = UnscaleInt(Almanac[1].omega0 / PI, -15);
	AlmData[7] |= COMPOSE_BITS(IntValue, 12, 16);
	IntValue = UnscaleInt(Almanac[1].omega_dot / PI, -33);
	AlmData[7] |= COMPOSE_BITS(IntValue, 1, 11);

	// Word 9
	AlmData[8] = 0x25000000 | COMPOSE_BITS(week, 20, 2) | COMPOSE_BITS((toa / 600), 10, 10);	// Type=9, IODa=4
	IntValue = UnscaleInt(Almanac[1].M0 / PI, -15);
	AlmData[8] |= COMPOSE_BITS(IntValue >> 6, 0, 10);
	AlmData[9] = COMPOSE_BITS(IntValue, 26, 6);
	IntValue = UnscaleInt(Almanac[1].af0, -19);
	AlmData[9] |= COMPOSE_BITS(IntValue, 10, 16);
	IntValue = UnscaleInt(Almanac[1].af1, -38);
	AlmData[9] |= COMPOSE_BITS(IntValue >> 3, 0, 10);
	AlmData[10] = COMPOSE_BITS(IntValue, 29, 3);
	AlmData[10] |= COMPOSE_BITS((Almanac[1].valid & 1) ? 0 : 5, 25, 4);
	AlmData[10] |= COMPOSE_BITS(Almanac[2].svid, 19, 6);	// SVID3 starts here
	IntValue = UnscaleInt(Almanac[2].sqrtA - SQRT_A0, -9);
	AlmData[10] |= COMPOSE_BITS(IntValue, 6, 13);
	UintValue = UnscaleUint(Almanac[2].ecc, -16);
	AlmData[10] |= COMPOSE_BITS(UintValue >> 5, 0, 6);
	AlmData[11] = COMPOSE_BITS(UintValue, 27, 5);
	IntValue = UnscaleInt(Almanac[2].w / PI, -15);
	AlmData[11] |= COMPOSE_BITS(IntValue, 11, 16);
	IntValue = UnscaleInt((Almanac[2].i0 - NORMINAL_I0) / PI, -14);
	AlmData[11] |= COMPOSE_BITS(IntValue, 0, 11);

	// Word 10
	AlmData[12] = 0x29000000;	// Type=10, IODa=4
	IntValue = UnscaleInt(Almanac[2].omega0 / PI, -15);
	AlmData[12] |= COMPOSE_BITS(IntValue, 6, 16);
	IntValue = UnscaleInt(Almanac[2].omega_dot / PI, -33);
	AlmData[12] |= COMPOSE_BITS(IntValue >> 5, 0, 6);
	AlmData[13] = COMPOSE_BITS(IntValue, 27, 5);
	IntValue = UnscaleInt(Almanac[2].M0 / PI, -15);
	AlmData[13] |= COMPOSE_BITS(IntValue, 11, 16);
	IntValue = UnscaleInt(Almanac[2].af0, -19);
	AlmData[13] |= COMPOSE_BITS(IntValue >> 5, 0, 11);
	AlmData[14] = COMPOSE_BITS(IntValue, 27, 5);
	IntValue = UnscaleInt(Almanac[2].af1, -38);
	AlmData[14] |= COMPOSE_BITS(IntValue, 14, 13);
	AlmData[14] |= COMPOSE_BITS((Almanac[2].valid & 1) ? 0 : 5, 10, 4);
	AlmData[15] = 0;

	return 0;
}

unsigned int *INavBit::GetWordData(int svid, int Word, int subframe)
{
	switch (Word)
	{
	case 0:
		return GalSpareData;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
		return GalEphData[svid-1] + 4 * (Word - 1);
	case 6:
		return GalUtcData;
	case 7:
	case 8:
	case 9:
	case 10:
		return GalAlmData[subframe/2] + 4 * (Word - 7);
	case 17:
	case 18:
	case 19:
	case 20:
		return GalRsVector[svid-1] + 4 * (Word - 17);
	case 63:
		return GalDummyData;
	default:
		return GalDummyData;
	}
}

// encode 2MSB of EncodeWord, left shift EncodeWord and update ConvEncodeBits
unsigned char INavBit::GalConvolutionEncode(unsigned char &ConvEncodeBits, unsigned int &EncodeWord)
{
	ConvEncodeBits = (ConvEncodeBits << 2) + (unsigned char)(EncodeWord >> 30);
	EncodeWord <<= 2;
	return ((ConvolutionEncode(ConvEncodeBits) & 0xf) ^ 0x5);	// invert G2
}

unsigned char INavBit::GF8IntMul(unsigned char a, unsigned char b)
{
	if (a && b)
		return Power2Oct[Oct2Power[a] + Oct2Power[b]];
	else
		return 0;
}

void INavBit::GenerateParityVector(unsigned char InformationVector[58], unsigned char ParityVector[60])
{
	int i, j;

	for (i = 0; i < 60; i ++)
	{
		ParityVector[i] = 0;
		for (j = 0; j < 58; j ++)
			ParityVector[i] ^= GF8IntMul(RsGenerateMatrix[i][j], InformationVector[j]);
	}
}

void INavBit::ComposeParityWords(unsigned int *EphData, unsigned int *ParityData)
{
	unsigned char Iod, InformationVector[58], ParityVector[60];
	int i, j;

	// put word1~4 data into InformationVector
	Iod = (EphData[0] >> 16) & 3;	// 2LSB of IOD
	InformationVector[0] = 0x4 | Iod;	// Type=1 plus 2LSB of IOD
	InformationVector[1] = (unsigned char)(EphData[0] >> 18);	// 8MSB of IOD
	for (i = 0; i < 4; i ++)
	{
		InformationVector[i*14+2] = (unsigned char)(EphData[i*4] >> 8);
		InformationVector[i*14+3] = (unsigned char)EphData[i*4];
		for (j = 1; j < 4; j ++)
		{
			InformationVector[i*14+j*4  ] = (unsigned char)(EphData[i*4+j] >> 24);
			InformationVector[i*14+j*4+1] = (unsigned char)(EphData[i*4+j] >> 16);
			InformationVector[i*14+j*4+2] = (unsigned char)(EphData[i*4+j] >> 8);
			InformationVector[i*14+j*4+3] = (unsigned char)EphData[i*4+j];
		}
	}
	// generate parity vector octets
	GenerateParityVector(InformationVector, ParityVector);
	// put parity vector into word17~20
	for (i = 0; i < 4; i ++)
	{
		ParityData[i*4] = ((17 + i) << 26) + (Iod << 24);
		ParityData[i*4] |= (((unsigned int)ParityVector[i*15]) << 16);
		ParityData[i*4] |= (((unsigned int)ParityVector[i*15+1]) << 8);
		ParityData[i*4] |= ParityVector[i*15+2];
		for (j = 1; j < 4; j ++)
		{
			ParityData[i*4+j]  = (((unsigned int)ParityVector[i*15+j*4-1]) << 24);
			ParityData[i*4+j] |= (((unsigned int)ParityVector[i*15+j*4+0]) << 16);
			ParityData[i*4+j] |= (((unsigned int)ParityVector[i*15+j*4+1]) << 8);
			ParityData[i*4+j] |= ParityVector[i*15+j*4+2];
		}
	}
}
