# GrowTower Firmware

This directory contains the ESP32 Arduino-based firmware for the GrowTower, designed for the **Seeed Studio XIAO ESP32C3** (configured as `seeed_xiao_esp32c3` in PlatformIO).

## Features

- **Web-based Control**: Access the controller via any web browser at `http://growtower.local`
- **REST API**: HTTP endpoints for all control functions
- **mDNS Support**: Automatic discovery via `*.local` domain (configurable hostname)
- **Real-time Status**: Live updates of system state in the web interface
- **OTA Updates**: Over-the-air firmware updates via PlatformIO
- **Automatic Timer**: Light scheduling with configurable on/off hours
- **Fan Control**: PWM-based speed control with configurable min/max range
- **Persistent Settings**: All configuration stored in flash memory

## Prerequisites

- **VS Code** with **PlatformIO** extension installed.
- OR **PlatformIO Core** (CLI) installed.

## Configuration

Settings are managed via a local `.env` file (ensuring credentials are not committed to git).

### 1. WiFi Credentials (.env)

You **MUST** create a file named `.env` in the `firmware/` directory. This file is used to generate `secrets.h` before every build.

**Example `firmware/.env` content:**
```ini
WIFI_SSID="YourWiFiName"
WIFI_PASS="YourSuperSecretPassword"
```
*Note: This file is ignored by git.*

### 2. Pin Definitions

Pin definitions are located in `src/main.cpp`.

By default:
- **Light Relay**: GPIO 1 (D1)
- **Fan PWM**: GPIO 4 (D2)

### 3. Hostname Configuration

The default hostname is `growtower`. You can change it via:
- **Web Interface**: Settings → Device Name
- **Serial Command**: `HOST <newname>`

After changing, the device will restart and be available at `http://<newname>.local`

## Building and Flashing

Open your terminal in this directory (`growtower/firmware`).

### 1. Build

Compiles the firmware:

```powershell
pio run
```

### 2. Flash (USB)

Uploads firmware via USB:

```powershell
pio run -t upload
```

*Note: If the upload fails, ensure the device is in bootloader mode (hold BOOT, press RESET, release BOOT).*

### 3. Flash (OTA - Over The Air)

Uploads firmware wirelessly (after initial USB flash):

```powershell
pio run -t upload -e main_ota
```

Or specify the hostname/IP:
```powershell
pio run -t upload --upload-port growtower.local
```

### 4. Monitor

Opens the serial monitor for debugging:

```powershell
pio device monitor
```
*Note: Exit with `Ctrl+C`.*

## Web Interface

Once connected to WiFi, open your browser and navigate to:

```
http://growtower.local
```

### Features

- **Dashboard**: Real-time status of light, fan, timer settings
- **Light Control**: Turn light ON/OFF manually
- **Fan Control**: Adjust speed (0-100%) and set min/max range
- **Timer Settings**: Configure automatic light schedule (24h format)
- **Network Settings**: Change device hostname (requires restart)

The interface automatically updates every 2 seconds and is optimized for both desktop and mobile devices.

## REST API

All functions are accessible via HTTP GET requests:

| Endpoint | Parameters | Description |
|----------|------------|-------------|
| `GET /api/status` | - | Returns JSON with all current values |
| `GET /api/light` | `state=0\|1` | Turn light OFF (0) or ON (1) |
| `GET /api/fan` | `speed=0-100` | Set fan speed percentage |
| `GET /api/fanrange` | `min=0-100&max=0-100` | Set fan min/max range |
| `GET /api/timer` | `on=0-23&off=0-23` | Set light timer hours |
| `GET /api/hostname` | `name=<hostname>` | Change hostname (reboots) |

### Example API Responses

**Status Request:**
```bash
curl http://growtower.local/api/status
```

**Response:**
```json
{
  "light": true,
  "fan": 30,
  "fanMin": 0,
  "fanMax": 100,
  "lightOn": 18,
  "lightOff": 14,
  "hostname": "growtower",
  "ip": "192.168.1.100",
  "wifiConnected": true,
  "hasTime": true,
  "currentTime": "14:30:25"
}
```

## Serial Console Commands

You can also control the device via serial monitor:

| Command | Description | Example |
|---------|-------------|---------|
| `ON` | Turn light ON | `ON` |
| `OFF` | Turn light OFF | `OFF` |
| `FAN <0-100>` | Set fan speed | `FAN 50` |
| `FANMIN <0-100>` | Set minimum fan speed | `FANMIN 10` |
| `FANMAX <0-100>` | Set maximum fan speed | `FANMAX 90` |
| `LIGHTON <0-23>` | Set light ON hour | `LIGHTON 18` |
| `LIGHTOFF <0-23>` | Set light OFF hour | `LIGHTOFF 14` |
| `HOST <name>` | Set device hostname | `HOST mytower` |
| `TIME` | Show current time | `TIME` |
| `STATUS` | Show full status | `STATUS` |
| `RESET` | Reset all settings | `RESET` |
| `HELP` | Show command list | `HELP` |

## Default Settings

After first flash or reset:

- **Light**: OFF
- **Fan Speed**: 30%
- **Fan Range**: 0% - 100%
- **Light Timer**: 18:00 ON → 14:00 OFF
- **Hostname**: growtower

## Troubleshooting

### Cannot access growtower.local

1. Ensure your device is on the same network as the ESP32
2. Try accessing via IP address (check serial monitor for IP)
3. On Windows: Install Bonjour Print Services or iTunes (for mDNS support)
4. On Linux: Ensure `avahi-daemon` is running
5. Some routers may block mDNS - use IP address instead

### Web interface not loading

1. Check WiFi connection in serial monitor
2. Verify the ESP32 got an IP address
3. Try clearing browser cache
4. Check firewall settings

### Upload fails

1. **USB Upload**: Check COM port with `pio device list`
2. **OTA Upload**: Ensure device is online and hostname resolves
3. **Bootloader Mode**: For first flash:
   - Hold `BOOT` button
   - Press and release `RESET`
   - Release `BOOT`
   - Try uploading again

### Settings not persisting

Ensure the ESP32 has enough flash space and isn't being reset during write operations.

## Technical Details

- **Framework**: Arduino (PlatformIO)
- **Platform**: ESP32-C3 (Seeed Studio XIAO)
- **Web Server**: ESPAsyncWebServer
- **mDNS**: ESPmDNS
- **Storage**: Preferences (NVS)
- **OTA**: ArduinoOTA

## Project Structure

```
firmware/
├── src/
│   └── main.cpp          # Main firmware code
├── include/
│   └── secrets.h         # Auto-generated from .env
├── platformio.ini        # PlatformIO configuration
├── .env                  # Your WiFi credentials (gitignored)
└── README.md            # This file
```
