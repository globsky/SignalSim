# CMake project for IFdataGen, include source and define project-specific logic here.
cmake_minimum_required (VERSION 3.8)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

# Declare the project name
project ("IFdataGen")

# Include directories for header files
include_directories(../inc)

# Add source files to the project
add_executable (IFdataGen
    "IFdataGen.cpp"
    "../src/SatIfSignal.cpp"
    "../src/Almanac.cpp"
    "../src/BCNav1Bit.cpp"
    "../src/BCNav2Bit.cpp"
    "../src/BCNav3Bit.cpp"
    "../src/BCNavBit.cpp"
    "../src/CNav2Bit.cpp"
    "../src/CNavBit.cpp"
    "../src/ComplexNumber.cpp"
    "../src/Coordinate.cpp"
    "../src/D1D2NavBit.cpp"
    "../src/FNavBit.cpp"
    "../src/GNavBit.cpp"
    "../src/GnssTime.cpp"
    "../src/INavBit.cpp"
    "../src/JsonInterpreter.cpp"
    "../src/JsonParser.cpp"
    "../src/LNavBit.cpp"
    "../src/NavBit.cpp"
    "../src/NavData.cpp"
    "../src/PilotBit.cpp"
    "../src/PowerControl.cpp"
    "../src/PrnGenerate.cpp"
    "../src/Rinex.cpp"
    "../src/SatelliteParam.cpp"
    "../src/SatelliteSignal.cpp"
    "../src/Trajectory.cpp"
    "../src/XmlArguments.cpp"
    "../src/XmlElement.cpp"
    "../src/XmlInterpreter.cpp"
)

# Specify the C++ standard if CMake version is >= 3.12
if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET IFdataGen PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
