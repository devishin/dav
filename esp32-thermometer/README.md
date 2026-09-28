# ESP32 Wi‑Fi термометр → Zabbix 5.0 / ESP32 Wi‑Fi thermometer → Zabbix 5.0

Прошивка для **ESP32-S3-DevKitC-1** с модулем **ESP32-S3-WROOM-1-N16R8** (16 MB flash, 8 MB OPI PSRAM), датчик **DHT22** на **GPIO4**, отправка метрик на Zabbix **5.0** протоколом **Zabbix sender (ZBXD)**.

Firmware for **ESP32-S3-DevKitC-1** with **N16R8** module, **DHT22** on **GPIO4**, metrics to **Zabbix 5.0** via **ZBXD** trapper.

## Требования / Requirements

- [PlatformIO](https://platformio.org/)
- USB‑кабель и драйвер для ESP32-S3
- Zabbix 5.0 с включённым trapper (порт **10051** по умолчанию)

## Быстрый старт / Quick start

```bash
cd esp32-thermometer
cp include/secrets.example.h include/secrets.h   # локально, не коммитить / local only, do not commit
# отредактируйте Wi‑Fi и ZABBIX_* / edit Wi‑Fi and ZABBIX_*
pio run -e release
pio run -e release -t upload
pio device monitor
```

**Важно:** файл `include/secrets.h` в `.gitignore`. В репозитории только `secrets.example.h`.

## Секреты / Secrets

| Макрос | Назначение |
|--------|------------|
| `WIFI_SSID`, `WIFI_PASS` | Wi‑Fi |
| `ZABBIX_SERVER` | IP/host trapper (пример: `192.168.11.52`) |
| `ZABBIX_PORT` | Обычно `10051` |
| `ZABBIX_HOST` | Имя хоста в Zabbix (макрос шаблона `{$ESP32_HOST}`) |

## Платформа / Board (N16R8)

В PlatformIO id платы `esp32-s3-devkitc-1` по умолчанию соответствует **N8 без PSRAM**. Для модуля **N16R8** в `platformio.ini` задано:

- `board_build.flash_size = 16MB`
- `board_build.partitions = default_16MB.csv`
- `board_build.arduino.memory_type = qio_opi`
- `board_build.psram_type = opi`

Проверьте модуль на вашей плате (маркировка **N16R8** на WROOM).

## Два режима сборки / Build environments

| Environment | Назначение |
|-------------|------------|
| `hw-test` | Фаза 1: только DHT + Serial (GPIO4), без Wi‑Fi/Zabbix |
| `release` | Полное приложение: DHT + Wi‑Fi + Zabbix batch |

```bash
pio run -e hw-test -t upload    # сначала железо / hardware first
pio run -e release -t upload    # прод / production
```

## Логика опроса / Telemetry timing

| Действие | Интервал | Константа |
|----------|----------|-----------|
| Чтение DHT | **10 с** | `DHT_READ_INTERVAL_MS` |
| Пакет в Zabbix | **60 с** | `ZABBIX_SEND_INTERVAL_MS` |

DHT опрашивается чаще, чем отправка в Zabbix: в мониторинг уходит **последнее успешное** значение на момент минутного batch. При ошибках DHT счётчик растёт; после **`DHT_FAILURE_LIMIT` (3)** подряд в Zabbix уходит `esp32.dht.status=1` (FAILED).

Главный цикл **не блокирует** надолго: Wi‑Fi переподключается с таймаутом попытки и паузой между попытками (`wifi_manager`).

## Zabbix

- Шаблон: [`../zabbix/Template_ESP32_Thermometer.xml`](../zabbix/Template_ESP32_Thermometer.xml)
- Ключи: `esp32.temperature`, `esp32.humidity`, `esp32.wifi.rssi`, `esp32.dht.status`, `esp32.uptime`
- Импорт в Zabbix 5.0 → привязать шаблон к хосту с именем как `ZABBIX_HOST`

## Облачная сборка / Cloud build (Phase A)

Сборка на VM без ESP32: см. [`docs/PHASE-A-VM-BUILD.md`](docs/PHASE-A-VM-BUILD.md).

**NOT VERIFIED без железа:** upload, COM, DHT, Wi‑Fi, live Zabbix.

## Структура проекта / Layout

```
esp32-thermometer/
  platformio.ini
  include/config.h, secrets.example.h
  src/main.cpp, wifi_manager.*, zabbix_sender.*
zabbix/Template_ESP32_Thermometer.xml
```
