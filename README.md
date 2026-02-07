# SignalSim

A multi-stage GNSS signal and data simulator

This is a GNSS signal and data simulator and generator to help the development of GNSS receivers.

This program supports multiple stages of simulation output:

1. **Reference trajectory of receiver** (as standard reference result)
2. **Observation** (helps to debug and test PVT program)
3. **Baseband correlation result** (combined with local channel configuration parameters, helps to debug baseband tracking and control program)
4. **Digital IF result** (helps to debug baseband process algorithm)
5. **A realtime GNSS signal simulator** (with hardware signal interpolation and up converter)

## System Capabilities

SignalSim provides comprehensive support for multi-constellation GNSS simulation with the following capabilities:

### Constellation Support

- **GPS**: Full constellation of 32 satellites
- **BeiDou (BDS)**: Complete constellation of 63 satellites
- **Galileo**: Full constellation of 36 satellites
- **GLONASS**: Complete constellation of 24 satellites

### Technical Specifications

- **Constellation**: Support GPS, BDS, GLONASS and Galileo, QZSS/IRNSS interface reserved for future expansion
- **Multiple frequency bands**: Support for L1/L2/L5, B1/B2/B3, E1/E5/E6, G1/G2
- **Data formats**: RINEX 2/3/4 support for observation and navigation data
- **Configuration**: JSON-based scenario configuration
- **Signal generation**: Digital IF samples with configurable sampling rate and bit depth

SignalSim is designed for both educational and professional use, with commercial licensing available for advanced features.

## Signal Support and Testing Status

The [Table](./IFdataGen/README.md#signal-support-and-testing-status) shows the current testing status of various GNSS signals.

## Key Components

### IFdataGen

The IF Data Generator component creates GNSS Intermediate Frequency (IF) sample files that can be used with RF front-ends or GNSS software-defined radio applications. For detailed information on building and using this component, see the [IFdataGen README](./IFdataGen/README.md).

### JsonObsGen

The JSON Observation Generator creates GNSS observation data based on JSON configuration files. This component replaced the older XML-based generator.

### Library Core

The core libraries provide fundamental GNSS data processing capabilities including:

- Ephemeris and almanac parsing
- Navigation bit stream generation
- Satellite signal modeling
- RINEX file handling
- Trajectory calculation

---

## Change List

### 2026

- **2/6/2026**
  - Swap L1C data and pilot code generation array because they are mixed up before
  - Add protection on GLONASS/BDS GEO ephemeris to almanac conversion in case pos/vel does not conform to an elliptical orbit

- **1/20/2026**
  - Add correct GLONASS half cycle compensation on start carrier phase for odd frequency satellites

- **1/4/2026**
  - Sort power control list by time in JsonInterpreter.cpp (sorted in XML but missed in JSON)
  - Remove GLONASS half cycle compensation in IF generation (no need for floating point carrier calculation)
  - Fix bug in signal amplitude calculation (SNR of each sample should be SNR=CN0-10*lg(fs)=20*lg(A/sigma))

### 2025

- **11/25/2025**
  - BCNavBit.h was forgot to be included in the previous commit

- **11/22/2025**
  - Assign Galileo week# in Galileo ephemeris structure instead of GPS week# (1024 difference)
  - Fix bug that Galileo uses incorrect scale factor on toa when converting ephemeris to almanac
  - Fix bug on BDS GEO satellite determination (add SV59 and above)
  - Initialize arrays not initialized in INAV/CNAV/BCNAV
  - Broadcast all types of messages in BCNAV2 data stream

- **9/26/2025**
  - Fix bug in CNAV2 message compose and put valid contents in subframe3
  - Fix bug in BCNAV message compose and put valid contents in subframe3 of BCNAV1

- **9/8/2025**
  - Merge PR#59 for bug fix in parsing RINEX 3.02 nav data file
  - Merge PR#60 to add config for GNSS-SDR

- **7/28/2025**
  - Add message output control with output target stream and level selection
  - Fully implement --validate-only (-vo) option

- **7/17/2025**
  - Improve speed of rotate factor calculation by using fixed point angles
  - Update multi-thread version IFdataGenThread.cpp using same command argument as IFdataGen

- **7/15/2025**
  - Change ephemeris increase method in class CNavData to double the size (more efficient)
  - Fix bug of using GPS week number instead of BDS in BDS CNAV2 and CNAV3 data
  - Add midi-almanac and reduced almanac to BDS CNAV3 message type 40

- **7/14/2025**
  - Add USRP support (IQ16)
    
- **7/9/2025**
  - Use LFS to track all *.pdf files
  - Config LFS to not maintain old pdf files to reduce storage space

- **7/5/2025**
  - Update/Fix the `README.md` of `IFdatagGen` directory
  - Fix typos in command line arguments of help print function

- **7/1/2025**
  - Merge Fix, information display and command line arguments in PR#36
  - Fix performance overheads in Nested Noise addition loop in `IFdataGen.cpp`
  - Add Build Instructions for Linux
  - Enhance signal generation status logs on terminal
  - Improve `CMakeLists.txt` configuration
  - Update `README.md` documentations
  - Add support for generating metadata tag file for generated signals
  - Add 2-bit IQ support (compatible with PocketSDR, testing pending)
  - Update [Test Table](./IFdataGen/README.md#signal-support-and-testing-status) with additional test cases

- **6/30/2025**
  - Add benchmark to check execution speed
  - Remove lookup-table sin/cos calculation cause it seems slower than FPU math lib
  - A trial version multi-thread program `IFdataGenThread.cpp` released using std::thread
  - Other minor modifications

- **6/27/2025**
  - Add OpenMP parallelization for signal generation
  - Implemented FastMath class with lookup tables for trigonometric functions
  - Optimized noise generation using Marsaglia Polar method
  - Add Makefile with optimization flags
  - Add clipping counter for quantization monitoring
  - Add SVID boundary check and protection
  - Correction on L2C time-multiplex signal generation

- **6/18/2025**
  - Fix bug of access out of array boundary in BCNAV1 data stream compose
  - Update `IFdataGen` VS2022 project file
  - Add VS2022 project file to `JsonObsGen`

- **6/6/2025**
  - Add `configs` folder with examples `.json` files for multiple constellations.
  - Explain how to set center frequency and bandwidth for each constellation in the [IFdataGen GNSS_Signal_Calculations](./IFdataGen/GNSS_Signal_Calculations.md).
  - Expand unit and integration tests for signal generation and update the test matrix accordingly.
  - Update  [IFdataGen README](./IFdataGen/README.md) with new examples and configuration guidance.

- **5/14/2025**
  - Rewrite BCNAV1/2/3 navigation data stream generation method (ephemeris generation completed, almanac in next release)
  - Rename `PrnCode` to `PrnGenerate`
  - Add variable valid in struct GPS_ALMANAC and redefine contents of variable flag

- **5/6/2025**
  - Relocate shared RINEX data files from `/XmlObsGen` to root
  - Relocate SatIfSignal.h and SatIfSignal.cpp from `IFdataGen/` to `inc/` and `src/`

- **4/22/2025**
  - Enhanced documentation with updated main `README.md`
  - Added `CMakeLists.txt` to support `IFdataGen` builds
  - Fixed initial bugs in `IFdataGen.cpp` implementation
  - Implemented command-line support for JSON configuration files
  - Created detailed `README.md` in `IFdataGen` folder with demo instructions and usage examples
  - Integrated `EphData` folder into `IFdataGen` Folder

- **3/11/2025**
  - Demo program to generate IF signal is added with sample .json config file
  - Most common signals are supported but with only limited verification
  - Following signals will be only available with commercial license: L1P/L2P, E5 AltBOC, TMBOC, ACEBOC, QMBOX, LEX CSK
  - Multi-thread version which runs dozen times faster only available with commercial license
  
- **1/28/2025**
  - Bug fix and improvements on almanac data stream composition
  - Fix bug on possible incorrect week number in data stream of I/NAV and F/NAV if almanac of some satellite missing
  - Fix bug on incorrect week number set in data stream
  - Second parameter of function CompleteAlmanac() uses UTC time
  - Add BDS GEO satellite ephemeris to almanac conversion
  - Add GLONASS satellite ephemeris to almanac conversion
  
- **1/15/2025**
  - Rearrange some definitions and declarations
  - Change FREQ_INDEX_XXX definitions to SIGNAL_INDEX_XXX for better discrimination of signals at same frequency
  - Key word extension in JSON for future IF data generation support

### 2024

- **10/20/2024**
  - Add JSON format scenario control file to replace XML format
  - Add JsonParser.cpp/JsonParser.h to read JSON data structure from file
  - Add JsonInterpreter.cpp/JsonInterpreter.h to translate JSON data structure to scenario control
  - Add a JsonObsGen project that has the same output using test_obs2.json (same parameters as test_obs2.xml used in XmlObsGen project)
  
- **10/5/2024**
  - CMakeLists.txt added to XmlObsGen folder to guide how to build project using cmake
  - Minor fixes
  
- **10/1/2024**
  - New CNavBit class for CNAV bit stream generation
  - Expand URA index range to support corresponding data field in CNAV
  - L2CM and L5I signal use CNAV data modulation
- **9/21/2024**
  
  - Add Galileo E5a F/NAV data modulation support
  - Bug fix of group delay and signal modulation phase in B2b signal
  
- **6/5/2024**
  - Add almanac word (word 7~10) to Galileo E1 data stream
  - Add Reed-Solomon encoded ephemeris (word 17~20) to Galileo E1 data stream
  - Bug fixes on E1 ephemeris word and TOW composition
  
- **2/29/2024**
  - Enable almanac read and data stream generation containing almanac subframe/string
  - Change function parameter of SetAlmanac() in NavBit and all derived classes
  - Add functions to read almanac file in Almanac.cpp
  - Add array to store almanacs in CNavData class
  - Add functions to read almanac files and convert ephemeris to almanac in CNavData class
  - Ephemeris to almanac conversion for GLONASS and BDS GEO satellites will be added in future version
  - The time parameter of FindEphemeris() and FindGloEphemeris() definition changes to follow corresponding system
  - Add almanac subframe/string generation for D1/D2, LNAC and GNAV data stream
  
- **1/6/2024**
  - Add GPS L1C CNAV2 navigation stream generation class
  - Fix bugs in BDS D1/D2 navigation stream generation
  - Add L1C/L5/E6 support into CSatelliteSignal class
  - Observation support multi-frequency in structure and RINEX output

### 2023

- **11/27/2023**
  - Change BCNavBit to virtual class and derive three classes for B-CNAV1/2/3 data stream
  - Combine iode2/iode3 to iode in ephemeris structure
  - Add B-CNAV2 and B-CNAV3 support in CSatelliteSignal
  - Modify GetTravelTime() and GetCarrierPhase() to use correct group delay
  
- **11/13/2023**
  - Add support to RINEX 4 format navigation file
  - Modify Rinex.cpp to support RINEX 4 format data set
  - Expand and modify GPS_EPHEMERIS structure to support ephemeris from different navigation data format
  - Calculate clock/delay for different frequency with corresponding parameters
  
- **10/28/2023**
  - Replace UnscaleDouble and roundi/roundu with UnscaleInt/UnscaleUint
  - Add UnscaleLong/UnscaleULong to better support data fields longer than 32bit (C-NAV and BC-NAV)
  
- **9/19/2023**
  - Add complex_number class
  - Add E5 support
  - Add dummy F/NAV data stream generation
  - Satellite signal generation allows NULL pointer for data bit to generate all 0 modulation data
  
- **8/30/2023**
  - Add GLONASS satellite parameter and raw measurement calculation
  - Add GLONASS slot/freq output to RINEX file
  - Add GLONASS GNAV data stream composition
  - Leap second correction in CSatelliteSignal
  - Update the PDF file for design description
  
- **8/24/2023**
  - Add Galileo I/NAV data stream generation class
  - Add inter-signal correction (delay between different frequencies) support in XML format
  
- **8/15/2023**
  - Add pilot bit generation function
  - Remove pilot bit generation in NavBit and CSatelliteSignal class
  - GetFrameData() method in NavBit (and derived classes) no longer supports pilot bit generation
  
- **8/9/2023**
  - Add a new class CSatelliteSignal to get data/pilot modulation
  - Add a new NavBit derived class D1D2NavBit to generate BDS2 data stream
  - Call to GetFrameData() in NavBit class will be obsolete in the future
  
- **7/28/2023**
  - Modifications to comply with stricter syntax checking
  - Add SignalSim.h to include all necessary header files for package users
  
- **1/23/2023**
  - Add some variables and functions for future multi-frequency support
  - Minor bug fix on satellite acc calculation for GEO satellite

### 2022

- **2/12/2022**
  - Bug fix on B-CNAV1 stream generation
  - Option to use Vel/Acc to calculate satellite position
  
- **1/5/2022**
  - Add support for B-CNAV2 data stream generation

### 2021

- **12/14/2021**
  - Fix bug of CN0 assign incorrect value during satellite add/remove
  - Add support for system select
  
- **12/8/2021**
  - Add support for B1C and E1C
  
- **11/25/2021**
  - Restore CN0 field in SATELLITE_PARAM structure as stored CN0
  
- **9/30/2021**
  - Add satellite signal power control support in XML
  - Add RINEX output functions
  
- **8/30/2021**
  - Fix bug of TOW add extra 1 when put into LNAV data stream
  - Fix bug of not put in week number in LNAV data stream
  - Use complete GNSS time to get LNAV data stream
  
- **8/13/2021**
  - Change format of GNSS_TIME to improve accuracy
  - Add GetTransmitTime() method
  - Remove obsolete file
  
- **8/4/2021**
  - Add class NavBit and LNavBit to generate GPS LNAV data stream
  
- **7/5/2021**
  - Add XML format trajectory output
  - Add support to load BDS ionosphere parameter in RINEX header
  - Add support to load GPS/BDS/Galileo UTC parameters in RINEX header
  - Add support to load BDS/Galileo ephemeris in RINEX file
  - Optimize XML file content interpreter
  
- **7/1/2021**
  - An optimized initial version re-published on github

---
