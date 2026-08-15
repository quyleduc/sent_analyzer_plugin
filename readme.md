# Saleae Logic 2 SENT (SAE J2716) Analyzer

[![CI Build](https://github.com/quyleduc/sent_analyzer_plugin/actions/workflows/build.yml/badge.svg)](https://github.com/quyleduc/sent_analyzer_plugin/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Saleae Logic 2](https://img.shields.io/badge/Saleae%20Logic-2.4%2B-green.svg)](https://www.saleae.com/)

A high-performance C++ Low-Level Analyzer (LLA) plugin for **Saleae Logic 2** that decodes the **SAE J2716 SENT (Single Edge Nibble Transmission)** automotive sensor protocol with full support for **SAE J2716 Sensor Profiles**.

---

## ⚡ Key Features

- **SAE J2716 Sensor Profiles**: Decode raw nibbles directly into physical engineering units (°C, kPa, kg/h, %, Rolling Counter).
- **Data Table View**: Complete frame summaries directly in Saleae Logic 2 Data Table (`Profile`, `Status`, `Physical Channel 1`, `Physical Channel 2`, `CRC`, `Pause`, `CRC Check`).
- **Adaptive Bubble Text**: Dynamic multi-tier labels scaling with zoom level (`SYNC (56T)`, `Status: 0x0`, `D0: 4` ... `CRC: 0xA [PASS]`, `Pause: 110T`).
- **SAE J2716 Standard CRC-4**:
  - **Legacy Mode**: 6 data nibbles, seed = 5 (`CRC4_TABLE[crc ^ data]`).
  - **APR2016 Recommended Mode**: 7 nibbles (Status + 6 data nibbles), seed = 3.
  - Distinct visual error markers on CRC mismatch.
- **Pause Pulse Support**: Automatic detection and measurement of fixed frame period pause pulses ($\ge 12\text{ ticks}$).
- **Dynamic Clock Calibration**: Automatic tick-time tracking from each 56-tick Sync pulse with a $\pm 20\%$ drift tolerance window.
- **Cross-Platform**: Windows (x86_64, ARM64), macOS (Intel & Apple Silicon Universal), and Linux (x86_64).

---

## 🏎️ Supported SAE J2716 Sensor Profiles

| Profile | Target Application | Channel 1 Decoding | Channel 2 Decoding | Safety / Diagnostic Checks |
|---|---|---|---|---|
| **Raw Fast Channels** | Generic / Custom | Fast 1 (12-bit raw) | Fast 2 (12-bit raw) | Raw hex & decimal display |
| **A.1: Dual Throttle (TPS)** | Electronic Throttle / Pedal | Throttle 1 ($0.0 - 100.0\%$) | Throttle 2 ($0.0 - 100.0\%$) | Redundancy cross-check ($\text{TPS}_1 + \text{TPS}_2 \approx 4095$) |
| **A.2: TMAP** | Manifold Air Pressure & Temp | Pressure ($20.0 - 300.0\text{ kPa}$) | Temp ($-40.0 - +150.0\text{ }^\circ\text{C}$) | Physical range validation |
| **A.3: Mass Air Flow (MAF)**| Intake Air Flow & Temp | Flow ($14\text{-bit}, 0 - 640\text{ kg/h}$) | Temp ($10\text{-bit}, -40 - +120\text{ }^\circ\text{C}$)| Non-uniform bit slicing ($14\text{b} + 10\text{b}$) |
| **A.4: Secure Sensor** | Steering Angle / Safety | Signal (12-bit) | Rolling Counter ($8\text{-bit}, 0 - 255$)| Inverted nibble integrity check ($\text{D}_0 \oplus \text{D}_5 = 0\text{xF}$) |
| **A.5: Single High Res** | High-Precision Sensor | High-Res Signal ($16\text{-bit}$) | Diagnostic ($8\text{-bit}$) | 4-nibble aggregation |

---

## 📐 Frame Structure

```text
+-----------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+-------------+
| SYNC(56T) | Status (12..27)| Fast D0 (0..F) | Fast D1 (0..F) | Fast D2 (0..F) | Fast D3 (0..F) | Fast D4 (0..F) | Fast D5 (0..F) | CRC4 (0..F) | Pause (opt) |
+-----------+----------------+----------------+----------------+----------------+----------------+----------------+----------------+-------------+
|<------------------------------------------ Fast Channel 1 (12-bit) ---------->|<---------------- Fast Channel 2 (12-bit) ------>|
```

---

## 📥 Installation

1. Download the pre-built library from [Releases](https://github.com/quyleduc/sent_analyzer_plugin/releases) for your OS:
   - **Windows**: `SENTAnalyzer.dll`
   - **Linux**: `libSENTAnalyzer.so`
   - **macOS**: `libSENTAnalyzer.dylib`
2. Copy the file into Saleae Logic 2 custom analyzer folder:
   - **Windows**: `C:\Program Files\Logic\resources\windows-x64\Analyzers\`
   - **Linux**: `~/.local/share/Saleae/Analyzers/`
   - **macOS**: `/Applications/Logic.app/Contents/Resources/Analyzers/`
3. Restart **Saleae Logic 2** $\rightarrow$ Add **`SENT (SAE J2716)`** from the Analyzers panel.

---

## ⚙️ Configuration Parameters

| Parameter | Type | Default | Description |
|---|---|---|---|
| **Serial** | Channel | Channel 0 | Digital input channel for the SENT signal. |
| **tick time (half us)** | Integer | `6` ($3.0\mu s$) | Nominal SENT tick period in half-microseconds ($6 \times 0.5\mu s = 3.0\mu s$). |
| **Pause pulse** | Bool | `True` | Enable fixed-period frame mode with pause pulse. |
| **Number of data nibbles**| Integer | `6` | Number of fast data nibbles per frame (e.g. 6 nibbles = two 12-bit signals). |
| **Legacy CRC** | Bool | `True` | `True`: Legacy CRC (6 nibbles, seed=5). `False`: APR2016 (7 nibbles, seed=3). |
| **Sensor Profile** | Choice | `Raw (12b+12b)` | Choose SAE J2716 sensor profile for physical unit conversion. |

---

## 🛠️ Build from Source

### Prerequisites
- CMake 3.13+
- C++11 compiler:
  - **Windows**: Visual Studio 2019/2022 (MSVC x64)
  - **Linux**: `gcc` / `g++` (`sudo apt install build-essential cmake`)
  - **macOS**: Xcode Command Line Tools

### Build Commands

```bash
# Clone the repository
git clone https://github.com/quyleduc/sent_analyzer_plugin.git
cd sent_analyzer_plugin

# Build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Automated helper scripts:
- **Windows**: `.\build.ps1`
- **Linux / macOS**: `./build.sh`

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
