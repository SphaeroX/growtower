#include "secrets.h"
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

// --- Configuration ---
// Define the pin for the light control (PWM)
// Please check your actual wiring. D0 is a placeholder provided in the plan.
// On Seeed XIAO ESP32S3, D0 is GPIO1 (usually).
#define LIGHT_PIN D0

// NTP Server settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;     // GMT+1
const int daylightOffset_sec = 3600; // Summer time +1hr

// PWM Channel configuration
const int pwmFreq = 5000;
// const int pwmChannel = 0; // Not needed for Arduino ESP32 v3.0+
const int pwmResolution = 8;
const int maxDutyCycle = 255; // 2^8 - 1

// Internal state
int currentBrightnessPercent = 0;

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setLight(int percent) {
  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  currentBrightnessPercent = percent;

  // Map 0-100 to 0-255
  int dutyCycle = map(percent, 0, 100, 0, 255);

  // New API: ledcWrite(pin, duty)
  ledcWrite(LIGHT_PIN, dutyCycle);

  Serial.print("Light set to ");
  Serial.print(percent);
  Serial.println("%");
}

void processCommand(String command) {
  command.trim();
  command.toUpperCase();

  if (command == "ON") {
    setLight(100);
  } else if (command == "OFF") {
    setLight(0);
  } else if (command.startsWith("PWM:")) {
    String valueStr = command.substring(4);
    int value = valueStr.toInt();
    setLight(value);
  } else if (command == "TIME") {
    printLocalTime();
  } else {
    Serial.println("Unknown command. Available: ON, OFF, PWM:<0-100>, TIME");
  }
}

void setup() {
  Serial.begin(115200);

  // Wait for Serial to be ready (useful for debugging on some boards)
  // delay(2000);

  // Configure PWM
  // New API: ledcAttach(pin, freq, resolution)
  if (!ledcAttach(LIGHT_PIN, pwmFreq, pwmResolution)) {
    Serial.println("PWM Setup Failed!");
  }

  // Initial State: Off
  setLight(0);

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
