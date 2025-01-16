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
1/23/2023
	Add some variables and functions for future multi-frequency support
	Minor bug fix on satellite acc calculation for GEO satellite
7/28/2023
	Modifications to comply with stricter syntax checking
	Add SignalSim.h to include all necessary header files for package users
8/9/2023
	Add a new class CSatelliteSignal to get data/pilot modulation
	Add a new NavBit derived class D1D2NavBit to generate BDS2 data stream
	Call to GetFrameData() in NavBit class will be obsolete in the future
8/15/2023
	Add pilot bit generation function
	Remove pilot bit generation in NavBit and CSatelliteSignal class
	GetFrameData() method in NavBit (and derived classes) no longer support pilot bit generation
8/24/2023
	Add Galileo I/NAV data stream generation class
	Add inter-signal correction (delay between different frequencies) support in XML format
8/30/2023
	Add GLONASS satellite parameter and raw measurement calculation
	Add GLONASS slot/freq output to RINEX file
	Add GLONASS GNAV data stream composition
	Leap second correction in CSatelliteSignal
	Update the PDF file for design description
9/19/2023
	Add complex_number class
	Add E5 support
	Add dummy F/NAV data stream generation
	Satellite signal generation allows NULL pointer for data bit to generate all 0 modulation data
10/28/2023
	Replace UnscaleDouble and roundi/roundu with UnscaleInt/UnscaleUint
	Add UnscaleLong/UnscaleULong to better support data fields longer than 32bit (C-NAV and BC-NAV)
11/13/2023
	Add support to RINEX 4 format navigation file
	Modify Rinex.cpp to support RINEX 4 format data set
	Expand and modify GPS_EPHEMERIS structure to support ephemeris from different navigation data format
	Calculate clock/delay for different frequency with corresponding parameters
11/27/2023
	Change BCNavBit to virtual class and derive three classes for B-CNAV1/2/3 data stream
	Combine iode2/iode3 to iode in ephemeris structure
	Add B-CNAV2 and B-CNAV3 support in CSatelliteSignal
	Modify GetTravelTime() and GetCarrierPhase() to use correct group delay
1/6/2024
	Add GPS L1C CNAV2 navigation stream generation class
	Fix bugs in BDS D1/D2 navigation stream generation
	Add L1C/L5/E6 support into CSatelliteSignal class
	Observation support multi-frequency in structure and RINEX output
2/29/2024
	Made following changes to enable almanac read and data stream generation containing almanac subframe/string
	Change function parameter of SetAlmanac() in NavBit and all derived classes
	Add functions to read almanac file in Almanac.cpp
	Add array to store almanacs in CNavData class
	Add functions to read almanac files and convert ephemeris to almanac in CNavData class
	Ephemeris to almanac conversion for GLONASS and BDS GEO satellites will be added in future version
	The time parameter of FindEphemeris() and FindGloEphemeris() definition changes to follow corresponding system
	Add almanac subframe/string generation for D1/D2, LNAC and GNAV data stream
6/5/2024
	Add almanac word (word 7~10) to Galileo E1 data stream
	Add Reed-Solomon encoded ephemeris (word 17~20) to Galileo E1 data stream
	Bug fixes on E1 ephemeris word and TOW composition
9/21/2024
	Add Galileo E5a F/NAV data modulation support
	Bug fix of group delay and signal modulation phase in B2b signal
10/1/2024
	New CNavBit class for CNAV bit stream generation
	Expand URA index range to support corresponding data field in CNAV
	L2CM and L5I signal use CNAV data modulation
10/5/2024
	CMakeLists.txt added to XmlObsGen folder to guide how to build project using cmake
	Minor fixes
10/20/2024
	Add JSON format scenario control file to replace XML format
	Add JsonParser.cpp/JsonParser.h to read JSON data structure from file
	Add JsonInterpreter.cpp/JsonInterpreter.h to translate JSON data structure to scenario control
	Add a JsonObsGen project that has the same output using test_obs2.json (same parameters as test_obs2.xml used in XmlObsGen project)
1/15/2025
	Rearrange some definitions and declarations
	Change FREQ_INDEX_XXX definitions to SIGNAL_INDEX_XXX for better discriminate signals at same frequency
	Key word extension in JSON for future IF data generation support
