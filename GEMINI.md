# GEMINI.md - GrowTower Project Context

## Project Overview
GrowTower ist ein minimalistisches, vertikales Anbausystem, das speziell für den Heimanbau von Cannabis entwickelt wurde. Es kombiniert 3D-Druck, eingebettete Elektronik (ESP32) und ein semi-hydroponisches Bewässerungssystem.

### Key Features
- **Web-Steuerung:** Integrierter Webserver auf dem ESP32 zur Steuerung von Licht und Lüfter über `http://growtower.local`.
- **Automatisierung:** Konfigurierbarer 24h-Licht-Timer und PWM-Lüftersteuerung zur Temperatur- und Geruchsregulierung.
- **Geruchskontrolle:** Integriertes Aktivkohlefiltersystem mit einem Arctic P14 PC-Lüfter.
- **Modularität:** Stapelbares Design aus 3D-gedruckten PETG-Komponenten.
- **Trocknungsmodus:** Optionales Modul zur Nutzung des Towers als geruchsfreie Trocknungskammer.

---

## Technical Stack

### Firmware (C++/Arduino)
- **Framework:** Arduino Framework auf **Seeed Studio XIAO ESP32-C3**.
- **Build System:** PlatformIO.
- **Libraries:**
  - `ESPAsyncWebServer` & `AsyncTCP`: Für das Web-Interface und die REST API.
  - `ArduinoOTA`: Für kabellose Firmware-Updates.
  - `Preferences`: Zur dauerhaften Speicherung von Einstellungen (Timer, Lüfterkurven, Logbuch).
- **Konfiguration:** 
  - `firmware/src/config.h`: Pin-Belegung (`D1` Licht, `D2` Lüfter) und PWM-Parameter.
  - `firmware/src/secrets.h`: Enthält WiFi-Zugangsdaten (wird automatisch aus `.env` generiert).

### 3D-Druck & Hardware
- **Material:** Vorzugsweise **PETG Weiß** (innen für Reflexion) und **Schwarz** (für Filter und Wurzelbereich zur Algenvermeidung).
- **Elektronik:** 24V Netzteil, Step-Down Module für den ESP32, Relais für das Licht.
- **Lüfter:** Arctic P14 PWM PST (140mm), gesteuert via 25kHz PWM.

---

## Development Workflow

### Building and Running
- **Firmware kompilieren:** `pio run` (im Verzeichnis `firmware/`).
- **Upload via USB:** `pio run -t upload`.
- **Upload via OTA:** `pio run -t upload --upload-port growtower.local`.
- **Serieller Monitor:** `pio device monitor`.

### Configuration
1. Kopiere `firmware/.env.example` nach `firmware/.env` und trage deine WiFi-Daten ein.
2. Das Skript `generate_secrets.py` erstellt beim Build automatisch die `secrets.h`.

---

## Project Structure
- `firmware/`: Quellcode, PlatformIO-Konfiguration und Build-Skripte.
- `stls/`: 3D-Modelle und detaillierte Druckanweisungen (siehe `stls/README.md`).
- `docs/`: Web-Dokumentation.
- `images/`: Projektfotos und Slicer-Screenshots.
- `BOM.md`: Stückliste aller benötigten Komponenten.

---

## Conventions & Notes
- **Git Workflow:** Nach jeder Änderung (Code, Dokumentation oder Konfiguration) muss zwingend ein Git-Commit mit einer aussagekräftigen Nachricht erstellt werden.
- **Firmware Validierung:** Nach jeder Änderung im Verzeichnis `firmware/` muss ein Test-Build mit `pio run` durchgeführt werden, um sicherzustellen, dass der Code weiterhin fehlerfrei kompiliert.
- **Lichtdichtheit:** Die weiße Außenschale muss von außen lichtdicht gemacht werden (z. B. mit PlastiDip), um die Photoperiode der Pflanzen nicht zu stören.
- **Bewässerung:** Immer von unten (Bottom Watering), um Nährstoffschocks durch Salzansammlungen an der Oberfläche zu vermeiden.
- **Sicherheit:** Niemals Passwörter oder API-Keys in `secrets.h` committen; stattdessen `.env` nutzen.
- **Hardware-Varianz:** Die BOM nennt teils den ESP32-C6, die Firmware ist aktuell auf den **ESP32-C3** (XIAO) konfiguriert. Bei Hardware-Wechsel die `platformio.ini` anpassen.
