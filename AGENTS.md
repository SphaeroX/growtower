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
├── firmware/              # ESP32 firmware
│   ├── src/main.cpp       # Main firmware code
│   ├── platformio.ini     # PlatformIO configuration
│   ├── lib/GrowTowerBLE/  # Custom BLE library
│   └── .env               # WiFi credentials (gitignored)
└── web-app/               # BLE web controller
    ├── index.html         # Single-page web interface
    └── js/modules/        # JS modules

```

## Technology Stack

### Firmware
- **Platform**: ESP32-C3 (Seeed Studio XIAO)
- **Framework**: Arduino (PlatformIO)
- **Communication**: BLE (Bluetooth Low Energy)
- **Features**: PWM fan control, relay light control, OTA updates

### Web Application
- **Type**: Single-page HTML/JS application
- **Protocol**: Web Bluetooth API
- **Language**: English UI
- **Features**: Real-time device control, logging, scheduling

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

### Web Application
- No build required - serve `index.html` directly
- Requires HTTPS or localhost for Web Bluetooth
- Use Chrome or Edge browser

## Key Hardware Pins
- **Fan PWM**: GPIO 3
- **Light Relay**: GPIO 2

## BLE Service UUID
`4fafc201-1fb5-459e-8fcc-c5c9c331914b`

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
- BLE UUIDs in `GrowTowerBLE.h`

### Web App (JavaScript)
- ES6+ class-based architecture
- Async/await for BLE operations
- English language for UI strings
- CSS variables for theming

## Testing
- Firmware: Use serial monitor commands (ON, OFF, FAN 0-100, TIME)
- Web App: Connect via Chrome/Edge, check browser console for logs

## Git Ignore Patterns
- `firmware/.env` - WiFi credentials
- `firmware/include/secrets.h` - Generated credentials
- `.vscode/` - Editor settings

## Common Issues

1. **BLE Connection Fails**: Remove "TOWER" from Windows Bluetooth settings first
2. **Upload Fails**: Check COM port, try manual bootloader mode
3. **Web Bluetooth Not Working**: Use Chrome/Edge, ensure HTTPS or localhost
