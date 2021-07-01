//----------------------------------------------------------------------
// CommonOps.cpp:
//   Common operation class and function implementation
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#include "CommonOps.h"

int __builtin_popcount(unsigned int data)
{
	data = (data & 0x55555555) + ((data >> 1) & 0x55555555);
	data = (data & 0x33333333) + ((data >> 2) & 0x33333333);
	data = (data & 0x0f0f0f0f) + ((data >> 4) & 0x0f0f0f0f);
	data = (data & 0x00ff00ff) + ((data >> 8) & 0x00ff00ff);
	data = (data & 0x0000ffff) + ((data >> 16) & 0x0000ffff);
	return data;
}

int __builtin_clz(unsigned int data)
{
	if (data == 0)
		return 32;
	data |= data >> 1;
	data |= data >> 2;
	data |= data >> 4;
	data |= data >> 8;
	data |= data >> 16;
	return 32 - __builtin_popcount(data);
}

complex_int complex_int::operator + (complex_int data) { return complex_int(this->real+data.real, this->imag+data.imag); }
void complex_int::operator += (complex_int data) { this->real += data.real; this->imag += data.imag; }
complex_int complex_int::operator - (complex_int data) { return complex_int(this->real-data.real, this->imag-data.imag); }
void complex_int::operator -= (complex_int data) { this->real -= data.real; this->imag -= data.imag; }
complex_int complex_int::operator * (complex_int data)
{
	complex_int result;
	result.real = (this->real) * (data.real) - (this->imag) * (data.imag);
	result.imag = (this->real) * (data.imag) + (this->imag) * (data.real);
	return result;
}
void complex_int::operator *= (complex_int data)
{
	int temp = this->real;
	
	this->real = (this->real) * (data.real) - (this->imag) * (data.imag);
	this->imag = (temp) * (data.imag) + (this->imag) * (data.real);
}
complex_int complex_int::operator * (int data) { return complex_int((this->real)*data, (this->imag)*data); }
void complex_int::operator *= (int data) { this->real *= data; this->imag *= data; }

complex_int complex_int::operator - () { return complex_int(-(this->real), -(this->imag)); }
complex_int complex_int::operator ~ () { return complex_int(~(this->real), ~(this->imag)); }
