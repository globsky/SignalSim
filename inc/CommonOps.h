//----------------------------------------------------------------------
// CommonOps.h:
//   Common operation class, function and macro definition
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __COMMON_OPS_H__
#define __COMMON_OPS_H__

// define of types
typedef int reg_int;
typedef unsigned int reg_uint;
typedef signed int S32;
typedef unsigned int U32;
typedef signed short S16;
typedef unsigned short U16;
typedef signed char S8;
typedef unsigned char U8;

struct complex_int {
    int real;
    int imag;

	complex_int() {};
	complex_int(int real_value, int imag_value) { real = real_value; imag = imag_value; }
	complex_int operator + (complex_int data);
	void operator += (complex_int data);
	complex_int operator - (complex_int data);
	void operator -= (complex_int data);
	complex_int operator * (complex_int data);
	void operator *= (complex_int data);
	complex_int operator * (int data);
	void operator *= (int data);
	complex_int operator - ();
	complex_int operator ~ ();
};

///////////////////// bit operation
#define BIT_MASK(n) ((1 << (n)) - 1)
#define EXTRACT_UINT(data, start_bit, length) ((unsigned int)((data >> start_bit) & BIT_MASK(length)))
#define EXTRACT_INT(data, start_bit, length) ((signed int)(data << (32 - start_bit - length)) >> (32 - length))

///////////////////// round operation
// bit position to be round up
#define ROUND_BIT(shift_bit) (1 << (shift_bit-1))
// bit value at round up position
#define ROUND_FLAG(data, shift_bit) ((shift_bit == 0) ? 0 : ((data & ROUND_BIT(shift_bit)) ? 1 : 0))
// mask to determine convergent round (shift_bit+1 number of 1s)
#define ROUND_MASK_BIT(shift_bit) ((1 << (shift_bit+1)) - 1)
// flag to indicate it is an exact even number + 0.5 (for shift 4 bit case, whether last 5bit is 01000)
#define ROUND_EVEN_FLAG(data, shift_bit) (((data & ROUND_MASK_BIT(shift_bit)) ^ ROUND_BIT(shift_bit)) == 0)
// normal round shift
#define ROUND_SHIFT_RAW(data, shift_bit) ((data >> shift_bit) + ROUND_FLAG(data, shift_bit))
// convergent round shift
#define CONVERGENT_ROUND_SHIFT(data, shift_bit) ((data >> shift_bit) + (ROUND_EVEN_FLAG(data, shift_bit) ? 0 : ROUND_FLAG(data, shift_bit)))
// simplified convergent round shift 1bit
#define UNBIASED_SHIFT(data) ((((data) & 0x3) == 3) ? (((data) + 1) >> 1) : ((data) >> 1))

#define SATURATE(data, bit) \
do { \
	if ((data) > ((1 << (bit-1)) - 1)) \
		data = ((1 << (bit-1)) - 1); \
	else if ((data) < (-(1 << (bit-1)))) \
		data = (-(1 << (bit-1))); \
} while(0)

int __builtin_popcount(unsigned int data);
int __builtin_clz(unsigned int data);

#endif //__COMMON_OPS_H__
