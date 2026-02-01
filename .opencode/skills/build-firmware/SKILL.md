---
name: build-firmware
description: Build the ESP32 firmware using PlatformIO for the GrowTower project. Compiles C++ code for XIAO ESP32C3 with BLE and PWM support.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  platform: esp32
  framework: platformio
  language: cpp
---

## When to Use

- After making code changes to the firmware
- To check for compilation errors
- Before flashing the device
- Continuous integration checks

## Prerequisites

- PlatformIO installed (VS Code extension or CLI)
- `.env` file with WiFi credentials in `firmware/` directory
- ESP-IDF framework dependencies downloaded

## Instructions

1. Navigate to firmware directory
2. Run PlatformIO build command
3. Review output for errors
4. Address any compilation issues

## Command

```powershell
cd firmware
pio run
```

## Expected Output

- Successful build shows `[SUCCESS]` message
- Firmware binary created in `.pio/build/main/firmware.bin`
- No compilation errors or warnings

## Troubleshooting

- If build fails, check `.env` file exists with proper format
- Clean build with `pio run -t clean` if stale artifacts
- Ensure PlatformIO Core is up to date
