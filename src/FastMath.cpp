#include "../inc/FastMath.h"

// Static member definitions
double FastMath::sin_lut[FastMath::TRIG_LUT_SIZE + TRIG_LUT_SIZE / 4];
//double FastMath::cos_lut[FastMath::TRIG_LUT_SIZE];
bool FastMath::lut_initialized = false;
