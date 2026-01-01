# GrowTower Firmware

This directory contains the ESP-IDF based firmware for the GrowTower, designed for the **Seeed Studio XIAO ESP32C3** (configured as `seeed_xiao_esp32c3` in PlatformIO).

## Prerequisites

- **VS Code** with **PlatformIO** extension installed.
- OR **PlatformIO Core** (CLI) installed.

## Configuration
Settings are managed via a local `.env` file (which is ensuring credentials are not committed to git) and `src/main.cpp`.

### 1. Credentials (.env)
You **MUST** create a file named `.env` in the `firmware/` directory. This file is used to generate `secrets.h` before every build.

**Example `firmware/.env` content:**
```ini
WIFI_SSID="YourWiFiName"
WIFI_PASS="YourSuperSecretPassword"
```
*Note: This file is ignored by git.*

### 2. Pin Definitions
Pin definitions and other logic are located in `src/main.cpp`.

By default:
- **Fan PWM**: GPIO 3
- **Light Relay**: GPIO 2

## Command Line Instructions

Open your terminal (PowerShell or CMD) in this directory (`growtower/firmware`).

### 1. Build
Compiles the firmware. The first run will download the ESP-IDF framework (may take a while).

```powershell
pio run
```

### 2. Flash (Upload)
Builds and uploads the firmware to the connected ESP32-C6.

```powershell
pio run -t upload
```

*Note: If the upload fails, ensure the device is in bootloader mode (hold BOOT, press RESET, release BOOT).*

### 3. Monitor (Debug)
Opens the serial monitor to see logs and send commands.

```powershell
pio device monitor
```
*Note: Exit the monitor with `Ctrl+C`.*

## Serial Console Commands

Once the monitor is running, you can type these commands and press Enter:

| Command | Description | Example |
| :--- | :--- | :--- |
| `fan <0-100>` | Set fan speed in percent. | `fan 50` |
| `light on` | Turn grow light ON manually. | `light on` |
| `light off` | Turn grow light OFF manually. | `light off` |
| `auto` | Resume automatic light schedule. | `auto` |
| `status` | Show current time, fan speed, & WiFi status. | `status` |

## Troubleshooting


- **"Board doesn't support arduino framework":** We are using `framework = espidf` in `platformio.ini` to avoid this. Ensure you are not using an old Arduino configuration.
- **Upload fails:** Check if the COM port is correct. You can specify the port with `--upload-port COMx` (e.g., `pio run -t upload --upload-port COM5`).
- **Initial Flash / Connection Issues:** If the upload hangs or the port is not found, entering **Bootloader Mode** manually is often required for the first flash. Process:
  1. Hold the `BOOT` button on the Xiao.
  2. Press and release the `RESET` button.
  3. Release the `BOOT` button.
  4. Try uploading again.
