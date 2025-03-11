//----------------------------------------------------------------------
// ComplexNumber.h:
//   Definition of complex number and operators
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined (__COMPLEX_NUMBER_H__)
#define __COMPLEX_NUMBER_H__

class complex_number
{
public:
	double real;
	double imag;

	complex_number() { real = imag = 0.; };
	complex_number(double real_part, double imag_part);
	complex_number operator + (const complex_number data) const;
	void operator += (const complex_number data);
	complex_number operator - (const complex_number data) const;
	void operator -= (const complex_number data);
	complex_number operator * (const complex_number data) const;
	void operator *= (const complex_number data);
	complex_number operator * (const double data);
	void operator *= (const double data);
	double abs();
	complex_number conj();
};

#endif //!defined(__COMPLEX_NUMBER_H__)
