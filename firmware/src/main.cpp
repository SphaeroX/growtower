#include "secrets.h"
#include <Arduino.h>
#include <BleApp.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>


// --- Configuration ---
// Define the pin for the light control (PWM)
// Please check your actual wiring. D0 is a placeholder provided in the plan.
// On Seeed XIAO ESP32S3, D0 is GPIO1 (usually).
#define LIGHT_PIN D1
// Fan connected to GPIO4 (D2)
#define FAN_PIN D2

// NTP Server settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;     // GMT+1
const int daylightOffset_sec = 3600; // Summer time +1hr

// PWM Channel configuration (for Fan)
const int pwmFreq =
    25000; // Standard for PC fans usually, or 5000 if generic driver
const int pwmResolution = 8;
const int maxDutyCycle = 255;

// Preferences
Preferences preferences;

// Configurable Variables (with defaults)
int fanMinPercent = 0;
int fanMaxPercent = 100;
int lightOnHour = 18;
int lightOffHour = 14;

// Internal state
// int currentBrightnessPercent = 0; // Removed
bool isLightOn = false; // Tracks the current state of the light

// Forward declarations of setters for BLE
void setFanMin(int minVal);
void setFanMax(int maxVal);
void setLightOnTime(int hour);
void setLightOffTime(int hour);

void loadSettings() {
  preferences.begin("growtower", true); // Read-only mode
  // If keys don't exist, defaults (0, 100, 18, 14) are used
  fanMinPercent = preferences.getInt("fanMin", 0);
  fanMaxPercent = preferences.getInt("fanMax", 100);
  lightOnHour = preferences.getInt("onHour", 18);
  lightOffHour = preferences.getInt("offHour", 14);
  preferences.end(); // Close

  Serial.printf("Settings loaded: FanMin=%d, FanMax=%d, On=%d, Off=%d\n",
                fanMinPercent, fanMaxPercent, lightOnHour, lightOffHour);
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setLight(bool on) {
  if (on) {
    digitalWrite(LIGHT_PIN, HIGH);
    Serial.println("Light set to ON");
  } else {
    digitalWrite(LIGHT_PIN, LOW);
    Serial.println("Light set to OFF");
  }
  isLightOn = on;
}

void checkTimer() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // If we don't have time yet, we can't do timer logic.
    return;
  }

  int hour = timeinfo.tm_hour;
  // Desired: ON from lightOnHour to lightOffHour

  bool shouldBeOn = false;
  if (lightOnHour > lightOffHour) {
    // Crosses midnight (e.g. 18 to 14)
    shouldBeOn = (hour >= lightOnHour || hour < lightOffHour);
  } else {
    // Same day (e.g. 08 to 20)
    shouldBeOn = (hour >= lightOnHour && hour < lightOffHour);
  }

  // Updates only if state changes to avoid spamming setLight (which prints to
  // Serial)
  if (shouldBeOn != isLightOn) {
    Serial.printf("Timer Update: Time is %02d:%02d. Setting Light to %s\n",
                  hour, timeinfo.tm_min, shouldBeOn ? "ON" : "OFF");
    setLight(shouldBeOn);
  }
}

// Arctic P14 PWM PST Min Duty Cycle (~5% of 255)
// This is CONSTANT for the hardware.
// The user 'fanMinPercent' is logical scaling.
const int hardwareFanMinDuty = 13;

void setFan(int percent) {
  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  int dutyCycle = 0;

  if (percent == 0) {
    dutyCycle = 0; // Off
  } else {
    // Map logic:
    // Input 1-100% maps to Output [fanMin, fanMax]
    // Then map Output to hardware duty [hardwareFanMinDuty, 255]

    // Safety check if min > max
    int effMin = fanMinPercent;
    int effMax = fanMaxPercent;
    if (effMin > effMax) {
      effMin = effMax;
    }

    long mappedPercent = map(percent, 1, 100, effMin, effMax);

    dutyCycle = map(mappedPercent, 0, 100, hardwareFanMinDuty, 255);
  }

  // New API: ledcWrite(pin, duty)
  ledcWrite(FAN_PIN, dutyCycle);

  Serial.print("Fan set to input ");
  Serial.print(percent);
  Serial.print("% -> Mapped ");
  // Re-calculate for display
  int effMin = fanMinPercent;
  int effMax = fanMaxPercent;
  if (effMin > effMax)
    effMin = effMax;
  Serial.print(map(percent, 1, 100, effMin, effMax));

  Serial.print("% (Duty: ");
  Serial.print(dutyCycle);
  Serial.println(")");
}

// --- Setters with Persistence for BLE ---

void setFanMin(int minVal) {
  if (minVal < 0)
    minVal = 0;
  if (minVal > 100)
    minVal = 100;
  fanMinPercent = minVal;

  preferences.begin("growtower", false); // RW
  preferences.putInt("fanMin", fanMinPercent);
  preferences.end();
  Serial.printf("Config: Fan Min set to %d%%\n", fanMinPercent);
  // Optional: Refresh fan speed if currently running
}

void setFanMax(int maxVal) {
  if (maxVal < 0)
    maxVal = 0;
  if (maxVal > 100)
    maxVal = 100;
  fanMaxPercent = maxVal;

  preferences.begin("growtower", false);
  preferences.putInt("fanMax", fanMaxPercent);
  preferences.end();
  Serial.printf("Config: Fan Max set to %d%%\n", fanMaxPercent);
}

void setLightOnTime(int hour) {
  if (hour < 0)
    hour = 0;
  if (hour > 23)
    hour = 23;
  lightOnHour = hour;

  preferences.begin("growtower", false);
  preferences.putInt("onHour", lightOnHour);
  preferences.end();
  Serial.printf("Config: Light On Hour set to %d:00\n", lightOnHour);
  checkTimer(); // Re-check immediately
}

void setLightOffTime(int hour) {
  if (hour < 0)
    hour = 0;
  if (hour > 23)
    hour = 23;
  lightOffHour = hour;

  preferences.begin("growtower", false);
  preferences.putInt("offHour", lightOffHour);
  preferences.end();
  Serial.printf("Config: Light Off Hour set to %d:00\n", lightOffHour);
  checkTimer(); // Re-check immediately
}

void processCommand(String command) {
  command.trim();
  command.toUpperCase();

  if (command == "ON") {
    setLight(true);
  } else if (command == "OFF") {
    setLight(false);
  } else if (command.startsWith("FAN")) {
    // Control Fan
    // Support "FAN 50", "FAN:50", "FAN50"
    String valueStr = command.substring(3);
    valueStr.trim(); // Remove leading/trailing whitespace
    if (valueStr.startsWith(":")) {
      valueStr.remove(0, 1); // Remove colon if present
      valueStr.trim();
    }

    int value = valueStr.toInt();
    setFan(value);
  } else if (command == "TIME") {

    printLocalTime();
  } else {
    Serial.println("Unknown command. Available: ON, OFF, FAN <0-100>, TIME");
  }
}

void setup() {
  Serial.begin(115200);

  // Wait for Serial to be ready (useful for debugging on some boards)
  // delay(2000);

  // Load settings from Flash
  loadSettings();

  // Configure Light Pin as Output
  pinMode(LIGHT_PIN, OUTPUT);
  // Initial State: Check logic? Default off for safety, loop will fix it
  setLight(false);

  // Configure Fan PWM
  // New API: ledcAttach(pin, freq, resolution)
  if (!ledcAttach(FAN_PIN, pwmFreq, pwmResolution)) {
    Serial.println("Fan PWM Setup Failed!");
  }
  // Initial State: Fan Off
  setFan(0);

  Serial.println("Starting Firmware...");

  // Initialize BLE
  // We need to pass the new setters
  initBLE(setLight, setFan, setFanMin, setFanMax, setLightOnTime,
          setLightOffTime);

  // Connect to WiFi
  // WIFI_SSID and WIFI_PASS are defined in secrets.h (generated from .env)
#ifdef WIFI_SSID
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
#ifdef WIFI_PASS
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#else
  // Fallback or error if PASS is missing but SSID used
  WiFi.begin(WIFI_SSID);
  Serial.println("Warning: WIFI_PASS not defined.");
#endif

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println("");

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Init and get the time
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    printLocalTime();
  } else {
    Serial.println("WiFi connection failed.");
  }
#else
  Serial.println(
      "Error: WIFI_SSID not defined. Check .env and secrets generation.");
#endif
}

void loop() {
  // Check for Serial Input
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    processCommand(input);
  }

  // Optional: Periodically sync time or just rely on background NTP
  // Minimal blocking in loop
  // Check timer every loop
  checkTimer();

  // Short delay to avoid WDT trigger but keep responsive
  delay(100);
}
