#ifndef FAST_MATH_H
#define FAST_MATH_H

#include <cmath>
#include <cstdint>
#include "ConstVal.h"
#include "ComplexNumber.h"

#ifdef _MSC_VER
    #define FORCE_INLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE inline __attribute__((always_inline))
#else
    #define FORCE_INLINE inline
#endif

// Fast math functions for signal generation optimization

class FastMath {
public:
    // Lookup table size - must be power of 2 for fast modulo
    static constexpr int TRIG_LUT_SIZE = 65536;
    static constexpr double TRIG_LUT_SCALE = TRIG_LUT_SIZE / PI2;
    
private:
    // Static lookup tables
    static double sin_lut[TRIG_LUT_SIZE];
    static double cos_lut[TRIG_LUT_SIZE];
    static bool lut_initialized;
    
    // Initialize lookup tables
    static void InitializeLUT() {
        if (!lut_initialized) {
            for (int i = 0; i < TRIG_LUT_SIZE; i++) {
                double angle = (PI2 * i) / TRIG_LUT_SIZE;
                sin_lut[i] = std::sin(angle);
                cos_lut[i] = std::cos(angle);
            }
            lut_initialized = true;
        }
    }

public:
    // Fast sine using lookup table - force inline for performance
    static FORCE_INLINE double FastSin(double angle) {
        if (!lut_initialized) InitializeLUT();
        
        // Normalize angle to [0, 2*PI)
        angle = std::fmod(angle, PI2);
        if (angle < 0) angle += PI2;
        
        // Convert to table index
        int index = static_cast<int>(angle * TRIG_LUT_SCALE) & (TRIG_LUT_SIZE - 1);
        return sin_lut[index];
    }
    
    // Fast cosine using lookup table - force inline for performance
    static FORCE_INLINE double FastCos(double angle) {
        if (!lut_initialized) InitializeLUT();
        
        // Normalize angle to [0, 2*PI)
        angle = std::fmod(angle, PI2);
        if (angle < 0) angle += PI2;
        
        // Convert to table index
        int index = static_cast<int>(angle * TRIG_LUT_SCALE) & (TRIG_LUT_SIZE - 1);
        return cos_lut[index];
    }
    
    // Fast complex rotation using lookup tables - force inline for performance
    static FORCE_INLINE complex_number FastRotate(double angle) {
        if (!lut_initialized) InitializeLUT();
        
        // Normalize angle to [0, 2*PI)
        angle = std::fmod(angle, PI2);
        if (angle < 0) angle += PI2;
        
        // Convert to table index
        int index = static_cast<int>(angle * TRIG_LUT_SCALE) & (TRIG_LUT_SIZE - 1);
        return complex_number(cos_lut[index], sin_lut[index]);
    }
    
    // Fast noise generation using Box-Muller with cached values
    static complex_number FastGaussianNoise(double sigma) {
        static bool hasSpare = false;
        static double spare;
        
        if (hasSpare) { // this will result the Gauss noise only has real part on every other sample
            hasSpare = false;
            return complex_number(spare * sigma, 0);
        }
        
        double u1, u2, mag;
        do {
            u1 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
            u2 = 2.0 * ((double)rand() / RAND_MAX) - 1.0;
            mag = u1 * u1 + u2 * u2;
        } while (mag >= 1.0 || mag == 0.0);
        
        double factor = std::sqrt(-2.0 * std::log(mag) / mag);
        spare = u2 * factor;
        hasSpare = true;
        
        return complex_number(u1 * factor * sigma, u2 * factor * sigma);
    }
    
    // Batch noise generation for better cache efficiency
    static void GenerateNoiseBlock(complex_number* output, int count, double sigma) {
        for (int i = 0; i < count; i++) {
            output[i] = FastGaussianNoise(sigma);
        }
    }
};

#endif // FAST_MATH_H