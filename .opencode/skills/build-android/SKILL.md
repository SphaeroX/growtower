---
name: build-android
description: Build the GrowTower Android app using Gradle. Creates debug or release APK with Kotlin, Jetpack Compose, and BLE support.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  platform: android
  language: kotlin
  build-tool: gradle
---

## When to Use

- After making code changes to the Android app
- Creating APK for installation on device
- Continuous integration builds
- Testing app functionality

## Prerequisites

- `local.properties` file in `android/` with `sdk.dir` path
- JDK installed with `JAVA_HOME` environment variable set
- Android SDK installed

## Instructions

1. Ensure local.properties exists with valid SDK path
2. Navigate to android directory
3. Run Gradle build command
4. Locate output APK
5. Install on Android device

## Commands

**Debug build:**
```powershell
cd android
.\gradlew assembleDebug
```

**Release build:**
```powershell
cd android
.\gradlew assembleRelease
```

**With debug info:**
```powershell
.\gradlew assembleDebug --stacktrace
```

## Output Location

- Debug: `app/build/outputs/apk/debug/app-debug.apk`
- Release: `app/build/outputs/apk/release/app-release.apk`

## Prerequisites Details

**local.properties format:**
```properties
sdk.dir=C:\\Users\\<username>\\AppData\\Local\\Android\\Sdk
```

**Environment:**
- JAVA_HOME must point to valid JDK
- Android Studio usually includes compatible JDK

## Troubleshooting

- If build fails, check SDK path in local.properties
- Run `clean-build` skill if stale artifacts
- Use `--stacktrace` for detailed error output
- Ensure minimum Android SDK API 26+ for BLE support
