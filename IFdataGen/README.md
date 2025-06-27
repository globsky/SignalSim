# IFdataGen - GNSS IF Signal Generator

Minimal standalone version for compilation.

## Quick Start

### Linux/Unix:
```bash
make
./IFdataGen example.json
```

### Windows (MSYS2):
```bash
make
./IFdataGen.exe example.json
```

## Requirements
- C++ compiler (g++ or clang++)
- Make
- OpenMP (optional, for better performance)

## Directory Structure
```
IFdataGen_standalone/
├── IFdataGen.cpp      # Main program
├── src/               # Source files
├── inc/               # Header files
├── EphData/           # RINEX ephemeris data
├── Makefile           # Build configuration
├── example.json       # Example configuration
└── README.md          # This file
```

## Build Options
- `make` - Build with default settings
- `make clean` - Clean build files
- `make CXX=clang++` - Build with clang
- `make CXXFLAGS="-g"` - Build with debug symbols

## Output
The program generates .C8 files (8-bit IQ format) containing GNSS IF signals.