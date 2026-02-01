---
name: generate-secrets
description: Generate secrets.h header file from .env configuration for WiFi credentials. Runs automatically during build but can be triggered manually.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  component: firmware
  file: secrets
  action: generate
---

## When to Use

- After modifying `.env` file
- If `secrets.h` is missing or corrupted
- Before building firmware if credentials changed
- First-time setup of firmware project

## Prerequisites

- `.env` file in `firmware/` directory
- Python installed
- `generate_secrets.py` script exists

## Instructions

1. Ensure `.env` file exists with proper format
2. Navigate to firmware directory
3. Run Python script manually (optional)
4. Or let it run automatically during build

## Command

```powershell
cd firmware
python generate_secrets.py
```

## .env File Format

Create `firmware/.env` with this content:

```ini
WIFI_SSID="YourWiFiName"
WIFI_PASS="YourSuperSecretPassword"
```

## Generated Output

Creates `firmware/include/secrets.h`:

```cpp
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "YourWiFiName"
#define WIFI_PASS "YourSuperSecretPassword"

#endif
```

## Security Notes

- `.env` file is gitignored - never commit credentials
- `secrets.h` is also gitignored
- Credentials only exist locally on your machine
- Script runs automatically before each build

## Troubleshooting

- If script fails, check Python is installed
- Verify `.env` syntax (no spaces around =)
- Ensure `.env` is in `firmware/` directory, not root
- Check file permissions allow reading
