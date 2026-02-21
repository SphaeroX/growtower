#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <time.h>

#include "config.h"
#include "state.h"
#include "frontend.h"
#include "webserver.h"

Preferences preferences;
char currentHostname[32] = "growtower";

int fanMinPercent = 0;
int fanMaxPercent = 100;
int lightOnHour = 18;
int lightOffHour = 14;

bool isLightOn = false;
int currentFanSpeed = 30;

PhaseData phases[3] = {{0, false}, {0, false}, {0, false}};
PlantPhase currentPhase = PHASE_NONE;

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n");
  Serial.println("╔══════════════════════════════════════════════════════════════╗");
  Serial.println("║                    🌱 GrowTower v3.0                         ║");
  Serial.println("║           Web-Based Cultivation System                       ║");
  Serial.println("╚══════════════════════════════════════════════════════════════╝");
  Serial.println("\n[SYS] System initializing...\n");

  Serial.println("[SYS] Loading configuration from flash...");
  loadSettings();

  Serial.println("[SYS] Initializing light control...");
  pinMode(LIGHT_PIN, OUTPUT);
  setLight(false);

  Serial.println("[SYS] Initializing fan PWM...");
  initPWM();
  setFan(currentFanSpeed);

  Serial.println("[SYS] Initializing WiFi...");
  initWiFi();

  Serial.println("[SYS] Initializing OTA...");
  initOTA();

  Serial.println("[SYS] Initializing Web Server...");
  initWebServer();

  Serial.println("\n[SYS] Initialization complete!");
  printStatus();
  Serial.println("\n[SYS] Ready. Access the controller at: http://" +
                 String(currentHostname) + ".local\n");
}

void loop() {
  ArduinoOTA.handle();
  checkTimer();

  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      processCommand(input);
    }
  }

  delay(100);
}

void initPWM() {
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(FAN_PIN, PWM_CHANNEL);
  Serial.printf("[PWM] Initialized: Channel=%d, Freq=%dHz, Resolution=%dbit\n",
                PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
}

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

    Serial.println("[NTP] Initializing time synchronization...");
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
    printLocalTime();
  } else {
    Serial.println("[WIFI] Connection failed!");
  }
#else
  Serial.println("[WIFI] Error: WIFI_SSID not defined in secrets.h");
#endif
}

void initOTA() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] WiFi not connected, OTA disabled");
    return;
  }

  if (!MDNS.begin(currentHostname)) {
    Serial.println("[OTA] Error setting up mDNS responder!");
    return;
  }
  Serial.printf("[OTA] mDNS responder started: %s.local\n", currentHostname);

  MDNS.addService("http", "tcp", 80);

  ArduinoOTA.setHostname(currentHostname);
  ArduinoOTA.setPort(OTA_PORT);

  ArduinoOTA.onStart([]() {
    String type = ArduinoOTA.getCommand() == U_FLASH ? "sketch" : "filesystem";
    Serial.printf("[OTA] Start updating %s\n", type.c_str());
  });

  ArduinoOTA.onEnd([]() { Serial.println("[OTA] Update complete"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("[OTA] Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive failed");
    else if (error == OTA_END_ERROR) Serial.println("End failed");
  });

  ArduinoOTA.begin();
  Serial.printf("[OTA] Ready on port %d\n", OTA_PORT);
}

String getStatusJSON() {
  struct tm timeinfo;
  bool hasTime = getLocalTime(&timeinfo);

  String json = "{";
  json += "\"light\":" + String(isLightOn ? "true" : "false") + ",";
  json += "\"fan\":" + String(currentFanSpeed) + ",";
  json += "\"fanMin\":" + String(fanMinPercent) + ",";
  json += "\"fanMax\":" + String(fanMaxPercent) + ",";
  json += "\"lightOn\":" + String(lightOnHour) + ",";
  json += "\"lightOff\":" + String(lightOffHour) + ",";
  json += "\"hostname\":\"" + String(currentHostname) + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"wifiConnected\":" + String(WiFi.status() == WL_CONNECTED ? "true" : "false") + ",";
  json += "\"hasTime\":" + String(hasTime ? "true" : "false");
  if (hasTime) {
    char timeStr[25];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    json += ",\"currentTime\":\"" + String(timeStr) + "\"";
  }
  json += "," + getPhaseJSON();
  json += "}";
  return json;
}

void loadSettings() {
  preferences.begin("growtower", true);

  fanMinPercent = preferences.getInt("fanMin", 0);
  fanMaxPercent = preferences.getInt("fanMax", 100);
  lightOnHour = preferences.getInt("onHour", 18);
  lightOffHour = preferences.getInt("offHour", 14);

  String savedHostname = preferences.getString("hostname", DEFAULT_HOSTNAME);
  strncpy(currentHostname, savedHostname.c_str(), sizeof(currentHostname) - 1);
  currentHostname[sizeof(currentHostname) - 1] = '\0';

  preferences.end();

  loadPhaseData();

  Serial.printf(
      "[CONFIG] Loaded: FanMin=%d%%, FanMax=%d%%, LightOn=%d:00, LightOff=%d:00, "
      "Hostname=%s\n",
      fanMinPercent, fanMaxPercent, lightOnHour, lightOffHour, currentHostname);
}

void saveFanMin(int minVal) {
  if (minVal < 0) minVal = 0;
  if (minVal > 100) minVal = 100;

  fanMinPercent = minVal;
  preferences.begin("growtower", false);
  preferences.putInt("fanMin", fanMinPercent);
  preferences.end();

  Serial.printf("[CONFIG] Fan Min saved: %d%%\n", fanMinPercent);
  setFan(currentFanSpeed);
}

void saveFanMax(int maxVal) {
  if (maxVal < 0) maxVal = 0;
  if (maxVal > 100) maxVal = 100;

  fanMaxPercent = maxVal;
  preferences.begin("growtower", false);
  preferences.putInt("fanMax", fanMaxPercent);
  preferences.end();

  Serial.printf("[CONFIG] Fan Max saved: %d%%\n", fanMaxPercent);
  setFan(currentFanSpeed);
}

void saveLightOnHour(int hour) {
  if (hour < 0) hour = 0;
  if (hour > 23) hour = 23;

  lightOnHour = hour;
  preferences.begin("growtower", false);
  preferences.putInt("onHour", lightOnHour);
  preferences.end();

  Serial.printf("[CONFIG] Light On Hour saved: %d:00\n", lightOnHour);
  checkTimer();
}

void saveLightOffHour(int hour) {
  if (hour < 0) hour = 0;
  if (hour > 23) hour = 23;

  lightOffHour = hour;
  preferences.begin("growtower", false);
  preferences.putInt("offHour", lightOffHour);
  preferences.end();

  Serial.printf("[CONFIG] Light Off Hour saved: %d:00\n", lightOffHour);
  checkTimer();
}

void saveHostname(const char *hostname) {
  preferences.begin("growtower", false);
  preferences.putString("hostname", hostname);
  preferences.end();

  strncpy(currentHostname, hostname, sizeof(currentHostname) - 1);
  currentHostname[sizeof(currentHostname) - 1] = '\0';

  Serial.printf("[CONFIG] Hostname saved: %s\n", currentHostname);
}

void setLight(bool on) {
  if (on) {
    digitalWrite(LIGHT_PIN, HIGH);
    Serial.println("[LIGHT] State: ON");
  } else {
    digitalWrite(LIGHT_PIN, LOW);
    Serial.println("[LIGHT] State: OFF");
  }
  isLightOn = on;
}

void setFan(int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  currentFanSpeed = percent;
  int dutyCycle = 0;

  if (percent == 0) {
    dutyCycle = 0;
  } else {
    int effectiveMin = fanMinPercent;
    int effectiveMax = fanMaxPercent;
    if (effectiveMin > effectiveMax) {
      effectiveMin = effectiveMax;
    }

    long mappedPercent = map(percent, 1, 100, effectiveMin, effectiveMax);
    dutyCycle = map(mappedPercent, 0, 100, HARDWARE_FAN_MIN_DUTY, MAX_DUTY_CYCLE);
  }

  ledcWrite(PWM_CHANNEL, dutyCycle);

  Serial.printf(
      "[FAN] Speed: %d%% (Effective Range: %d%%-%d%%, Duty: %d/255)\n", percent,
      fanMinPercent, fanMaxPercent, dutyCycle);
}

void checkTimer() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return;
  }

  int currentHour = timeinfo.tm_hour;
  bool shouldBeOn = false;

  if (lightOnHour > lightOffHour) {
    shouldBeOn = (currentHour >= lightOnHour || currentHour < lightOffHour);
  } else {
    shouldBeOn = (currentHour >= lightOnHour && currentHour < lightOffHour);
  }

  if (shouldBeOn != isLightOn) {
    Serial.printf("[TIMER] Time: %02d:%02d | Auto-switching light %s\n",
                  currentHour, timeinfo.tm_min, shouldBeOn ? "ON" : "OFF");
    setLight(shouldBeOn);
  }
}

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
  Serial.printf("  Light Timer:  %02d:00 - %02d:00\n", lightOnHour, lightOffHour);
  Serial.printf("  Hostname:     %s.local\n", currentHostname);
  Serial.printf("  IP Address:   %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("  Web Server:   %s\n",
                WiFi.status() == WL_CONNECTED ? "Running ✓" : "Disabled ✗");
  Serial.println("═══════════════════════════════════════════════\n");
}

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
    Serial.println("║  HOST <name>     - Set hostname               ║");
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
  } else if (command.startsWith("HOST ")) {
    String valueStr = command.substring(5);
    valueStr.trim();
    valueStr.toLowerCase();
    if (valueStr.length() > 0 && valueStr.length() < 32) {
      saveHostname(valueStr.c_str());
      Serial.println("[CMD] Hostname saved. Restart to apply changes.");
    } else {
      Serial.println("[CMD] Invalid hostname (1-31 chars)");
    }
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

void loadPhaseData() {
  preferences.begin("growtower", true);
  
  phases[PHASE_SEEDLING].startTime = preferences.getLong("seedlingStart", 0);
  phases[PHASE_VEG].startTime = preferences.getLong("vegStart", 0);
  phases[PHASE_FLOWER].startTime = preferences.getLong("flowerStart", 0);
  
  phases[PHASE_SEEDLING].active = preferences.getBool("seedlingActive", false);
  phases[PHASE_VEG].active = preferences.getBool("vegActive", false);
  phases[PHASE_FLOWER].active = preferences.getBool("flowerActive", false);
  
  preferences.end();
  
  currentPhase = PHASE_NONE;
  if (phases[PHASE_SEEDLING].active) currentPhase = PHASE_SEEDLING;
  else if (phases[PHASE_VEG].active) currentPhase = PHASE_VEG;
  else if (phases[PHASE_FLOWER].active) currentPhase = PHASE_FLOWER;
  
  Serial.printf("[PHASE] Loaded: Seedling=%d, Veg=%d, Flower=%d, Current=%d\n",
                phases[PHASE_SEEDLING].active ? 1 : 0,
                phases[PHASE_VEG].active ? 1 : 0,
                phases[PHASE_FLOWER].active ? 1 : 0,
                currentPhase);
}

void savePhaseData() {
  preferences.begin("growtower", false);
  
  preferences.putLong("seedlingStart", phases[PHASE_SEEDLING].startTime);
  preferences.putLong("vegStart", phases[PHASE_VEG].startTime);
  preferences.putLong("flowerStart", phases[PHASE_FLOWER].startTime);
  
  preferences.putBool("seedlingActive", phases[PHASE_SEEDLING].active);
  preferences.putBool("vegActive", phases[PHASE_VEG].active);
  preferences.putBool("flowerActive", phases[PHASE_FLOWER].active);
  
  preferences.end();
}

void setPhase(PlantPhase phase) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[PHASE] Cannot set phase: NTP time not available");
    return;
  }
  
  time_t now = mktime(&timeinfo);
  
  if (phase == PHASE_NONE) {
    phases[PHASE_SEEDLING].active = false;
    phases[PHASE_VEG].active = false;
    phases[PHASE_FLOWER].active = false;
    currentPhase = PHASE_NONE;
    Serial.println("[PHASE] All phases reset");
  } else {
    phases[PHASE_SEEDLING].active = (phase == PHASE_SEEDLING);
    phases[PHASE_VEG].active = (phase == PHASE_VEG);
    phases[PHASE_FLOWER].active = (phase == PHASE_FLOWER);
    
    if (phase == PHASE_SEEDLING && phases[PHASE_SEEDLING].startTime == 0) {
      phases[PHASE_SEEDLING].startTime = now;
    } else if (phase == PHASE_VEG && phases[PHASE_VEG].startTime == 0) {
      phases[PHASE_VEG].startTime = now;
    } else if (phase == PHASE_FLOWER && phases[PHASE_FLOWER].startTime == 0) {
      phases[PHASE_FLOWER].startTime = now;
    }
    
    currentPhase = phase;
    
    const char* phaseNames[] = {"None", "Seedling", "Veg", "Flower"};
    Serial.printf("[PHASE] Set to: %s\n", phaseNames[phase]);
  }
  
  savePhaseData();
}

int getPhaseDays(PlantPhase phase) {
  if (!phases[phase].active || phases[phase].startTime == 0) return 0;
  
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return 0;
  
  time_t now = mktime(&timeinfo);
  double diffSeconds = difftime(now, phases[phase].startTime);
  return (int)(diffSeconds / 86400.0);
}

int getTotalDays() {
  int total = 0;
  if (phases[PHASE_SEEDLING].active || phases[PHASE_SEEDLING].startTime > 0) {
    total += getPhaseDays(PHASE_SEEDLING);
  }
  if (phases[PHASE_VEG].active || phases[PHASE_VEG].startTime > 0) {
    total += getPhaseDays(PHASE_VEG);
  }
  if (phases[PHASE_FLOWER].active || phases[PHASE_FLOWER].startTime > 0) {
    total += getPhaseDays(PHASE_FLOWER);
  }
  return total;
}

void resetPhase(PlantPhase phase) {
  phases[phase].startTime = 0;
  phases[phase].active = false;
  
  if (currentPhase == phase) {
    currentPhase = PHASE_NONE;
  }
  
  savePhaseData();
  
  const char* phaseNames[] = {"All", "Seedling", "Veg", "Flower"};
  Serial.printf("[PHASE] Reset: %s\n", phaseNames[phase]);
}

String getPhaseJSON() {
  const char* phaseNames[] = {"none", "seedling", "veg", "flower"};
  
  String json = "\"phase\":\"" + String(phaseNames[currentPhase]) + "\",";
  json += "\"currentPhase\":" + String(currentPhase) + ",";
  json += "\"totalDays\":" + String(getTotalDays()) + ",";
  json += "\"seedling\":{\"active\":" + String(phases[PHASE_SEEDLING].active ? "true" : "false") + 
          ",\"days\":" + String(getPhaseDays(PHASE_SEEDLING)) + "},";
  json += "\"veg\":{\"active\":" + String(phases[PHASE_VEG].active ? "true" : "false") + 
           ",\"days\":" + String(getPhaseDays(PHASE_VEG)) + "},";
  json += "\"flower\":{\"active\":" + String(phases[PHASE_FLOWER].active ? "true" : "false") + 
           ",\"days\":" + String(getPhaseDays(PHASE_FLOWER)) + "}";
  
  return json;
}
