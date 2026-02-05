#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <GrowTowerBLE.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

// =============================================================================
// PIN CONFIGURATION
// =============================================================================
#define LIGHT_PIN D1 // GPIO1 for Light relay control
#define FAN_PIN D2   // GPIO4 for Fan PWM control

// =============================================================================
// PWM CONFIGURATION
// =============================================================================
const int PWM_FREQUENCY = 25000; // 25kHz standard for PC fans
const int PWM_RESOLUTION = 8;    // 8-bit resolution (0-255)
const int PWM_CHANNEL = 0;       // PWM channel 0
const int MAX_DUTY_CYCLE = 255;

// Hardware minimum duty cycle (~5% for Arctic P14 fan)
const int HARDWARE_FAN_MIN_DUTY = 13;

// =============================================================================
// OTA CONFIGURATION
// =============================================================================
const char *OTA_HOSTNAME = "growtower";
const char *OTA_PASSWORD = "growtower123";  // Change this!
const uint16_t OTA_PORT = 3232;

// =============================================================================
// NTP CONFIGURATION
// =============================================================================
const char *NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = 3600;     // GMT+1 (Berlin)
const int DAYLIGHT_OFFSET_SEC = 3600; // Summer time +1hr

// =============================================================================
// CONFIGURATION VARIABLES
// =============================================================================
Preferences preferences;

// Configurable parameters (loaded from flash, with defaults)
int fanMinPercent = 0;
int fanMaxPercent = 100;
int lightOnHour = 18;
int lightOffHour = 14;

// Current state
bool isLightOn = false;
int currentFanSpeed = 30; // Default 30% (not off for safety)

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================
void loadSettings();
void saveFanMin(int minVal);
void saveFanMax(int maxVal);
void saveLightOnHour(int hour);
void saveLightOffHour(int hour);
void setLight(bool on);
void setFan(int percent);
void checkTimer();
void printLocalTime();
void processCommand(String command);
void initWiFi();
void initPWM();
void printStatus();
void initOTA();

// =============================================================================
// SETUP FUNCTION
// =============================================================================
void setup() {
  // Initialize Serial
  Serial.begin(115200);
  delay(1000); // Wait for Serial to stabilize

  Serial.println("\n");
  Serial.println(
      "╔══════════════════════════════════════════════════════════════╗");
  Serial.println(
      "║                    🌱 GrowTower v2.0                         ║");
  Serial.println(
      "║              Professional Cultivation System                 ║");
  Serial.println(
      "╚══════════════════════════════════════════════════════════════╝");
  Serial.println("\n[SYS] System initializing...\n");

  // Load saved settings
  Serial.println("[SYS] Loading configuration from flash...");
  loadSettings();

  // Initialize Light pin
  Serial.println("[SYS] Initializing light control...");
  pinMode(LIGHT_PIN, OUTPUT);
  setLight(false); // Start with light off for safety

  // Initialize Fan PWM
  Serial.println("[SYS] Initializing fan PWM...");
  initPWM();
  setFan(currentFanSpeed);

  // Initialize BLE
  Serial.println("[SYS] Initializing BLE server...");
  growtower_ble_init(setLight,         // Light callback
                     setFan,           // Fan callback
                     saveFanMin,       // Fan min callback
                     saveFanMax,       // Fan max callback
                     saveLightOnHour,  // Light on hour callback
                     saveLightOffHour, // Light off hour callback
                     isLightOn,        // Initial light state
                     currentFanSpeed,  // Initial fan speed
                     fanMinPercent,    // Initial fan min
                     fanMaxPercent,    // Initial fan max
                     lightOnHour,      // Initial light on hour
                     lightOffHour      // Initial light off hour
  );

  // Initialize WiFi and NTP
  Serial.println("[SYS] Initializing WiFi...");
  initWiFi();

  // Initialize OTA
  Serial.println("[SYS] Initializing OTA...");
  initOTA();

  // Print initial status
  Serial.println("\n[SYS] Initialization complete!");
  printStatus();
  Serial.println(
      "\n[SYS] Ready for commands. Type 'HELP' for available commands.\n");
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  // Handle OTA updates
  ArduinoOTA.handle();

  // Check for serial commands
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      processCommand(input);
    }
  }

  // Check timer for automatic light control
  checkTimer();

  // Small delay to prevent WDT issues
  delay(100);
}

// =============================================================================
// PWM INITIALIZATION
// =============================================================================
void initPWM() {
  // Configure PWM using legacy ESP32 API
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(FAN_PIN, PWM_CHANNEL);
  Serial.printf("[PWM] Initialized: Channel=%d, Freq=%dHz, Resolution=%dbit\n",
                PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
}

// =============================================================================
// WIFI INITIALIZATION
// =============================================================================
void initWiFi() {
#ifdef WIFI_SSID
  Serial.printf("[WIFI] Connecting to: %s\n", WIFI_SSID);

#ifdef WIFI_PASS
  WiFi.begin(WIFI_SSID, WIFI_PASS);
#else
  WiFi.begin(WIFI_SSID);
  Serial.println("[WIFI] Warning: No password defined");
#endif

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 30) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WIFI] Connected! IP: %s\n",
                  WiFi.localIP().toString().c_str());

    // Initialize NTP
    Serial.println("[NTP] Initializing time synchronization...");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    printLocalTime();
  } else {
    Serial.println("[WIFI] Connection failed! Continuing without WiFi...");
    Serial.println("[WIFI] Timer functions will not work without time sync.");
  }
#else
  Serial.println("[WIFI] Error: WIFI_SSID not defined in secrets.h");
  Serial.println("[WIFI] Create firmware/.env file with your WiFi credentials");
#endif
}

// =============================================================================
// OTA INITIALIZATION
// =============================================================================
void initOTA() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] WiFi not connected, OTA disabled");
    return;
  }

  // Configure mDNS
  if (!MDNS.begin(OTA_HOSTNAME)) {
    Serial.println("[OTA] Error setting up mDNS responder!");
    return;
  }
  Serial.printf("[OTA] mDNS responder started: %s.local\n", OTA_HOSTNAME);

  // Configure ArduinoOTA
  ArduinoOTA.setHostname(OTA_HOSTNAME);
  ArduinoOTA.setPort(OTA_PORT);
  // ArduinoOTA.setPassword(OTA_PASSWORD);  // Uncomment to enable password

  ArduinoOTA.onStart([]() {
    String type = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";
    Serial.printf("[OTA] Start updating %s\n", type.c_str());
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("[OTA] Update complete");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End failed");
    }
  });

  ArduinoOTA.begin();
  Serial.printf("[OTA] Ready on port %d\n", OTA_PORT);
  Serial.printf("[OTA] Upload command: pio run -t upload -e main_ota\n");
}

// =============================================================================
// SETTINGS MANAGEMENT
// =============================================================================
void loadSettings() {
  preferences.begin("growtower", true); // Read-only mode

  fanMinPercent = preferences.getInt("fanMin", 0);
  fanMaxPercent = preferences.getInt("fanMax", 100);
  lightOnHour = preferences.getInt("onHour", 18);
  lightOffHour = preferences.getInt("offHour", 14);

  preferences.end();

  Serial.printf("[CONFIG] Loaded: FanMin=%d%%, FanMax=%d%%, LightOn=%d:00, "
                "LightOff=%d:00\n",
                fanMinPercent, fanMaxPercent, lightOnHour, lightOffHour);
}

void saveFanMin(int minVal) {
  if (minVal < 0)
    minVal = 0;
  if (minVal > 100)
    minVal = 100;

  fanMinPercent = minVal;
  preferences.begin("growtower", false);
  preferences.putInt("fanMin", fanMinPercent);
  preferences.end();

  Serial.printf("[CONFIG] Fan Min saved: %d%%\n", fanMinPercent);

  // Refresh current fan speed with new min/max
  setFan(currentFanSpeed);
}

void saveFanMax(int maxVal) {
  if (maxVal < 0)
    maxVal = 0;
  if (maxVal > 100)
    maxVal = 100;

  fanMaxPercent = maxVal;
  preferences.begin("growtower", false);
  preferences.putInt("fanMax", fanMaxPercent);
  preferences.end();

  Serial.printf("[CONFIG] Fan Max saved: %d%%\n", fanMaxPercent);

  // Refresh current fan speed with new min/max
  setFan(currentFanSpeed);
}

void saveLightOnHour(int hour) {
  if (hour < 0)
    hour = 0;
  if (hour > 23)
    hour = 23;

  lightOnHour = hour;
  preferences.begin("growtower", false);
  preferences.putInt("onHour", lightOnHour);
  preferences.end();

  Serial.printf("[CONFIG] Light On Hour saved: %d:00\n", lightOnHour);
  checkTimer(); // Re-check immediately
}

void saveLightOffHour(int hour) {
  if (hour < 0)
    hour = 0;
  if (hour > 23)
    hour = 23;

  lightOffHour = hour;
  preferences.begin("growtower", false);
  preferences.putInt("offHour", lightOffHour);
  preferences.end();

  Serial.printf("[CONFIG] Light Off Hour saved: %d:00\n", lightOffHour);
  checkTimer(); // Re-check immediately
}

// =============================================================================
// LIGHT CONTROL
// =============================================================================
void setLight(bool on) {
  if (on) {
    digitalWrite(LIGHT_PIN, HIGH);
    Serial.println("[LIGHT] State: ON");
  } else {
    digitalWrite(LIGHT_PIN, LOW);
    Serial.println("[LIGHT] State: OFF");
  }
  isLightOn = on;

  // Update BLE characteristic
  growtower_ble_update_light(on);
}

// =============================================================================
// FAN CONTROL
// =============================================================================
void setFan(int percent) {
  // Clamp input
  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

  currentFanSpeed = percent;
  int dutyCycle = 0;

  if (percent == 0) {
    dutyCycle = 0; // Fan off
  } else {
    // Ensure min <= max
    int effectiveMin = fanMinPercent;
    int effectiveMax = fanMaxPercent;
    if (effectiveMin > effectiveMax) {
      effectiveMin = effectiveMax;
    }

    // Map input to effective range
    long mappedPercent = map(percent, 1, 100, effectiveMin, effectiveMax);

    // Map to hardware duty cycle
    dutyCycle =
        map(mappedPercent, 0, 100, HARDWARE_FAN_MIN_DUTY, MAX_DUTY_CYCLE);
  }

  // Apply PWM
  ledcWrite(PWM_CHANNEL, dutyCycle);

  Serial.printf(
      "[FAN] Speed: %d%% (Effective Range: %d%%-%d%%, Duty: %d/255)\n", percent,
      fanMinPercent, fanMaxPercent, dutyCycle);

  // Update BLE characteristic
  growtower_ble_update_fan(percent);
}

// =============================================================================
// TIMER LOGIC
// =============================================================================
void checkTimer() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return; // No time available yet
  }

  int currentHour = timeinfo.tm_hour;
  bool shouldBeOn = false;

  // Calculate if light should be on
  if (lightOnHour > lightOffHour) {
    // Spanning midnight (e.g., 18:00 to 14:00)
    shouldBeOn = (currentHour >= lightOnHour || currentHour < lightOffHour);
  } else {
    // Same day (e.g., 08:00 to 20:00)
    shouldBeOn = (currentHour >= lightOnHour && currentHour < lightOffHour);
  }

  // Only update if state changed
  if (shouldBeOn != isLightOn) {
    Serial.printf("[TIMER] Time: %02d:%02d | Auto-switching light %s\n",
                  currentHour, timeinfo.tm_min, shouldBeOn ? "ON" : "OFF");
    setLight(shouldBeOn);
  }
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================
void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[TIME] Failed to get time from NTP");
    return;
  }

  char timeString[50];
  strftime(timeString, sizeof(timeString), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  Serial.printf("[TIME] Current: %s\n", timeString);
}

void printStatus() {
  Serial.println("\n═══════════════ CURRENT STATUS ═══════════════");
  Serial.printf("  Light:        %s\n", isLightOn ? "ON ✓" : "OFF ✗");
  Serial.printf("  Fan Speed:    %d%%\n", currentFanSpeed);
  Serial.printf("  Fan Range:    %d%% - %d%%\n", fanMinPercent, fanMaxPercent);
  Serial.printf("  Light Timer:  %02d:00 - %02d:00\n", lightOnHour,
                lightOffHour);
  Serial.printf("  BLE Status:   %s (%d client(s))\n",
                growtower_ble_is_connected() ? "Connected ✓" : "Advertising...",
                growtower_ble_get_connected_count());
  Serial.println("═══════════════════════════════════════════════\n");
}

// =============================================================================
// SERIAL COMMAND PROCESSOR
// =============================================================================
void processCommand(String command) {
  command.trim();
  command.toUpperCase();

  Serial.printf("[CMD] Received: '%s'\n", command.c_str());

  if (command == "HELP" || command == "?") {
    Serial.println("\n╔════════════════════════════════════════════════╗");
    Serial.println("║              AVAILABLE COMMANDS                ║");
    Serial.println("╠════════════════════════════════════════════════╣");
    Serial.println("║  ON              - Turn light ON               ║");
    Serial.println("║  OFF             - Turn light OFF              ║");
    Serial.println("║  FAN <0-100>     - Set fan speed (percent)    ║");
    Serial.println("║  FANMIN <0-100>  - Set fan minimum speed      ║");
    Serial.println("║  FANMAX <0-100>  - Set fan maximum speed      ║");
    Serial.println("║  LIGHTON <0-23>  - Set light ON hour          ║");
    Serial.println("║  LIGHTOFF <0-23> - Set light OFF hour         ║");
    Serial.println("║  TIME            - Show current time          ║");
    Serial.println("║  STATUS          - Show system status         ║");
    Serial.println("║  RESET           - Reset all settings         ║");
    Serial.println("║  HELP            - Show this help             ║");
    Serial.println("╚════════════════════════════════════════════════╝\n");
  } else if (command == "ON") {
    setLight(true);
  } else if (command == "OFF") {
    setLight(false);
  } else if (command.startsWith("FAN ")) {
    String valueStr = command.substring(4);
    valueStr.trim();
    int value = valueStr.toInt();
    setFan(value);
  } else if (command.startsWith("FANMIN ")) {
    String valueStr = command.substring(7);
    valueStr.trim();
    int value = valueStr.toInt();
    saveFanMin(value);
  } else if (command.startsWith("FANMAX ")) {
    String valueStr = command.substring(7);
    valueStr.trim();
    int value = valueStr.toInt();
    saveFanMax(value);
  } else if (command.startsWith("LIGHTON ")) {
    String valueStr = command.substring(8);
    valueStr.trim();
    int value = valueStr.toInt();
    saveLightOnHour(value);
  } else if (command.startsWith("LIGHTOFF ")) {
    String valueStr = command.substring(9);
    valueStr.trim();
    int value = valueStr.toInt();
    saveLightOffHour(value);
  } else if (command == "TIME") {
    printLocalTime();
  } else if (command == "STATUS") {
    printStatus();
  } else if (command == "RESET") {
    Serial.println("[SYS] Resetting all settings to defaults...");
    preferences.begin("growtower", false);
    preferences.clear();
    preferences.end();
    Serial.println("[SYS] Settings cleared. Please restart device.");
  } else {
    Serial.printf("[CMD] Unknown command: '%s'\n", command.c_str());
    Serial.println("[CMD] Type 'HELP' for available commands");
  }
}