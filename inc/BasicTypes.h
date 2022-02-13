//----------------------------------------------------------------------
// BasicTypes.h:
//   Definition of basic types commonly used
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __BASIC_TYPE_H__
#define __BASIC_TYPE_H__

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#ifndef NULL
#define NULL (0)
#endif

enum GnssSystem { GpsSystem, BdsSystem, GalileoSystem, GlonassSystem };

typedef struct
{
	double ve, vn, vu;
	double speed, course;
} LOCAL_SPEED, *PLOCAL_SPEED;

typedef struct
{
	union
	{
		struct
		{
			double x, y, z;
			double vx, vy, vz;
		};
		double PosVel[6];
	};
} KINEMATIC_INFO, *PKINEMATIC_INFO;

typedef struct
{
	double lat;
	double lon;
	double alt;
} LLA_POSITION, *PLLA_POSITION;

typedef struct
{
	double x2e, y2e;
	double x2n, y2n, z2n;
	double x2u, y2u, z2u;
} CONVERT_MATRIX, *PCONVERT_MATRIX;

typedef struct
{
	int Year;
	int Month;
	int Day;
	int Hour;
	int Minute;
	double Second;
} UTC_TIME, *PUTC_TIME;

typedef struct
{
	int Week;
	int MilliSeconds;
	double SubMilliSeconds;
} GNSS_TIME, *PGNSS_TIME;

typedef struct
{
	int LeapYear;
	int Day;
	int MilliSeconds;
	double SubMilliSeconds;
} GLONASS_TIME, *PGLONASS_TIME;

typedef struct // GPS ephemeris, also used by BDS and Galileo
{
	unsigned short	iodc;
	unsigned char	iode2;
	unsigned char	iode3;

	unsigned char	ura;	// URA in 4LSB, code on L2 in bit4/5, L2 channel code in bit6, fit interval flag in bit7
	unsigned char	flag;	// bit0 means ephemeris valid
	unsigned char	health;
	unsigned char	svid;	// satellite PRN number starting from 1

	int	toe;
	int	toc;
	int	week;

	// parameter
	double M0;			// Mean Anomaly at Reference Time
	double delta_n;		// Mean Motion Difference from Computed Value
	double ecc;			// Eccentricity
	double sqrtA;		// Square Root of the Semi-Major Axis
	double axis_dot;	// Change rate of axis, valid for L1C/B1C
	double omega0;		// Longitude of Ascending Node of Orbit Plane at Weekly Epoch
	double i0;			// Inclination Angle at Reference Time
	double w;			// Argument of Perigee
	double omega_dot;	// Rate of Right Ascension
	double idot;		// Rate of Inclination Angle
	double cuc;			// Amplitude of the Cosine Harmonic Correction Term to the Argument of Latitude
	double cus;			// Amplitude of the Sine Harmonic Correction Term to the Argument of Latitude
	double crc;			// Amplitude of the Cosine Harmonic Correction Term to the Orbit Radius
	double crs;			// Amplitude of the Sine Harmonic Correction Term to the Orbit Radius
	double cic;			// Amplitude of the Cosine Harmonic Correction Term to the Angle of Inclination
	double cis;			// Amplitude of the Sine Harmonic Correction Term to the Angle of Inclination
	double tgd;			// Group Delay
	double tgd2;		// Group Delay for secondary frequency (eg. B2 in BDS)
	double af0;			// Satellite Clock Correction
	double af1;			// Satellite Clock Correction
	double af2;			// Satellite Clock Correction

	// variables derived from basic data, to avoid calculate every time
	double axis;		// Semi-major Axis of Orbit, equals to sqrtA^2
	double n;			// Corrected Mean Angular Rate, equals to WGS_SQRT_GM/sqrtA^3 + delta_n
	double root_ecc;	// Square Root of One Minus Ecc Square, equals to sqrt(1-ecc^2)
	double omega_t;		// Longitude of Ascending Node of Orbit Plane at toe, equals to omega0 - WGS_OMEGDOTE * toe
	double omega_delta;	// Delta Between omega_dot and WGS_OMEGDOTE, equals to omega_dot - WGS_OMEGDOTE
	double Ek;			// Ek, derived from Mk
	double Ek_dot;		// change rate of Ek
} GPS_EPHEMERIS, *PGPS_EPHEMERIS;

typedef struct        			
{
	unsigned char	flag;
	unsigned char	dummy;	// IOD in Galileo
	unsigned char	health;	// for Galileo, bit5/4 E5aHS, bit3/2 E5bHS, bit1/0 E1BHS
	unsigned char	svid;

	int	toa;
	int	week;

	// variables decoded from stream data
	double M0;			// Mean Anomaly at Reference Time
	double ecc;			// Eccentricity
	double sqrtA;		// Square Root of the Semi-Major Axis
	double omega0;		// Longitude of Ascending Node of Orbit Plane at Weekly Epoch
	double i0;			// Inclination Angle at Reference Time
	double w;			// Argument of Perigee
	double omega_dot;	// Rate of Right Ascension
	double af0;			// Satellite Clock Correction
	double af1;			// Satellite Clock Correction
} GPS_ALMANAC, *PGPS_ALMANAC;

typedef struct
{
	unsigned char flag;	// bit0 means ephemeris valid
						// bit1 means position and velocity at tc valid (avoid extrapolation from beginning)
	signed char freq;	// frequency number of satellite
	unsigned char P;	// place P1, P2, P3, P4, ln, P from LSB at bit
						//      0/1,  2,  3,  4,  5, 6
	unsigned char M;	// satellite type 00 - GLONASS, 01 - GLONASS-M
	unsigned char Ft;	// indicator of accuracy of measurements
	unsigned char n;	// slot number that transmit signal
	unsigned char Bn;	// healthy flag
	unsigned char En;	// age of the immediate information
	unsigned int tb;	// index of 15 minutes interval within current day
	unsigned short day;	// day number of the leap year corresponding to tb
	unsigned short tk;  // hour:b11~b7, minute:b6~b1, second:b0 * 30
	double gamma;		// relative deviation of predicted carrier frequency
	double tn;			// satellite clock error
	double dtn;			// time difference between L1 and L2
	double x, y, z;		// posistion in PZ-90 at instant tb
	double vx, vy, vz;	// velocity in PZ-90 at instant tb
	double ax, ay, az;	// acceleration in PZ-90 at instant tb

	// derived variables
	double tc;			// reference time giving the following position and velocity
	KINEMATIC_INFO PosVelT;	// position and velocity in CIS coordinate at instant tc
} GLONASS_EPHEMERIS, *PGLONASS_EPHEMERIS;

typedef struct
{
    double	a0;  // 2**-30
    double	a1;  // 2**-27
    double	a2;  // 2**-24
    double	a3;  // 2**-24
    double	b0;  // 2**11
    double	b1;  // 2**14
    double	b2;  // 2**16
    double	b3;  // 2**16
    unsigned long	flag; // bit0:1, availble, bit8~13: svid
} IONO_PARAM, *PIONO_PARAM;

// UTC parameters
typedef  struct _PACKED_
{
	double	A0;  // second, 2**-30
	double	A1;  // second/second, 2**-50
	short	WN;
	short	WNLSF;
	unsigned char	tot; // 2**12 for GPS/BDS and 3600 for Galileo
	signed char	TLS; // leap second
	signed char	TLSF;
	unsigned char	DN;
	unsigned long	flag; // bit0: UTC parameter available, bit1: leap second available
} UTC_PARAM, *PUTC_PARAM;

typedef struct
{
	int system;
	int svid;
	double PseudoRange;
	double CarrierPhase;
	double Doppler;
	double CN0;
} SAT_OBSERVATION, *PSAT_OBSERVATION;

#endif //__BASIC_TYPE_H__
