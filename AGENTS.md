# OpenCode Agents for GrowTower

This document defines specialized agents for the GrowTower project - a vertical cannabis cultivation system with ESP32 firmware, Android app, and 3D-printed components.

## Available Agents

### 1. firmware-agent
**Purpose:** ESP32 firmware development and debugging (C++, PlatformIO, BLE)

**Use when:**
- Writing or modifying firmware code in `firmware/`
- Working with BLE protocol and characteristics
- Debugging PWM fan control or light relay logic
- Working with ESP-IDF framework
- Setting up build configurations in `platformio.ini`

**Key knowledge areas:**
- Seeed Studio XIAO ESP32C3
- ESP-IDF framework (not Arduino)
- NimBLE-Arduino library for BLE
- PWM control for fans
- GPIO control for relays
- RTC and NTP time synchronization

**Important project-specific rules:**
- Code and comments MUST be in English only
- No InfluxDB integration (not planned)
- Fan runs continuously with MIN_SPEED as baseline
- Light follows schedule in AUTO mode

---

### 2. android-agent
**Purpose:** Android app development (Kotlin, Jetpack Compose, BLE)

**Use when:**
- Working on the Android app in `android/`
- Implementing BLE communication from Android side
- Creating UI components with Jetpack Compose
- Working with MVVM architecture
- Setting up permissions and BLE scanning

**Key knowledge areas:**
- Kotlin with coroutines and Flow
- Jetpack Compose with Material 3
- Android BLE API
- Hilt dependency injection
- MVVM pattern with ViewModel
- BLE characteristic UUIDs used by GrowTower

**Project BLE UUIDs:**
- Service: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- Light Control: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Fan Control: `82563452-9477-4b78-953e-38ec6f43e592`
- Device Info: `c0b9a304-1234-4567-89ab-cdef0123456b`

---

### 3. hardware-agent
**Purpose:** 3D printing, hardware assembly, and BOM management

**Use when:**
- Working with STL files in `stls/`
- Updating BOM.md with parts
- Discussing print settings (PETG, infill patterns)
- Assembly instructions and hardware recommendations
- Mechanical design improvements

**Key knowledge areas:**
- 3D printing with PETG
- Internal Sieve printing (30% rectilinear infill)
- Arctic P14 fan specifications
- LED strip selection (Auxmer 2835, 4000K)
- Carbon filter assembly

---

### 4. protocol-agent
**Purpose:** BLE protocol definition and API design

**Use when:**
- Adding new BLE characteristics
- Modifying existing protocol between firmware and app
- Designing data formats for device communication
- Documenting UUID assignments
- Ensuring compatibility between firmware and app

**Protocol documentation location:**
- Firmware: `firmware/README.md` (BLE Commands section)
- Android: `android/README.md` (BLE Protocol section)

**Important:** Always update BOTH documentation files when changing the protocol!

---

## Agent Selection Guide

| Task | Recommended Agent |
|------|------------------|
| PWM fan logic not working | `firmware-agent` |
| BLE connection failing | `android-agent` or `firmware-agent` |
| New BLE characteristic needed | `protocol-agent` |
| UI redesign | `android-agent` |
| 3D model modifications | `hardware-agent` |
| Adding sensor support | `protocol-agent` + `firmware-agent` |
| Build failures | Context-dependent |

## Common Workflows

### Adding a New Feature (e.g., Temperature Sensor)
1. `protocol-agent`: Design BLE characteristic for temperature data
2. `firmware-agent`: Implement sensor reading and BLE notification
3. `android-agent`: Add UI to display temperature
4. `protocol-agent`: Update both README files with new protocol

### Debugging BLE Issues
1. Check `firmware-agent` for ESP32-side implementation
2. Check `android-agent` for Android-side implementation
3. Use `protocol-agent` to verify UUIDs match on both sides

### Build Commands

**Firmware:**
```powershell
cd firmware
pio run              # Build
pio run -t upload    # Flash
pio device monitor   # Serial monitor
```

**Android:**
```powershell
cd android
.\gradlew assembleDebug    # Debug build
.\gradlew assembleRelease  # Release build
```

## Notes

- All agents should follow the coding standards from GEMINI.md
- Firmware code and comments must be in English
- Temporary debug files should be cleaned up after use
- When modifying one side of the BLE protocol, always check the other side
