# monitor_v2 backend contract (firmware notes)

This document summarizes how firmware should send measurements to **monitor_v2**.
It is written from the firmware perspective and is meant to stay short and practical.

> Canonical source of truth is the backend repo. If backend changes, update this file together with firmware.

---

## Ingest endpoint

Firmware sends measurements via HTTP:

- **Method:** `POST`
- **Path:** `/api/ingest/{device_uuid}`
- **Auth header:** `X-Device-Secret: <device_secret>`
- **Body:** JSON
- **Content-Type:** `application/json`

### Example (curl)

```bash
curl -X POST "https://monitor.example.com/api/ingest/DEVICE_UUID" \
  -H "Content-Type: application/json" \
  -H "X-Device-Secret: DEVICE_SECRET" \
  -d '{"temp":21.5,"temp2":20.9,"rssi":-61,"fw":"temp_x2@1.0.0"}'
```

---

## Payload formats

### 1) Flat payload (recommended for most devices)

Send one JSON object with metric keys:

```json
{
  "gate": 1,
  "temp": 21.5,
  "temp2": 20.9,
  "voltage": 24.7,
  "rssi": -61,
  "fw": "gate_with_temp@1.0.0",
  "recorded_at": "2026-02-18T12:00:00Z"
}
```

Notes:
- `recorded_at` is optional; if omitted, backend uses server time.
- `fw` and `rssi` are optional but recommended for debugging/monitoring.
- Use numeric types for numeric metrics (avoid strings).

### 2) Batch payload (future/offline buffering)

If firmware buffers readings (offline mode), use a batch structure:

```json
{
  "batch_id": "boot_1708251200",
  "samples": [
    {
      "recorded_at": "2026-02-18T12:00:00Z",
      "metrics": { "temp": 21.5, "temp2": 20.9 }
    },
    {
      "recorded_at": "2026-02-18T12:01:00Z",
      "metrics": { "temp": 21.6, "temp2": 21.0 }
    }
  ]
}
```

---

## Metric keys (naming conventions)

Backend validates metric keys against device channel configuration.
Send **only** keys that are enabled for that device.

Recommended canonical keys:
- `gate` — gate state (0/1)
- `temp` — temperature sensor #1 (float)
- `temp2` — temperature sensor #2 (float)
- `temp3` — temperature sensor #3 (float)
- `temp4` — temperature sensor #4 (float)
- `voltage` — DC voltage (float)

Metadata keys (optional):
- `fw` — firmware identifier (e.g. `temp_x2@1.0.3`)
- `rssi` — WiFi RSSI in dBm (e.g. `-63`)
- `uptime_s` — uptime in seconds
- `heap` — free heap (bytes), optional for debugging

**Important:** if backend expects `temp` (not `temp1`), sending `temp1` will be rejected as an unknown key.

---

## Suggested device profiles

- **Gate without temperature**
  - sends: `gate`, `rssi`, `fw`
  - send policy: on change + heartbeat (e.g. every 10–30 min)

- **Gate with temperature**
  - sends: `gate`, `temp`, `temp2`, `rssi`, `fw`

- **Two temperature sensors**
  - sends: `temp`, `temp2`, `rssi`, `fw`

- **Four temperature sensors**
  - sends: `temp`, `temp2`, `temp3`, `temp4`, `rssi`, `fw`

- **Actual Voltage DC (0–26V)**
  - sends: `voltage`, `rssi`, `fw`

---

## Response codes (practical expectations)

Firmware should treat these as typical outcomes:

- `200/201` — OK (accepted)
- `401` — unauthorized (wrong UUID/secret or header missing)
- `422` — validation error (unknown metric key, wrong data types)
- `429` — rate limited (back off / reduce frequency)
- `5xx` — backend error (retry with exponential backoff)

Recommended behavior:
- On `401`: stop spamming; require re-provisioning.
- On `422`: log payload + status; fix firmware config (keys/types).
- On `5xx`: retry with backoff.

---

## Firmware implementation notes

- Use `ArduinoJson` to build payloads reliably.
- Keep TLS simple:
  - for ESP8266/ESP32 you may use `WiFiClientSecure`.
  - if you use `setInsecure()`, document it and consider pinning cert later.
- Avoid sending too frequently:
  - gate sensor: send on state change + heartbeat
  - temperature: e.g. every 1–5 minutes (or less, depending on needs)
  - voltage: e.g. every 5–30 seconds if you need graphs, otherwise slower

---

## Provisioning reminders

Each device needs:
- `device_uuid`
- `device_secret`
- backend base URL

Store them in EEPROM (ESP8266) / NVS Preferences (ESP32) and allow re-provisioning.
