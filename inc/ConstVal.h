//----------------------------------------------------------------------
// ConstVal.h:
//   Definition of constant values
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#if !defined(__CONST_VALUES_H__)
#define __CONST_VALUES_H__

// constant value for generally use
#define LIGHT_SPEED 299792458
#define PI 3.14159265358979323846264338328
#define PI2 (2 * PI)
#define RAD2DEG(r) (r * (180. / PI))
#define DEG2RAD(d) (d * (PI / 180.))

// constant values of WGS84
#define WGS_PI			3.1415926535898		// WGS 84 value of pi
#define WGS_AXIS_A		6378137.0				// A - WGS-84 earth's semi major axis
#define	WGS_AXIS_B		6356752.3142451795		// B - WGS-84 earth's semi minor axis
#define WGS_E1_SQR		0.006694379990141317	// 1-(B/A)^2, 1st numerical eccentricity
#define WGS_E2_SQR		0.006739496742276435	// (A/B)^2-1, 2nd numerical eccentricity
#define WGS_SQRT_GM		19964981.8432173887		// square root of GM
#define WGS_OMEGDOTE	7.2921151467e-5			// earth rotate rate
#define WGS_F_GTR		-4.442807633e-10		// factor of general theory of relativity

// constant values of CGS2000
#define CGCS2000_SQRT_GM	19964980.3856652962	// square root of GM
#define CGCS2000_OMEGDOTE	7.292115e-5			// earth rotate rate

// constant values of PZ90
#define PZ90_AE			6378136.0				// PZ90 Ae
#define PZ90_AE2		(PZ90_AE * PZ90_AE)		// square of PZ90 Ae
#define PZ90_GM			3.9860044e+14			// PZ90 gravitational constant
#define PZ90_C20		1082.63e-6				// Second zonal coefficient of spherical harmonic expansion
#define PZ90_C20AE2		(PZ90_C20 * PZ90_AE2)	// product of PZ90_C20 and PZ90_AE2
#define PZ90_OMEGDOTE	7.292115e-5				// earth rotate rate

#endif //!defined(__CONST_VALUES_H__)
