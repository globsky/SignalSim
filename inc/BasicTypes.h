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

typedef enum { GpsSystem, BdsSystem, GalileoSystem, GlonassSystem, SbasSystem, QzssSystem, NavICSystem } GnssSystem;

// signal index for different GNSS system
#define SIGNAL_INDEX_L1CA			0
#define SIGNAL_INDEX_L1C			1
#define SIGNAL_INDEX_L2C			2
#define SIGNAL_INDEX_L2P			3
#define SIGNAL_INDEX_L5				4

#define SIGNAL_INDEX_B1C			0
#define SIGNAL_INDEX_B1I			1
#define SIGNAL_INDEX_B2I			2
#define SIGNAL_INDEX_B3I			3
#define SIGNAL_INDEX_B2a			4
#define SIGNAL_INDEX_B2b			5
#define SIGNAL_INDEX_B2ab			6
#define SIGNAL_INDEX_B1X			7

#define SIGNAL_INDEX_E1				0
#define SIGNAL_INDEX_E5a			1
#define SIGNAL_INDEX_E5b			2
#define SIGNAL_INDEX_E5				3
#define SIGNAL_INDEX_E6				4

#define SIGNAL_INDEX_G1				0
#define SIGNAL_INDEX_G2				1

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

typedef struct // GPS ephemeris, also used by BDS, Galileo, QZSS and NavIC
{
	signed short	ura;	// URA for corresponding system
	unsigned short	iodc;
	unsigned char	iode;
	unsigned char	svid;	// satellite PRN number starting from 1
	unsigned char	source;	// source of navigation (from which type of navigation message)
	unsigned char	valid;	// bit0 means ephemeris valid, other bits means valid for tgd_ext
	unsigned short	flag;	// definition various
	unsigned short	health;	// definition various

	int	toe;
	int	toc;
	int top;
	int	week;

	// orbit parameter
	double M0;			// Mean Anomaly at Reference Time
	double delta_n;		// Mean Motion Difference from Computed Value
	double delta_n_dot;	// Rate of Mean Motion Difference from Computed Value
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
	// clock and delay parameters
	double af0;			// Satellite Clock Correction
	double af1;			// Satellite Clock Correction
	double af2;			// Satellite Clock Correction
	double tgd;			// Group Delay for primary frequency (L1C/A, B1I, E1)
	double tgd2;		// Group Delay for secondary frequency (eg. L2C and B2I)
	double tgd_ext[5];	// Group Delay for other frequency and channels (TGD - ISC)

	// variables derived from basic data, to avoid calculate every time
	double axis;		// Semi-major Axis of Orbit, equals to sqrtA^2
	double n;			// Corrected Mean Angular Rate, equals to WGS_SQRT_GM/sqrtA^3 + delta_n
	double root_ecc;	// Square Root of One Minus Ecc Square, equals to sqrt(1-ecc^2)
	double omega_t;		// Longitude of Ascending Node of Orbit Plane at toe, equals to omega0 - WGS_OMEGDOTE * toe
	double omega_delta;	// Delta Between omega_dot and WGS_OMEGDOTE, equals to omega_dot - WGS_OMEGDOTE
	double Ek;			// Ek, derived from Mk
	double Ek_dot;		// change rate of Ek
} GPS_EPHEMERIS, *PGPS_EPHEMERIS;

// definitions for source field
#define EPH_SOURCE_LNAV 0
#define EPH_SOURCE_D1D2 0
#define EPH_SOURCE_INAV 0
#define EPH_SOURCE_CNAV 1
#define EPH_SOURCE_CNV1 1
#define EPH_SOURCE_FNAV 1
#define EPH_SOURCE_CNV2 2
#define EPH_SOURCE_CNV3 3

typedef struct        			
{
	unsigned char   valid;	// bit0 means almanac valid
	unsigned char	flag;	// IOD for Galileo, 2LSB as SatType for BDS
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
						//      0/1, 2, 3, 4, 5, 6/7
	unsigned char M;	// satellite type 00 - GLONASS, 01 - GLONASS-M
	unsigned char Ft;	// indicator of accuracy of measurements
	unsigned char n;	// slot number that transmit signal
	unsigned char Bn;	// healthy flag
	unsigned char En;	// age of the immediate information
	unsigned int tb;	// reference time in seconds within current day
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
	unsigned char flag;	// bit0 means almanac valid
	signed char freq;	// frequency number of satellite
	short leap_year;	// reference leap year of almanac
	short day;			// reference day of almanac
	double t;			// time of first ascending node
	double lambda;		// longitude of ascending node of orbit
	double di;			// inclination correction
	double ecc;			// eccentricity
	double w;			// Argument of Perigee
	double dt;			// correction to the mean value of Draconian period
	double dt_dot;		// rate of change of orbital period
	double clock_error;	// correction of satellite clock
} GLONASS_ALMANAC, * PGLONASS_ALMANAC;

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

typedef struct
{
    double	ai0;
    double	ai1;
    double	ai2;
    unsigned long	flag; // bit0:1, availble, bit8~13: svid
} IONO_NEQUICK, *PIONO_NEQUICK;

typedef struct
{
    double	alpha1;
    double	alpha2;
    double	alpha3;
    double	alpha4;
    double	alpha5;
    double	alpha6;
    double	alpha7;
    double	alpha8;
    double	alpha9;
    unsigned long	flag; // bit0:1, availble, bit8~13: svid
} IONO_BDGIM, *PIONO_BDGIM;

// UTC parameters
typedef  struct _PACKED_
{
	double	A0;  // second
	double	A1;  // second/second
	double	A2;  // second/second^2
	short	WN;
	short	WNLSF;
	unsigned char	tot; // scale factor 2**12 for GPS/BDS and 3600 for Galileo
	signed char	TLS; // leap second
	signed char	TLSF;
	unsigned char	DN;
	unsigned long	flag; // bit0: UTC parameter available, bit1: leap second available
} UTC_PARAM, *PUTC_PARAM;

#define MAX_OBS_NUMBER 6

typedef struct
{
	int system;
	int svid;
	unsigned int ValidMask;
	double PseudoRange[MAX_OBS_NUMBER];
	double CarrierPhase[MAX_OBS_NUMBER];
	double Doppler[MAX_OBS_NUMBER];
	double CN0[MAX_OBS_NUMBER];
} SAT_OBSERVATION, *PSAT_OBSERVATION;

typedef enum { OutputTypePosition, OutputTypeObservation, OutputTypeIFdata, OutputTypeBaseband } OutputType;
typedef enum { OutputFormatEcef, OutputFormatLla, OutputFormatNmea, OutputFormatKml, OutputFormatRinex, OutputFormatIQ8, OutputFormatIQ4 } OutputFormat;

typedef struct
{
	char filename[256];
	OutputType Type;
	OutputFormat Format;
	unsigned long GpsMaskOut;
	unsigned long GlonassMaskOut;
	unsigned long long BdsMaskOut;
	unsigned long long GalileoMaskOut;
	double ElevationMask;
	int Interval;	// in millisecond
	int SampleFreq, CenterFreq;	// in kHz
	unsigned int FreqSelect[4];	// Frequency select mask, 0~3 for GPS/BDS/Galileo/GLONASS respectively, bit selection uses SIGNAL_INDEX_XXXX
} OUTPUT_PARAM, *POUTPUT_PARAM;

typedef struct
{
	double SystemDelay[4];	// system time difference to GPS, 0 for GPS (always 0), 1 for BDS, 2 for Galileo, 3 for GLONASS
	double ReceiverDelay[4][8];	// receiver RF delay difference for each frequency to primary frequency, [][0] always 0
} DELAY_CONFIG, *PDELAY_CONFIG;

typedef struct
{
	GnssSystem system;
	int svid;
	int FreqID;	// for GLONASS only
	int CN0;	// scale factor 0.01
	int PosTimeTag;
	KINEMATIC_INFO PosVel;
	double Acc[3];
	double TravelTime;	// travel time including corrections except group delay and ionosphere delay in second
	double IonoDelay;	// ionosphere delay in meter
	double GroupDelay[8];	// group delay in second, first element for TGD of L1, all followings for ISC using SIGNAL_INDEX_XXXX as index
	double Elevation;	// satellite elevation in rad
	double Azimuth;		// satellite azimuth in rad
	double RelativeSpeed;	// satellite to receiver relative speed in m/s
	double LosVector[3];	// LOS vecter

} SATELLITE_PARAM, *PSATELLITE_PARAM;

#endif //__BASIC_TYPE_H__
