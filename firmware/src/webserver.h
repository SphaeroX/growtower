#ifndef WEBSERVER_ROUTES_H
#define WEBSERVER_ROUTES_H

#include <ESPAsyncWebServer.h>
#include "frontend.h"
#include "state.h"

extern AsyncWebServer server;

void initWebServer() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WEB] WiFi not connected, Web Server disabled");
        return;
    }

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", getStatusJSON());
    });

    server.on("/api/light", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("state")) {
            int state = request->getParam("state")->value().toInt();
            setLight(state == 1);
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing state param\"}");
        }
    });

    server.on("/api/fan", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("speed")) {
            int speed = request->getParam("speed")->value().toInt();
            setFan(speed);
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing speed param\"}");
        }
    });

    server.on("/api/fanrange", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("min") && request->hasParam("max")) {
            int min = request->getParam("min")->value().toInt();
            int max = request->getParam("max")->value().toInt();
            saveFanMin(min);
            saveFanMax(max);
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing params\"}");
        }
    });

    server.on("/api/timer", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("on") && request->hasParam("off")) {
            int on = request->getParam("on")->value().toInt();
            int off = request->getParam("off")->value().toInt();
            saveLightOnHour(on);
            saveLightOffHour(off);
            request->send(200, "application/json", "{\"success\":true}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing params\"}");
        }
    });

    server.on("/api/timerenable", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("enabled")) {
            int enabled = request->getParam("enabled")->value().toInt();
            saveTimerEnabled(enabled == 1);
            request->send(200, "application/json", "{\"success\":true,\"enabled\":" + String(timerEnabled ? "true" : "false") + "}");
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing enabled param\"}");
        }
    });

    server.on("/api/reset", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "application/json", "{\"success\":true,\"message\":\"Resetting to factory defaults...\"}");
        delay(500);
        resetAllSettings();
    });

    server.on("/api/hostname", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("name")) {
            String name = request->getParam("name")->value();
            saveHostname(name.c_str());
            request->send(200, "application/json", "{\"success\":true,\"message\":\"Rebooting...\"}");
            delay(1000);
            ESP.restart();
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing name param\"}");
        }
    });

    server.on("/api/phase", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("phase")) {
            String phaseStr = request->getParam("phase")->value();
            phaseStr.toLowerCase();
            if (phaseStr == "seedling") { setPhase(PHASE_SEEDLING); request->send(200, "application/json", "{\"success\":true,\"phase\":\"seedling\"}"); }
            else if (phaseStr == "veg") { setPhase(PHASE_VEG); request->send(200, "application/json", "{\"success\":true,\"phase\":\"veg\"}"); }
            else if (phaseStr == "flower") { setPhase(PHASE_FLOWER); request->send(200, "application/json", "{\"success\":true,\"phase\":\"flower\"}"); }
            else if (phaseStr == "none" || phaseStr == "reset") { setPhase(PHASE_NONE); request->send(200, "application/json", "{\"success\":true,\"phase\":\"none\"}"); }
            else { request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid phase\"}"); }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing phase param\"}");
        }
    });

    server.on("/api/phaseinfo", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{" + getPhaseJSON() + "}";
        request->send(200, "application/json", json);
    });

    server.on("/api/phasereset", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("phase")) {
            String phaseStr = request->getParam("phase")->value();
            phaseStr.toLowerCase();
            if (phaseStr == "seedling") { resetPhase(PHASE_SEEDLING); request->send(200, "application/json", "{\"success\":true}"); }
            else if (phaseStr == "veg") { resetPhase(PHASE_VEG); request->send(200, "application/json", "{\"success\":true}"); }
            else if (phaseStr == "flower") { resetPhase(PHASE_FLOWER); request->send(200, "application/json", "{\"success\":true}"); }
            else if (phaseStr == "all") { resetPhase(PHASE_SEEDLING); resetPhase(PHASE_VEG); resetPhase(PHASE_FLOWER); request->send(200, "application/json", "{\"success\":true}"); }
            else { request->send(400, "application/json", "{\"success\":false,\"error\":\"Invalid phase\"}"); }
        } else {
            request->send(400, "application/json", "{\"success\":false,\"error\":\"Missing phase param\"}");
        }
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(404, "text/plain", "Not Found");
    });

    server.begin();
    Serial.println("[WEB] Web server started on port 80");
}

#endif
