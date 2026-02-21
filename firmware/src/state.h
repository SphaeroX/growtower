#ifndef STATE_H
#define STATE_H

#include <Preferences.h>
#include "config.h"

extern Preferences preferences;
extern char currentHostname[32];

extern int fanMinPercent;
extern int fanMaxPercent;
extern int lightOnHour;
extern int lightOffHour;
extern bool timerEnabled;

extern bool isLightOn;
extern int currentFanSpeed;

enum PlantPhase { PHASE_NONE, PHASE_SEEDLING, PHASE_VEG, PHASE_FLOWER };

struct PhaseData {
    time_t startTime;
    bool active;
};

extern PhaseData phases[3];
extern PlantPhase currentPhase;

void loadSettings();
void saveFanMin(int minVal);
void saveFanMax(int maxVal);
void saveLightOnHour(int hour);
void saveLightOffHour(int hour);
void saveTimerEnabled(bool enabled);
void saveHostname(const char *hostname);
void setLight(bool on);
void setFan(int percent);
void checkTimer();
void processCommand(String command);
void initWiFi();
void initPWM();
void printStatus();
void initOTA();
void initWebServer();
String getStatusJSON();
void loadPhaseData();
void savePhaseData();
void setPhase(PlantPhase phase);
void resetPhase(PlantPhase phase);
void resetAllSettings();
String getPhaseJSON();
void printLocalTime();

#endif
