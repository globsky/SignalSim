//----------------------------------------------------------------------
// Rinex.h:
//   Declaration of RINEX file read/write functions
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __RINEX_H__
#define __RINEX_H__

#include <stdio.h>
#include "BasicTypes.h"

#define RINEX_MAX_FREQ 3

typedef struct
{
	int MajorVersion;
	int MinorVersion;
	unsigned int HeaderFlag;
	char Program[20];
	char Agency[20];
	UTC_TIME DateTime;
	char Comment[60];
	char MakerName[60];
	char MakerNumber[60];
	char MakerType[60];
	char Observer[60];
	char ReceiverType[60];
	char AntennaType[60];
	double ApproxPos[3];
	double AntennaDelta[3];
	// each system output maximum observation of 3 frequencies
	// bit0~3 as mask for PSR/ADR/Doppler/CN0 respectively, bit4~7 as channel select (I/Q/D/P etc.), bit8~12 as frequency select
	unsigned int SysObsTypeGps[RINEX_MAX_FREQ];
	unsigned int SysObsTypeGlonass[RINEX_MAX_FREQ];
	unsigned int SysObsTypeBds[RINEX_MAX_FREQ];
	unsigned int SysObsTypeGalileo[RINEX_MAX_FREQ];
	UTC_TIME FirstObs;
	UTC_TIME LastObs;
	unsigned int GlonassSlotMask;
	int GlonassFreqNumber[24];
	int LeapSecond;
	double Interval;
} RINEX_HEADER, *PRINEX_HEADER;

// definition for HeaderFlag field
#define RINEX_HEADER_PGM		0x00000001
#define RINEX_HEADER_AGENCY		0x00000002
#define RINEX_HEADER_DATE		0x00000004
#define RINEX_HEADER_COMMENT	0x00000008
#define RINEX_HEADER_MK_NAME	0x00000010
#define RINEX_HEADER_MK_NUMBER	0x00000020
#define RINEX_HEADER_MK_TYPE	0x00000040
#define RINEX_HEADER_OBSERVER	0x00000080
#define RINEX_HEADER_RCVR_TYPE	0x00000100
#define RINEX_HEADER_ANT_TYPE	0x00000200
#define RINEX_HEADER_APPROX_POS	0x00001000
#define RINEX_HEADER_ANT_DELTA	0x00002000
#define RINEX_HEADER_SLOT_FREQ	0x00004000
#define RINEX_HEADER_FIRST_OBS	0x00010000
#define RINEX_HEADER_LAST_OBS	0x00020000

// definitions commonly used for SysObsTypexxx
#define OBS_TYPE_MASK_PSR       0x1
#define OBS_TYPE_MASK_ADR       0x2
#define OBS_TYPE_MASK_DOP       0x4
#define OBS_TYPE_MASK_CN0       0x8
#define OBS_TYPE_MASK_ALL       0xf
#define OBS_CHANNEL_GPS_CA      (0x0 << 4)
#define OBS_CHANNEL_GPS_L1CD    (0x1 << 4)
#define OBS_CHANNEL_GPS_L1CP    (0x2 << 4)
#define OBS_CHANNEL_GPS_L1CDP   (0x3 << 4)
#define OBS_CHANNEL_GPS_L2CM    (0x2 << 4)
#define OBS_CHANNEL_GPS_L2CL    (0x3 << 4)
#define OBS_CHANNEL_GPS_L2CML   (0x4 << 4)
#define OBS_CHANNEL_GPS_L2P     (0x5 << 4)
#define OBS_CHANNEL_GPS_L2Z     (0x6 << 4)
#define OBS_CHANNEL_GLO_CA      (0x0 << 4)
#define OBS_CHANNEL_GAL_E1B     (0x1 << 4)
#define OBS_CHANNEL_GAL_E1C     (0x2 << 4)
#define OBS_CHANNEL_GAL_E1BC    (0x3 << 4)
#define OBS_CHANNEL_I           (0x0 << 4)
#define OBS_CHANNEL_Q           (0x1 << 4)
#define OBS_CHANNEL_IQ          (0x2 << 4)
#define OBS_CHANNEL_D           (0x0 << 4)
#define OBS_CHANNEL_P           (0x1 << 4)
#define OBS_CHANNEL_DP          (0x2 << 4)
/*#define OBS_FREQUENCY_GPS_L1    (0x0 << 8)
#define OBS_FREQUENCY_GPS_L2    (0x1 << 8)
#define OBS_FREQUENCY_GPS_L5    (0x2 << 8)
#define OBS_FREQUENCY_GLO_G1    (0x0 << 8)
#define OBS_FREQUENCY_GLO_G2    (0x1 << 8)
#define OBS_FREQUENCY_GAL_E1    (0x0 << 8)
#define OBS_FREQUENCY_GAL_E5a   (0x1 << 8)
#define OBS_FREQUENCY_GAL_E5b   (0x2 << 8)
#define OBS_FREQUENCY_GAL_E5    (0x3 << 8)
#define OBS_FREQUENCY_GAL_E6    (0x4 << 8)
#define OBS_FREQUENCY_BDS_B1C   (0x0 << 8)
#define OBS_FREQUENCY_BDS_B1    (0x1 << 8)
#define OBS_FREQUENCY_BDS_B2    (0x2 << 8)
#define OBS_FREQUENCY_BDS_B3    (0x3 << 8)
#define OBS_FREQUENCY_BDS_B2a   (0x4 << 8)
#define OBS_FREQUENCY_BDS_B2b   (0x5 << 8)*/

enum NavDataType {
	NavDataEnd = 0, NavDataUnknown,
	NavDataGpsIono, NavDataBdsIonoA, NavDataBdsIonoB, NavDataGalileoIono,
	NavDataGpsUtc, NavDataBdsUtc, NavDataGalileoUtc, NavDataGalileoGps, NavDataLeapSecond,
	NavDataGlonassFreq,
	NavDataGpsLnav, NavDataGpsCnav, NavDataGpsCnav2,
	NavDataBdsD1D2, NavDataBdsCnav1, NavDataBdsCnav2, NavDataBdsCnav3,
	NavDataGalileoINav, NavDataGalileoFNav,
	NavDataNavICLnav,
	NavDataGlonassFdma,
	NavDataSbasNav,
	NavDataIonGps, NavDataIonBds, NavDataIonBdgim, NavDataIonGalileo, NavDataIonQzss, NavDataIonIrnss,
};

NavDataType LoadNavFileHeader(FILE *fp_nav, void *NavData);
NavDataType LoadNavFileContents(FILE *fp_nav, void *NavData);
void OutputHeader(FILE *fp, PRINEX_HEADER Header);
void OutputObservation(FILE *fp, UTC_TIME time, int TotalObsNumber, SAT_OBSERVATION Observations[]);

#endif // __RINEX_H__
