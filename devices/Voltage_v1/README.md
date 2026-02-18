# Voltagev1 (ESP8266 + INA219)

Simple firmware for **ESP8266 NodeMCU (LoLin/Wemos V3)** that reads **bus/load voltage** from an **INA219** module
and sends it to the `monitor_v2` backend via the ingest API.

---

## Hardware

- ESP8266 NodeMCU (LoLin / Wemos / NodeMCU v3)
- INA219 DC Current Monitor (I2C), bus voltage **0–26V**
  - measured circuit current up to ~3.2A (board dependent)
  - module logic supply: **3.3V (recommended)**

> Note: `monitor_v2` currently stores `voltage` only. INA219 can also read current, but backend would need a `current` measurement type to store it.

---

## Wiring (NodeMCU → INA219)

### I2C lines
- **D2 (GPIO4)** → **SDA**
- **D1 (GPIO5)** → **SCL**
- **3V3** → **VCC**
- **GND** → **GND**

### Measurement terminals (high-side)
- **VIN+** → supply positive (before shunt)
- **VIN-** → load positive (after shunt)

**Important:** The INA219 module **GND must be connected** to the measured circuit ground (negative),
otherwise bus/load voltage will be wrong.

Default I2C address is typically **0x40**.

---

## Backend ingest

Firmware sends a **flat JSON** payload (do NOT use `{key,value,unit}` form):

- `POST http://monitor.inspiroo.pl/api/ingest/<DEVICE_UUID>`
- Header: `X-Device-Secret: <DEVICE_SECRET>`
- JSON body example:

```json
{
  "voltage": 12.34,
  "fw": "Voltagev1@0.1.0",
  "rssi": -60
}
```

---

## Configuration (secrets)

Copy `include/secrets.example.h` to `include/secrets.h` and fill it in.

**Never commit `secrets.h`.** Add to your root `.gitignore`:

```gitignore
# PlatformIO device secrets
devices/**/include/secrets.h
```

---

## Build / Upload (PlatformIO)

From VSCode (PlatformIO extension):
- **Build**
- **Upload**
- **Monitor** (Serial)

CLI equivalents:

```bash
pio run
pio run -t upload
pio device monitor
```

If upload port is not detected automatically, set it in `platformio.ini`:

```ini
upload_port = COM5
monitor_port = COM5
```

---

## Notes

- Reporting interval is controlled by `REPORT_INTERVAL_MS` in `src/main.cpp`.
- Backend has throttling; don’t send too frequently (30/min is safe).
