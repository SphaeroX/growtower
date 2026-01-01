#ifndef BLE_APP_H
#define BLE_APP_H

#include <Arduino.h>
#include <functional>

// Control Callbacks
// Light callback: true = ON, false = OFF
typedef std::function<void(bool)> LightCallback;
// Fan callback: 0-100 percent
typedef std::function<void(int)> FanCallback;
// Int callback for config (Min/Max/Hrs)
typedef std::function<void(int)> IntCallback;

// Initialize BLE with all callbacks
void initBLE(LightCallback lightCb, FanCallback fanCb, IntCallback fanMinCb,
             IntCallback fanMaxCb, IntCallback lightOnCb,
             IntCallback lightOffCb);

#endif
