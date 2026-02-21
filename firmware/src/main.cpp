#include "secrets.h"
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
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
const char *OTA_PASSWORD = "growtower123";
const uint16_t OTA_PORT = 3232;

// =============================================================================
// WEB SERVER CONFIGURATION
// =============================================================================
AsyncWebServer server(80);
const char *DEFAULT_HOSTNAME = "growtower";
char currentHostname[32] = "growtower";

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
// PHASE TRACKING
// =============================================================================
enum PlantPhase { PHASE_NONE, PHASE_SEEDLING, PHASE_VEG, PHASE_FLOWER };

struct PhaseData {
    time_t startTime;
    bool active;
};

PhaseData phases[3] = {{0, false}, {0, false}, {0, false}};
PlantPhase currentPhase = PHASE_NONE;

// Forward declarations
void loadSettings();
void saveFanMin(int minVal);
void saveFanMax(int maxVal);
void saveLightOnHour(int hour);
void saveLightOffHour(int hour);
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
String getPhaseJSON();
void printLocalTime();

// =============================================================================
// HTML FRONTEND (Embedded)
// =============================================================================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>GrowTower Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            min-height: 100vh;
            color: #fff;
            padding: 20px;
        }
        
        .container {
            max-width: 600px;
            margin: 0 auto;
        }
        
        h1 {
            text-align: center;
            margin-bottom: 30px;
            font-size: 2rem;
            background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .status-card {
            background: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            border-radius: 20px;
            padding: 25px;
            margin-bottom: 20px;
            border: 1px solid rgba(255, 255, 255, 0.2);
        }
        
        .status-title {
            font-size: 1.1rem;
            color: #94a3b8;
            margin-bottom: 15px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .status-value {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 12px;
        }
        
        .status-value:last-child {
            margin-bottom: 0;
        }
        
        .status-label {
            color: #cbd5e1;
        }
        
        .status-indicator {
            display: flex;
            align-items: center;
            gap: 8px;
            font-weight: 600;
        }
        
        .status-indicator.on {
            color: #4ade80;
        }
        
        .status-indicator.off {
            color: #f87171;
        }
        
        .dot {
            width: 12px;
            height: 12px;
            border-radius: 50%;
            background: currentColor;
            box-shadow: 0 0 10px currentColor;
        }
        
        .control-group {
            margin-bottom: 25px;
        }
        
        .control-label {
            display: block;
            margin-bottom: 10px;
            color: #cbd5e1;
            font-size: 0.95rem;
        }
        
        .slider-container {
            display: flex;
            align-items: center;
            gap: 15px;
        }
        
        input[type="range"] {
            flex: 1;
            -webkit-appearance: none;
            appearance: none;
            height: 8px;
            background: rgba(255, 255, 255, 0.2);
            border-radius: 4px;
            outline: none;
        }
        
        input[type="range"]::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 24px;
            height: 24px;
            background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 2px 10px rgba(74, 222, 128, 0.5);
            transition: transform 0.2s;
        }
        
        input[type="range"]::-webkit-slider-thumb:hover {
            transform: scale(1.1);
        }
        
        input[type="range"]::-moz-range-thumb {
            width: 24px;
            height: 24px;
            background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
            border-radius: 50%;
            cursor: pointer;
            border: none;
            box-shadow: 0 2px 10px rgba(74, 222, 128, 0.5);
        }
        
        .slider-value {
            min-width: 50px;
            text-align: right;
            font-weight: 600;
            color: #4ade80;
        }
        
        .button-group {
            display: flex;
            gap: 10px;
        }
        
        button {
            flex: 1;
            padding: 15px 25px;
            border: none;
            border-radius: 12px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .btn-on {
            background: linear-gradient(135deg, #4ade80 0%, #22c55e 100%);
            color: #1a1a2e;
        }
        
        .btn-on:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(74, 222, 128, 0.4);
        }
        
        .btn-off {
            background: linear-gradient(135deg, #f87171 0%, #ef4444 100%);
            color: #1a1a2e;
        }
        
        .btn-off:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(248, 113, 113, 0.4);
        }
        
        button:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none !important;
        }
        
        .time-inputs {
            display: flex;
            align-items: center;
            gap: 15px;
        }
        
        .time-input {
            flex: 1;
        }
        
        input[type="number"] {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid rgba(255, 255, 255, 0.2);
            border-radius: 10px;
            background: rgba(255, 255, 255, 0.1);
            color: #fff;
            font-size: 1.1rem;
            text-align: center;
            transition: border-color 0.3s;
        }
        
        input[type="number"]:focus {
            outline: none;
            border-color: #4ade80;
        }
        
        input[type="text"] {
            width: 100%;
            padding: 12px 15px;
            border: 2px solid rgba(255, 255, 255, 0.2);
            border-radius: 10px;
            background: rgba(255, 255, 255, 0.1);
            color: #fff;
            font-size: 1rem;
            transition: border-color 0.3s;
        }
        
        input[type="text"]:focus {
            outline: none;
            border-color: #4ade80;
        }
        
        .time-separator {
            color: #94a3b8;
            font-weight: 600;
        }
        
        .save-btn {
            margin-top: 15px;
            width: 100%;
            padding: 15px;
            background: linear-gradient(135deg, #60a5fa 0%, #3b82f6 100%);
            color: white;
            border: none;
            border-radius: 12px;
            font-size: 1rem;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            text-transform: uppercase;
        }
        
        .save-btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 5px 20px rgba(59, 130, 246, 0.4);
        }
        
        .message {
            position: fixed;
            bottom: 20px;
            right: 20px;
            padding: 15px 25px;
            border-radius: 10px;
            font-weight: 600;
            animation: slideIn 0.3s ease;
            z-index: 1000;
        }
        
        @keyframes slideIn {
            from {
                transform: translateX(100%);
                opacity: 0;
            }
            to {
                transform: translateX(0);
                opacity: 1;
            }
        }
        
        .message.success {
            background: #4ade80;
            color: #1a1a2e;
        }
        
        .message.error {
            background: #f87171;
            color: white;
        }
        
        .current-time {
            text-align: center;
            color: #94a3b8;
            font-size: 0.9rem;
            margin-top: 20px;
        }
        
        .section-title {
            font-size: 1.3rem;
            color: #fff;
            margin-bottom: 20px;
            padding-bottom: 10px;
            border-bottom: 2px solid rgba(255, 255, 255, 0.1);
        }
        
        @media (max-width: 480px) {
            h1 {
                font-size: 1.5rem;
            }
            
            .status-card {
                padding: 20px;
            }
            
            button {
                padding: 12px 20px;
                font-size: 0.9rem;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌱 GrowTower Controller</h1>
        
        <div class="status-card">
            <div class="status-title">Aktueller Status</div>
            <div class="status-value">
                <span class="status-label">Licht:</span>
                <span class="status-indicator" id="lightStatus">
                    <span class="dot"></span>
                    <span id="lightText">-</span>
                </span>
            </div>
            <div class="status-value">
                <span class="status-label">Lüfter:</span>
                <span class="status-indicator">
                    <span id="fanValue">-</span>%
                </span>
            </div>
            <div class="status-value">
                <span class="status-label">Lüfter Bereich:</span>
                <span class="status-indicator">
                    <span id="fanMin">-</span>% - <span id="fanMax">-</span>%
                </span>
            </div>
            <div class="status-value">
                <span class="status-label">Licht Timer:</span>
                <span class="status-indicator">
                    <span id="lightOn">-</span>:00 - <span id="lightOff">-</span>:00
                </span>
            </div>
            <div class="status-value">
                <span class="status-label">Geräte-Name:</span>
                <span class="status-indicator" id="hostnameDisplay">
                    -
                </span>
            </div>
        </div>
        
        <div class="status-card">
            <div class="section-title">Licht Steuerung</div>
            <div class="button-group">
                <button class="btn-on" onclick="setLight(true)">Licht AN</button>
                <button class="btn-off" onclick="setLight(false)">Licht AUS</button>
            </div>
        </div>
        
        <div class="status-card">
            <div class="section-title">Lüfter Steuerung</div>
            <div class="control-group">
                <label class="control-label">Geschwindigkeit: <span id="fanPercent">30</span>%</label>
                <div class="slider-container">
                    <input type="range" id="fanSlider" min="0" max="100" value="30" oninput="updateFanLabel(this.value)">
                    <span class="slider-value" id="fanDisplay">30%</span>
                </div>
            </div>
            <button class="save-btn" onclick="setFan()">Lüfter einstellen</button>
        </div>
        
        <div class="status-card">
            <div class="section-title">Lüfter Bereich</div>
            <div class="control-group">
                <label class="control-label">Minimum: <span id="fanMinLabel">0</span>%</label>
                <div class="slider-container">
                    <input type="range" id="fanMinSlider" min="0" max="100" value="0" oninput="updateFanMinLabel(this.value)">
                    <span class="slider-value" id="fanMinDisplay">0%</span>
                </div>
            </div>
            <div class="control-group">
                <label class="control-label">Maximum: <span id="fanMaxLabel">100</span>%</label>
                <div class="slider-container">
                    <input type="range" id="fanMaxSlider" min="0" max="100" value="100" oninput="updateFanMaxLabel(this.value)">
                    <span class="slider-value" id="fanMaxDisplay">100%</span>
                </div>
            </div>
            <button class="save-btn" onclick="setFanRange()">Bereich speichern</button>
        </div>
        
        <div class="status-card">
            <div class="section-title">Licht Timer</div>
            <div class="control-group">
                <label class="control-label">Einschaltzeit</label>
                <div class="time-inputs">
                    <input type="number" id="onHour" min="0" max="23" value="18" class="time-input" oninput="updateLightDuration()">
                    <span class="time-separator">:</span>
                    <input type="number" value="00" disabled class="time-input" style="opacity: 0.5;">
                </div>
            </div>
            <div class="control-group">
                <label class="control-label">Ausschaltzeit</label>
                <div class="time-inputs">
                    <input type="number" id="offHour" min="0" max="23" value="14" class="time-input" oninput="updateLightDuration()">
                    <span class="time-separator">:</span>
                    <input type="number" value="00" disabled class="time-input" style="opacity: 0.5;">
                </div>
            </div>
            <div style="margin: 15px 0; padding: 12px; background: rgba(74, 222, 128, 0.2); border-radius: 10px; text-align: center; border: 1px solid rgba(74, 222, 128, 0.3);">
                <span style="color: #4ade80; font-weight: 600;">Lichtphase: <span id="lightDuration">20h</span></span>
            </div>
            <button class="save-btn" onclick="setLightTimer()">Timer speichern</button>
        </div>
        
        <div class="status-card">
            <div class="section-title">🌱 Pflanzen Tracker</div>
            <div class="control-group">
                <div style="display: flex; gap: 10px; margin-bottom: 15px;">
                    <button id="btnSeedling" onclick="setPhase('seedling')" style="flex:1; padding:12px; background:rgba(139,92,246,0.3); border:2px solid rgba(139,92,246,0.5); border-radius:8px; color:white; cursor:pointer; font-weight:600;">🌿 Seedling</button>
                    <button id="btnVeg" onclick="setPhase('veg')" style="flex:1; padding:12px; background:rgba(16,185,129,0.3); border:2px solid rgba(16,185,129,0.5); border-radius:8px; color:white; cursor:pointer; font-weight:600;">🌱 Veg</button>
                    <button id="btnFlower" onclick="setPhase('flower')" style="flex:1; padding:12px; background:rgba(245,158,11,0.3); border:2px solid rgba(245,158,11,0.5); border-radius:8px; color:white; cursor:pointer; font-weight:600;">🌸 Blüte</button>
                </div>
                <div style="display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; margin-bottom: 15px;">
                    <div style="background:rgba(139,92,246,0.2); padding:15px; border-radius:10px; text-align:center;">
                        <div style="font-size:0.8rem; color:#94a3b8;">Seedling</div>
                        <div id="seedlingDays" style="font-size:1.5rem; font-weight:bold; color:#8b5cf6;">0</div>
                        <div style="font-size:0.75rem; color:#94a3b8;">Tage</div>
                    </div>
                    <div style="background:rgba(16,185,129,0.2); padding:15px; border-radius:10px; text-align:center;">
                        <div style="font-size:0.8rem; color:#94a3b8;">Veg</div>
                        <div id="vegDays" style="font-size:1.5rem; font-weight:bold; color:#10b981;">0</div>
                        <div style="font-size:0.75rem; color:#94a3b8;">Tage</div>
                    </div>
                    <div style="background:rgba(245,158,11,0.2); padding:15px; border-radius:10px; text-align:center;">
                        <div style="font-size:0.8rem; color:#94a3b8;">Blüte</div>
                        <div id="flowerDays" style="font-size:1.5rem; font-weight:bold; color:#f59e0b;">0</div>
                        <div style="font-size:0.75rem; color:#94a3b8;">Tage</div>
                    </div>
                </div>
                <div style="background:rgba(74,222,128,0.2); padding:15px; border-radius:10px; text-align:center; margin-bottom:15px;">
                    <div style="font-size:0.9rem; color:#94a3b8;">Gesamttage</div>
                    <div id="totalDays" style="font-size:2rem; font-weight:bold; color:#4ade80;">0</div>
                    <div style="font-size:0.8rem; color:#94a3b8;">Tage alt</div>
                </div>
                <div style="display:flex; gap: 10px;">
                    <select id="resetPhaseSelect" style="flex:1; padding:10px; background:rgba(255,255,255,0.1); border:1px solid rgba(255,255,255,0.2); border-radius:8px; color:white;">
                        <option value="seedling">Seedling</option>
                        <option value="veg">Veg</option>
                        <option value="flower">Blüte</option>
                        <option value="all">Alle</option>
                    </select>
                    <button onclick="resetPhase()" style="padding:10px 20px; background:#ef4444; border:none; border-radius:8px; color:white; cursor:pointer; font-weight:600;">Zurücksetzen</button>
                </div>
            </div>
        </div>
        
        <div class="status-card">
            <div class="section-title">Netzwerk Einstellungen</div>
            <div class="control-group">
                <label class="control-label">Geräte-Name (für growtower.local)</label>
                <input type="text" id="hostnameInput" placeholder="growtower" maxlength="31">
            </div>
            <button class="save-btn" onclick="setHostname()">Namen speichern & Neustart</button>
        </div>
        
        <div class="current-time">
            Letzte Aktualisierung: <span id="lastUpdate">-</span>
        </div>
    </div>

    <script>
        let currentStatus = {};
        
        function showMessage(text, type = 'success') {
            const msg = document.createElement('div');
            msg.className = `message ${type}`;
            msg.textContent = text;
            document.body.appendChild(msg);
            setTimeout(() => msg.remove(), 3000);
        }
        
        function updateFanLabel(value) {
            document.getElementById('fanPercent').textContent = value;
            document.getElementById('fanDisplay').textContent = value + '%';
        }
        
        function updateFanMinLabel(value) {
            document.getElementById('fanMinLabel').textContent = value;
            document.getElementById('fanMinDisplay').textContent = value + '%';
        }
        
        function updateFanMaxLabel(value) {
            document.getElementById('fanMaxLabel').textContent = value;
            document.getElementById('fanMaxDisplay').textContent = value + '%';
        }
        
        function updateLightDuration() {
            const onHour = parseInt(document.getElementById('onHour').value) || 0;
            const offHour = parseInt(document.getElementById('offHour').value) || 0;
            let duration;
            
            if (onHour === offHour) {
                duration = 24;
            } else if (offHour > onHour) {
                duration = offHour - onHour;
            } else {
                duration = 24 - onHour + offHour;
            }
            
            document.getElementById('lightDuration').textContent = duration + 'h';
        }
        
        function updateUI(status) {
            currentStatus = status;
            
            // Light status
            const lightStatus = document.getElementById('lightStatus');
            const lightText = document.getElementById('lightText');
            if (status.light) {
                lightStatus.className = 'status-indicator on';
                lightText.textContent = 'AN';
            } else {
                lightStatus.className = 'status-indicator off';
                lightText.textContent = 'AUS';
            }
            
            // Fan values
            document.getElementById('fanValue').textContent = status.fan;
            document.getElementById('fanMin').textContent = status.fanMin;
            document.getElementById('fanMax').textContent = status.fanMax;
            
            // Timer
            document.getElementById('lightOn').textContent = String(status.lightOn).padStart(2, '0');
            document.getElementById('lightOff').textContent = String(status.lightOff).padStart(2, '0');
            
            // Hostname
            document.getElementById('hostnameDisplay').textContent = status.hostname + '.local';
            
            // Phase data
            updatePhaseUI(status);
            
            // Update controls if they haven't been touched
            if (document.activeElement !== document.getElementById('fanSlider')) {
                document.getElementById('fanSlider').value = status.fan;
                updateFanLabel(status.fan);
            }
            if (document.activeElement !== document.getElementById('fanMinSlider')) {
                document.getElementById('fanMinSlider').value = status.fanMin;
                updateFanMinLabel(status.fanMin);
            }
            if (document.activeElement !== document.getElementById('fanMaxSlider')) {
                document.getElementById('fanMaxSlider').value = status.fanMax;
                updateFanMaxLabel(status.fanMax);
            }
            if (document.activeElement !== document.getElementById('onHour')) {
                document.getElementById('onHour').value = status.lightOn;
            }
            if (document.activeElement !== document.getElementById('offHour')) {
                document.getElementById('offHour').value = status.lightOff;
            }
            updateLightDuration();
            if (document.activeElement !== document.getElementById('hostnameInput')) {
                document.getElementById('hostnameInput').value = status.hostname;
            }
            
            // Last update
            const now = new Date();
            document.getElementById('lastUpdate').textContent = now.toLocaleTimeString('de-DE');
        }
        
        async function fetchStatus() {
            try {
                const response = await fetch('/api/status');
                const status = await response.json();
                updateUI(status);
            } catch (error) {
                console.error('Error fetching status:', error);
            }
        }
        
        async function setLight(on) {
            try {
                const response = await fetch(`/api/light?state=${on ? 1 : 0}`);
                const result = await response.json();
                if (result.success) {
                    showMessage(on ? 'Licht eingeschaltet' : 'Licht ausgeschaltet');
                    fetchStatus();
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Schalten', 'error');
            }
        }
        
        async function setFan() {
            const value = document.getElementById('fanSlider').value;
            try {
                const response = await fetch(`/api/fan?speed=${value}`);
                const result = await response.json();
                if (result.success) {
                    showMessage(`Lüfter auf ${value}% eingestellt`);
                    fetchStatus();
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Einstellen', 'error');
            }
        }
        
        async function setFanRange() {
            const min = document.getElementById('fanMinSlider').value;
            const max = document.getElementById('fanMaxSlider').value;
            try {
                const response = await fetch(`/api/fanrange?min=${min}&max=${max}`);
                const result = await response.json();
                if (result.success) {
                    showMessage(`Lüfter Bereich: ${min}%-${max}%`);
                    fetchStatus();
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Speichern', 'error');
            }
        }
        
        async function setLightTimer() {
            const on = document.getElementById('onHour').value;
            const off = document.getElementById('offHour').value;
            try {
                const response = await fetch(`/api/timer?on=${on}&off=${off}`);
                const result = await response.json();
                if (result.success) {
                    showMessage(`Timer: ${String(on).padStart(2, '0')}:00 - ${String(off).padStart(2, '0')}:00`);
                    fetchStatus();
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Speichern', 'error');
            }
        }
        
        async function setHostname() {
            const hostname = document.getElementById('hostnameInput').value.trim();
            if (!hostname) {
                showMessage('Bitte einen Namen eingeben', 'error');
                return;
            }
            if (!/^[a-zA-Z0-9-]+$/.test(hostname)) {
                showMessage('Nur Buchstaben, Zahlen und Bindestriche erlaubt', 'error');
                return;
            }
            try {
                const response = await fetch(`/api/hostname?name=${encodeURIComponent(hostname)}`);
                const result = await response.json();
                if (result.success) {
                    showMessage('Gerät wird neu gestartet...');
                    setTimeout(() => {
                        window.location.href = `http://${hostname}.local`;
                    }, 5000);
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Speichern', 'error');
            }
        }
        
        let currentPhaseStatus = { seedling: {active:false}, veg: {active:false}, flower: {active:false} };
        
        async function setPhase(phase) {
            var phaseToSet = phase;
            if (phase === 'seedling' && currentPhaseStatus.seedling.active) phaseToSet = 'none';
            else if (phase === 'veg' && currentPhaseStatus.veg.active) phaseToSet = 'none';
            else if (phase === 'flower' && currentPhaseStatus.flower.active) phaseToSet = 'none';
            
            try {
                const response = await fetch(`/api/phase?phase=${phaseToSet}`);
                const result = await response.json();
                if (result.success) {
                    const phaseNames = { seedling: 'Seedling', veg: 'Veg', flower: 'Blüte', none: 'Alle abgebrochen' };
                    showMessage(`${phaseNames[phaseToSet] || phaseToSet}`);
                    fetchStatus();
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Setzen der Phase', 'error');
            }
        }
        
        async function resetPhase() {
            const phase = document.getElementById('resetPhaseSelect').value;
            try {
                const response = await fetch(`/api/phasereset?phase=${phase}`);
                const result = await response.json();
                if (result.success) {
                    showMessage(`${phase === 'all' ? 'Alle' : phase} zurückgesetzt`);
                    fetchStatus();
                } else {
                    showMessage('Fehler: ' + result.error, 'error');
                }
            } catch (error) {
                showMessage('Fehler beim Zurücksetzen', 'error');
            }
        }
        
        function updatePhaseUI(status) {
            if (status.seedling !== undefined) {
                document.getElementById('seedlingDays').textContent = status.seedling.days || 0;
                currentPhaseStatus.seedling = { active: status.seedling.active };
                var btnS = document.getElementById('btnSeedling');
                if (status.seedling.active) {
                    btnS.style.background = '#8b5cf6';
                    btnS.style.borderColor = '#a78bfa';
                    btnS.style.boxShadow = '0 0 15px rgba(139,92,246,0.6)';
                } else {
                    btnS.style.background = 'rgba(139,92,246,0.3)';
                    btnS.style.borderColor = 'rgba(139,92,246,0.5)';
                    btnS.style.boxShadow = 'none';
                }
            }
            if (status.veg !== undefined) {
                document.getElementById('vegDays').textContent = status.veg.days || 0;
                currentPhaseStatus.veg = { active: status.veg.active };
                var btnV = document.getElementById('btnVeg');
                if (status.veg.active) {
                    btnV.style.background = '#10b981';
                    btnV.style.borderColor = '#34d399';
                    btnV.style.boxShadow = '0 0 15px rgba(16,185,129,0.6)';
                } else {
                    btnV.style.background = 'rgba(16,185,129,0.3)';
                    btnV.style.borderColor = 'rgba(16,185,129,0.5)';
                    btnV.style.boxShadow = 'none';
                }
            }
            if (status.flower !== undefined) {
                document.getElementById('flowerDays').textContent = status.flower.days || 0;
                currentPhaseStatus.flower = { active: status.flower.active };
                var btnF = document.getElementById('btnFlower');
                if (status.flower.active) {
                    btnF.style.background = '#f59e0b';
                    btnF.style.borderColor = '#fbbf24';
                    btnF.style.boxShadow = '0 0 15px rgba(245,158,11,0.6)';
                } else {
                    btnF.style.background = 'rgba(245,158,11,0.3)';
                    btnF.style.borderColor = 'rgba(245,158,11,0.5)';
                    btnF.style.boxShadow = 'none';
                }
            }
            if (status.totalDays !== undefined) {
                document.getElementById('totalDays').textContent = status.totalDays || 0;
            }
        }
        
        // Initial fetch and periodic updates
        fetchStatus();
        setInterval(fetchStatus, 2000);
    </script>
</body>
</html>
)rawliteral";

// =============================================================================
// SETUP FUNCTION
// =============================================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n");
  Serial.println(
      "╔══════════════════════════════════════════════════════════════╗");
  Serial.println(
      "║                    🌱 GrowTower v3.0                         ║");
  Serial.println(
      "║           Web-Based Cultivation System                       ║");
  Serial.println(
      "╚══════════════════════════════════════════════════════════════╝");
  Serial.println("\n[SYS] System initializing...\n");

  // Load saved settings
  Serial.println("[SYS] Loading configuration from flash...");
  loadSettings();

  // Initialize Light pin
  Serial.println("[SYS] Initializing light control...");
  pinMode(LIGHT_PIN, OUTPUT);
  setLight(false);

  // Initialize Fan PWM
  Serial.println("[SYS] Initializing fan PWM...");
  initPWM();
  setFan(currentFanSpeed);

  // Initialize WiFi
  Serial.println("[SYS] Initializing WiFi...");
  initWiFi();

  // Initialize OTA
  Serial.println("[SYS] Initializing OTA...");
  initOTA();

  // Initialize Web Server
  Serial.println("[SYS] Initializing Web Server...");
  initWebServer();

  Serial.println("\n[SYS] Initialization complete!");
  printStatus();
  Serial.println("\n[SYS] Ready. Access the controller at: http://" +
                 String(currentHostname) + ".local\n");
}

// =============================================================================
// MAIN LOOP
// =============================================================================
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

// =============================================================================
// PWM INITIALIZATION
// =============================================================================
void initPWM() {
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
    Serial.println("[WIFI] Connection failed!");
  }
#else
  Serial.println("[WIFI] Error: WIFI_SSID not defined in secrets.h");
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
  if (!MDNS.begin(currentHostname)) {
    Serial.println("[OTA] Error setting up mDNS responder!");
    return;
  }
  Serial.printf("[OTA] mDNS responder started: %s.local\n", currentHostname);

  // Add web server service to mDNS
  MDNS.addService("http", "tcp", 80);

  // Configure ArduinoOTA
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
}

// =============================================================================
// WEB SERVER INITIALIZATION
// =============================================================================
void initWebServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WEB] WiFi not connected, Web Server disabled");
    return;
  }

  // Main page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // API: Get status
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "application/json", getStatusJSON());
  });

  // API: Set light
  server.on("/api/light", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("state")) {
      int state = request->getParam("state")->value().toInt();
      setLight(state == 1);
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json",
                    "{\"success\":false,\"error\":\"Missing state param\"}");
    }
  });

  // API: Set fan speed
  server.on("/api/fan", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("speed")) {
      int speed = request->getParam("speed")->value().toInt();
      setFan(speed);
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json",
                    "{\"success\":false,\"error\":\"Missing speed param\"}");
    }
  });

  // API: Set fan range
  server.on("/api/fanrange", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("min") && request->hasParam("max")) {
      int min = request->getParam("min")->value().toInt();
      int max = request->getParam("max")->value().toInt();
      saveFanMin(min);
      saveFanMax(max);
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json",
                    "{\"success\":false,\"error\":\"Missing params\"}");
    }
  });

  // API: Set light timer
  server.on("/api/timer", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("on") && request->hasParam("off")) {
      int on = request->getParam("on")->value().toInt();
      int off = request->getParam("off")->value().toInt();
      saveLightOnHour(on);
      saveLightOffHour(off);
      request->send(200, "application/json", "{\"success\":true}");
    } else {
      request->send(400, "application/json",
                    "{\"success\":false,\"error\":\"Missing params\"}");
    }
  });

  // API: Set hostname
  server.on("/api/hostname", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("name")) {
      String name = request->getParam("name")->value();
      saveHostname(name.c_str());
      request->send(200, "application/json",
                    "{\"success\":true,\"message\":\"Rebooting...\"}");
      delay(1000);
      ESP.restart();
    } else {
      request->send(400, "application/json",
                    "{\"success\":false,\"error\":\"Missing name param\"}");
    }
  });

  // API: Set phase
  server.on("/api/phase", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("phase")) {
      String phaseStr = request->getParam("phase")->value();
      phaseStr.toLowerCase();
      
      if (phaseStr == "seedling") {
        setPhase(PHASE_SEEDLING);
        request->send(200, "application/json", "{\"success\":true,\"phase\":\"seedling\"}");
      } else if (phaseStr == "veg") {
        setPhase(PHASE_VEG);
        request->send(200, "application/json", "{\"success\":true,\"phase\":\"veg\"}");
      } else if (phaseStr == "flower") {
        setPhase(PHASE_FLOWER);
        request->send(200, "application/json", "{\"success\":true,\"phase\":\"flower\"}");
      } else if (phaseStr == "none" || phaseStr == "reset") {
        setPhase(PHASE_NONE);
        request->send(200, "application/json", "{\"success\":true,\"phase\":\"none\"}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid phase\"}");
      }
    } else {
      request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing phase param\"}");
    }
  });

  // API: Get phase info
  server.on("/api/phaseinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{" + getPhaseJSON() + "}";
    request->send(200, "application/json", json);
  });

  // API: Reset specific phase
  server.on("/api/phasereset", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("phase")) {
      String phaseStr = request->getParam("phase")->value();
      phaseStr.toLowerCase();
      
      if (phaseStr == "seedling") {
        resetPhase(PHASE_SEEDLING);
        request->send(200, "application/json", "{\"success\":true}");
      } else if (phaseStr == "veg") {
        resetPhase(PHASE_VEG);
        request->send(200, "application/json", "{\"success\":true}");
      } else if (phaseStr == "flower") {
        resetPhase(PHASE_FLOWER);
        request->send(200, "application/json", "{\"success\":true}");
      } else if (phaseStr == "all") {
        resetPhase(PHASE_SEEDLING);
        resetPhase(PHASE_VEG);
        resetPhase(PHASE_FLOWER);
        request->send(200, "application/json", "{\"success\":true}");
      } else {
        request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid phase\"}");
      }
    } else {
      request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing phase param\"}");
    }
  });

  // 404 handler
  server.onNotFound(
      [](AsyncWebServerRequest *request) { request->send(404, "text/plain", "Not Found"); });

  server.begin();
  Serial.println("[WEB] Web server started on port 80");
}

// =============================================================================
// STATUS JSON GENERATOR
// =============================================================================
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

// =============================================================================
// SETTINGS MANAGEMENT
// =============================================================================
void loadSettings() {
  preferences.begin("growtower", true);

  fanMinPercent = preferences.getInt("fanMin", 0);
  fanMaxPercent = preferences.getInt("fanMax", 100);
  lightOnHour = preferences.getInt("onHour", 18);
  lightOffHour = preferences.getInt("offHour", 14);

  // Load hostname with default fallback
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
  if (minVal < 0)
    minVal = 0;
  if (minVal > 100)
    minVal = 100;

  fanMinPercent = minVal;
  preferences.begin("growtower", false);
  preferences.putInt("fanMin", fanMinPercent);
  preferences.end();

  Serial.printf("[CONFIG] Fan Min saved: %d%%\n", fanMinPercent);
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
  checkTimer();
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
}

// =============================================================================
// FAN CONTROL
// =============================================================================
void setFan(int percent) {
  if (percent < 0)
    percent = 0;
  if (percent > 100)
    percent = 100;

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

// =============================================================================
// TIMER LOGIC
// =============================================================================
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
  Serial.printf("  Light Timer:  %02d:00 - %02d:00\n", lightOnHour, lightOffHour);
  Serial.printf("  Hostname:     %s.local\n", currentHostname);
  Serial.printf("  IP Address:   %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("  Web Server:   %s\n",
                WiFi.status() == WL_CONNECTED ? "Running ✓" : "Disabled ✗");
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

// =============================================================================
// PHASE TRACKING FUNCTIONS
// =============================================================================
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
