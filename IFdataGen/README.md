# IFdataGen - SignalSim IF Data Generator

## Overview

IFdataGen is a component of the SignalSim project that generates GNSS Intermediate Frequency (IF) data for various satellite navigation systems. The generated binary files can be used with RF front-ends or GNSS software-defined radio applications.

## Signal Support and Testing Status

The following table shows the current testing status of various GNSS signals in SignalSim:

### Single-Constallations

| Constellation | Frequency Band | Implementation | Testing Status  | Notes |
|---------------|---------------|----------------|------------------|-------|
| GPS           | L1CA          | 🟢             | 🟢             | Default signal, fully tested |
|               | L1C           | 🟢             | 🔴             | Program Crash |
|               | L2C (L2CM)    | 🟢             | 🟡             | To Be Tested |
|               | L5            | 🟢             | 🟡             | To Be Tested |
|               | L1P/L2P       | 🟢             | 🟡             | Commercial license only |
| BDS           | B1I           | 🟢             | 🔴             | Program Crash |
|               | B1C           | 🟢             | 🔴             | Program Crash |
|               | B2I           | 🟢             | 🔴             | Program Crash |
|               | B2a           | 🟢             | 🔴             | Program Crash |
|               | B2b           | 🟢             | 🔴             | Program Crash |
|               | B3I           | 🟢             | 🔴             | Program Crash |
| Galileo       | E1            | 🟢             | 🟢             | I/NAV data modulation |
|               | E5a           | 🟢             | 🟡             | F/NAV data modulation |
|               | E5b           | 🟢             | 🟡             | I/NAV data modulation |
|               | E5 AltBOC     | 🟢             | 🟡             | Commercial license only |
|               | E6            | 🟡             | 🟡             | Under development |
| GLONASS       | G1            | 🟢             | 🟡             | FDMA implementation |
|               | G2            | 🟢             | 🟡             | FDMA implementation |

### Multi-Constallations

| Constellation | Frequency Band        |  Testing Status  | Notes            |
|---------------|-----------------------|------------------|------------------|
| GPS + Galileo | L1CA + E1             | 🟢               | Tested and works |
|               | L1CA + E5a            | 🟡               | To Be Tested     |
|               | L1CA + E5b            | 🟡               | To Be Tested     |
|               | L1CA + E5 AltBOC      | 🟡               | To Be Tested     |
|               | L1CA + E6             | 🟡               | To Be Tested     |

> **Legend**:
>
> - 🟢 Working / Complete: Feature has been fully implemented and verified
> - 🟡 Limited / Partial: Feature has partial implementation or limited verification
> - 🔴 Not Working: Feature is not implemented or not functioning correctly

### Note

> The `"Testing Status"` indicates that the signals for the respective frequency bands have been successfully generated. However, the detailed evaluation of their characteristics—such as spectral purity, phase noise, modulation accuracy, and overall signal integrity—has not yet been performed.​

## Build Instructions on Windows

### Prerequisites

- Microsoft Visual Studio (tested with VS 2022)
- CMake support enabled in Visual Studio (included by default in recent versions)

### Steps to Build IFdataGen

#### 1. Obtain the Repository

Either clone the repository to your PC:

``` cmd
git clone https://github.com/MuhammadQaisarAli/SignalSim.git
```

Or download the ZIP archive and extract it.

#### 2. Open the Project Folder

- Launch Visual Studio
- Navigate to File > Open > Folder...
- Browse to the cloned SignalSim repository and select the `SignalSim` folder

#### 3. Configure the CMake Project

- Upon opening the folder, Visual Studio will detect the `CMakeLists.txt` files and a `CMake Integration` popup will appear
  
  ![CMake Integration Popup](images/CMakeIntegrationPopup.png)
  
- Click `Enable and set source directory` and select the `CMakeLists.txt` file located at `<path-to-repo>/SignalSim/IFdataGen/CMakeLists.txt`
- Wait for the configuration process to complete. You should see this output in the console:

  ``` cmd
  1> Extracted CMake variables.
  1> Extracted source files and headers.
  1> Extracted code model.
  1> Extracted toolchain configurations.
  1> Extracted includes paths.
  1> CMake generation finished.
  ```

#### 4. Build the Project

- Once the configuration is complete, build the project by navigating to Build > Build All or pressing `Ctrl + Shift + B`
- Upon successful build, you should see:

  ``` cmd
  [32/32] Linking CXX executable IFdataGen.exe

  Build All succeeded.
  ```

- The `IFdataGen.exe` executable will be generated in `<path-to-repo>/SignalSim/IFdataGen/out/build/x64-Debug/` folder

## Running IFdataGen

### 1. Configure the Input JSON File

- Navigate to the `/SignalSim/IFdataGen/` directory and open `IfGenTest.json` in a text editor
- Update the `ephemeris` section with the absolute path to your RINEX file:

  ```json
  "ephemeris": {
      "type": "RINEX",
      "name": "D:/SignalSim/IFdataGen/EphData/BRDC00IGS_R_20211700000_01D_MN.rnx"
  }
  ```

- The provided `BRDC00IGS_R_20211700000_01D_MN.rnx` is a mixed RINEX file containing data for multiple constellations (GPS, Galileo, BeiDou, GLONASS)
- By default, only GPS L1CA signals are selected for generation

### 2. Generate the IF Data

- Open a command prompt in the build directory `<path-to-repo>/SignalSim/IFdataGen/out/build/x64-Debug/`
- Run the following command:

  ```cmd
  IFdataGen.exe "<path-to-repo>/SignalSim/IFdataGen/IfGenTest.json"
  ```

  Example:

  ```cmd
  IFdataGen.exe "D:/SignalSim/IFdataGen/IfGenTest.json"
  ```
  
- On successful execution, you'll see output similar to:

  ``` cmd
    Generate IF data with following satellite signals:
    GPS visible SVs 9.
    BDS visible SVs 0.
    Galileo visible SVs 0.
    Glonass visible SVs 0.

    GPS L1CA with IF -4580kHz
            SV03 with Doppler -3765Hz
            SV04 with Doppler -1899Hz
            SV07 with Doppler 893Hz
            SV08 with Doppler 2480Hz
            SV09 with Doppler 133Hz
            SV14 with Doppler 3721Hz
            SV16 with Doppler -3263Hz
            SV27 with Doppler 1169Hz
            SV30 with Doppler 2352Hz
  Generate IF data completed    490 ms
  ```

- A binary file `IfGenTest8.bin` will be created in the same directory as the executable

## Using the Generated Data

The generated binary file can be:

- Fed into RF front-end hardware such as HackRF, PlutoSDR, etc.
- Processed by GNSS software like GNSS-SDR or PocketSDR
- Used for testing and development of GNSS signal processing algorithms

## Supported Constellations

- GPS (L1CA enabled by default)
- Galileo
- BeiDou
- GLONASS

## Additional Resources

For more information on SignalSim and its components, refer to the main project documentation in the repository root directory.
