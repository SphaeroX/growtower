---
name: flash-firmware
description: Build and upload ESP32 firmware to the XIAO ESP32C3 hardware. Supports automatic port detection and manual bootloader mode.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  platform: esp32
  hardware: xiao-esp32c3
  action: deploy
---

## When to Use

- Deploying firmware changes to the hardware
- First-time device setup
- Recovering from firmware issues
- Testing on physical device

## Prerequisites

- ESP32 connected via USB
- Correct COM port (auto-detected or specify manually)
- Device in bootloader mode if needed (hold BOOT, press RESET, release BOOT)
- Firmware successfully built

## Instructions

1. Connect ESP32 via USB
2. Optional: Enter bootloader mode if first flash
3. Run upload command
4. Wait for upload to complete
5. Device will reboot automatically

## Commands

**Standard upload:**
```powershell
cd firmware
pio run -t upload
```

**Upload with monitoring:**
```powershell
cd firmware
pio run -t upload -t monitor
```

**Specify COM port:**
```powershell
pio run -t upload --upload-port COM5
```

## Troubleshooting

- If upload fails, enter bootloader mode manually:
  1. Hold BOOT button on XIAO
  2. Press and release RESET button
  3. Release BOOT button
  4. Try uploading again
- Check COM port with `pio device list`
- Verify USB cable supports data transfer
- Close serial monitors before uploading
