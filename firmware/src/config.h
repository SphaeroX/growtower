#ifndef CONFIG_H
#define CONFIG_H

#define LIGHT_PIN D1
#define FAN_PIN D2

const int PWM_FREQUENCY = 25000;
const int PWM_RESOLUTION = 8;
const int PWM_CHANNEL = 0;
const int MAX_DUTY_CYCLE = 255;
const int HARDWARE_FAN_MIN_DUTY = 13;

const char *OTA_PASSWORD = "growtower123";
const uint16_t OTA_PORT = 3232;

const char *DEFAULT_HOSTNAME = "growtower";

const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 3600;
const int DAYLIGHT_OFFSET_SEC = 3600;

#endif
