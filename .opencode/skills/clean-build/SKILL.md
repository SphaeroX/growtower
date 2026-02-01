---
name: clean-build
description: Clean build artifacts for firmware or Android to resolve stubborn build issues and ensure fresh builds
license: MIT
compatibility: opencode
metadata:
  project: growtower
  action: maintenance
  platforms: esp32,android
---

## When to Use

- Resolving stubborn build issues
- Ensuring completely fresh build
- After changing build configuration
- When builds behave unexpectedly
- After switching branches with different dependencies

## Instructions

1. Navigate to the appropriate project directory
2. Run clean command for your platform
3. Rebuild from scratch

## Commands

**Firmware clean:**
```powershell
cd firmware
pio run -t clean
```

**Android clean:**
```powershell
cd android
.\gradlew clean
```

**Both platforms:**
```powershell
cd firmware && pio run -t clean
cd ..\android && .\gradlew clean
```

## Next Steps

After cleaning, rebuild with:
- Firmware: `build-firmware` skill
- Android: `build-android` skill

## Notes

- Cleaning removes all build artifacts including cached dependencies
- First build after clean will take longer
- Does not affect source files or configuration
