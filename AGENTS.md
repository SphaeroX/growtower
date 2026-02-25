# AGENTS.md - GrowTower Project

## Project Overview
GrowTower is a minimalist vertical grow system for home cannabis cultivation with IoT control capabilities.

## Project Structure

```
growtower/
├── README.md              # Project documentation
├── BOM.md                 # Bill of Materials
├── AGENTS.md              # This file
├── images/                # Project photos
├── stls/                  # 3D printable STL files
└── firmware/              # ESP32 firmware
    ├── src/               # Source code
    │   ├── main.cpp       # Main firmware code
    │   ├── frontend.h     # Web interface (embedded)
    │   ├── webserver.h    # Web server routes
    │   ├── state.h        # State management
    │   └── config.h       # Configuration
    ├── platformio.ini     # PlatformIO configuration
    └── .env               # WiFi credentials (gitignored)

```

## Technology Stack

### Firmware
- **Platform**: ESP32-C3 (Seeed Studio XIAO)
- **Framework**: Arduino (PlatformIO)
- **Communication**: WiFi (Web server)
- **Features**: PWM fan control, relay light control, OTA updates
- **Web Interface**: Embedded in `src/frontend.h`, served directly by the ESP32

## Build Commands

### Firmware (in `/firmware` directory)
```bash
# Build
pio run

# Upload to device
pio run -t upload

# Upload and monitor
pio run -t upload -t monitor

# Serial monitor
pio device monitor

# OTA upload
pio run -t upload --upload-port growtower.local
```

## Key Hardware Pins
- **Fan PWM**: GPIO 3
- **Light Relay**: GPIO 2

## Development Notes

1. **WiFi Credentials**: Create `firmware/.env` file with:
   ```ini
   WIFI_SSID="YourNetwork"
   WIFI_PASS="YourPassword"
   ```

2. **First Upload**: May require manual bootloader mode:
   - Hold BOOT button
   - Press and release RESET
   - Release BOOT button
   - Then upload

3. **OTA Issues**: If mDNS doesn't resolve, use IP address instead

## Code Conventions

### Firmware (C++)
- Use `camelCase` for functions and variables
- Use `ALL_CAPS` for constants and defines
- Pin definitions in `main.cpp`

## Testing
- Firmware: Use serial monitor commands (ON, OFF, FAN 0-100, LIGHTON, LIGHTTIME, TIME)
- Web Interface: Open http://growtower.local in browser

## Git Ignore Patterns
- `firmware/.env` - WiFi credentials
- `firmware/include/secrets.h` - Generated credentials
- `.vscode/` - Editor settings

## Common Issues

1. **Upload Fails**: Check COM port, try manual bootloader mode
2. **Web Interface Not Loading**: Check WiFi connection, verify IP address
