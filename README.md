# ANDER-IOT-FW

Firmware repository for my ESP8266 / ESP32 IoT devices (gate sensors, temperature probes, DC voltage monitoring).
The goal is a **consistent, reusable codebase** with shared modules and a standardized data contract to the `monitor_v2` backend.

> This repository is private, but **secrets are never stored in source code** (WiFi credentials, Telegram tokens, device secrets, etc.).

---

## What’s inside

- **`legacy/`** – older Arduino sketches (`.ino`) kept for reference and migration.
- **`core/`** – shared logic to be extracted and reused across devices (networking, OTA, provisioning, sensor mapping).
- **`devices/`** – target structure for “production” firmwares (written against `monitor_v2` conventions).
- **`docs/`** – documentation (backend contract, payload keys, OTA notes, wiring).
- **`tools/`** – helpers (e.g., DS18B20 scanner, provisioning utilities).

Current focus: migrate the most useful `legacy/` sketches into `devices/` and standardize the transport to `monitor_v2`.

---

## Supported boards

- **ESP8266** (NodeMCU / Wemos D1 mini and similar)
- **ESP32** (Espressif DevKit variants and similar)

---

## Backend: `monitor_v2`

New/target devices are written to send metrics to the `monitor_v2` ingestion API (UUID-based endpoint + secret header).
Backend request format, supported metric keys, and examples live here:

- `docs/backend-monitor_v2.md`

---

## Arduino IDE setup

### 1) Install board packages (Boards Manager)

In Arduino IDE go to:
**File → Preferences → Additional Boards Manager URLs**  
Add these URLs (one per line or comma-separated):

```txt
ESP8266:
https://arduino.esp8266.com/stable/package_esp8266com_index.json

ESP32 (Espressif):
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

Then open:
**Tools → Board → Boards Manager** and install:

- **ESP8266 by ESP8266 Community**
- **esp32 by Espressif Systems**

### 2) Libraries (Library Manager)

Install what your sketch needs. Common dependencies across devices:

**DS18B20 temperature sensors**
- `OneWire`
- `DallasTemperature`

**JSON + HTTP**
- `ArduinoJson`
- ESP8266: built-in `ESP8266HTTPClient`, `WiFiClientSecure`
- ESP32: built-in `HTTPClient`, `WiFiClientSecure`

**Telegram (legacy / optional notifications)**
- `Universal Arduino Telegram Bot` (often listed as “UniversalArduinoTelegramBot”)
  - requires `ArduinoJson`

**Provisioning / storage (platform built-ins)**
- ESP8266: `EEPROM`
- ESP32: `Preferences`
- Web server (if used): `ESP8266WebServer` (ESP8266) / `WebServer` (ESP32)

> Note: some modules are bundled with the board packages. Only install extra libraries if compilation complains about missing headers.

---

## Secrets & configuration (important)

Do **not** commit secrets into Git.

Recommended pattern:
- keep a local `secrets.h`
- commit a template `secrets.example.h`
- ignore `secrets.h` via `.gitignore`

Example structure (idea):

- `secrets.example.h` (committed)
- `secrets.h` (local, ignored)

---

## Repository layout notes (Arduino IDE vs PlatformIO)

### Arduino IDE (current)
Arduino sketches work best when the main `.ino` is in the **root of the device folder**.
Adding `README.md`, wiring images, and helper headers alongside the `.ino` is safe.

Example:
```
legacy/ESP_Temperature_HTTPS/
  ESP_Temperature_HTTPS.ino
  README.md
  assets/
```

Avoid moving `.ino` into `src/` if you plan to compile with Arduino IDE (it’s not the standard sketch layout).

### PlatformIO (planned migration)
PlatformIO provides a cleaner multi-device workflow:
- multiple environments (ESP8266/ESP32) in one repo
- reproducible dependencies (`lib_deps`)
- CI-friendly builds producing `.bin` artifacts
- natural `src/` / `include/` layout
- easier OTA distribution via GitHub Releases/Pages

This repo is structured to migrate smoothly to PlatformIO later (devices + shared `core`).

---

## Technical TODOs / reminders

- Standardize metric keys to match backend channels (typical set: `temp`, `temp2`, `temp3`, `temp4`, `gate`, `voltage`).
- Implement shared `core` modules:
  - `MonitorV2Client` (ingest requests, retries, status codes)
  - `Provisioning` (WiFi + device UUID/secret storage)
  - `OTAUpdater` (manifest/version check + scheduled updates)
- DS18B20: remove hardcoded sensor addresses in code.
  - goal: store address→channel mapping in EEPROM/NVS with a simple “wizard” (serial or captive portal).

---

## License / usage

Internal project repository for my devices. Not intended as a public library (yet).
