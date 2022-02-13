# SignalSim
 A multi-stage GNSS signal and data simulator

This is a GNSS signal and data simulator and generator to help the development of GNSS receiver.
This program support multiple stage of simulation output:
1. Reference trajectory of receiver (as standard reference result)
2. Observation (help to debug and test PVT program)
3. Baseband correlation result (combine with local channel configuration parameters help to debug baseband tracking and control program)
4. Digital IF result (help to debug baseband process algorithm)
5. A realtime GNSS signal simulator (with hardware signal interpolation and up convertor)

Change list as below:
7/1/2021
	An optimized initial version re-published on github
7/5/2021
	Add XML format trajectory output
	Add support to load BDS ionosphere parameter in RINEX header
	Add support to load GPS/BDS/Galileo UTC parameters in RINEX header
	Add support to load BDS/Galileo ephemeris in RINEX file
	Optimize XML file content interpreter
8/4/2021
	Add class NavBit and LNavBit to generate GPS LNAV data stream
8/13/2021
	Change format of GNSS_TIME to improve accuracy
	Add GetTransmitTime() method
	Remove obsolete file
8/30/2021
	Fix bug of TOW add extra 1 when put into LNAV data stream
	Fix bug of not put in week number in LNAV data stream
	Use complete GNSS time to get LNAV data stream
9/30/2021
	Add satellite signal power control support in XML
	Add RINEX output functions
11/25/2021
	Restore CN0 field in SATELLITE_PARAM structure as stored CN0
12/8/2021
	Add support for B1C and E1C
12/14/2021
	Fix bug of CN0 assign incorrect value during satellite add/remove
	Add support for system select
1/5/2022
	Add support for B-CNAV2 data stream generation
2/12/2022
	Bug fix on B-CNAV1 stream generation
	Option to use Vel/Acc to calculated satellite position
