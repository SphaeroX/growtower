#ifndef GROWTOWER_BLE_H
#define GROWTOWER_BLE_H

#include <Arduino.h>

// Callback function types
typedef void (*LightCallback)(bool on);
typedef void (*FanCallback)(int percent);
typedef void (*ConfigCallback)(int value);

// Initialize BLE
void growtower_ble_init(
    LightCallback light_cb,
    FanCallback fan_cb,
    ConfigCallback fan_min_cb,
    ConfigCallback fan_max_cb,
    ConfigCallback light_on_cb,
    ConfigCallback light_off_cb,
    bool initial_light_on,
    int initial_fan_speed,
    int initial_fan_min,
    int initial_fan_max,
    int initial_light_on_hour,
    int initial_light_off_hour
);

// Update characteristic values (for when hardware state changes)
void growtower_ble_update_light(bool on);
void growtower_ble_update_fan(int speed);
void growtower_ble_update_fan_min(int min);
void growtower_ble_update_fan_max(int max);
void growtower_ble_update_light_on_hour(int hour);
void growtower_ble_update_light_off_hour(int hour);

// Check connection status
bool growtower_ble_is_connected(void);
int growtower_ble_get_connected_count(void);

// Restart advertising (useful after disconnect)
void growtower_ble_restart_advertising(void);

#endif // GROWTOWER_BLE_H