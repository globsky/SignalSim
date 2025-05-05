# Detailed GNSS Signal Frequency Planning Calculations

This document provides the detailed mathematical calculations behind the center frequencies and bandwidths for generation of **`IF Signal`** via **`IFDataGen`** package for multiple GNSS signal combinations.

## 1. L1CA + L1C + B1C + E1

### 1.1 Signal Frequencies and Bandwidths

- L1CA: 1575.42 MHz, bandwidth 2.046 MHz (1574.397–1576.443 MHz)
- L1C: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- B1C: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- E1: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)

### 1.2 Frequency Range Analysis

- All four signals share the same center frequency: 1575.42 MHz
- Lowest frequency: L1C/B1C/E1's lower edge = 1573.374 MHz
- Highest frequency: L1C/B1C/E1's upper edge = 1577.466 MHz
- Total frequency span = 1577.466 - 1573.374 = 4.092 MHz

### 1.3 Center Frequency Calculation

- Center frequency = 1575.42 MHz (already perfectly aligned)
- Required bandwidth = 4.092 MHz (to capture all BOC modulated signals)
- No further optimization needed as the signals perfectly overlap at their center frequencies

### 1.4 Sampling Frequency Calculation

- Minimum Fs = 4.092 MHz (theoretical minimum for complex sampling)
- Recommended Fs = 5-6 MHz (to accommodate filter roll-off and implementation margin)
- For better processing gain, Fs = 8-10 MHz can be used

## 2. L1CA + L1C + B1C + B1I + E1

### 2.1 Signal Frequencies and Bandwidths

- L1CA: 1575.42 MHz, bandwidth 2.046 MHz (1574.397–1576.443 MHz)
- L1C: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- B1C: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- E1: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- B1I: 1561.098 MHz, bandwidth 4.092 MHz (1559.052–1563.144 MHz)

### 2.2 Frequency Range Analysis

- Lowest frequency: B1I's lower edge = 1559.052 MHz
- Highest frequency: L1C/B1C/E1's upper edge = 1577.466 MHz
- Total frequency span = 1577.466 - 1559.052 = 18.414 MHz

### 2.3 Center Frequency Calculation

- Midpoint center frequency = (1559.052 + 1577.466) / 2 = 1568.259 MHz ≈ 1568.26 MHz
- Required bandwidth = 18.414 MHz ≈ 18.41 MHz
- Verification:
  - Distance to lower edge: 1568.26 - 1559.052 = 9.208 MHz
  - Distance to upper edge: 1577.466 - 1568.26 = 9.206 MHz
  - Required full bandwidth = 9.208 + 9.206 = 18.414 MHz ≈ 18.41 MHz

### 2.4 Sampling Frequency Calculation

- Minimum Fs = 18.41 MHz (theoretical minimum for complex sampling)
- Recommended Fs = 20-25 MHz (to accommodate filter roll-off and implementation margin)

## 3. L1CA + L1C + B1C + B1I + E1 + G1

### 3.1 Signal Frequencies and Bandwidths

- L1CA: 1575.42 MHz, bandwidth 2.046 MHz (1574.397–1576.443 MHz)
- L1C: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- B1C: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- E1: 1575.42 MHz, bandwidth 4.092 MHz (1573.374–1577.466 MHz)
- B1I: 1561.098 MHz, bandwidth 4.092 MHz (1559.052–1563.144 MHz)
- G1 (GLONASS):
  - Center frequency: ~1602 MHz (varies by satellite channel)
  - Channel formula: 1602 + k × 0.5625 MHz, where k = -7 to +6
  - Bandwidth per channel: ~1 MHz
  - Full band range: 1598.0625 MHz to 1605.375 MHz

### 3.2 Frequency Range Analysis

- Lowest frequency: B1I's lower edge = 1559.052 MHz
- Highest frequency: G1's upper edge = 1605.375 MHz
- Total frequency span = 1605.375 - 1559.052 = 46.323 MHz

### 3.3 Center Frequency Calculation

- Midpoint center frequency = (1559.052 + 1605.375) / 2 = 1582.2135 MHz ≈ 1582.21 MHz
- Required bandwidth = 46.323 MHz ≈ 46.32 MHz
- Verification:
  - Distance to lower edge: 1582.21 - 1559.052 = 23.158 MHz
  - Distance to upper edge: 1605.375 - 1582.21 = 23.165 MHz
  - Required full bandwidth = 23.158 + 23.165 = 46.323 MHz ≈ 46.32 MHz

### 3.4 Sampling Frequency Calculation

- Minimum Fs = 46.32 MHz (theoretical minimum for complex sampling)
- Recommended Fs = 50-60 MHz (to accommodate filter roll-off and implementation margin)
- For practical implementation, Fs = 50 MHz is typically used for this frequency combination

## 4. L2C + B2I + B2b + E5b + G2

### 4.1 Signal Frequencies and Bandwidths

- L2C (GPS): 1227.60 MHz, bandwidth 2.046 MHz (1226.577–1228.623 MHz)
- B2I (BeiDou): 1207.14 MHz, bandwidth 4.092 MHz (1205.094–1209.186 MHz)
- B2b (BeiDou): 1207.14 MHz, bandwidth 20.46 MHz (1196.91–1217.37 MHz)
- E5b (Galileo): 1207.14 MHz, bandwidth 20.46 MHz (1196.91–1217.37 MHz)
- G2 (GLONASS):
  - Center frequency: ~1246 MHz (varies by satellite channel)
  - Channel formula: 1246 + k × 0.4375 MHz, where k = -7 to +6
  - Bandwidth per channel: ~0.8 MHz
  - Full band range: 1242.9375 MHz to 1248.625 MHz

### 4.2 Frequency Range Analysis

- Lowest frequency: B2b/E5b's lower edge = 1196.91 MHz
- Highest frequency: G2's upper edge = 1248.625 MHz
- Total frequency span = 1248.625 - 1196.91 = 51.715 MHz

### 4.3 Center Frequency Calculation

- Midpoint center frequency = (1196.91 + 1248.625) / 2 = 1222.7675 MHz ≈ 1222.77 MHz
- Required bandwidth = 51.715 MHz ≈ 51.72 MHz
- Verification:
  - Distance to lower edge: 1222.77 - 1196.91 = 25.86 MHz
  - Distance to upper edge: 1248.625 - 1222.77 = 25.855 MHz
  - Required full bandwidth = 25.86 + 25.855 = 51.715 MHz ≈ 51.72 MHz

### 4.4 Sampling Frequency Calculation

- Minimum Fs = 51.72 MHz (theoretical minimum for complex sampling)
- Recommended Fs = 55-60 MHz (to accommodate filter roll-off and implementation margin)

## 5. L5 + B2a + E5a

### 5.1 Signal Frequencies and Bandwidths

- L5 (GPS): 1176.45 MHz, bandwidth 20.46 MHz (1166.22–1186.68 MHz)
- B2a (BeiDou): 1176.45 MHz, bandwidth 20.46 MHz (1166.22–1186.68 MHz)
- E5a (Galileo): 1176.45 MHz, bandwidth 20.46 MHz (1166.22–1186.68 MHz)

### 5.2 Frequency Range Analysis

- All three signals have identical center frequencies and bandwidths
- Lowest frequency: 1166.22 MHz (shared by all signals)
- Highest frequency: 1186.68 MHz (shared by all signals)
- Total frequency span = 1186.68 - 1166.22 = 20.46 MHz

### 5.3 Center Frequency Calculation

- Center frequency = 1176.45 MHz (already perfectly aligned)
- Required bandwidth = 20.46 MHz
- No further optimization needed as the signals perfectly overlap

### 5.4 Sampling Frequency Calculation

- Minimum Fs = 20.46 MHz (theoretical minimum for complex sampling)
- Recommended Fs = 25-30 MHz (to accommodate filter roll-off and implementation margin)

## 6. B3I + E6

### 6.1 Signal Frequencies and Bandwidths

- B3I (BeiDou): 1268.52 MHz, bandwidth 20.46 MHz (1258.29–1278.75 MHz)
- E6 (Galileo): 1278.75 MHz, bandwidth 40.92 MHz (1258.29–1299.21 MHz)

### 6.2 Frequency Range Analysis

- Lowest frequency: B3I/E6 lower edge = 1258.29 MHz (they share the same lower edge)
- Highest frequency: E6 upper edge = 1299.21 MHz
- Total frequency span = 1299.21 - 1258.29 = 40.92 MHz

### 6.3 Center Frequency Calculation

- Midpoint center frequency = (1258.29 + 1299.21) / 2 = 1278.75 MHz
- Required bandwidth = 40.92 MHz
- Verification:
  - Distance to lower edge: 1278.75 - 1258.29 = 20.46 MHz
  - Distance to upper edge: 1299.21 - 1278.75 = 20.46 MHz
  - Required full bandwidth = 20.46 + 20.46 = 40.92 MHz

### 6.4 Sampling Frequency Calculation

- Minimum Fs = 40.92 MHz (theoretical minimum for complex sampling)
- Recommended Fs = 45-50 MHz (to accommodate filter roll-off and implementation margin)
