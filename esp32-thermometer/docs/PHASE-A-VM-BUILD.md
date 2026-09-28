# Phase A — cloud VM build (PlatformIO)

This phase was executed on a **Cursor Cloud Agent VM** without a physical ESP32-S3 or serial adapter.

## Verified on VM

| Step | Result |
|------|--------|
| PlatformIO install | OK (`platformio` 6.x via pip) |
| `espressif32` platform | OK (7.x) |
| `pio run -e release -e hw-test` | **BUILD PASS** (2026-09-28, see build log) |
| `platformio.ini` N16R8 overrides | Documented (16 MB flash, OPI PSRAM) |

## NOT VERIFIED on VM

| Item | Reason |
|------|--------|
| USB COM port / upload | No device attached |
| Serial monitor / boot log | No COM |
| DHT22 on GPIO4 | No sensor |
| Wi-Fi association | No hardware on VM LAN |
| Zabbix trapper to `192.168.11.52:10051` | Server not reachable / no live sender |
| Template import on Zabbix 5.0 | Not executed on this VM |
| Triggers / graphs in production | Requires live data |

## Next phase (user laptop)

1. Copy `include/secrets.example.h` → `include/secrets.h` locally.
2. `pio run -e hw-test -t upload && pio device monitor` — confirm DHT readings.
3. `pio run -e release -t upload` — confirm Wi-Fi + Zabbix items updating.
