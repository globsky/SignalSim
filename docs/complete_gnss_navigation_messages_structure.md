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
| 5-6   |             |                                     | -                  |
| 7-8   | P1          | tb update flag                      | -                  |
| 9-13  | tk (hours)  | Current time - hours (0-23)         | hour               |
| 14-19 | tk (minutes)| Current time - minutes (0-59)       | min                |
| 20    | tk (30s)    | Current time - 30 sec interval      | 0 or 1             |
| 21-44 | xn(tb)      | X coordinate                        | km                 |
| 45-49 | ẋn(tb)      | X velocity                          | km/s               |
| 50-76 | ẍn(tb)      | X acceleration                      | km/s²              |
| 77-84 | HC          | Hamming code                        | -                  |
|    85 | TM          | Time mark                           | -                  |

**STRING 2 - Satellite Navigation Data (Y coordinate)**

| Bits  | Parameter   | Description                         | Units              |
|-------|-------------|-------------------------------------|--------------------|
| 1-4   | m           | String number (0010)                | -                  |
| 5-7   | Bn          | Unreliability flag                  | -                  |
| 8     | P2          | Satellite mode flag                 | -                  |
| 9-15  | tb          | tb time index within day            | 15 min             |
| 16-20 |             |                                     | -                  |
| 21-44 | yn(tb)      | Y coordinate                        | km                 |
| 45-49 | ẏn(tb)      | Y velocity                          | km/s               |
| 50-76 | ÿn(tb)      | Y acceleration                      | km/s²              |
| 77-84 | HC          | Hamming code                        | -                  |
|    85 | TM          | Time mark                           | -                  |

**STRING 3 - Satellite Navigation Data (Z coordinate)**

| Bits  | Parameter   | Description                         | Units                   |
|-------|-------------|-------------------------------------|-------------------------|
| 1-4   | m           | String number (0011)                | -                       |
| 5     | P3          | Satellite time flag                 | -                       |
| 6-16  | γn(tb)      | Relative frequency deviation        | dimensionless (LSB=2^-40)|
| 17    | Reserved    | Reserved                            | -                       |
| 18-19 | p           | GLONASS-M flag                      | -                       |
| 20    | ln          | Health flag for KI                  | -                       |
| 21-44 | zn(tb)      | Z coordinate                        | km                      |
| 45-49 | żn(tb)      | Z velocity                          | km/s                    |
| 50-76 | z̈n(tb)      | Z acceleration                      | km/s²                   |
| 77-84 | HC          | Hamming code                        | -                       |
|    85 | TM          | Time mark                           | -                       |

**STRING 4 - Time Corrections and Additional Data**

| Bits  | Parameter   | Description                         | Units              |
|-------|-------------|-------------------------------------|--------------------|
| 1-4   | m           | String number (0100)                | -                  |
| 5-26  | τn(tb)      | Satellite n time scale correction   | sec (LSB=2^-30)    |
| 27-31 | Δτn         | tn and tc scale difference          | sec (LSB=2^-30)    |
| 32-36 | En          | Age of operational information      | days               |
| 37-50 | Reserved    | Reserved                            | -                  |
| 51    | P4          | Ephemeris update flag               | -                  |
| 52-55 | FT          | Predicted frequency correction      | -                  |
| 56-58 |             |                                     |                    |
| 59-69 | Nt          | Day number within four-year period  | days               |
| 70-74 | n           | Satellite number in system          | -                  |
| 75-76 | M           | Satellite type (01-M, 10-K1, 11-K2) | -                  |
| 77-84 | HC          | Hamming code                        | -                  |
|    85 | TM          | Time mark                           | -                  |

**STRINGS 5-15 - Almanac**

Strings 6, 8, 10, 12, 14 (odd almanac satellites):

| Bits  | Parameter   | Description                         |
|-------|-------------|-------------------------------------|
| 1-4   | m           | String number                       |
| 5     | Cn          | Generalized nA satellite flag       |
| 6-7   | Mn          |                                     |
| 8-12  | nA          | Almanac satellite frequency literal |
| 13-22 | τAn         |                                     |
| 23-43 | λnA         |                                     |
| 44-61 | ΔinA        | Inclination correction (+0.067π rad)|
| 62-76 | ΔT'nA       | Rate of change of period            |
| 77-84 | HC          | Hamming code                        |
|    85 | TM          | Time mark                           |

Strings 7, 9, 11, 13, 15 (even almanac satellites):

| Bits  | Parameter   | Description                         |
|-------|-------------|-------------------------------------|
| 1-4   | m           | String number                       |
| 5-20  | ωА          |                                     |
| 21-41 | tA  λn      | Generalized nA satellite flag       |
| 42-63 | τnA         | Coarse time correction value        |
| 64-70 | ΔT′A        |                                     |
| 71-75 | НA          |                                     |
|    76 |             |                                     |
| 77-84 | HC          | Hamming code                        |
|    85 | TM          | Time mark                           |

**STRING 5 - System Status**

| Bits  | Parameter              | Description                         |
|-------|----------------------- |-------------------------------------|
| 1-4   | m                      | String number (0101)                |
| 5-15  | NA                     | Day within four-year almanac period |
| 16-47 | τc                     | System time scale correction        |
| 48    |                        |                                     |
| 49-53 | N4                     | Four-year period number             |
| 54-75 | τGPS                   | GLONASS-GPS correction              |
| 76    | ln5                    | String 5 validity flag              |
| 77-84 | HC                     | Hamming code                        |
|    85 | TM                     | Time mark                           |

### GLONASS L3OC - Detailed Structure (CDMA) ##############################################################

#### General Parameters:
- **Frequency**: 1202.025 MHz
- **Data Rate**: 100 symbols/sec (200 bits/sec with encoding)
- **Code Length**: 10230 chips (primary), 250 chips (secondary)
- **Modulation**: BPSK(10)
- **FEC**: Convolutional encoding K=7, r=1/2
- **Structure**: L3OC-I (data) + L3OC-Q (pilot)

#### L3OC Navigation Message Structure:
- **String**: 3 seconds (300 bits) - standard string
- **Anomalous strings**: 2 seconds (200 bits) or 4 seconds (400 bits) - used during leap second corrections
- **Structure**: Flexible, no predefined constant frame structure
- **CRC**: Cyclic code (300,276) with 24 check bits

#### L3OC String Types:

| Type    | Content                                                              |
|---------|----------------------------------------------------------------------|
| 10,11,12| Operational information (ephemeris, time, clock)                     |
| 20      | Almanac                                                              |
| 25      | Earth rotation parameters, ionosphere model, UTC(SU)-TAI parameters  |
| 16      | SV orientation parameters in sun-pointing mode                       |
| 31, 32  | Long-term movement model parameters                                  |
| 60      | Text messages                                                        |
| 0       | For technological tasks. Ignored by users                           |
| 1       | Anomalous string used when day duration decreases by 1 sec          |
| 2       | Anomalous string used when day duration increases by 1 sec          |

Note: Strings 10, 11, and 12 form a packet - string 11 always follows string 10, and string 12 follows string 11.

#### General L3OC String Structure (300 bits):

**Service fields (common for all string types):**
| Bits   | Parameter | Description                              | Units      |
|--------|-----------|------------------------------------------|------------|
| 1-20   | СМВ       | Time mark signal                         | -          |
| 21-26  | Type      | String type                              | -          |
| 27-41  | ОМВ       | Time mark digitization                   | 3 sec      |
| 42-47  | j         | SV system number (PRN)                   | -          |
| 48     | Гj        | SV health flag (0=healthy, 1=unhealthy)  | -          |
| 49     | lj        | String data validity (0=valid, 1=invalid)| -          |
| 50-53  | П1        | Ground control call sign (not used)      | -          |
| 54     | П2        | SV orientation mode (0=sun, 1=maneuver)  | -          |
| 55-56  | КР        | Expected UTC(SU) correction              | -          |
| 57     | А         | Leap second correction flag              | -          |
| 58-276 | Info      | Information fields (string-type specific)| -          |
| 277-300| ЦК        | Cyclic code check bits                   | -          |

**Detailed L3OC String Types:**

**Strings 10, 11, 12 (Operational Information) - transmitted as a packet:**

String 10 contains:
- N4 (5 bits) - Four-year interval number  
- NT (11 bits) - Day number within four-year period
- Mj (3 bits) - Satellite type
- tb (10 bits) - Reference time of ephemeris
- Health and validity parameters
- Clock parameters: τj(tb), γj(tb), αj(tb)
- Ephemeris: xj(tb), ẋj(tb), ẍj(tb)

String 11 contains:
- Continuation of String 10 data
- Ephemeris: yj(tb), ẏj(tb), ÿj(tb) 

String 12 contains:
- Continuation of String 11 data
- Ephemeris: zj(tb), żj(tb), z̈j(tb)
- Frequency-time corrections

**String 20 (Almanac):**
Contains almanac data for one satellite including orbital parameters and health status.

**String 25 (Earth Parameters):**
Contains Earth rotation parameters, ionospheric model coefficients, and UTC(SU)-TAI time difference parameters.

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

### Galileo E5b I/NAV - Detailed Structure

**General Parameters:**
- **Frequency**: 1207.14 MHz
- **Data rate**: 125 symbols/sec (250 bits/sec with FEC)
- **Modulation**: BPSK
- **FEC**: Convolutional coding rate 1/2 with interleaving
- **Format**: Identical to E1-B I/NAV

**Page Structure**: 2 seconds (250 bits)

| Bits | Parameter | Description |
|------|-----------|-------------|
| 1-10 | Sync pattern | 0101100000 for nominal page |
| 11 | Even/Odd | 0=even, 1=odd |
| 12 | Page Type | 0=nominal, 1=alert |
| 13-128 | Data field | 116 bits (58 information bits after FEC) |
| 129-240 | Reserved | For future use |
| 241-250 | Tail | Convolutional code tail |

**Word Types:**

**Word 0 - Time and Status:**
| Bits | Parameter | Description | Scale Factor |
|------|-----------|-------------|--------------|
| 1-2 | Time | Type of time relation | - |
| 3-31 | WN | Week Number | weeks |
| 32-51 | TOW | Time of Week | seconds |
| 52-55 | Signal Health | E5b signal health status | - |
| 56-57 | DVS | Data Validity Status | - |

**Words 1-4 - Ephemeris:**
Contains orbital parameters identical to E1-B I/NAV

**Word 5 - Ionospheric Correction and BGD:**
| Bits | Parameter | Description | Scale Factor |
|------|-----------|-------------|--------------|
| 1-12 | ai0 | Ionospheric coefficient α₀ | 2^-2 sfu |
| 13-23 | ai1 | Ionospheric coefficient α₁ | 2^-8 sfu/semicircle |
| 24-37 | ai2 | Ionospheric coefficient α₂ | 2^-15 sfu/semicircle² |
| 39-48 | BGD E5a/E1 | Broadcast Group Delay | 2^-32 s |
| 49-58 | BGD E5b/E1 | Broadcast Group Delay | 2^-32 s |

**Word 6 - GST-UTC Conversion:**
Galileo System Time to UTC conversion parameters

**Words 7-10 - Almanac:**
Reduced precision orbital data for all satellites

### Galileo E1-C - Secondary Pilot Signal

**General Parameters:**
- **Frequency**: 1575.42 MHz (same as E1-B)
- **Chip rate**: 1.023 Mchip/s
- **Modulation**: CBOC(6,1,1/11) for pilot
- **Data**: No data (pilot signal only)
- **Code**: Memory code length 4092 chips (4 ms)
- **Secondary code**: 25 chips, 100 ms period

E1-C is used for:
- Enhanced carrier tracking
- More accurate pseudorange measurements
- Operation in weak signal conditions

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
| B2I      | 1207.14         | Open              | BPSK(2)         | 50                | Fully operational      |
| B2a      | 1176.45         | Open              | QPSK(10)        | 200               | Fully operational      |
| B2b      | 1207.14         | Open/PPP          | BPSK(1)         | 1000              | Fully operational      |
| B2(B2a+b)| 1191.795        | Open              | AltBOC(15,10)   | 200               | Fully operational      |
| B3I      | 1268.52         | Authorized        | BPSK(10)        | 50                | Fully operational      |
| B1A      | 1575.42         | Authorized        | BOC(14,2)       | 200               | Limited access         |
| B2A      | 1191.795        | Authorized        | ACE-BOC         | 200               | Limited access         |
| B3A      | 1268.52         | Authorized        | BOC(15,2.5)     | 200               | Limited access         |

## BeiDou D1 Navigation Message (B1I/B2I/B3I for MEO/IGSO)

### General Parameters
- **Used by**: MEO and IGSO satellites (SVID 6-58)
- **Signals**: B1I (1561.098 MHz), B2I (1207.14 MHz), B3I (1268.52 MHz)
- **Data Rate**: 50 bits/sec
- **Frame Duration**: 30 seconds (1500 bits)
- **Subframe Duration**: 6 seconds (300 bits)
- **Word Duration**: 0.6 seconds (30 bits)
- **Modulation**: BPSK
- **Error Correction**: BCH(15,11,1) Hamming code + Interleaving

### D1 Frame Structure

**Superframe**: 36,000 bits (12 minutes)
- 24 frames × 1500 bits

**Frame**: 1500 bits (30 seconds)
- 5 subframes × 300 bits

**Subframe**: 300 bits (6 seconds)
- 10 words × 30 bits

**Word**: 30 bits (0.6 seconds)
- 22 bits information + 4 bits BCH + 4 bits reserved

### D1 Subframe Allocation

| Subframe | Pages | Content |
|----------|-------|---------|
| 1 | - | Basic navigation parameters |
| 2 | - | Ephemeris data part 1 |
| 3 | - | Ephemeris data part 2 |
| 4 | 1-24 | Almanac for satellites 1-24 |
| 5 | 1-6 | Almanac for satellites 25-30 |
| 5 | 7-10 | Ionospheric, UTC, health |
| 5 | 11-23 | Almanac for satellites 31-63 |
| 5 | 24 | Health status |

### D1 Detailed Word Structure

#### Subframe 1 - Basic Navigation Parameters

| Word | Bits | Parameter | Description | Scale Factor |
|------|------|-----------|-------------|--------------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | Subframe ID (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 1 | 23-26 | BCH | Error correction | - |
| 1 | 27-30 | Reserved | - | - |
| 2 | 1-12 | SOW[11:0] | Seconds of Week LSB | 1 |
| 2 | 13 | SatH1 | Satellite health | - |
| 2 | 14-18 | IODC | Issue of Data Clock | - |
| 2 | 19-22 | URAI | User Range Accuracy Index | - |
| 3 | 1-13 | WN | Week Number | 1 |
| 3 | 14-22 | t_oc[16:8] | Clock reference time MSB | 2^3 |
| 4 | 1-8 | t_oc[7:0] | Clock reference time LSB | 2^3 |
| 4 | 9-18 | TGD1 | Group Delay B1I | 0.1 ns |
| 4 | 19-22 | TGD2[9:6] | Group Delay B2I MSB | 0.1 ns |
| 5 | 1-6 | TGD2[5:0] | Group Delay B2I LSB | 0.1 ns |
| 5 | 7-14 | α_0 | Ionospheric parameter | 2^-30 |
| 5 | 15-22 | α_1 | Ionospheric parameter | 2^-27 |
| 6 | 1-8 | α_2 | Ionospheric parameter | 2^-24 |
| 6 | 9-16 | α_3 | Ionospheric parameter | 2^-24 |
| 6 | 17-22 | β_0[7:2] | Ionospheric parameter MSB | 2^11 |
| 7 | 1-2 | β_0[1:0] | Ionospheric parameter LSB | 2^11 |
| 7 | 3-10 | β_1 | Ionospheric parameter | 2^14 |
| 7 | 11-18 | β_2 | Ionospheric parameter | 2^16 |
| 7 | 19-22 | β_3[7:4] | Ionospheric parameter MSB | 2^16 |
| 8 | 1-4 | β_3[3:0] | Ionospheric parameter LSB | 2^16 |
| 8 | 5-15 | a_2 | Clock drift rate | 2^-66 |
| 8 | 16-22 | a_0[23:17] | Clock bias MSB | 2^-33 |
| 9 | 1-17 | a_0[16:0] | Clock bias LSB | 2^-33 |
| 9 | 18-22 | a_1[21:17] | Clock drift MSB | 2^-50 |
| 10 | 1-17 | a_1[16:0] | Clock drift LSB | 2^-50 |
| 10 | 18-22 | AODE | Age of Data Ephemeris | - |

#### Subframe 2 - Ephemeris Data Part 1

| Word | Bits | Parameter | Description | Scale Factor |
|------|------|-----------|-------------|--------------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | Subframe ID (010) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-10 | Δn[15:6] | Mean motion correction MSB | π × 2^-43 |
| 3 | 1-6 | Δn[5:0] | Mean motion correction LSB | π × 2^-43 |
| 3 | 7-22 | C_uc | Cosine harmonic correction | 2^-31 |
| 4 | 1-2 | C_uc | Cosine harmonic correction | 2^-31 |
| 4 | 3-22 | M_0[31:12] | Mean anomaly MSB | π × 2^-31 |
| 5 | 1-12 | M_0[11:0] | Mean anomaly LSB | π × 2^-31 |
| 5 | 13-22 | e[31:22] | Eccentricity MSB | 2^-33 |
| 6-7 | All | e[21:0] | Eccentricity LSB | 2^-33 |
| 8 | 1-18 | C_us | Sine harmonic correction | 2^-31 |
| 8 | 19-22 | C_rc[17:14] | Cosine harmonic radius MSB | 2^-6 |
| 9 | 1-14 | C_rc[13:0] | Cosine harmonic radius LSB | 2^-6 |
| 9 | 15-22 | C_rs[17:10] | Sine harmonic radius MSB | 2^-6 |
| 10 | 1-10 | C_rs[9:0] | Sine harmonic radius LSB | 2^-6 |
| 10 | 11-22 | sqrt(A)[31:20] | Square root of semi-major axis MSB | 2^-19 |

#### Subframe 3 - Ephemeris Data Part 2

| Word | Bits | Parameter | Description | Scale Factor |
|------|------|-----------|-------------|--------------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | Subframe ID (011) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-10 | t_oe[9:0] | Ephemeris reference time | 2^3 |
| 3 | 1-5 | t_oe[4:0] | Ephemeris reference time | 2^3 |
| 3 | 6-22 | i_0[31:15] | Inclination angle MSB | π × 2^-31 |
| 4 | 1-15 | i_0[14:0] | Inclination angle LSB | π × 2^-31 |
| 4 | 16-22 | C_ic[17:11] | Cosine harmonic inclination MSB | 2^-31 |
| 5 | 1-11 | C_ic[10:0] | Cosine harmonic inclination LSB | 2^-31 |
| 5 | 12-22 | Ω_dot[23:13] | Rate of right ascension MSB | π × 2^-43 |
| 6 | 1-13 | Ω_dot[12:0] | Rate of right ascension LSB | π × 2^-43 |
| 6 | 14-22 | C_is[17:9] | Sine harmonic inclination MSB | 2^-31 |
| 7 | 1-9 | C_is[8:0] | Sine harmonic inclination LSB | 2^-31 |
| 7 | 10-22 | IDOT[13:1] | Rate of inclination angle MSB | π × 2^-43 |
| 8 | 1 | IDOT[0] | Rate of inclination angle LSB | π × 2^-43 |
| 8 | 2-22 | Ω_0[31:11] | Right ascension MSB | π × 2^-31 |
| 9 | 1-11 | Ω_0[10:0] | Right ascension LSB | π × 2^-31 |
| 9 | 12-22 | ω[31:21] | Argument of perigee MSB | π × 2^-31 |
| 10 | 1-21 | ω[20:0] | Argument of perigee LSB | π × 2^-31 |

## BeiDou D2 Navigation Message (B1I/B2I/B3I for GEO)

### General Parameters
- **Used by**: GEO satellites (SVID 1-5, 59-63)
- **Signals**: B1I (1561.098 MHz), B2I (1207.14 MHz), B3I (1268.52 MHz)
- **Data Rate**: 500 bits/sec
- **Frame Duration**: 3 seconds (1500 bits)
- **Subframe Duration**: 0.6 seconds (300 bits)
- **Word Duration**: 0.06 seconds (30 bits)
- **Modulation**: BPSK
- **Error Correction**: BCH(15,11,1) + Interleaving

### D2 Frame Structure

**Superframe**: 180,000 bits (6 minutes)
- 120 frames × 1500 bits

**Frame**: 1500 bits (3 seconds)
- 5 subframes × 300 bits

**Subframe**: 300 bits (0.6 seconds)
- 10 words × 30 bits

### D2 Subframe Allocation

| Subframe | Pages | Content |
|----------|-------|---------|
| 1 | 1-10 | Navigation message (10 pages) |
| 2 | - | Integrity and differential correction |
| 3 | - | Reserved |
| 4 | - | Reserved |
| 5 | - | Reserved |

### D2 Navigation Message Pages (Subframe 1)

| Page | Content |
|------|---------|
| 1 | Basic navigation parameters, health, IODC |
| 2 | Ionospheric parameters |
| 3 | Clock correction parameters (part 1) |
| 4 | Clock correction parameters (part 2), ephemeris (part 1) |
| 5 | Ephemeris (part 2) |
| 6 | Ephemeris (part 3) |
| 7 | Ephemeris (part 4) |
| 8 | Ephemeris (part 5) |
| 9 | Ephemeris (part 6) |
| 10 | Ephemeris (part 7) |

## D2 Detailed Word Structure by Pages

### Page 1 - Basic Navigation Parameters

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 1 | 23-26 | BCH | Error correction | - |
| 1 | 27-30 | Reserved | - | - |
| 2 | 1-4 | Pnum | Page number (0001) | - |
| 2 | 5 | SatH1 | Satellite health | - |
| 2 | 6-10 | IODC | Issue of Data Clock | - |
| 2 | 11-14 | URAI | User Range Accuracy Index | - |
| 2 | 15-22 | Reserved | - | - |
| 3 | 1-4 | URAI (continued) | User Range Accuracy Index | - |
| 3 | 5-17 | WN | Week Number BDT | 1 |
| 3 | 18-22 | t_oc[16:12] | Clock reference time MSB | 2^3 |
| 4 | 1-12 | t_oc[11:0] | Clock reference time LSB | 2^3 |
| 4 | 13-22 | TGD1 | Group Delay B1I | 0.1 ns |

### Page 2 - Ionospheric Parameters

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (0010) | - |
| 2 | 5-10 | TGD2 | Group Delay B2I | 0.1 ns |
| 2 | 11-22 | Reserved | - | - |
| 3 | 1-2 | Reserved | - | - |
| 3 | 3-10 | α_0 | Ionospheric parameter | 2^-30 |
| 3 | 11-18 | α_1 | Ionospheric parameter | 2^-27 |
| 3 | 19-22 | α_2[7:4] | Ionospheric parameter MSB | 2^-24 |
| 4 | 1-4 | α_2[3:0] | Ionospheric parameter LSB | 2^-24 |
| 4 | 5-12 | α_3 | Ionospheric parameter | 2^-24 |
| 4 | 13-20 | β_0 | Ionospheric parameter | 2^11 |
| 4 | 21-22 | β_1[7:6] | Ionospheric parameter MSB | 2^14 |

### Page 3 - Clock Correction Parameters (part 1)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (0011) | - |
| 2-3 | - | Reserved | - | - |
| 4 | 1-12 | a_0[23:12] | Clock bias MSB | 2^-33 |

### Page 4 - Clock Correction Parameters (part 2) and Ephemeris (part 1)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (0100) | - |
| 2 | 5-10 | a_0[11:6] | Clock bias | 2^-33 |
| 2 | 11-22 | a_1[21:10] | Clock drift MSB | 2^-50 |
| 3 | 1-10 | a_1[9:0] | Clock drift LSB | 2^-50 |
| 3 | 11-22 | a_2 | Clock drift rate | 2^-66 |
| 4 | 1 | a_2 (continued) | Clock drift rate | 2^-66 |
| 4 | 2-6 | IODE | Issue of Data Ephemeris | - |
| 4 | 7-22 | Δn | Mean motion correction | π × 2^-43 |

### Page 5 - Ephemeris (part 2)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (0101) | - |
| 2 | 5-8 | C_uc[17:14] | Cosine harmonic correction latitude MSB | 2^-31 |
| 2 | 9-22 | C_uc[13:0] | Cosine harmonic correction latitude LSB | 2^-31 |
| 3 | 1-2 | M_0[31:30] | Mean anomaly MSB | π × 2^-31 |
| 3 | 3-22 | M_0[29:10] | Mean anomaly | π × 2^-31 |
| 4 | 1-8 | M_0[9:2] | Mean anomaly LSB | π × 2^-31 |
| 4 | 9-22 | C_us[17:4] | Sine harmonic correction latitude MSB | 2^-31 |

### Page 6 - Ephemeris (part 3)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (0110) | - |
| 2 | 5-8 | C_us[3:0] | Sine harmonic correction latitude LSB | 2^-31 |
| 2 | 9-18 | e[31:22] | Eccentricity MSB | 2^-33 |
| 2 | 19-22 | e[21:18] | Eccentricity | 2^-33 |
| 3 | 1-16 | e[17:2] | Eccentricity | 2^-33 |
| 3 | 17-22 | sqrt(A)[31:26] | Square root of semi-major axis MSB | 2^-19 |
| 4 | 1-22 | sqrt(A)[25:4] | Square root of semi-major axis | 2^-19 |

### Page 7 - Ephemeris (part 4)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (0111) | - |
| 2 | 5-8 | sqrt(A)[3:0] | Square root of semi-major axis LSB | 2^-19 |
| 2 | 9-18 | C_ic[17:8] | Cosine harmonic correction inclination MSB | 2^-31 |
| 2 | 19-22 | C_ic[7:4] | Cosine harmonic correction inclination | 2^-31 |
| 3 | 1-2 | t_oe[16:15] | Ephemeris reference time MSB | 2^3 |
| 3 | 3-18 | C_is | Sine harmonic correction inclination | 2^-31 |
| 3 | 19-22 | t_oe[14:11] | Ephemeris reference time | 2^3 |
| 4 | 1-7 | t_oe[10:4] | Ephemeris reference time | 2^3 |
| 4 | 8-22 | i_0[31:17] | Inclination angle MSB | π × 2^-31 |

### Page 8 - Ephemeris (part 5)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (1000) | - |
| 2 | 5-10 | i_0[16:11] | Inclination angle | π × 2^-31 |
| 2 | 11-22 | C_ic[3:0] | Cosine harmonic correction inclination LSB | 2^-31 |
| 3 | 1 | C_rc[17] | Cosine harmonic correction radius MSB | 2^-6 |
| 3 | 2-18 | C_rc[16:0] | Cosine harmonic correction radius | 2^-6 |
| 3 | 19-22 | C_rs[17:14] | Sine harmonic correction radius MSB | 2^-6 |
| 4 | 1-3 | C_rs[13:11] | Sine harmonic correction radius | 2^-6 |
| 4 | 4-22 | Ω_dot[23:5] | Right ascension rate MSB | π × 2^-43 |

### Page 9 - Ephemeris (part 6)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (1001) | - |
| 2 | 5-9 | Ω_dot[4:0] | Right ascension rate LSB | π × 2^-43 |
| 2 | 10 | Ω_0[31] | Right ascension MSB | π × 2^-31 |
| 2 | 11-22 | C_rs[10:0] | Sine harmonic correction radius LSB | 2^-6 |
| 3 | 1-22 | Ω_0[30:9] | Right ascension | π × 2^-31 |
| 4 | 1-9 | Ω_0[8:0] | Right ascension LSB | π × 2^-31 |
| 4 | 10-22 | ω[31:19] | Argument of perigee MSB | π × 2^-31 |

### Page 10 - Ephemeris (part 7)

| Word | Bits | Parameter | Description | Scale Factor |
|-------|------|----------|----------|---------|
| 1 | 1-11 | Preamble | 11100010010 | - |
| 1 | 12-14 | FraID | ID subframe (001) | - |
| 1 | 15-22 | SOW[19:12] | Seconds of Week MSB | 1 |
| 2 | 1-4 | Pnum | Page number (1010) | - |
| 2 | 5-9 | ω[18:14] | Argument of perigee | π × 2^-31 |
| 2 | 10 | i_dot[13] | Speed изменения наклонения MSB | π × 2^-43 |
| 2 | 11-22 | i_0[10:0] | Inclination angle LSB | π × 2^-31 |
| 3 | 1-13 | i_dot[12:0] | Speed изменения наклонения LSB | π × 2^-43 |
| 3 | 14-22 | ω[13:5] | Argument of perigee | π × 2^-31 |
| 4 | 1-5 | ω[4:0] | Argument of perigee LSB | π × 2^-31 |

## D2 Notes

1. **Compactness**: D2 использует более компактное представление данных, распределяя параметры по 10 страницам
2. **Speed**: Передается в 10 раз быстрее чем D1 (500 бит/сек против 50 бит/сек)
3. **GEO specifics**: Оптимизировано для геостационарных спутников с минимальными изменениями орбиты
4. **Interleaving**: Применяется ко всем словам кроме первого, как и в D1
5. **BCH код**: Используется тот же (15,11,1) код Хэмминга для коррекции ошибок

*MSB - Most Significant Bits, LSB - Least Significant Bits*

## BeiDou B-CNAV1 (B1C) - Detailed Structure

### General Parameters
- **Frequency**: 1575.42 MHz
- **Data Rate**: 100 symbols/sec
- **Frame Duration**: 18 seconds (1800 symbols)
- **Modulation**: Data: BOC(1,1), Pilot: QMBOC(6,1,4/33)
- **Power Split**: 25% data, 75% pilot
- **FEC**: LDPC(64,56) + BCH(21,6)
- **Interleaving**: Block interleaving

### B-CNAV1 Frame Structure

**Frame**: 1800 symbols
- **Subframe 1**: 72 symbols (0.72 sec) - Fixed content
- **Subframe 2**: 1200 symbols (12 sec) - Variable content
- **Subframe 3**: 528 symbols (5.28 sec) - FEC

### B-CNAV1 Subframe 1 Structure

| Symbols | Parameter | Description | Value/Range |
|---------|-----------|-------------|-------------|
| 1-24 | PRE | Preamble | 0xEB90F1 |
| 25-30 | PRN | Satellite PRN | 1-63 |
| 31-38 | SOH | Seconds of Hour | 0-3599 |
| 39-41 | Reserved | - | 0 |
| 42-48 | HEOAA | Hour/Ephemeris/Others/Almanac Age | Variable |
| 49-55 | HAI | Health/Accuracy/Integrity | Variable |
| 56-59 | Reserved | - | 0 |
| 60-65 | BCH | Error correction | - |
| 66-72 | Reserved | - | 0 |

### B-CNAV1 Message Types

#### Message Type 10 - Ephemeris (Most Common)

| Bits | Parameter | Description | Scale Factor | Units |
|------|-----------|-------------|--------------|-------|
| 1-6 | MesType | Message type = 10 | - | - |
| 7-24 | SOW | Seconds of BDT week | 1 | s |
| 25-37 | WN | BDT week number | 1 | week |
| 38-46 | URAED_Index | User Range Accuracy | - | index |
| 47-54 | t_oe | Reference time of ephemeris | 300 | s |
| 55-86 | sqrt(A) | Square root of semi-major axis | 2^-6 | m^0.5 |
| 87-118 | e | Eccentricity | 2^-34 | - |
| 119-150 | ω | Argument of perigee | 2^-32 | semicircles |
| 151-167 | Δn_0 | Mean motion difference | 2^-44 | semicircles/s |
| 168-199 | M_0 | Mean anomaly at t_oe | 2^-32 | semicircles |
| 200-231 | Ω_0 | Longitude of ascending node | 2^-32 | semicircles |
| 232-263 | i_0 | Inclination angle at t_oe | 2^-32 | semicircles |
| 264-281 | Ω_dot | Rate of right ascension | 2^-44 | semicircles/s |
| 282-296 | i_dot | Rate of inclination angle | 2^-44 | semicircles/s |
| 297-314 | C_is | Sine harmonic correction inclination | 2^-30 | rad |
| 315-332 | C_ic | Cosine harmonic correction inclination | 2^-30 | rad |
| 333-350 | C_rs | Sine harmonic correction radius | 2^-6 | m |
| 351-368 | C_rc | Cosine harmonic correction radius | 2^-6 | m |
| 369-386 | C_us | Sine harmonic correction latitude | 2^-30 | rad |
| 387-404 | C_uc | Cosine harmonic correction latitude | 2^-30 | rad |
| 405-422 | a_dot | Semi-major axis rate | 2^-20 | m/s |
| 423-433 | Δn_0dot | Rate of mean motion difference | 2^-57 | semicircles/s^2 |
| 434-450 | t_oc | Clock data reference time | 300 | s |
| 451-477 | a_0 | Satellite clock bias | 2^-33 | s |
| 478-500 | a_1 | Satellite clock drift | 2^-50 | s/s |
| 501-512 | a_2 | Satellite clock drift rate | 2^-66 | s/s^2 |

#### Message Type 11 - Clock Corrections

| Bits | Parameter | Description | Scale Factor | Units |
|------|-----------|-------------|--------------|-------|
| 1-6 | MesType | Message type = 11 | - | - |
| 7-24 | SOW | Seconds of BDT week | 1 | s |
| 25-37 | WN | BDT week number | 1 | week |
| 38-55 | t_oc | Clock data reference time | 300 | s |
| 56-82 | a_0 | Satellite clock bias | 2^-33 | s |
| 83-105 | a_1 | Satellite clock drift | 2^-50 | s/s |
| 106-117 | a_2 | Satellite clock drift rate | 2^-66 | s/s^2 |
| 118-127 | TGD_B1Cp | Group delay B1C pilot | 2^-33 | s |
| 128-137 | TGD_B1Cd | Group delay B1C data | 2^-33 | s |
| 138-147 | TGD_B2ap | Group delay B2a pilot | 2^-33 | s |
| 148-157 | TGD_B2bi | Group delay B2b | 2^-33 | s |
| 158-167 | ISC_B1Cd | Inter-signal correction B1C data | 2^-33 | s |
| 168-177 | ISC_B2ad | Inter-signal correction B2a data | 2^-33 | s |

#### Message Type 30 - Ionosphere Parameters

| Bits | Parameter | Description | Scale Factor | Units |
|------|-----------|-------------|--------------|-------|
| 1-6 | MesType | Message type = 30 | - | - |
| 7-8 | IonoCorrectionType | 00=Klobuchar, 01=BDGIM | - | - |
| 9-16 | α_0 | Ionospheric parameter | 2^-30 | s |
| 17-24 | α_1 | Ionospheric parameter | 2^-27 | s/semicircle |
| 25-32 | α_2 | Ionospheric parameter | 2^-24 | s/semicircle^2 |
| 33-40 | α_3 | Ionospheric parameter | 2^-24 | s/semicircle^3 |
| 41-48 | β_0 | Ionospheric parameter | 2^11 | s |
| 49-56 | β_1 | Ionospheric parameter | 2^14 | s/semicircle |
| 57-64 | β_2 | Ionospheric parameter | 2^16 | s/semicircle^2 |
| 65-72 | β_3 | Ionospheric parameter | 2^16 | s/semicircle^3 |

## BeiDou B-CNAV2 (B2a) - Detailed Structure

### General Parameters
- **Frequency**: 1176.45 MHz
- **Data Rate**: 200 symbols/sec
- **Frame Duration**: 3 seconds (600 symbols)
- **Modulation**: QPSK(10)
- **Power Split**: 50% data, 50% pilot
- **FEC**: LDPC(96,72)

### B-CNAV2 Frame Structure

**Frame**: 600 symbols
- **Preamble**: 24 symbols
- **Data**: 432 symbols
- **FEC**: 144 symbols

### B-CNAV2 Message Types

Uses same message types as B-CNAV1 but with optimized encoding:
- More frequent ephemeris updates (every 3 seconds possible)
- Higher data rate allows faster acquisition
- Enhanced FEC for better performance

## BeiDou B-CNAV3 (B2b) - Detailed Structure

### General Parameters
- **Frequency**: 1207.14 MHz
- **Data Rate**: 1000 symbols/sec (500 bits/sec)
- **Frame Duration**: 1 second
- **Modulation**: BPSK(1)
- **Purpose**: PPP-B2b high-precision service
- **FEC**: LDPC

### B-CNAV3 Frame Structure

**Frame**: 1000 symbols
- Contains PPP corrections and integrity information
- Designed for real-time precise positioning

### B-CNAV3 Message Types

B-CNAV3 supports different message types for various applications:

**PPP (Precise Point Positioning) Message Types:**

| Type | Content | Update Rate |
|------|---------|-------------|
| 1 | Orbit corrections | 6 seconds |
| 2 | Clock corrections | 6 seconds |
| 3 | Code biases | 30 seconds |
| 4 | Phase biases | 30 seconds |
| 5 | VTEC corrections | 30 seconds |
| 6 | Integrity information | 6 seconds |

**Standard Navigation Message Types:**

| Type | Content | Size |
|------|---------|------|
| 10 | Ephemeris and clock corrections | 968 bits |
| 11 | Ionospheric parameters | 264 bits |
| 12 | Reduced ephemeris | 608 bits |
| 30 | ISM differential corrections | variable |
| 31 | Clock corrections | 264 bits |
| 32 | EOP parameters | 528 bits |
| 33 | UTC parameters | 424 bits |
| 34 | Text message | variable |
| 63 | Null message | min. 8 bits |

#### B-CNAV3 Message Types - Detailed Structure

**Type 10 - Ephemeris and Clock Corrections (968 bits):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-17 | WN | BeiDou week number | weeks |
| 18-25 | IODC | Issue of Data Clock | - |
| 26-33 | IODE | Issue of Data Ephemeris | - |
| 34-48 | toc | Clock correction reference time | 300 s |
| 49-73 | af0 | Satellite clock offset | 2^-34 s |
| 74-95 | af1 | Satellite clock drift | 2^-50 s/s |
| 96-112 | af2 | Satellite clock drift rate | 2^-66 s/s² |
| 113-119 | TGD_B1Cp | B1C pilot group delay | 10^-10 s |
| 120-126 | TGD_B1Cd | B1C data group delay | 10^-10 s |
| 127-133 | TGD_B2ap | B2a pilot group delay | 10^-10 s |
| 134-140 | TGD_B2bp | B2b pilot group delay | 10^-10 s |
| 141-153 | ISC_B1Cd | B1C inter-signal correction | 2^-35 s |
| 154-166 | ISC_B2ad | B2a inter-signal correction | 2^-35 s |
| 167-183 | toe | Ephemeris reference time | 300 s |
| 184-215 | M0 | Mean anomaly | 2^-31 semicircles |
| 216-233 | e | Eccentricity | 2^-33 |
| 234-265 | sqrt(A) | Square root of semi-major axis | 2^-19 m^1/2 |
| 266-297 | Ω0 | Longitude of ascending node | 2^-31 semicircles |
| 298-329 | i0 | Inclination angle | 2^-31 semicircles |
| 330-361 | ω | Argument of perigee | 2^-31 semicircles |
| 362-385 | Ω̇ | Rate of ascending node | 2^-43 semicircles/s |
| 386-400 | iDOT | Rate of inclination angle | 2^-43 semicircles/s |
| 401-416 | Δn | Mean motion correction | 2^-43 semicircles/s |
| 417-433 | Cuc | Cosine harmonic correction to argument of latitude | 2^-31 rad |
| 434-450 | Cus | Sine harmonic correction to argument of latitude | 2^-31 rad |
| 451-469 | Crc | Cosine harmonic correction to orbit radius | 2^-6 m |
| 470-488 | Crs | Sine harmonic correction to orbit radius | 2^-6 m |
| 489-507 | Cic | Cosine harmonic correction to inclination | 2^-31 rad |
| 508-526 | Cis | Sine harmonic correction to inclination | 2^-31 rad |
| 527-968 | Reserved | - | - |

**Type 11 - Ionospheric Parameters (264 bits):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-14 | α0 | Alpha coefficient 0 | 2^-30 s |
| 15-22 | α1 | Alpha coefficient 1 | 2^-27 s/semicircle |
| 23-30 | α2 | Alpha coefficient 2 | 2^-24 s/semicircle² |
| 31-38 | α3 | Alpha coefficient 3 | 2^-24 s/semicircle³ |
| 39-46 | β0 | Beta coefficient 0 | 2^11 s |
| 47-54 | β1 | Beta coefficient 1 | 2^14 s/semicircle |
| 55-62 | β2 | Beta coefficient 2 | 2^16 s/semicircle² |
| 63-70 | β3 | Beta coefficient 3 | 2^16 s/semicircle³ |
| 71-264 | Reserved | - | - |

**Type 12 - Reduced Ephemeris (608 bits):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-17 | WN | BeiDou week number | weeks |
| 18-28 | toa | Almanac reference time | 2^12 s |
| 29-36 | IODA | Issue of Data Almanac | - |
| 37-61 | δA | Semi-major axis difference | 2^-9 m |
| 62-78 | A-DOT | Semi-major axis rate | 2^-21 m/s |
| 79-95 | δn0 | Mean motion correction | 2^-44 semicircles/s |
| 96-112 | δn0-DOT | Mean motion rate | 2^-57 semicircles/s² |
| 113-139 | M0 | Mean anomaly | 2^-32 semicircles |
| 140-157 | e | Eccentricity | 2^-21 |
| 158-184 | ω | Argument of perigee | 2^-32 semicircles |
| 185-211 | Ω0 | Longitude of ascending node | 2^-32 semicircles |
| 212-229 | i0 | Inclination angle | 2^-19 semicircles |
| 230-248 | Ω̇ | Rate of ascending node | 2^-45 semicircles/s |
| 249-263 | δi | Inclination correction | 2^-44 semicircles/s |
| 264-277 | Cis | Sine harmonic correction to inclination | 2^-30 rad |
| 278-291 | Cic | Cosine harmonic correction to inclination | 2^-30 rad |
| 292-307 | Crs | Sine harmonic correction to orbit radius | 2^-8 m |
| 308-323 | Crc | Cosine harmonic correction to orbit radius | 2^-8 m |
| 324-339 | Cus | Sine harmonic correction to argument of latitude | 2^-30 rad |
| 340-355 | Cuc | Cosine harmonic correction to argument of latitude | 2^-30 rad |
| 356-366 | af0 | Satellite clock offset | 2^-20 s |
| 367-376 | af1 | Satellite clock drift | 2^-38 s/s |
| 377-608 | Reserved | - | - |

**Type 30 - ISM Differential Corrections (variable):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-8 | UD index | User differential range accuracy index | - |
| 9-17 | IODC | Issue of Data Clock | - |
| 18-26 | IODE | Issue of Data Ephemeris | - |
| 27-41 | Δaf0 | Delta clock offset | 2^-35 s |
| 42-55 | Δaf1 | Delta clock drift | 2^-51 s/s |
| 56-78 | ΔOrb_R | Radial orbit correction | 0.125 m |
| 79-98 | ΔOrb_T | Along-track orbit correction | 0.015625 m |
| 99-118 | ΔOrb_N | Cross-track orbit correction | 0.0625 m |
| 119-135 | ΔOrb_Ṙ | Radial orbit rate correction | 0.001 m/s |
| 136-152 | ΔOrb_Ṫ | Along-track orbit rate correction | 0.001 m/s |
| 153-169 | ΔOrb_Ṅ | Cross-track orbit rate correction | 0.001 m/s |
| 170+ | Additional data | Implementation specific | - |

**Type 31 - Clock Corrections (264 bits):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-17 | WN | BeiDou week number | weeks |
| 18-25 | IODC | Issue of Data Clock | - |
| 26-48 | toc | Clock correction reference time | 300 s |
| 49-73 | af0 | Satellite clock offset | 2^-34 s |
| 74-95 | af1 | Satellite clock drift | 2^-50 s/s |
| 96-112 | af2 | Satellite clock drift rate | 2^-66 s/s² |
| 113-119 | TGD_B1Cp | B1C pilot group delay | 10^-10 s |
| 120-126 | TGD_B1Cd | B1C data group delay | 10^-10 s |
| 127-133 | TGD_B2ap | B2a pilot group delay | 10^-10 s |
| 134-140 | TGD_B2bp | B2b pilot group delay | 10^-10 s |
| 141-153 | ISC_B1Cd | B1C inter-signal correction | 2^-35 s |
| 154-166 | ISC_B2ad | B2a inter-signal correction | 2^-35 s |
| 167-264 | Reserved | - | - |

**Type 32 - EOP Parameters (528 bits):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-21 | teop | EOP reference time | 2^4 s |
| 22-23 | PM source | Polar motion data source | - |
| 24-44 | xp | X pole coordinate | 2^-20 arc seconds |
| 45-65 | yp | Y pole coordinate | 2^-20 arc seconds |
| 66-80 | ẋp | X pole rate | 2^-21 arc seconds/day |
| 81-95 | ẏp | Y pole rate | 2^-21 arc seconds/day |
| 96-126 | UT1-UTC | UT1-UTC difference | 2^-24 s |
| 127-145 | UT1-UTC rate | UT1-UTC rate | 2^-25 s/day |
| 146-159 | Δt_LS | Current leap second | s |
| 160-167 | WN_LSF | Week of next leap second | weeks |
| 168-175 | DN_LSF | Day of next leap second | days |
| 176-189 | Δt_LSF | Next leap second value | s |
| 190-528 | Reserved | - | - |

**Type 33 - UTC Parameters (424 bits):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-32 | A0_UTC | UTC offset | 2^-30 s |
| 33-56 | A1_UTC | UTC drift | 2^-50 s/s |
| 57-64 | A2_UTC | UTC quadratic term | 2^-68 s/s² |
| 65-72 | Δt_LS | Current leap second | s |
| 73-80 | tot | UTC parameters reference time | 2^12 s |
| 81-88 | WNot | Week of tot | weeks |
| 89-96 | WN_LSF | Week of next leap second | weeks |
| 97-104 | DN | Day of next leap second | days |
| 105-112 | Δt_LSF | Next leap second value | s |
| 113-136 | A0_GPS | GPS-BDT offset | 2^-30 s |
| 137-152 | A1_GPS | GPS-BDT drift | 2^-50 s/s |
| 153-168 | A0_GLO | GLONASS-BDT offset | 2^-30 s |
| 169-184 | A1_GLO | GLONASS-BDT drift | 2^-50 s/s |
| 185-200 | A0_GAL | Galileo-BDT offset | 2^-30 s |
| 201-216 | A1_GAL | Galileo-BDT drift | 2^-50 s/s |
| 217-424 | Reserved | - | - |

**Type 34 - Text Message (variable):**

| Bits | Parameter | Description | Scale/Units |
|------|-----------|-------------|-------------|
| 1-6 | PRN | Satellite PRN number | - |
| 7-8 | Source ID | Message source identifier | - |
| 9-16 | Page number | Message page number | - |
| 17-24 | Total pages | Total number of pages | - |
| 25+ | Text | UTF-8 encoded text | - |

**Type 63 - Null Message (minimum 8 bits):**

| Bits | Parameter | Description |
|------|----------|----------|
| 1-6 | PRN | Satellite PRN number |
| 7+ | Padding | Zero padding |

## Implementation Notes

### BCH Encoding (D1/D2)
- Generator polynomial: g(x) = x^4 + x + 1
- Systematic code: information bits unchanged
- Applied to each 30-bit word

### Interleaving (D1/D2)
- Applied to all words except the first in each subframe
- Bit position mapping: i → ((i mod 30) × 30 + i/30)

### LDPC Encoding (B-CNAV)
- B1C: LDPC(64,56) - 8 parity bits
- B2a: LDPC(96,72) - 24 parity bits
- Provides better error correction than BCH

### Secondary Codes
- B1C pilot: 1800-bit secondary code
- B2a pilot: 100-bit secondary code
- B2b: No secondary code
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

**Detailed S-band Navigation Message Structure:**

**Superframe Structure**: 2400 bits
- 4 subframes of 600 bits each
- Data rate: 50 bits/sec
- Superframe duration: 48 seconds

**Subframe 1 - Ephemeris and Clock Parameters:**
| Bits | Parameter | Description | Scale Factor |
|------|-----------|-------------|--------------|
| 1-8 | TLM | Telemetry word | - |
| 9-30 | HOW | Handover Word | - |
| 31-37 | TOWC | Time of Week Count | 12 s |
| 38-39 | Alert flag | Alert flag | - |
| 40-41 | Autonav | Autonomous navigation flag | - |
| 42-49 | Spare | Reserved | - |
| 50-60 | WN | Week Number | weeks |
| 61-81 | af0 | Satellite clock bias | 2^-31 s |
| 82-98 | af1 | Satellite clock drift | 2^-43 s/s |
| 99-106 | af2 | Satellite clock drift rate | 2^-55 s/s² |
| 107-108 | Sura | User range accuracy | - |
| 109-112 | Health | Satellite health | - |
| 113-120 | TGD | Time group delay | 2^-31 s |
| 121-130 | IODC | Issue of Data Clock | - |
| 131-152 | toc | Clock data reference time | 2^4 s |
| 153-600 | Ephemeris | Orbital parameters | various |

**Detailed Ephemeris Structure in Subframe 1:**
| Bits | Parameter | Description | Scale Factor |
|------|-----------|-------------|--------------|
| 153-160 | Special flags | Special flags | - |
| 161-176 | CRS | Sine correction to radius | 2^-5 m |
| 177-184 | Δn | Mean motion difference | 2^-41 rad/s |
| 185-216 | M0 | Mean anomaly | 2^-31 semicircles |
| 217-232 | CUC | Cosine correction to argument of latitude | 2^-29 rad |
| 233-264 | e | Eccentricity | 2^-33 |
| 265-280 | CUS | Sine correction to argument of latitude | 2^-29 rad |
| 281-312 | A^1/2 | Square root of semi-major axis | 2^-19 m^1/2 |
| 313-330 | toe | Ephemeris reference time | 2^4 s |
| 331-346 | CIC | Cosine correction to inclination | 2^-29 rad |
| 347-378 | Ω0 | Longitude of ascending node | 2^-31 semicircles |
| 379-394 | CIS | Sine correction to inclination | 2^-29 rad |
| 395-426 | i0 | Inclination angle | 2^-31 semicircles |
| 427-442 | CRC | Cosine correction to radius | 2^-5 m |
| 443-474 | ω | Argument of perigee | 2^-31 semicircles |
| 475-498 | Ω̇ | Rate of ascending node | 2^-38 semicircles/s |
| 499-508 | IODE | Issue of Data Ephemeris | - |
| 509-522 | iDOT | Rate of inclination angle | 2^-43 semicircles/s |
| 523-600 | Reserved | - | - |

**Subframes 2 and 3 - Almanac:**
Contains almanac for all NavIC satellites:
- Subframe 2: Satellites 1-4
- Subframe 3: Satellites 5-7

**Almanac Structure per Satellite (150 bits):**
| Bits | Parameter | Description | Scale Factor |
|------|-----------|-------------|--------------|
| 1-7 | Health | Satellite health | - |
| 8-17 | e | Eccentricity | 2^-21 |
| 18-29 | toa | Almanac reference time | 2^12 s |
| 30-45 | δi | Inclination correction | 2^-19 semicircles |
| 46-61 | Ω̇ | Rate of right ascension | 2^-38 semicircles/s |
| 62-85 | A^1/2 | Square root of semi-major axis | 2^-11 m^1/2 |
| 86-109 | Ω0 | Longitude of ascending node | 2^-23 semicircles |
| 110-133 | ω | Argument of perigee | 2^-23 semicircles |
| 134-150 | M0 | Mean anomaly | 2^-23 semicircles |

**Subframe 4 - Special Messages:**
- Ionospheric parameters
- UTC parameters
- Text messages
- Differential correction parameters

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