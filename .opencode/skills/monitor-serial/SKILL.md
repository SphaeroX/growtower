---
name: monitor-serial
description: Open serial monitor for real-time debugging of ESP32 firmware. Displays log output and allows interactive command input.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  platform: esp32
  action: debug
  tool: serial-monitor
---

## When to Use

- Debugging firmware issues
- Testing serial commands interactively
- Monitoring runtime behavior and logs
- Verifying BLE connections
- Checking system status

## Prerequisites

- ESP32 connected via USB
- Firmware running on device
- Serial port available (not in use by other tools)

## Instructions

1. Connect ESP32 via USB
2. Run monitor command
3. View real-time logs
4. Type commands and press Enter
5. Exit when done

## Command

```powershell
cd firmware
pio device monitor
```

## Available Serial Commands

| Command | Description | Example |
|---------|-------------|---------|
| `ON` | Turn light ON | `ON` |
| `OFF` | Turn light OFF | `OFF` |
| `FAN <0-100>` | Set fan speed percent | `FAN 50` |
| `TIME` | Print current time from NTP | `TIME` |

## Exit

Press `Ctrl+C` to exit the monitor

## Tips

- Use with `flash-firmware` skill: `pio run -t upload -t monitor`
- Logs show BLE connections, PWM changes, and relay states
- Monitor helps verify BLE characteristic changes
