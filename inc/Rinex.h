//----------------------------------------------------------------------
// Trajectory.h:
//   Declaration of trajectory processing class
//
//          Copyright (C) 2020-2029 by Jun Mo, All rights reserved.
//
//----------------------------------------------------------------------

#ifndef __RINEX_H__
#define __RINEX_H__

#include <stdio.h>
#include "BasicTypes.h"

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
	unsigned int SysObsTypeGps;
	unsigned int SysObsTypeGlonass;
	unsigned int SysObsTypeBds;
	unsigned int SysObsTypeGalileo;
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
#define RINEX_HEADER_FIRST_OBS	0x00010000
#define RINEX_HEADER_LAST_OBS	0x00020000

enum NavDataType {
	NavDataEnd = 0, NavDataUnknown,
	NavDataGpsIono, NavDataBdsIonoA, NavDataBdsIonoB, NavDataGalileoIono,
	NavDataGpsUtc, NavDataBdsUtc, NavDataGalileoUtc, NavDataGalileoGps, NavDataLeapSecond,
	NavDataGpsEph, NavDataBdsEph, NavDataGalileoEph, NavDataGlonassEph
};

NavDataType LoadNavFileHeader(FILE *fp_nav, void *NavData);
NavDataType LoadNavFileEphemeris(FILE *fp_nav, void *NavData);
void OutputHeader(FILE *fp, PRINEX_HEADER Header);
void OutputObservation(FILE *fp, UTC_TIME time, int TotalObsNumber, SAT_OBSERVATION Observations[]);

#endif // __RINEX_H__
