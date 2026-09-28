# ESP32 Wi‑Fi термометр → Zabbix 5.0

Прошивка для **ESP32-S3-DevKitC-1** (модуль **N16R8**: 16 MB flash, 8 MB PSRAM), **DHT22** на **GPIO4**, push‑метрики в **Zabbix 5.0** (trapper, протокол **ZBXD**).

**Режим по умолчанию — локальная разработка** на вашем ПК (PlatformIO + USB **COM**). Пошагово: **[docs/LOCAL_DEVELOPMENT.md](docs/LOCAL_DEVELOPMENT.md)**.

---

## Быстрый старт (локально)

```bash
git clone https://github.com/devishin/dav.git
cd dav/esp32-thermometer
cp include/secrets.example.h include/secrets.h   # Wi‑Fi; не коммитить
pio device list
pio run -e hw-test -t upload && pio device monitor   # сначала DHT
pio run -e release -t upload && pio device monitor # Wi‑Fi + Zabbix
```

Откройте в Cursor/VS Code папку **`esp32-thermometer`** (не корень `dav`, если не настроен workspace).

---

## Подключение DHT22

| DHT22 | ESP32-S3 |
|-------|----------|
| VCC | 3V3 |
| DATA | GPIO4 |
| GND | GND |

Прошивка и Serial Monitor — разъём **COM** (USB‑UART), 115200 baud.

---

## Конфигурация

| Файл | Содержимое |
|------|------------|
| `include/secrets.h` | `WIFI_SSID`, `WIFI_PASSWORD` (локально, в `.gitignore`) |
| `include/config.h` | `DEVICE_NAME` (`thermo-01`), `FIRMWARE_VERSION`, ключи `thermometer.*`, `ZABBIX_SERVER`, интервалы |

**Host name в Zabbix** должен **точно** совпадать с **`DEVICE_NAME`**.

---

## Сборка (PlatformIO)

| Environment | Назначение |
|-------------|------------|
| `hw-test` | DHT + Serial, интервал ~3 с, без Wi‑Fi/Zabbix |
| `release` | DHT (10 с) + Wi‑Fi + Zabbix batch (60 с) |

`platformio.ini`: flash 16 MB, PSRAM OPI для **N16R8**.

---

## Метрики Zabbix

Ключи: `thermometer.temperature`, `thermometer.humidity`, `thermometer.rssi`, `thermometer.uptime`, `thermometer.sensor_ok`, `thermometer.firmware`.

Шаблон: **[zabbix/Template_ESP32_Thermometer.xml](zabbix/Template_ESP32_Thermometer.xml)** (дубликат в корне репо: `../zabbix/`).

---

## Структура

```
esp32-thermometer/
  platformio.ini
  include/config.h, secrets.example.h
  src/main.cpp, sensor.*, wifi_manager.*, zabbix_sender.*
  zabbix/Template_ESP32_Thermometer.xml
  docs/LOCAL_DEVELOPMENT.md
```

---

## Справка

- Облачная проверка без железа (только `pio run`): [docs/PHASE-A-VM-BUILD.md](docs/PHASE-A-VM-BUILD.md)
