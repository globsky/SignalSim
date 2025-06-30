# Complete Structure of GNSS Satellite System Navigation Messages

## Table of Contents
1. [GPS (USA)](#gps-usa)
2. [GLONASS (Russia)](#glonass-russia)  
3. [Galileo (EU)](#galileo-eu)
4. [BeiDou (China)](#beidou-china)
5. [QZSS (Japan)](#qzss-japan)
6. [NavIC/IRNSS (India)](#navicirnss-india)
7. [SBAS Systems](#sbas-systems)

---

## GPS (USA)

### Updated GPS Information (2025)

**Constellation Status:** As of June 2025, GPS consists of 32 operational satellites. Managed by the 2nd Space Operations Squadron (2SOPS) of Space Delta 8, United States Space Force.

**Constellation Modernization:**
- **GPS III**: 8 satellites launched (SV01-SV08), including latest SV07 and SV08
- **GPS III Improvements**: 3x accuracy improvement, 8x anti-jamming capability improvement
- **GPS IIIF**: Contract for 22 satellites with Lockheed Martin, first launch planned for 2026-2027
- **OCX**: Full operational capability expected by 2027

**Latest Interface Document Updates:**
- IS-GPS-200N (updated 08/22/2022) - L1/L2 specification
- IS-GPS-705J (updated 08/22/2022) - L5 specification
- IS-GPS-800J (updated 08/22/2022) - L1C specification
- IRN-003 (January 2024) - Interface Revision Notice
- Proposed changes for Civil Integrity Support Message (ISM) support - March 2025

### GPS Frequency Bands

| Band     | Frequency (MHz) | Purpose                         | Availability        | Status (2025)          |
|----------|-----------------|----------------------------------|---------------------|------------------------|
| L1       | 1575.42         | C/A, P(Y), M, L1C                | Civil/Military      | Fully operational      |
| L2       | 1227.60         | P(Y), L2C, M                     | Civil/Military      | Fully operational      |
| L3       | 1381.05         | NSS (Nuclear Detonation)         | Military            | Specialized            |
| L4       | 1379.913        | Additional ionospheric           | Research            | Experimental           |
| L5       | 1176.45         | L5                               | Civil               | Limited operational    |

### GPS L1 C/A - Detailed Structure ##############################################################

#### General Parameters:
- **Data Rate**: 50 bits/sec
- **C/A Code Length**: 1023 chips
- **C/A Code Frequency**: 1.023 MHz
- **Repetition Period**: 1 ms
- **Modulation**: BPSK

#### Navigation Message Structure:

**Superframe**: 12.5 minutes (37500 bits)
- 25 frames of 30 sec each

**Frame**: 30 seconds (1500 bits)
- 5 subframes of 6 sec each

**Subframe**: 6 seconds (300 bits)
- 10 words of 30 bits each

**Word**: 30 bits
- 24 data bits + 6 parity bits


#### Detailed Structure of Each Subframe:

**SUBFRAME 1 - Satellite Clock and Status Data**

| Bits  | Parameter        | Description                              | Units              |
|-------|------------------|------------------------------------------|--------------------|
| 1-8   | Preamble         | 10001011 (fixed sequence)                | -                  |
| 9-22  | TLM message      | Telemetry word                           | -                  |
| 23-24 | Integrity flag   | Data integrity status                    | -                  |
| 25-30 | TLM parity       | Parity bits for TLM                      | -                  |
| 31-52 | HOW              | Hand Over Word (transmission time)       | seconds            |
| 53-60 | HOW parity       | Parity bits for HOW                      | -                  |
| 61-70 | WN               | GPS week number                          | weeks              |
| 71-72 | L2 code          | (00=reserved, 01=P code, 10=C/A code)    | -                  |
| 73-76 | URA index        | User accuracy                            | -                  |
| 77-82 | SV health        | Satellite health status                  | -                  |
| 83-87 | IODC (MSB)       | 2 most significant bits Issue of Data Clock | -              |
| 88    | L2 P flag        | P code availability flag on L2           | -                  |
| 89-90 | Reserved         | -                                        | -                  |
| 91-103| TGD              | L1/L2 group delay                        | seconds (2^-31)    |
|104-111| IODC (LSB)       | 8 least significant bits Issue of Data Clock | -             |
|112-127| toc              | Clock correction reference time          | seconds (2^4)      |
|128-135| af2              | Satellite clock drift rate               | sec/sec² (2^-55)   |
|136-151| af1              | Satellite clock drift                    | sec/sec (2^-43)    |
|152-173| af0              | Satellite clock bias                     | seconds (2^-31)    |

**SUBFRAME 2 - Ephemeris Part 1**

| Bits  | Parameter        | Description                                  | Units               |
|-------|------------------|----------------------------------------------|---------------------|
| 1-30  | TLM+HOW          | Telemetry and transmission time              | -                   |
| 31-38 | IODE             | Issue of Data Ephemeris                      | -                   |
| 39-54 | Crs              | Sine harmonic correction to orbit radius     | meters (2^-5)       |
| 55-70 | Δn               | Mean motion difference                       | radians/sec (2^-43) |
| 71-102| M0               | Mean anomaly at reference time               | semicircles (2^-31) |
|103-118| Cuc              | Cosine harmonic correction to argument of latitude | radians (2^-29) |
|119-150| e                | Eccentricity                                 | dimensionless (2^-33)|
|151-166| Cus              | Sine harmonic correction to argument of latitude | radians (2^-29)  |
|167-198| √A               | Square root of semi-major axis               | √meters (2^-19)     |
|199-214| toe              | Ephemeris reference time                     | seconds (2^4)       |
|215-216| Fit flag         | Data fit interval                            | -                   |
|217-222| AODO             | Age of data offset                           | minutes             |
|223-248| Parity           | Parity bits                                  | -                   |

**SUBFRAME 3 - Ephemeris Part 2**

| Bits  | Parameter        | Description                                | Units              |
|-------|------------------|--------------------------------------------|--------------------|
| 1-30  | TLM+HOW          | Telemetry and transmission time            | -                  |
| 31-46 | Cic              | Cosine harmonic correction to inclination  | radians (2^-29)    |
| 47-78 | Ω0               | Longitude of ascending node at toe         | semicircles (2^-31)|
| 79-94 | Cis              | Sine harmonic correction to inclination    | radians (2^-29)    |
| 95-126| i0               | Inclination at toe                         | semicircles (2^-31)|
|127-142| Crc              | Cosine harmonic correction to orbit radius | meters (2^-5)      |
|143-174| ω                | Argument of perigee                        | semicircles (2^-31)|
|175-198| Ω̇                | Rate of change of longitude of node        | semicircles/sec (2^-43)|
|199-212| IODE             | Issue of Data Ephemeris                    | -                  |
|213-226| IDOT             | Rate of change of inclination              | semicircles/sec (2^-43)|
|227-248| Parity           | Parity bits                                | -                  |

**SUBFRAME 4 - Almanac and Ionospheric Data (pages 1-25)**

*Pages 1-24 (almanac for satellites 25-32)*
*Page 25 (special configuration)*

Content depends on page number in HOW:

**Pages 2, 3, 4, 5, 7, 8, 9, 10 (almanac)**
| Bits  | Parameter        | Description                              |
|-------|------------------|------------------------------------------|
| 31-38 | Data ID          | Almanac data identifier                  |
| 39-44 | SV ID            | Satellite number                         |
| 45-60 | e                | Eccentricity                             |
| 61-68 | toa              | Almanac reference time                   |
| 69-84 | δi               | Inclination correction relative to 0.3π  |
| 85-100| Ω̇                | Rate of change of longitude of node      |
|101-108| SVn health       | Satellite health status                  |
|109-132| √A               | Square root of semi-major axis           |
|133-156| Ω0               | Longitude of ascending node              |
|157-180| ω                | Argument of perigee                      |
|181-204| M0               | Mean anomaly                             |
|205-215| af0              | Clock correction                         |
|216-226| af1              | Clock drift                              |

**Page 18 (ionospheric and UTC parameters)**
| Bits  | Parameter        | Description                              |
|-------|------------------|------------------------------------------|
| 31-38 | Data ID          | Data identifier                          |
| 39-46 | α0               | Ionospheric parameter                    |
| 47-54 | α1               | Ionospheric parameter                    |
| 55-62 | α2               | Ionospheric parameter                    |
| 63-70 | α3               | Ionospheric parameter                    |
| 71-78 | β0               | Ionospheric parameter                    |
| 79-86 | β1               | Ionospheric parameter                    |
| 87-94 | β2               | Ionospheric parameter                    |
| 95-102| β3               | Ionospheric parameter                    |
|103-134| A1               | UTC parameter                            |
|135-166| A0               | UTC parameter                            |
|167-174| tot              | UTC data reference time                  |
|175-182| WNt              | UTC data week number                     |
|183-190| ΔtLS             | Leap seconds (current)                   |
|191-198| WNLSF            | Week number of future leap seconds       |
|199-206| DN               | Day of future leap seconds               |
|207-214| ΔtLSF            | Leap seconds (future)                    |

**SUBFRAME 5 - Almanac for Satellites 1-24**
Similar almanac page structure for satellites 1-24.


### GPS L2C - Detailed Structure ##############################################################

#### General Parameters:
- **Data Rate**: 25 symbols/sec (CM), 0.5 symbols/sec (CL) 
- **CM Code Length**: 10230 chips
- **CL Code Length**: 767250 chips
- **Modulation**: BPSK
- **Signal Structure**: Time-Multiplexed alternation of CM and CL

#### CNAV Message Structure:
- **Message**: 300 bits (12 seconds)
- **Data**: 276 bits + 24 bits CRC
- **Preamble**: 8 bits (10001011)
- **PRN ID**: 6 bits
- **Message Type**: 6 bits
- **TOW**: 17 bits (time of week)

**L2C CNAV Message Types:**

| Type | Name                    | Content                          | Priority   |
|------|-------------------------|----------------------------------|------------|
| 10   | Ephemeris-1             | Orbit parameters part 1          | High       |
| 11   | Ephemeris-2             | Orbit parameters part 2          | High       |
| 12   | Reduced almanac         | Reduced orbital data             | Medium     |
| 13   | Clock corrections       | Clock and group delay data       | High       |
| 14   | EOP and ionosphere      | Earth orientation and ionosphere | Low        |
| 15   | Text                    | Free text                        | Low        |
| 30   | ISM                     | Integrity Support Message        | High       |
| 31   | Clock corrections-2     | Extended clock parameters        | Medium     |
| 32   | EOP-2                   | Extended EOP parameters          | Low        |
| 33   | UTC                     | UTC time parameters              | Low        |
| 34   | Differential corrections| Regional corrections             | Medium     |
| 35   | GPS/GNSS time           | Inter-system offsets             | Low        |
| 36   | GGTO                    | GPS-GNSS Time Offset             | Low        |
| 37   | Constellation status    | Health of all satellites         | Medium     |

#### Detailed Structure of Message Type 10 (Ephemeris-1):

| Bits  | Parameter   | Description                          | Units                  |
|-------|-------------|--------------------------------------|------------------------|
| 1-8   | Preamble    | 10001011                             | -                      |
| 9-14  | PRN         | Satellite number                     | -                      |
| 15-20 | Message type| 10                                   | -                      |
| 21-37 | TOW         | Time of week                         | sec (LSB=6)            |
| 38    | Alert       | Alert flag                           | -                      |
| 39-48 | WN          | Truncated week number (mod 1024)     | weeks                  |
| 49-59 | toc         | Clock reference time                 | sec (LSB=300)          |
| 60-64 | URA_NED0    | Accuracy index                       | -                      |
| 65-72 | URA_NED1    | ED accuracy index                    | -                      |
| 73-80 | URA_NED2    | ED accuracy index                    | -                      |
| 81-90 | toe         | Ephemeris reference time             | sec (LSB=300)          |
| 91-114| A           | Semi-major axis difference from nominal | m (LSB=2^-9)        |
|115-131| Adot        | Rate of change of A                  | m/s (LSB=2^-21)        |
|132-148| Δn0         | Mean motion difference               | semicircles/s (LSB=2^-44)|
|149-165| Δndot       | Rate of change of Δn                 | semicircles/s² (LSB=2^-57)|
|166-198| M0          | Mean anomaly                         | semicircles (LSB=2^-32) |
|199-231| e           | Eccentricity                         | dimensionless (LSB=2^-34)|
|232-264| ω           | Argument of perigee                  | semicircles (LSB=2^-32) |

#### Detailed Structure of Message Type 11 (Ephemeris-2):

| Bits  | Parameter   | Description                          | Units                |
|-------|-------------|--------------------------------------|----------------------|
| 1-20  | Header      | Preamble, PRN, type                  | -                    |
| 21-37 | TOW         | Time of week                         | sec (LSB=6)          |
| 38    | Alert       | Alert flag                           | -                    |
| 39-72 | Ω0          | Longitude of ascending node          | semicircles (LSB=2^-32)|
| 73-106| i0          | Inclination                          | semicircles (LSB=2^-32)|
|107-124| Δi0-dot     | Rate of change of inclination        | semicircles/s (LSB=2^-44)|
|125-158| Ω-dot       | Rate of right ascension              | semicircles/s (LSB=2^-44)|
|159-175| Cis         | Sine harmonic amplitude inclination  | rad (LSB=2^-30)      |
|176-192| Cic         | Cosine harmonic amplitude inclination| rad (LSB=2^-30)      |
|193-213| Crs         | Sine harmonic amplitude radius       | m (LSB=2^-8)         |
|214-234| Crc         | Cosine harmonic amplitude radius     | m (LSB=2^-8)         |
|235-247| Cus         | Sine harmonic amplitude argument     | rad (LSB=2^-30)      |
|248-260| Cuc         | Cosine harmonic amplitude argument   | rad (LSB=2^-30)      |



### GPS L5 - Detailed Structure ##############################################################

#### General Parameters:
- **Data Rate**: 50 symbols/sec (100 bits/sec with encoding)
- **Code Length**: 10230 chips (I5, Q5)
- **Modulation**: QPSK
- **FEC**: Convolutional encoding K=7, r=1/2
- **Neuman-Hoffman code**: NH(10) on I5
- **Signal Structure**: I5 (data) + Q5 (pilot)

#### CNAV Message Structure:
- **Message**: 300 bits (6 seconds)
- **Data**: 276 bits + 24 bits CRC
- **Preamble**: 8 bits (10001011)
- **PRN ID**: 6 bits
- **Message Type**: 6 bits
- **TOW**: 17 bits

**L5 CNAV Message Types:**
Uses the same message types as L2C but with enhanced capabilities.

### GPS L1C - Detailed Structure ##############################################################

#### General Parameters:
- **Data Rate**: 100 symbols/sec (data), 0 symbols/sec (pilot)
- **Code Length**: 10230 chips
- **Modulation**: TMBOC(6,1,4/33) - Time Multiplexed BOC
- **FEC**: LDPC (Low Density Parity Check) 7/8
- **Interleaving**: Block interleaving
- **Overlay code**: 1800 symbols on L1C-D

#### L1C Signal Structure:
L1C consists of two components:
- **L1C-D (Data)**: navigation data transmission (25% power)
- **L1C-P (Pilot)**: pilot channel without data (75% power)

#### CNAV-2 Message Structure:
- **Frame**: 1800 symbols (18 seconds)
- **Subframe 1**: 9 symbols - TOW counter
- **Subframe 2**: 600 symbols - encoded data
- **Subframe 3**: 274 symbols - FEC symbols

**L1C CNAV-2 Message Types:**

| Type | Name                        | Content                    | Period   |
|------|---------------------------- |----------------------------|----------|
| 10   | Ephemeris-1                 | Orbital parameters part 1  | 18 sec   |
| 11   | Ephemeris-2                 | Orbital parameters part 2  | 18 sec   |
| 12   | Reduced almanac             | Constellation almanac      | 18 sec   |
| 13   | Clock corrections           | Clock and group delay data | 18 sec   |
| 14   | Reduced ephemeris           | Short-term orbital data    | 18 sec   |
| 15   | Text                        | Free text                  | 18 sec   |
| 30   | ISM-I                       | Integrity Support Message  | 18 sec   |
| 31   | ISM-A                       | ISM for almanac            | 18 sec   |
| 32   | EOP                         | Earth orientation parameters| 18 sec   |
| 33   | UTC                         | UTC time parameters        | 18 sec   |
| 34   | Differential corrections    | Regional enhancements      | 18 sec   |
| 35   | GGTO                        | GPS-GNSS Time Offset       | 18 sec   |
| 36   | Constellation data          | Status of all satellites   | 18 sec   |
| 37   | System time                 | Time synchronization       | 18 sec   |

#### Detailed Structure of Message Type 10 (Ephemeris-1) CNAV-2:

| Bits  | Parameter      | Description                       | Units                  |
|-------|----------------|-----------------------------------|------------------------|
| 1-9   | TOW            | Time of week (counter)            | 18-sec intervals       |
| 10-15 | PRN            | Satellite number                  | -                      |
| 16-21 | Message type   | 10 for ephemeris-1                | -                      |
| 22-32 | WN             | GPS week number                   | weeks                  |
| 33-40 | Health         | Satellite status                  | -                      |
| 41-51 | top            | Prediction time                   | sec (LSB=300)          |
| 52-62 | URA_NED        | Accuracy index                    | -                      |
| 63-73 | toe            | Ephemeris reference time          | sec (LSB=300)          |
| 74-99 | ΔA             | Semi-major axis difference        | m (LSB=2^-9)           |
|100-116| Δdot           | Rate of change of ΔA              | m/s (LSB=2^-21)        |
|117-139| Δn0            | Mean motion correction            | semicircles/s (LSB=2^-44)|
|140-161| Δndot          | Rate of change of Δn              | semicircles/s² (LSB=2^-57)|
|162-194| M0             | Mean anomaly                      | semicircles (LSB=2^-32) |
|195-227| e              | Eccentricity                      | dimensionless (LSB=2^-34)|
|228-260| ω              | Argument of perigee               | semicircles (LSB=2^-32) |
|261-274| CRC            | Checksum                          | -                      |

---

## GLONASS (Russia)

### Updated GLONASS Information (2025)

**Constellation Status:** As of June 2025, GLONASS includes 27 satellites in use (nominally 24 for full coverage). Last launch: May 26, 2025.

**Operator:** Roscosmos (Russian Federation)

**Accuracy:** 
- Open signal: 2.8-7.38 meters
- High-precision signal (HP): up to 0.6 meters (in development)

**Orbital Characteristics:**
- Orbital altitude: 19,140 km (medium Earth orbit)
- Inclination: 64.8°
- Orbital period: 11 hours 15 minutes 44 seconds
- 3 orbital planes with 8 satellites each

**Latest Satellite Generations:**
- **GLONASS-M**: modernized satellites with L2OF signal (main part of constellation)
- **GLONASS-K1**: added L3OC signal (CDMA), 2 satellites in orbit
- **GLONASS-K2**: launched in 2023-2025, full signal modernization, 3 satellites

**System Modernization:**
- Transition to CDMA signals (L3OC) for compatibility with other GNSS
- Implementation of high-precision code (HP) on L1/L2
- Development of inter-satellite links (ISL)
- Planned launch of GLONASS-V from 2027

### GLONASS Frequency Bands

| Band     | Base Frequency (MHz) | Frequency Step (MHz)| Channels k    | Purpose                     | Status (2025)           |
|----------|---------------------|-------------------- |---------------|----------------------------|-------------------------|
| L1OF     | 1602.0              | 0.5625              | -7...+6       | Standard accuracy (FDMA)    | Fully operational       |
| L2OF     | 1246.0              | 0.4375              | -7...+6       | Standard accuracy (FDMA)    | Fully operational       |
| L1SF     | 1600.995            | -                   | CDMA          | Protected signal            | Limited operational     |
| L2SF     | 1248.06             | -                   | CDMA          | Protected signal            | Limited operational     |
| L3OC     | 1202.025            | -                   | CDMA          | New generation (CDMA)       | Testing                 |
| L1OCM    | 1575.42             | -                   | CDMA          | Inter-system compatibility  | In development          |
| L5OC     | 1176.45             | -                   | CDMA          | GPS/Galileo compatibility   | In development          |

### GLONASS L1OF/L2OF - Detailed Structure (FDMA) ##############################################################

#### General Parameters:
- **Data Rate**: 50 bits/sec
- **Code Length**: 511 chips
- **Code Frequency**: 0.511 MHz
- **Repetition Period**: 1 ms
- **Modulation**: BPSK
- **Encoding**: Hamming code (8,4) with modulo 2 check

#### Navigation Message Structure:

**Superframe**: 2.5 minutes (7500 bits)
- 5 frames of 30 sec each

**Frame**: 30 seconds (1500 bits)
- 15 strings of 2 sec each

**String**: 2 seconds (100 bits)
- 85 information bits
- 8 Hamming code check bits
- 7 bits for time mark (padding)

#### Detailed String Structure:

**STRING 1 - Satellite Navigation Data (X coordinate)**

| Bits  | Parameter   | Description                         | Units              |
|-------|-------------|-------------------------------------|--------------------|
| 1-4   | m           | String number (0001)                | -                  |
| 5     | P           | Unreliability flag                  | -                  |
| 6-7   | Reserved    | Reserved                            | -                  |
| 8     | P1          | tb update flag                      | -                  |
| 9-13  | tk(h+m)     | Current time hours+minutes          | hour, min          |
| 14-19 | tk(m)       | Current time minutes                | min                |
| 20    | tk(s*2)     | Current time seconds (double)       | 30 sec             |
| 21-27 | xn(tb)      | X coordinate in PZ-90.11 (MSB)      | km (LSB=2^-11)     |
| 28-48 | xn(tb)      | X coordinate in PZ-90.11 (LSB)      | km (LSB=2^-11)     |
| 49-53 | ẋn(tb)      | X velocity (MSB)                    | km/s (LSB=2^-20)   |
| 54-68 | ẋn(tb)      | X velocity (LSB)                    | km/s (LSB=2^-20)   |
| 69-73 | ẍn(tb)      | X acceleration                      | km/s² (LSB=2^-30)  |
| 74-80 | Bn          | Satellite n unhealthy flag          | -                  |
| 81-85 | P2          | tb parity flag                      | -                  |

**STRING 2 - Satellite Navigation Data (Y coordinate)**

| Bits  | Parameter   | Description                         | Units              |
|-------|-------------|-------------------------------------|--------------------|
| 1-4   | m           | String number (0010)                | -                  |
| 5-7   | Bn          | Unhealthy flag (continued)          | -                  |
| 8     | P           | Satellite mode flag                 | -                  |
| 9-10  | tb          | tb time index within day            | 15 min             |
| 11    | Reserved    | Reserved                            | -                  |
| 12-16 | tb          | 15-minute interval number           | 15 min             |
| 17-21 | NT          | Calendar day number (MSB)           | days               |
| 22-31 | NT          | Calendar day number (LSB)           | days               |
| 32-36 | FT          | Carrier frequency correction        | -                  |
| 37    | Reserved    | Reserved                            | -                  |
| 38-41 | n           | Satellite number in system          | -                  |
| 42-46 | ΔτnA        | Coarse time correction              | sec (LSB=2^-9)     |
| 47-51 | yn(tb)      | Y coordinate in PZ-90.11 (MSB)      | km (LSB=2^-11)     |
| 52-68 | yn(tb)      | Y coordinate in PZ-90.11 (LSB)      | km (LSB=2^-11)     |
| 69-73 | ẏn(tb)      | Y velocity (MSB)                    | km/s (LSB=2^-20)   |
| 74-84 | ẏn(tb)      | Y velocity (LSB)                    | km/s (LSB=2^-20)   |
| 85    | ÿn(tb)      | Y acceleration                      | km/s² (LSB=2^-30)  |

**STRING 3 - Satellite Navigation Data (Z coordinate)**

| Bits  | Parameter   | Description                         | Units                   |
|-------|-------------|-------------------------------------|-------------------------|
| 1-4   | m           | String number (0011)                | -                       |
| 5     | P           | Satellite time flag                 | -                       |
| 6-10  | γn(tb)      | Relative frequency deviation        | dimensionless (LSB=2^-40)|
| 11    | Reserved    | Reserved                            | -                       |
| 12-13 | P3          | GLONASS-M flag                      | -                       |
| 14-18 | ln          | Health flag for KI                  | -                       |
| 19-23 | zn(tb)      | Z coordinate in PZ-90.11 (MSB)      | km (LSB=2^-11)          |
| 24-40 | zn(tb)      | Z coordinate in PZ-90.11 (LSB)      | km (LSB=2^-11)          |
| 41-45 | żn(tb)      | Z velocity (MSB)                    | km/s (LSB=2^-20)        |
| 46-60 | żn(tb)      | Z velocity (LSB)                    | km/s (LSB=2^-20)        |
| 61-65 | z̈n(tb)      | Z acceleration                      | km/s² (LSB=2^-30)       |
| 66-80 | Reserved    | Reserved                            | -                       |
| 81-85 | Reserved    | Reserved                            | -                       |

**STRING 4 - Time Corrections and Additional Data**

| Bits  | Parameter   | Description                         | Units              |
|-------|-------------|-------------------------------------|--------------------|
| 1-4   | m           | String number (0100)                | -                  |
| 5-22  | τn(tb)      | Satellite n time scale correction   | sec (LSB=2^-30)    |
| 23-27 | Δτn         | tn and tc scale difference          | sec (LSB=2^-30)    |
| 28-32 | En          | Age of operational information      | days               |
| 33    | Reserved    | Reserved                            | -                  |
| 34    | P4          | Ephemeris update flag               | -                  |
| 35-49 | FT          | Predicted frequency correction      | -                  |
| 50-60 | NT          | Day number within four-year period  | days               |
| 61-65 | n           | Satellite number in system          | -                  |
| 66-67 | M           | Satellite type (01-M, 10-K1, 11-K2) | -                  |
| 68-79 | NA          | Calendar almanac day number         | days               |
| 80-81 | τc          | System scale correction             | sec (LSB=2^-31)    |
| 82-84 | N4          | Four-year period number             | 4 years            |
| 85    | τGPS        | GPS-GLONASS correction              | sec (LSB=2^-30)    |

**STRINGS 5-15 - Almanac**

Strings 6, 8, 10, 12, 14 (odd almanac satellites):

| Bits  | Parameter   | Description                         |
|-------|-------------|-------------------------------------|
| 1-4   | m           | String number                       |
| 5     | nA          | Almanac satellite frequency literal |
| 6     | HA          | nA unusability flag                 |
| 7-21  | λnA         | Longitude of ascending node (MSB)   |
| 22-26 | λnA         | Longitude of ascending node (LSB)   |
| 27-47 | tiA         | Ascending node passage time         |
| 48-57 | ΔinA        | Inclination correction (+0.067π rad)|
| 58-72 | ΔTA         | Orbital period correction           |
| 73-77 | ΔT'nA       | Rate of change of period            |
| 78-84 | εnA         | Eccentricity                        |
| 85    | ωnA         | Argument of perigee                 |

Strings 7, 9, 11, 13, 15 (even almanac satellites):

| Bits  | Parameter   | Description                         |
|-------|-------------|-------------------------------------|
| 1-4   | m           | String number                       |
| 5-15  | Continuation of previous satellite almanac     |
| 16    | CnA         | Generalized nA satellite flag       |
| 17-41 | Similar structure for even satellite           |
| 42-63 | τnA         | Coarse time correction value        |
| 64-65 | lnA         | Almanac modification flag           |
| 66-80 | CnA         | Satellite status flag               |

**STRING 5 - System Status**

| Bits  | Parameter              | Description                         |
|-------|----------------------- |-------------------------------------|
| 1-4   | m                      | String number (0101)                |
| 5-15  | NA                     | Day within four-year almanac period |
| 16-17 | τc                     | System time scale correction        |
| 18-19 | N4                     | Four-year period number             |
| 20-26 | τGPS                   | GLONASS-GPS correction              |
| 27    | ln5                    | String 5 validity flag              |
| 28-49 | Reserved               | Reserved                            |
| 50-80 | System status word     | Additional data                     |
| 81-85 | Reserved               | Reserved                            |

### GLONASS L3OC - Detailed Structure (CDMA) ##############################################################

#### General Parameters:
- **Frequency**: 1202.025 MHz
- **Data Rate**: 100 symbols/sec (200 bits/sec with encoding)
- **Code Length**: 10230 chips (primary), 250 chips (secondary)
- **Modulation**: BPSK(10)
- **FEC**: Convolutional encoding K=7, r=1/2
- **Structure**: L3OC-I (data) + L3OC-Q (pilot)

#### L3OC Navigation Message Structure:
- **Superframe**: 2 minutes (12000 bits)
- **Frame**: 10 seconds (1000 bits)
- **String**: 2 seconds (200 bits)
- **Word**: 100 bits information + 100 bits after encoding

#### L3OC String Types:

| Type | Content                     | Repetition Period |
|------|----------------------------|-------------------|
| 1    | Ephemeris and time         | 10 sec            |
| 2    | Clock parameters           | 10 sec            |
| 3    | Satellite almanac          | 60 sec            |
| 4    | System parameters          | 60 sec            |
| 5    | Constellation status       | 120 sec           |
| 6    | UTC and ionospheric params | 120 sec           |
| 10   | Additional data            | Variable          |
| 12   | Integrity messages         | 10 sec            |
| 14   | Reserved                   | -                 |
| 16   | Text messages              | Variable          |

---

## Galileo (EU)

### Updated Galileo Information (2025)

**Constellation Status:** As of June 2025, Galileo includes 28 satellites in use (nominally 24 active and 6 spare).

**Operator:** EUSPA (EU Agency for the Space Programme), ESA

**Accuracy:** 
- Open Service (OS): Up to 20 cm horizontally, 40 cm vertically
- High Accuracy Service (HAS): Up to 20 cm in real-time (since January 2023)
- Commercial Service (CS): Closed in 2023, functions transferred to HAS

**Orbital Characteristics:**
- Orbital altitude: 23,222 km
- Inclination: 56°
- Orbital period: 14 hours 4 minutes 45 seconds
- 3 orbital planes (A, B, C) with 8 active satellites each
- Walker 24/3/1 constellation + 6 spares

**Latest Launches and Updates:**
- Last launch: September 17, 2024 (2 satellites)
- Galileo second generation (G2): contract signed, first launch planned for 2026
- Full Operational Capability (FOC) achieved in 2024

**Galileo Services (2025):**
1. **Open Service (OS)** - free open service
2. **High Accuracy Service (HAS)** - high accuracy service (free since 2023)
3. **Public Regulated Service (PRS)** - secure government service
4. **Search and Rescue (SAR)** - search and rescue service with return link

### Galileo Frequency Bands

| Band     | Frequency (MHz) | Service      | Modulation      | Rate (symbols/sec) | Status (2025)          |
|----------|-----------------|--------------|-----------------|-------------------|------------------------|
| E1       | 1575.42         | OS/CS/PRS    | CBOC(6,1,1/11)  | 250               | Fully operational      |
| E5a      | 1176.45         | OS           | AltBOC(15,10)   | 50                | Fully operational      |
| E5b      | 1207.14         | OS/CS        | AltBOC(15,10)   | 250               | Fully operational      |
| E5       | 1191.795        | OS           | AltBOC(15,10)   | 50/250            | Fully operational      |
| E6       | 1278.75         | CS/HAS/PRS   | BPSK(5)         | 1000              | Fully operational      |

### Galileo E1-B I/NAV - Detailed Structure

#### General Parameters:
- **Data Rate**: 250 symbols/sec (125 bits/sec before encoding)
- **Code Length**: 4092 chips
- **Modulation**: CBOC(6,1,1/11) for E1-B and E1-C
- **FEC**: Convolutional encoding K=7, r=1/2
- **Interleaving**: Block interleaving 8x30

#### Navigation Message Structure:

**Nominal Page**: 2 seconds
- Even/odd part: 1 second each (125 symbols)
- 120 data symbols + 6 tail symbols

**Subframe**: 30 seconds
- 15 nominal pages

**Frame**: 720 seconds
- 24 subframes

#### I/NAV Page Content:

**Word 0 - Time and Status**
| Bits  | Parameter   | Description                         | Units            |
|-------|-------------|-------------------------------------|------------------|
| 1-6   | Type        | Word type (0)                       | -                |
| 7-8   | Time        | Time status                         | -                |
| 9-10  | Spare       | Reserved                            | -                |
| 11-20 | WN          | Galileo week number                 | weeks            |
| 21-32 | TOW         | Time of week                        | sec              |
| 33-38 | Signal      | DVS signal status                   | -                |
| 39-44 | SAR         | Search and rescue data              | -                |
| 45-46 | Spare       | Reserved                            | -                |
| 47-50 | CRC+tail    | Checksum                            | -                |

**Word 1 - Ephemeris Part 1**
| Bits  | Parameter   | Description                       | Units                  |
|-------|-------------|-----------------------------------|------------------------|
| 1-6   | Type        | Word type (1)                     | -                      |
| 7-16  | IODnav      | Issue of Data navigation          | -                      |
| 17-32 | t0e         | Ephemeris reference time          | sec (LSB=60)           |
| 33-64 | M0          | Mean anomaly at t0e               | semicircles (LSB=2^-31)|
| 65-96 | e           | Eccentricity                      | dimensionless (LSB=2^-33)|
| 97-128| √A          | Square root of semi-major axis    | √m (LSB=2^-19)         |

**Word 2 - Ephemeris Part 2**
| Bits  | Parameter   | Description                       | Units                |
|-------|-------------|-----------------------------------|----------------------|
| 1-6   | Type        | Word type (2)                     | -                    |
| 7-16  | IODnav      | Issue of Data navigation          | -                    |
| 17-48 | Ω0          | Longitude of ascending node at t0e| semicircles (LSB=2^-31)|
| 49-80 | i0          | Inclination at t0e                | semicircles (LSB=2^-31)|
| 81-112| ω           | Argument of perigee               | semicircles (LSB=2^-31)|
|113-126| IDOT        | Rate of change of inclination     | semicircles/s (LSB=2^-43)|
|127-128| Spare       | Reserved                          | -                    |

**Word 3 - Ephemeris Part 3 and SISA**
| Bits  | Parameter   | Description                       | Units                |
|-------|-------------|-----------------------------------|----------------------|
| 1-6   | Type        | Word type (3)                     | -                    |
| 7-16  | IODnav      | Issue of Data navigation          | -                    |
| 17-40 | Ω̇           | Rate of change of longitude of node| semicircles/s (LSB=2^-43)|
| 41-56 | Δn          | Mean motion difference            | semicircles/s (LSB=2^-43)|
| 57-72 | Cuc         | Cosine harmonic correction latitude| rad (LSB=2^-29)     |
| 73-88 | Cus         | Sine harmonic correction latitude | rad (LSB=2^-29)      |
| 89-104| Crc         | Cosine harmonic correction radius | m (LSB=2^-5)         |
|105-120| Crs         | Sine harmonic correction radius   | m (LSB=2^-5)         |
|121-128| SISA        | Signal in space accuracy index    | -                    |

**Word 4 - Ephemeris Part 4 and Clock Corrections**
| Bits  | Parameter   | Description                          | Units             |
|-------|-------------|--------------------------------------|-------------------|
| 1-6   | Type        | Word type (4)                        | -                 |
| 7-16  | IODnav      | Issue of Data navigation             | -                 |
| 17-22 | SVID        | Satellite identifier                 | -                 |
| 23-38 | Cic         | Cosine harmonic correction inclination| rad (LSB=2^-29)  |
| 39-54 | Cis         | Sine harmonic correction inclination  | rad (LSB=2^-29)  |
| 55-68 | t0c         | Clock reference time                 | sec (LSB=60)      |
| 69-100| af0         | Satellite clock bias                 | sec (LSB=2^-34)   |
|101-121| af1         | Satellite clock drift                | sec/sec (LSB=2^-46)|
|122-128| af2         | Satellite clock drift rate           | sec/sec² (LSB=2^-59)|

**Word 5 - Ionospheric Corrections, BGD, Signal Health and GST-UTC**
| Bits  | Parameter   | Description                       | Units             |
|-------|-------------|-----------------------------------|-------------------|
| 1-6   | Type        | Word type (5)                     | -                 |
| 7-17  | ai0         | Effective ionization              | sfu (LSB=2^-2)    |
| 18-28 | ai1         | First order coefficient           | sfu/deg (LSB=2^-8)|
| 29-42 | ai2         | Second order coefficient          | sfu/deg² (LSB=2^-15)|
| 43-47 | Iono flags  | Regional ionosphere flags         | -                 |
| 48-57 | BGD E1/E5a  | E1-E5a group delay                | ns (LSB=2^-32)    |
| 58-67 | BGD E1/E5b  | E1-E5b group delay                | ns (LSB=2^-32)    |
| 68-69 | E5b-HS      | E5b signal health                 | -                 |
| 70-71 | E1B-HS      | E1-B signal health                | -                 |
| 72-73 | E5a-HS      | E5a signal health                 | -                 |
| 74-75 | E1A-HS      | E1-A signal health                | -                 |
| 76-107| A0          | GST-UTC bias constant             | sec (LSB=2^-30)   |
|108-128| A1          | GST-UTC drift coefficient         | sec/sec (LSB=2^-50)|

**Word 6 - GST-UTC Conversion**
| Bits  | Parameter   | Description                       | Units           |
|-------|-------------|-----------------------------------|-----------------|
| 1-6   | Type        | Word type (6)                     | -               |
| 7-38  | A0          | Bias constant (continued)         | sec (LSB=2^-30) |
| 39-62 | A1          | Drift coefficient (continued)     | sec/sec (LSB=2^-50)|
| 63-70 | ΔtLS        | Current leap second               | sec             |
| 71-78 | t0t         | UTC reference time                | hour (LSB=3600) |
| 79-86 | WN0t        | UTC week number                   | weeks           |
| 87-94 | WNLSF       | Week number of future correction  | weeks           |
| 95-102| DN          | Day of week of future correction  | days            |
|103-110| ΔtLSF       | Future leap second                | sec             |
|111-128| TOW         | Time of week                      | sec             |

**Words 7-9 - Almanac**
Contain almanac data for Galileo satellites (2 satellites per word).

**Word 10 - Almanac and A0G, A1G, t0G, WN0G**
Contains almanac and GST-GPS offset parameters.

**Word 0 (alternative) - SAR Messages**
Used for transmitting search and rescue service data.

### Galileo F/NAV (E5a) - Detailed Structure

#### General Parameters:
- **Data Rate**: 50 symbols/sec (25 bits/sec before encoding)
- **Code Length**: 10230 chips for E5a-I and E5a-Q
- **Modulation**: Part of AltBOC(15,10)
- **FEC**: Convolutional encoding K=7, r=1/2
- **Interleaving**: Block interleaving

#### F/NAV Page Structure:
- **Page**: 10 seconds (250 bits before encoding)
- **After encoding**: 500 symbols + 8 tail symbols
- **Preamble**: 12 synchronization symbols

#### F/NAV Page Types:

| Type | Content                    | Update Frequency |
|------|---------------------------|------------------|
| 1    | Ephemeris SVID and IODnav | 50 sec           |
| 2    | Ephemeris                 | 50 sec           |
| 3    | Ephemeris and GST-UTC     | 50 sec           |
| 4    | Ephemeris and GST-GPS     | 50 sec           |
| 5    | Almanac, WN and TOW       | 50 sec           |
| 6    | Almanac and reserved      | As needed        |

#### Detailed Structure of F/NAV Page Type 1:

| Bits  | Parameter   | Description                       | Units                  |
|-------|-------------|-----------------------------------|------------------------|
| 1-6   | Type        | Page type (1)                     | -                      |
| 7-8   | Spare       | Reserved                          | -                      |
| 9-18  | IODnav      | Issue of Data navigation          | -                      |
| 19-32 | t0e         | Ephemeris reference time          | min (LSB=10)           |
| 33-64 | M0          | Mean anomaly at t0e               | semicircles (LSB=2^-31)|
| 65-96 | e           | Eccentricity                      | dimensionless (LSB=2^-33)|
| 97-128| √A          | Square root of semi-major axis    | √m (LSB=2^-19)         |
|129-160| Ω0          | Longitude of ascending node       | semicircles (LSB=2^-31)|
|161-192| i0          | Inclination at t0e                | semicircles (LSB=2^-31)|
|193-224| ω           | Argument of perigee               | semicircles (LSB=2^-31)|
|225-238| IDOT        | Rate of change of inclination     | semicircles/s (LSB=2^-43)|
|239-244| Spare       | Reserved                          | -                      |

### Galileo E6 HAS - Detailed Structure

#### General Parameters:
- **Frequency**: 1278.75 MHz
- **Data Rate**: 1000 symbols/sec (500 bits/sec before encoding)
- **Code Length**: 5115 chips for E6-B and E6-C
- **Modulation**: BPSK(5)
- **FEC**: Reed-Solomon code (255,223)
- **Purpose**: High Accuracy Service (HAS)

#### HAS Message Structure:
- **HAS Page**: 1 second (492 bits of information)
- **Header**: 24 bits
- **Data**: 448 bits
- **FEC**: 20 check symbol bits
- **CRC**: 24 bits

#### E6 Components:
- **E6-B**: Data channel (50% power)
- **E6-C**: Pilot channel (50% power)

#### HAS Message Types:

| MT  | Name                       | Content                   | Size     |
|-----|---------------------------|---------------------------|----------|
| 1   | Mask                      | Satellite and signal mask | Variable |
| 2   | Orbit corrections         | Full orbit corrections    | Variable |
| 3   | Full clock corrections    | Full clock corrections    | Variable |
| 4   | Partial clock corrections | Clock correction subsets  | Variable |
| 5   | Code bias                 | Code biases               | Variable |
| 6   | Phase bias                | Carrier phase biases      | Variable |
| 7   | Combined bias             | Combined biases           | Variable |
| 8-15| Reserved                  | Future use                | -        |

#### Detailed HAS Header Structure:

| Bits | Parameter     | Description                       | Units       |
|------|---------------|-----------------------------------|-------------|
| 1-8  | Status        | HAS status and message type       | -           |
| 9-13 | Message Type  | Message Type (MT)                 | -           |
| 14-19| Message ID    | Message ID (MID)                  | -           |
| 20-24| Size          | Message size                      | 32-bit words|

#### MT1 Message Format (Mask):

| Bits  | Parameter        | Description                  |
|-------|------------------|------------------------------|
| 1-5   | Validity Time    | Validity time                |
| 6-17  | IOD Set ID       | Data set identifier          |
| 18-81 | GNSS Mask        | GNSS systems mask            |
| 82-145| Satellite Mask   | Satellite mask               |
|146-209| Signal Mask      | Signal mask                  |
|210-224| Mask IOD         | IOD for mask                 |

---

## BeiDou (China)

### Updated BeiDou Information (2025)

**Constellation Status:** As of June 2025, BeiDou-3 includes 35 satellites in use (nominally 30 active).

**Operator:** China National Space Administration (CNSA)

**Accuracy:** 
- Global: 3.6 m (horizontal), 5.1 m (vertical)
- Asia-Pacific region: 2.6 m (horizontal), 4.3 m (vertical)
- PPP service: 10 cm (encrypted)
- SBAS service: 1 m (regional)

**Orbital Characteristics:**
- 24 satellites in medium Earth orbits (MEO) - altitude 21,528 km
- 3 satellites in inclined geosynchronous orbits (IGSO) - altitude 35,786 km
- 3 satellites in geostationary orbits (GEO) - altitude 35,786 km

**System Evolution:**
- **BeiDou-1**: 2000-2012 (experimental system)
- **BeiDou-2**: 2012-2020 (regional system)
- **BeiDou-3**: since 2020 (global system)
- **BeiDou-3 enhanced**: 2025+ (enhanced capabilities)

**Latest Updates:**
- BeiDou-3 full operational capability: July 31, 2020
- Latest launch: March 2025 (backup satellite)
- Planned launches: 6-8 satellites by 2027

### BeiDou Frequency Bands

| Band     | Frequency (MHz) | Service           | Modulation      | Rate (symbols/sec) | Status (2025)          |
|----------|-----------------|-------------------|-----------------|-------------------|------------------------|
| B1I      | 1561.098        | Open              | BPSK(2)         | 50                | Fully operational      |
| B1C      | 1575.42         | Open              | BOC(1,1)+QMBOC  | 100               | Fully operational      |
| B2a      | 1176.45         | Open              | QPSK(10)        | 200               | Fully operational      |
| B2b      | 1207.14         | Open/PPP          | QPSK(10)        | 200               | Fully operational      |
| B2(B2a+b)| 1191.795        | Open              | AltBOC(15,10)   | 200               | Fully operational      |
| B3I      | 1268.52         | Authorized        | BPSK(10)        | 50                | Fully operational      |
| B1A      | 1575.42         | Authorized        | BOC(14,2)       | 200               | Limited access         |
| B2A      | 1191.795        | Authorized        | ACE-BOC         | 200               | Limited access         |
| B3A      | 1268.52         | Authorized        | BOC(15,2.5)     | 200               | Limited access         |

### BeiDou B1I D1 NAV - Detailed Structure (Legacy Format)

#### General Parameters:
- **Data Rate**: 50 bits/sec
- **Code Length**: 2046 chips
- **Code Frequency**: 2.046 MHz
- **Repetition Period**: 1 ms
- **Modulation**: BPSK(2)
- **Usage**: BeiDou-2 and early BeiDou-3 satellites

#### D1 Navigation Message Structure:

**Superframe**: 12 minutes (36000 bits)
- 24 frames of 30 sec each

**Frame**: 30 seconds (1500 bits)
- 5 subframes of 6 sec each

**Subframe**: 6 seconds (300 bits)
- 10 words of 30 bits each

**Word**: 30 bits
- 22 data bits + 8 parity bits (BCH)

### BeiDou B1I D2 NAV - Detailed Structure (for GEO)

#### General Parameters:
- **Data Rate**: 500 bits/sec
- **Message Period**: 0.6 seconds (300 bits)
- **Modulation**: BPSK(2)
- **Usage**: GEO satellites only

### BeiDou B-CNAV1 (B1C) - Detailed Structure

#### General Parameters:
- **Data Rate**: 100 symbols/sec (data), 0 symbols/sec (pilot)
- **Code Length**: 10230 chips
- **Modulation**: BOC(1,1) for data + QMBOC(6,1,4/33) overall
- **Power Ratio**: 1:3 (data:pilot)
- **FEC**: LDPC(64,56) + BCH

#### B1C Signal Structure:
B1C consists of two components:
- **B1C-data**: navigation data transmission (25% power)
- **B1C-pilot**: pilot channel for improved tracking (75% power)

#### B-CNAV1 Frame Structure:
- **Frame**: 1800 symbols (18 seconds)
- **Subframe 1**: 72 symbols (synchronization and status)
- **Subframe 2**: 1200 symbols (navigation data)
- **Subframe 3**: 528 symbols (FEC)

**B-CNAV1 Message Types:**

| Type | Name                        | Content                    | Priority |
|------|---------------------------- |----------------------------|----------|
| 10   | Ephemeris                   | Full orbital parameters    | High     |
| 11   | Clock corrections           | Satellite clock parameters | High     |
| 30   | Ionospheric corrections     | BDGIM model                | Medium   |
| 31   | BDT-UTC time                | Time conversion parameters | Medium   |
| 32   | BDT-GNSS time               | Inter-system offsets       | Low      |
| 33   | EOP parameters              | Earth orientation params   | Low      |
| 34   | Reduced almanac             | MIDI almanac data          | Medium   |
| 35   | MIDI almanac                | Reduced constellation data | Medium   |
| 40   | Health status               | All satellites status      | Medium   |
| 50   | PPP-B2b corrections         | Differential corrections   | High     |
| 51   | PPP-B2b mask                | PPP service mask           | High     |
| 60   | Differential corrections    | Regional enhancements      | Medium   |
| 63   | Reserved                    | Future extensions          | -        |

#### Detailed Structure of B-CNAV1 Message Type 10 (Ephemeris):

| Bits  | Parameter   | Description                          | Units                  |
|-------|-------------|--------------------------------------|------------------------|
| 1-12  | PRN         | BeiDou satellite number              | -                      |
| 13-18 | MesType     | Message type (10)                    | -                      |
| 19-36 | SOW         | BeiDou week seconds                  | sec                    |
| 37-49 | WN          | BeiDou week number                   | weeks                  |
| 50-58 | IODC        | Issue of Data Clock                  | -                      |
| 59-67 | IODE        | Issue of Data Ephemeris              | -                      |
| 68-76 | SatH1       | Satellite health status              | -                      |
| 77-85 | SISA        | Signal accuracy index                | -                      |
| 86-103| t0e         | Ephemeris reference time             | sec (LSB=300)          |
|104-135| A           | Semi-major axis                      | m (LSB=2^-6)           |
|136-153| Adot        | Rate of change of A                  | m/s (LSB=2^-20)        |
|154-171| Δn0         | Mean motion correction               | semicircles/s (LSB=2^-44)|
|172-188| Δndot       | Rate of change of Δn                 | semicircles/s² (LSB=2^-57)|
|189-221| M0          | Mean anomaly at t0e                  | semicircles (LSB=2^-32) |
|222-254| e           | Eccentricity                         | dimensionless (LSB=2^-34)|
|255-287| ω           | Argument of perigee                  | semicircles (LSB=2^-32) |
|288-320| Ω0          | Longitude of ascending node          | semicircles (LSB=2^-32) |
|321-353| i0          | Inclination at t0e                   | semicircles (LSB=2^-32) |
|354-371| Ω̇           | Rate of change of longitude of node  | semicircles/s (LSB=2^-44)|
|372-386| İ           | Rate of change of inclination        | semicircles/s (LSB=2^-44)|
|387-404| Cuc         | Cosine harmonic correction latitude  | rad (LSB=2^-30)        |
|405-422| Cus         | Sine harmonic correction latitude    | rad (LSB=2^-30)        |
|423-440| Crc         | Cosine harmonic correction radius    | m (LSB=2^-6)           |
|441-458| Crs         | Sine harmonic correction radius      | m (LSB=2^-6)           |
|459-476| Cic         | Cosine harmonic correction inclination| rad (LSB=2^-30)       |
|477-494| Cis         | Sine harmonic correction inclination  | rad (LSB=2^-30)       |

### BeiDou B-CNAV2 (B2a) - Detailed Structure

#### General Parameters:
- **Data Rate**: 200 symbols/sec (data), 0 symbols/sec (pilot)
- **Code Length**: 10230 chips
- **Modulation**: QPSK(10)
- **Power Ratio**: 1:1 (data:pilot)
- **FEC**: LDPC(96,72)

#### B-CNAV2 Frame Structure:
- **Frame**: 600 symbols (3 seconds)
- **Preamble**: 24 symbols
- **Data**: 432 symbols
- **FEC**: 144 symbols

**B-CNAV2 Message Types:**
Uses the same message types as B-CNAV1 but with modified frame structure.

### BeiDou B-CNAV3 (B2b) - Detailed Structure

#### General Parameters:
- **Data Rate**: 1000 symbols/sec (500 bits/sec before encoding)
- **Modulation**: QPSK(10)
- **FEC**: LDPC
- **Purpose**: PPP-B2b high accuracy service

---

## QZSS (Japan)

### Updated QZSS Information (2025)

**Constellation Status:** As of June 2025, QZSS includes 5 operational satellites with plans to expand to 7 by 2026.

**Operator:** Cabinet Office, Government of Japan

**Accuracy:**
- Standalone: 1-2 m (horizontal)
- SLAS (Sub-meter Level Augmentation Service): 1 m
- CLAS (Centimeter Level Augmentation Service): 3-12 cm
- MADOCA-PPP: 10 cm globally

**Orbital Characteristics:**
- 3 satellites in quasi-zenith orbits (QZO) - altitude ~32,000-40,000 km
- 1 satellite in geostationary orbit (GEO) - altitude 35,786 km  
- 1 satellite in geosynchronous orbit (GSO) - altitude 35,786 km

**QZSS Satellites (2025):**
1. **QZS-1 (Michibiki-1)**: QZO, launched 2010
2. **QZS-2 (Michibiki-2)**: QZO, launched 2017
3. **QZS-3 (Michibiki-3)**: GEO, launched 2017
4. **QZS-4 (Michibiki-4)**: QZO, launched 2017
5. **QZS-5 (Michibiki-5)**: GSO, launched 2023
6. **QZS-6**: planned 2025
7. **QZS-7**: planned 2026

**QZSS Services:**
1. **PNT (Positioning, Navigation, Timing)** - basic navigation
2. **SLAS** - sub-meter accuracy for Japan
3. **CLAS** - centimeter accuracy for Japan
4. **MADOCA-PPP** - global PPP service
5. **DC Report** - disaster and crisis reports
6. **QZNMA** - navigation message authentication

### QZSS Frequency Bands

| Band      | Frequency (MHz) | Signal          | Modulation       | Compatibility | Status (2025)          |
|---------- |-----------------|-----------------|------------------|---------------|------------------------|
| L1        | 1575.42         | L1C/A, L1C, L1S | BPSK, BOC        | GPS L1        | Fully operational      |
| L2        | 1227.60         | L2C             | BPSK             | GPS L2C       | Fully operational      |
| L5        | 1176.45         | L5, L5S         | QPSK             | GPS L5        | Fully operational      |
| L6        | 1278.75         | L6 (CLAS/MADOCA)| BPSK             | Unique        | Fully operational      |
| S         | 2000-2110       | S-band          | Special          | Unique        | Experimental           |

### QZSS L1C/A - Detailed Structure

Fully compatible with GPS L1 C/A. Uses the same parameters and message structure.

### QZSS L1S SLAS - Detailed Structure

#### General Parameters:
- **Data Rate**: 250 bits/sec
- **Code Length**: 1023 chips (like GPS C/A)
- **Code Frequency**: 1.023 MHz
- **Modulation**: BPSK
- **FEC**: Reed-Solomon code (255,223)

#### L1S Message Structure:
- **Message**: 250 bits (1 second)
- **Preamble**: 8 bits (01010011)
- **Message Type**: 6 bits
- **Data**: 212 bits
- **CRC**: 24 bits

**L1S SLAS Message Types:**

| MT  | Name                        | Content                    | Interval         |
|-----|---------------------------- |----------------------------|------------------|
| 0   | Test message                | For testing                | -                |
| 1   | PRN mask                    | Satellite mask and IODP    | 30 sec           |
| 2-5 | Vector corrections          | Satellite corrections      | 30 sec           |
| 6   | Integrity parameters        | UDRE and degradation       | 30 sec           |
| 7   | Degradation time correction | Degradation parameters     | 30 sec           |
| 8   | Reserved                    | Reserved                   | -                |
| 9   | GEO navigation info         | GEO ephemeris              | 120 sec          |
| 10  | Degradation and parameters  | Additional information     | 120 sec          |
| 11  | Reserved                    | Reserved                   | -                |
| 12  | QZSS time                   | Time parameters            | 300 sec          |
|14-16| Reserved                    | Reserved                   | -                |
|24-30| Reserved                    | Reserved                   | -                |
|31-43| JMA messages                | Meteorological information | As needed        |
|44-46| Disaster messages           | Crisis information         | As needed        |
|47-53| Reserved                    | Reserved                   | -                |
|56-61| Reserved                    | Reserved                   | -                |
| 62  | Null message                | Filler                     | -                |
| 63  | Reserved                    | Reserved                   | -                |

### QZSS L6 CLAS/MADOCA - Detailed Structure

#### General Parameters:
- **Data Rate**: 2000 bits/sec (CLAS), 2000 bits/sec (MADOCA)
- **Code Length**: 10230 chips
- **Modulation**: BPSK(5) 
- **FEC**: Reed-Solomon code (255,223)
- **Structure**: CSK (Code Shift Keying) modulation

#### L6 Message Structure:
- **Frame**: 1 second (2000 bits)
- **Subframe**: 250 bits
- **Data word**: 218 bits information + 32 bits RS parity

#### L6 CLAS Message Types:

| Type  | Name                      | Content                   | Size     |
|-------|---------------------------|---------------------------|----------|
| 1     | Compact network mask      | Network definition        | Variable |
| 2     | Orbit corrections         | Orbit corrections         | Variable |
| 3     | Clock corrections         | Satellite clock corrections| Variable |
| 4     | Combined corrections      | Orbit + clock             | Variable |
| 5     | Code corrections          | Code biases               | Variable |
| 6     | Phase corrections         | Phase biases              | Variable |
| 7     | URA corrections           | User Range Accuracy       | Variable |
| 8     | STEC corrections          | Ionospheric corrections   | Variable |
| 9     | Grid coefficients         | Interpolation parameters  | Variable |
| 10    | Auxiliary information     | Additional data           | Variable |
| 11    | Combined SSR              | Full SSR corrections      | Variable |
| 12    | Atmospheric corrections   | Tropospheric parameters   | Variable |

### QZSS L5S - Detailed Structure

#### General Parameters:
- **Frequency**: 1176.45 MHz
- **Data Rate**: 250 bits/sec
- **Modulation**: BPSK
- **Purpose**: Enhanced positioning services
- **Compatibility**: Similar to L1S but on L5 frequency

---

## NavIC/IRNSS (India)

### Updated NavIC Information (2025)

**Constellation Status:** As of June 2025, NavIC includes 9 operational satellites (7 primary + 2 backup).

**Operator:** ISRO (Indian Space Research Organisation)

**Accuracy:**
- Positioning: 5-10 m (over India)
- Positioning: 10-20 m (extended coverage area)
- Timing: < 50 ns

**Coverage Area:**
- Primary: India and surrounding regions (1500 km from borders)
- Extended: From 30°E to 130°E and from 30°S to 50°N

**Orbital Characteristics:**
- 3 satellites in geostationary orbits (GEO) - 32.5°E, 83°E, 131.5°E
- 4 satellites in inclined geosynchronous orbits (IGSO) - inclination 29°
- 2 backup satellites (IGSO)

**NavIC Satellites (2025):**
1. **IRNSS-1A**: IGSO (non-operational since 2017, replaced by IRNSS-1I)
2. **IRNSS-1B**: IGSO (active)
3. **IRNSS-1C**: GEO (active)
4. **IRNSS-1D**: IGSO (active)
5. **IRNSS-1E**: IGSO (active)
6. **IRNSS-1F**: GEO (active)
7. **IRNSS-1G**: GEO (active)
8. **IRNSS-1I**: IGSO (replacement for IRNSS-1A, active)
9. **IRNSS-1J**: IGSO (backup, launched 2024)

**NavIC Modernization:**
- Transition to L1 navigation signals (planned from 2026)
- Constellation increase to 11 satellites by 2027
- Atomic clock improvements (rubidium replaced with hydrogen masers)

### NavIC Frequency Bands

| Band     | Frequency (MHz) | Service         | Modulation   | Bandwidth | Status (2025)          |
|----------|-----------------|-----------------|--------------|-----------|------------------------|
| L5       | 1176.45         | SPS (open)      | BPSK(1)      | 24 MHz    | Fully operational      |
| S        | 2492.028        | SPS+RS          | BPSK(1)+BOC  | 16.5 MHz  | Fully operational      |
| L1       | 1575.42         | SPS (future)    | BOC          | 24 MHz    | In development         |

**NavIC Services:**
1. **SPS (Standard Positioning Service)** - open service for civilian users
2. **RS (Restricted Service)** - encrypted service for authorized users

### NavIC L5 SPS - Detailed Structure

#### General Parameters:
- **Data Rate**: 50 symbols/sec (25 bits/sec before encoding)
- **Code Length**: 1023 chips
- **Code Frequency**: 1.023 MHz
- **Modulation**: BPSK(1)
- **FEC**: Convolutional encoding K=7, r=1/2

#### Navigation Message Structure:

**Master Frame**: 2400 symbols (48 seconds)
- 4 subframes of 600 symbols

**Subframe**: 600 symbols (12 seconds)
- 292 data bits + 16 FEC tail bits

#### Detailed NavIC Subframe Structure:

**Subframe 1 - Time and Ephemeris Part 1**

| Bits  | Parameter   | Description                       | Units            |
|-------|-------------|-----------------------------------|------------------|
| 1-8   | TLM         | Telemetry word                    | -                |
| 9-22  | Preamble    | 10001011101110                    | -                |
| 23-24 | Alert       | Alert flag                        | -                |
| 25-26 | Autonav     | Autonomous navigation flag        | -                |
| 27-36 | WN          | Week number                       | weeks            |
| 37-53 | t0e         | Ephemeris reference time          | sec (LSB=16)     |
| 54-57 | IODC        | Issue of Data Clock               | -                |
| 58-73 | a0          | Clock bias                        | sec (LSB=2^-35)  |
| 74-91 | a1          | Clock drift                       | sec/sec (LSB=2^-55)|
| 92-99 | a2          | Clock drift rate                  | sec/sec² (LSB=2^-66)|
| 100-104| URA        | User Range Accuracy               | -                |
| 105-112| TGD        | Group delay                       | sec (LSB=2^-35)  |
| 113-135| Δn         | Mean motion correction            | semicircles/s (LSB=2^-43)|
| 136-141| IODEC      | Issue of Data Ephemeris and Clock | -                |
| 142-147| Reserved   | Reserved                          | -                |
| 148-164| Cuc        | Cosine latitude correction        | rad (LSB=2^-28)  |
| 165-181| Cus        | Sine latitude correction          | rad (LSB=2^-28)  |
| 182-199| Cic        | Cosine inclination correction     | rad (LSB=2^-28)  |
| 200-217| Cis        | Sine inclination correction       | rad (LSB=2^-28)  |
| 218-235| Crc        | Cosine radius correction          | m (LSB=2^-4)     |
| 236-253| Crs        | Sine radius correction            | m (LSB=2^-4)     |
| 254-261| IDOT       | Rate of change of inclination     | semicircles/s (LSB=2^-43)|

**Subframe 2 - Ephemeris Part 2**

| Bits  | Parameter   | Description                       | Units            |
|-------|-------------|-----------------------------------|------------------|
| 1-32  | M0          | Mean anomaly                      | semicircles (LSB=2^-31)|
| 33-49 | t0e         | Ephemeris reference time          | sec (LSB=16)     |
| 50-81 | e           | Eccentricity                      | dimensionless (LSB=2^-33)|
| 82-113| √A          | Square root of semi-major axis    | √m (LSB=2^-19)   |
| 114-145| Ω0         | Longitude of ascending node       | semicircles (LSB=2^-31)|
| 146-177| ω          | Argument of perigee               | semicircles (LSB=2^-31)|
| 178-200| Ω̇          | Rate of longitude of node         | semicircles/s (LSB=2^-41)|
| 201-232| i0         | Inclination                       | semicircles (LSB=2^-31)|

**Subframes 3 and 4 - Secondary Navigation Parameters**

Contain messages of various types:

| ID  | Message Type               | Subframe | Content                |
|-----|---------------------------|----------|------------------------|
| 0   | Null message              | 3,4      | Filler                 |
| 1   | Time parameters           | 3        | UTC parameters         |
| 2   | Time parameters-2         | 3        | GPS-NavIC offset       |
| 3   | Reserved                  | 3        | Reserved               |
| 4   | Time parameters-3         | 3        | GLONASS-NavIC offset   |
| 5   | Reserved                  | 3        | Reserved               |
| 6   | Reserved                  | 3        | Reserved               |
| 7   | Ionospheric corrections   | 3        | Ionosphere coefficients|
| 8   | Reserved                  | 3        | Reserved               |
| 9   | Special message           | 4        | Text message           |
| 10  | Reserved                  | 3,4      | Reserved               |
| 11  | Earth rotation parameters | 4        | EOP parameters         |
| 12-13| Reserved                 | 3,4      | Reserved               |
| 14  | Differential corrections  | 4        | Regional corrections   |
| 15-17| Reserved                 | 3,4      | Reserved               |
| 18  | Special message-2         | 4        | Additional text        |

### NavIC S-band - Detailed Structure

#### General Parameters:
- **Frequency**: 2492.028 MHz
- **Data Rate**: 50 symbols/sec (SPS), 50 symbols/sec (RS)
- **Modulation**: BPSK(1) for SPS, BOC(5,2) for RS
- **Structure**: Similar to L5 but with additional RS signal

---

## SBAS Systems

### SBAS Systems Overview (2025)

SBAS (Satellite Based Augmentation Systems) - satellite differential correction systems that improve GNSS accuracy and integrity.

**Operational SBAS Systems:**

| System  | Region              | Operator      | Satellites | Status (2025)          |
|---------|---------------------|---------------|------------|------------------------|
| WAAS    | North America       | FAA (USA)     | 3          | Fully operational      |
| EGNOS   | Europe              | ESA/EC        | 4          | Fully operational      |
| MSAS    | Japan               | JCAB          | 2          | Fully operational      |
| GAGAN   | India               | AAI/ISRO      | 3          | Fully operational      |
| SDCM    | Russia              | Roscosmos     | 3          | Fully operational      |
| BDSBAS  | China               | CNSA          | 3          | Testing                |
| KASS    | South Korea         | KARI          | 2          | Initial operation      |
| A-SBAS  | Africa              | ASECNA        | 2          | In development         |
| SPANS   | Australia           | Geoscience Australia | 2   | Testing                |

### General SBAS Message Structure

#### Signal Parameters:
- **Frequency**: 1575.42 MHz (L1) and 1176.45 MHz (L5)
- **Data Rate**: 250 bits/sec (L1), 500 bits/sec (L5)
- **Modulation**: BPSK
- **FEC**: Convolutional encoding K=7, r=1/2

#### SBAS Message Structure:
- **Message**: 250 bits (1 second)
- **Preamble**: 8 bits (01010011 for first 3, 10011010 for subsequent)
- **Message Type**: 6 bits
- **Data**: 212 bits
- **CRC**: 24 bits

### SBAS Message Types

| MT  | Name                               | Content                   | Interval   |
|-----|-----------------------------------|---------------------------|------------|
| 0   | Do not use                        | Test message              | -          |
| 1   | PRN mask                          | Satellite mask            | 120 sec    |
| 2-5 | Fast corrections                  | Pseudorange corrections   | 6-60 sec   |
| 6   | Integrity                         | Integrity information     | 6 sec      |
| 7   | Fast correction degradation factor| Degradation parameters    | 120 sec    |
| 8   | Reserved                          | Reserved                  | -          |
| 9   | GEO navigation message            | GEO ephemeris             | 120 sec    |
| 10  | Degradation time                  | Degradation parameters    | 120 sec    |
| 11  | Reserved                          | Reserved                  | -          |
| 12  | SBAS time offset                  | Time parameters           | 300 sec    |
|13-16| Reserved                          | Reserved                  | -          |
| 17  | GEO almanac                       | GEO satellite almanac     | 300 sec    |
| 18  | Ionospheric grid mask             | IGP mask                  | 300 sec    |
|19-23| Reserved                          | Reserved                  | -          |
| 24  | Mixed fast/long corrections       | FC/LTC corrections        | 6-120 sec  |
| 25  | Long-term corrections             | Orbit and clock corrections| 120 sec   |
| 26  | Ionospheric delays                | Vertical delays           | 300 sec    |
| 27  | SBAS service message              | Regional information      | 300 sec    |
| 28  | Clock-ephemeris covariance        | Covariance matrix         | 120 sec    |

### Detailed Structure of Key SBAS Message Types

#### Type 1 - PRN Mask

| Bits   | Parameter   | Description                       |
|--------|-------------|-----------------------------------|
| 1-8    | Preamble    | 01010011 or 10011010              |
| 9-14   | MT          | Message type (1)                  |
| 15-210 | PRN Mask    | 210 bits for satellites 1-210     |
| 211-224| IODP        | Issue of Data PRN                 |
| 225-250| CRC         | Checksum                          |

#### Type 2-5 - Fast Corrections

| Bits   | Parameter   | Description                       |
|--------|-------------|-----------------------------------|
| 1-8    | Preamble    | Synchronization                   |
| 9-14   | MT          | Message type (2-5)                |
| 15-16  | IODF        | Issue of Data Fast Corrections    |
| 17-18  | IODP        | Issue of Data PRN Mask            |
| 19-31  | PRC[1]      | Pseudorange correction #1         |
| 32-44  | PRC[2]      | Pseudorange correction #2         |
| ...    | ...         | ... (up to 13 satellites)         |
| -224   | UDREI[n]    | User Differential Range Error     |
| 225-250| CRC         | Checksum                          |

#### Type 26 - Ionospheric Delays

| Bits  | Parameter    | Description                       |
|-------|------------- |-----------------------------------|
| 1-8   | Preamble     | Synchronization                   |
| 9-14  | MT           | Message type (26)                 |
| 15-18 | Band ID      | Band identifier                   |
| 19-22 | Block ID     | Block identifier                  |
| 23-31 | IGP[1] GIVD  | Vertical delay point 1            |
| 32-40 | IGP[2] GIVD  | Vertical delay point 2            |
| ...   | ...          | ... (up to 15 points)             |
|212-215| IGP[15] GIVEI| Error indicator point 15          |
|216-222| IODI         | Issue of Data Ionosphere          |
|223-224| Spare        | Reserved                          |
|225-250| CRC          | Checksum                          |

### Additional SBAS Features

**Regional Features:**

1. **WAAS (USA)**:
   - Coverage: North America, parts of Pacific and Atlantic oceans
   - Accuracy: < 1 m horizontal, < 1.5 m vertical
   - Availability: > 99%

2. **EGNOS (Europe)**:
   - Coverage: Europe and parts of Africa
   - Certified for aviation (APV-I)
   - V3 being deployed with dual-frequency support

3. **MSAS (Japan)**:
   - Integrated with QZSS
   - Optimized for region with high ionospheric activity

4. **GAGAN (India)**:
   - Coverage: Indian subcontinent
   - First SBAS in equatorial region

5. **SDCM (Russia)**:
   - Integrated with GLONASS
   - Coverage: Russian territory

### Future SBAS Development

**Dual-Frequency SBAS (DFMC SBAS):**
- Use of L1 and L5 frequencies
- Improved resistance to ionospheric disturbances
- Standard developed, implementation from 2025-2027

**Integration with PPP:**
- Combining SBAS and PPP technologies
- Achieving centimeter-level accuracy

---

*Document updated: June 2025*
*Version: 2.0*

## Modern GNSS Standards and Interoperability (2025)

### International GNSS Coordination

**International GNSS Committee (ICG):**
Interoperability standards between systems are coordinated internationally through the ICG, including representatives from USA, Russia, EU, China, Japan, and India.

**Key Interoperability Achievements (2025):**
- GPS L1C and Galileo E1 operate on the same frequency with compatible signals
- BeiDou B1C is compatible with GPS L1C 
- QZSS is fully compatible with GPS on all frequencies
- High-precision correction formats are standardized

### New Technologies and Trends

**1. High Precision Services (PPP-RTK)**
- Galileo HAS: free high-precision corrections since January 24, 2023
- BeiDou PPP-B2b: high-precision service for Asia-Pacific region
- QZSS CLAS: centimeter accuracy for Japan and region
- GPS/GNSS PPP: global commercial services

**2. Signal Authentication**
- GPS Navigation Message Authentication (NMA) in development
- Galileo OSNMA: operational authentication since 2023
- BeiDou B1C/B2a include anti-spoofing protection
- QZSS QZNMA: regional authentication

**3. Multi-frequency Receivers**
- Standard receivers support 3-4 frequencies
- Professional solutions use all available signals
- IoT devices integrate multi-GNSS in small form factors
- Smartphones with L1/L5 support becoming standard

### GNSS Frequency Spectrum Compatibility

| Frequency (MHz) | GPS | GLONASS | Galileo | BeiDou | QZSS | NavIC | Notes                  |
|-----------------|-----|---------|---------|--------|------|-------|------------------------|
| 1176.45         | L5  | L5      | E5a     | B2a    | L5   | L5    | Full compatibility     |
| 1575.42         | L1  | -       | E1      | B1C    | L1C  | L1    | Center frequency       |
| 1227.60         | L2  | -       | -       | -      | L2C  | -     | GPS unique             |
| 1602±k×0.5625   | -   | L1      | -       | -      | -    | -     | FDMA unique            |
| 1207.14         | -   | -       | E5b     | B2b    | -    | -     | Galileo/BeiDou         |
| 1278.75         | -   | -       | E6      | -      | L6   | -     | Commercial services    |
| 1268.52         | -   | -       | -       | B3     | -    | -     | BeiDou unique          |
| 2492.028        | -   | -       | -       | -      | -    | S     | NavIC unique           |
| 1202.025        | -   | L3      | -       | -      | -    | -     | GLONASS CDMA           |
| 1191.795        | -   | -       | E5      | B2     | -    | -     | Wideband signal        |

### Next Generation Data Formats

**CNAV (Civil Navigation):**
- Used in GPS L2C, L5 and future L1C
- Enhanced message type flexibility
- Improved error correction (CRC-24)
- Support for future extensions
- Forward Error Correction (FEC)

**I/NAV and F/NAV (Galileo):**
- I/NAV: basic service on E1 and E5b
- F/NAV: enhanced service on E5a
- C/NAV: commercial service on E6
- Integration with HAS high-precision corrections
- Support for SAR Return Link Service

**B-CNAV (BeiDou):**
- New formats for B1C/B2a signals
- PPP correction support
- Compatibility with international standards
- Integration with regional enhancements

### Development Perspectives (2025-2030)

**1. Next Generation Satellites:**
- **GPS IIIF**: planned from 2026-2027
  - Enhanced jamming resistance
  - Laser retroreflectors
  - Regional military code support
  - 15+ year lifetime
  
- **Galileo G2G**: second generation after 2026
  - Electric propulsion
  - Inter-satellite links
  - New navigation signals
  - Improved atomic clocks
  
- **GLONASS-K2/K3**: new capabilities
  - Full transition to CDMA
  - New frequencies L1, L3, L5
  - Improved clock accuracy
  - Extended lifetime
  
- **BeiDou-4**: global expansion
  - LEO augmentation
  - Global PPP service
  - 5G integration
  - Quantum technologies

**2. New Technologies:**
- **5G/6G Network Integration**
  - Hybrid positioning
  - Using 5G as pseudolites
  - Millimeter accuracy in cities
  
- **AI for Signal Processing**
  - Machine learning for interference suppression
  - Multipath prediction
  - Adaptive algorithms
  
- **Quantum Cryptography**
  - Navigation data protection
  - Quantum key distribution
  - Quantum computer resistance
  
- **LEO Augmentation**
  - Low Earth orbit satellite constellations
  - GNSS signal enhancement
  - Fast PPP convergence

**3. Applications:**
- **Autonomous Transport**
  - Real-time centimeter accuracy
  - Guaranteed integrity
  - V2X integration
  
- **Internet of Things**
  - Ultra-low power consumption
  - Cloud-GNSS processing
  - Mass applications
  
- **Critical Infrastructure**
  - Redundant timing systems
  - Secure positioning
  - Deformation monitoring

### Security and Integrity Standards

**Anti-spoofing and Anti-jamming:**

1. **Signal-level Authentication**
   - Cryptographic data signatures
   - Galileo OSNMA (operational)
   - GPS Chimera (in development)
   - BeiDou service authentication

2. **Measurement-level Authentication**
   - Secure pseudorange measurements
   - Galileo CAS (Commercial Authentication Service)
   - Military P(Y) and M-codes

3. **Receiver-level Monitoring**
   - RAIM/ARAIM algorithms
   - Multi-antenna systems
   - Inertial integration
   - Signal power analysis

**Integrity Monitoring:**

1. **SBAS Evolution**
   - DFMC (Dual Frequency Multi-Constellation)
   - L1/L5 support for all GNSS
   - Improved ionospheric models
   - Global interoperability

2. **ARAIM (Advanced RAIM)**
   - Horizontal and vertical navigation
   - Multi-constellation processing
   - Optimized algorithms
   - Aviation certification

3. **Ground Monitoring**
   - Global reference station networks
   - Real-time signal quality control
   - Fast anomaly detection
   - User warnings

**Resilience and Redundancy:**

1. **Multi-GNSS Strategies**
   - Minimum 2 systems for critical applications
   - Optimal use of all available signals
   - Intelligent switching between systems

2. **Alternative Technologies**
   - Inertial systems (IMU)
   - Visual odometry
   - 5G/WiFi positioning
   - Magnetometers and barometers

3. **System Architecture**
   - Distributed processing
   - Cloud computing
   - Edge computing for low latency
   - Blockchain for data integrity

### Comparative System Performance Table (2025)

| Parameter        | GPS    | GLONASS | Galileo | BeiDou  | QZSS    | NavIC   |
|------------------|--------|---------|---------|---------|---------|---------|
| Accuracy (m)     | 5-10   | 2.8-7.4 | 1-4     | 2.6-3.6 | 1-2     | 5-10    |
| Availability (%) | >95    | >95     | >95     | >95     | >99*    | >95*    |
| Integrity        | RAIM   | RAIM    | OSNMA   | ISM     | SLAS    | RAIM    |
| Cold TTFF (s)    | 30     | 35      | 30      | 30      | 30      | 35      |
| Warm TTFF (s)    | 20     | 25      | 20      | 20      | 20      | 25      |
| Power consumption| Base   | +10%    | -5%     | Equal   | Equal   | +5%     |

*Within system coverage area

### Key Differences in Message Structures

**Encoding Philosophy:**
- **GPS**: Simplicity and backward compatibility
- **GLONASS**: Efficiency and compactness
- **Galileo**: Flexibility and extensibility
- **BeiDou**: Regional optimization
- **QZSS**: GPS augmentation and enhancement
- **NavIC**: Regional specialization

**Unique Features:**
- **GPS**: First global system, de facto standard
- **GLONASS**: FDMA channel separation (unique)
- **Galileo**: Free high-accuracy service HAS
- **BeiDou**: Three orbit types (MEO+IGSO+GEO)
- **QZSS**: Quasi-zenith orbits for urban canyons
- **NavIC**: S-band for better signal penetration

## Conclusion

This document presents a comprehensive technical description of navigation message structures for all major GNSS systems in the world as of June 2025. Each system reflects a unique approach to solving global positioning challenges, considering:

- Historical development context
- Regional requirements and features
- Technological capabilities at the time of creation
- Strategic goals of system operators

The modern GNSS era is characterized by:
- Convergence of technologies and standards
- Accuracy improvements to centimeter level
- Enhanced protection against intentional and unintentional interference
- Integration with other positioning technologies

The future of GNSS is linked to creating a unified global system of systems, where different constellations work together to provide continuous, accurate, and secure positioning anywhere on Earth and in near-Earth space.

---

*Document prepared based on official Interface Control Documents (ICD) of all GNSS systems*
*Last updated: June 2025*
*Version: 2.0*

---