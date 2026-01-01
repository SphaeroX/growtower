#include "secrets.h"
#include <Arduino.h>
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

// Internal state
// int currentBrightnessPercent = 0; // Removed

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
}

// Arctic P14 PWM PST Min Duty Cycle (~5% of 255)
const int fanMinDuty = 13;

void setFan(int percent) {
  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  // Map 0-100% to fanMinDuty-255
  // 0% input = Minimum running speed (~200 RPM)
  // 100% input = Maximum speed
  int dutyCycle = map(percent, 0, 100, fanMinDuty, 255);

  // New API: ledcWrite(pin, duty)
  ledcWrite(FAN_PIN, dutyCycle);

  Serial.print("Fan set to ");
  Serial.print(percent);
  Serial.print("% (Duty: ");
  Serial.print(dutyCycle);
  Serial.println(")");
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

  // Configure Light Pin as Output
  pinMode(LIGHT_PIN, OUTPUT);
  // Initial State: Light Off
  setLight(false);

  // Configure Fan PWM
  // New API: ledcAttach(pin, freq, resolution)
  if (!ledcAttach(FAN_PIN, pwmFreq, pwmResolution)) {
    Serial.println("Fan PWM Setup Failed!");
  }
  // Initial State: Fan Off
  setFan(0);

  Serial.println("Starting Firmware...");

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
  delay(10);
}
