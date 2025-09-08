# GNSS-SDR — Quick Start Guide

This README provides instructions to build, install, and run **GNSS-SDR**, along with guidance on the types of outputs you can expect and how to convert them into mapping formats such as KML, GeoJSON, and GPX.

---

## 1. Running GNSS-SDR

Check if GNSS-SDR is installed and accessible:

```bash
gnss-sdr --version
```

Run GNSS-SDR with a configuration file:

```bash
gnss-sdr -c conf_file.conf
```

> Start with an example config file (e.g., `conf/gnss-sdr_GPS_L1.conf`) and adapt paths, sampling rates, IF, and other parameters to match your front-end or data file.

---

## 2. Build & Install (Ubuntu/Debian)

Update packages and install build dependencies:

```bash
sudo apt update
sudo apt build-dep gnss-sdr
```

Clone the repository and switch to the development branch:

```bash
git clone https://github.com/gnss-sdr/gnss-sdr
cd gnss-sdr
git checkout next
```

Configure and build:

```bash
mkdir -p build && cd build

cmake .. \
  -DENABLE_OPENCL=OFF \        # Disable OpenCL if unsupported
  -DENABLE_UNIT_TESTING=OFF    # Disable unit tests for lean build

make -j"$(nproc)"
sudo make install
```

---

## 3. Supported Satellite Systems

GNSS-SDR supports a wide range of GNSS constellations and signals:

- **GPS (USA)**
  - L1 C/A
  - L2C
  - L5
- **Galileo (Europe)**
  - E1 B/C
  - E5a
  - E5b
- **GLONASS (Russia)**
  - L1 C/A
  - L2 C/A
- **BeiDou (China)**
  - B1I
  - B3I
- **QZSS (Japan)**
  - L1 C/A
  - L2C
  - L5
- **SBAS (WAAS, EGNOS, etc.)**
  - L1 C/A

> Support depends on your configuration and available RF front-end. Not all signals are enabled in the default builds.

---

## 4. Expected Outputs

Depending on your `.conf` setup, GNSS-SDR produces different outputs:

### Navigation Messages
- Decoded ephemerides, almanac, and clock parameters.
- Exported as **RINEX NAV (`.nav`)** files.

### Observables
- Pseudorange, carrier phase, Doppler, C/N0 per satellite.
- Exported as **RINEX OBS (`.obs`)** files.

### PVT / Position, Velocity, Time
- **NMEA messages** (`.nmea`), including:
  - `GGA`: fix, lat/lon/alt, satellites, HDOP
  - `RMC`: position, speed, course, UTC date/time
  - `GSA/GSV`: satellite geometry/status
- **CSV PVT logs** (lat, lon, alt, velocity, clock bias).

---

## 5. Interpreting Key Outputs

- **RINEX NAV (`.nav`)**  
  Satellite broadcast ephemerides, usable for post-processing and orbit/clock analysis.

- **RINEX OBS (`.obs`)**  
  Code, carrier, Doppler, and SNR measurements. Suitable for RTK/PPP workflows.

- **NMEA (`.nmea`)**  
  Human-readable GNSS messages, easily convertible to GPX/KML.

- **CSV PVT (`.csv`)**  
  Plain-text logs of computed positions and time. Ideal for plotting or conversion.

- **KML / GeoJSON / GPX**  
  Final map overlays for Google Earth (KML), web maps (GeoJSON), or GPS devices (GPX).

---

## 6. Troubleshooting

- **No lock / no PVT:** Check sampling rate, IF, and IQ swap settings.  
- **Weak C/N0:** Ensure proper RF gain and clear sky view.  
- **Empty RINEX:** Enable RINEX logger blocks in the config file and verify output paths.  

---

## 7. Example Workflow

```bash
# 1) Run GNSS-SDR with a config
gnss-sdr -c my_config.conf

# 2) Convert NMEA → KML
gpsbabel -i nmea -f ./logs/output.nmea -o kml -F ./logs/track.kml

# 3) Open in Google Earth
```

---

## Notes

- Keep logs in UTC for easier merging.  
- Always record sampling rate and IF settings alongside data.  
- For long surveys, split logs into smaller files to simplify processing.  

---

**Happy tracking!**
