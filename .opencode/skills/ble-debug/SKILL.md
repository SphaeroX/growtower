---
name: ble-debug
description: Debug Bluetooth Low Energy communication issues between GrowTower firmware and Android app. Verify UUIDs, test connections, and diagnose protocol mismatches.
license: MIT
compatibility: opencode
metadata:
  project: growtower
  protocol: ble
  components: firmware,android
  action: debug
---

## When to Use

- BLE connection fails or drops unexpectedly
- Commands not working or not received
- Verifying UUIDs match between firmware and app
- Android app cannot discover or connect to device
- Data not syncing correctly

## Instructions

1. Check firmware serial output using `monitor-serial` skill
2. Verify UUIDs match between firmware and Android
3. Test with manual serial commands
4. Check Android logs with adb logcat or Android Studio
5. Compare protocol documentation

## BLE UUIDs Reference

**Service UUID:** `4fafc201-1fb5-459e-8fcc-c5c9c331914b`

| Characteristic | UUID | Firmware | Android |
|---------------|------|----------|---------|
| Light Control | `beb5483e-36e1-4688-b7f5-ea07361b26a8` | Read/Write | Read/Write/Notify |
| Fan Control | `82563452-9477-4b78-953e-38ec6f43e592` | Read/Write | Read/Write/Notify |
| Device Info | `c0b9a304-1234-4567-89ab-cdef0123456b` | Read | Read |
| Uptime | `c0b9a304-1234-4567-89ab-cdef0123456c` | Read | Read |
| Factory Reset | `c0b9a304-1234-4567-89ab-cdef0123456d` | Write | Write |

## Common Issues

**UUID mismatch:**
- Compare `firmware/README.md` with `android/README.md`
- Ensure both use exact same UUID strings

**Wrong characteristic properties:**
- Check if property is Read, Write, or Notify as expected
- Firmware and Android must agree on properties

**BLE permissions not granted:**
- Android requires BLUETOOTH_SCAN and BLUETOOTH_CONNECT
- Location permission needed for scanning on Android 11 and below

**Device not advertising:**
- Check firmware serial output for "Advertising started"
- Verify device name is "GrowTower-BLE"
- Ensure firmware is running and not crashed

## Debugging Steps

1. **Check firmware side:**
   - Use `monitor-serial` skill
   - Look for connection events
   - Check if characteristics are created successfully

2. **Check Android side:**
   - Use Android Studio Logcat
   - Look for GATT callbacks
   - Check for permission errors

3. **Test manually:**
   - Use nRF Connect app to test BLE services
   - Verify characteristic values can be read/written
   - Check notification subscriptions work

## Related Skills

- `monitor-serial` - Check firmware output
- `build-android` - Rebuild app if needed
- `flash-firmware` - Update firmware if protocol changed
