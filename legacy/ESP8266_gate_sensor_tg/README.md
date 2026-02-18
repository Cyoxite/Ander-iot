# Czujka_hala_ESP32_ESP8266_tg_updates__wifi_ap_eeprom (legacy)

**Status:** Legacy / prototype (needs refactor before reuse)  
**Purpose:** Gate sensor sketch with Telegram notifications + OTA update concept + WiFi AP provisioning attempt + EEPROM storage.

This sketch was an experimental “all-in-one” version intended to support:
- gate open/close detection
- Telegram notifications (bot)
- OTA updates via HTTP
- captive portal / AP mode for WiFi setup
- config persistence via EEPROM

---

## Notes on platform support

Despite the name, the current implementation is **ESP8266-centric**:
- uses `ESP8266WebServer`, `ESP8266WiFiMulti`, `ESPhttpUpdate`, etc.
- it will not compile for ESP32 without splitting platform-specific code paths.

If you want a truly dual-target sketch (ESP8266 + ESP32), migrate shared logic into `core/`
and implement platform adapters for:
- WiFi / web server
- EEPROM/NVS storage
- OTA update mechanism

---

## Known issues / pitfalls (why this is legacy)

This sketch is kept for reference, but it has several problems that prevent “production” use as-is:

- **Provisioning not wired properly**
  - AP portal setup exists, but the flow is incomplete.
  - `server.handleClient()` is missing in the main loop, so HTTP requests won’t be handled.
- **Logic bugs**
  - Several conditions use assignment (`=`) instead of comparison (`==`) which breaks counters/flow.
- **OTA update check is too aggressive**
  - Update logic is triggered too often (should be scheduled, e.g. on boot + periodic interval).
- **Hardcoded / manual configuration**
  - Intended to store config in EEPROM, but needs a clean contract and a reset/reprovision mode.

---

## Dependencies

Typical libraries used by this sketch:
- `Universal Arduino Telegram Bot` (Telegram integration)
- `ArduinoJson` (payload parsing for TG lib)
- ESP8266 built-ins: `ESP8266WiFi`, `ESP8266WebServer`, `ESP8266HTTPClient`, `ESPhttpUpdate`, `EEPROM`

> Secrets (WiFi, TG tokens) must be provided locally and must NOT be committed.

---

## Recommended migration path

Target state is a clean firmware under `devices/` written against `monitor_v2`:
- `core/Provisioning` (WiFi + UUID/secret storage, AP captive portal)
- `core/OTAUpdater` (manifest-based update, scheduled checks)
- `core/MonitorV2Client` (ingest API client)

Then implement:
- `devices/gate_without_temp`
- `devices/gate_with_temp`

This sketch should remain in `legacy/` as a reference only.
