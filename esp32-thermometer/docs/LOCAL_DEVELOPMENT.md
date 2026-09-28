# Локальная разработка ESP32 Thermometer

Этот репозиторий рассчитан на **работу на вашем компьютере**: PlatformIO, USB‑прошивка через разъём **COM**, Serial Monitor. Облачные агенты **не нужны** для сборки и upload.

---

## 1. Открыть проект в Cursor / VS Code

1. Клонировать репозиторий (если ещё нет):

   ```bash
   git clone https://github.com/devishin/dav.git
   ```

2. **File → Open Folder** → каталог **`dav/esp32-thermometer`** (именно эта папка — там `platformio.ini`).

3. Установить расширение **PlatformIO IDE** (PlatformIO → Install).

4. (Опционально) Cursor предложит установить рекомендуемые расширения из `.vscode/extensions.json`.

---

## 2. Железо

| Компонент | Подключение |
|-----------|-------------|
| ESP32-S3 DevKitC-1 compatible (N16R8) | Питание/USB: разъём **COM** (USB‑UART), не native USB |
| DHT22 VCC | 3V3 |
| DHT22 DATA | **GPIO4** |
| DHT22 GND | GND |

---

## 3. Секреты Wi‑Fi (один раз)

```bash
cd esp32-thermometer
cp include/secrets.example.h include/secrets.h
```

Отредактировать **`include/secrets.h`**:

- `WIFI_SSID` — ваша сеть (например `t-co`)
- `WIFI_PASSWORD` — пароль

Файл **не коммитить** (уже в `.gitignore`).

Имя хоста Zabbix и сервер задаются в **`include/config.h`**:

- `DEVICE_NAME` — должно **точно** совпадать с **Host name** в Zabbix (по умолчанию `thermo-01`)
- `ZABBIX_SERVER` — `192.168.11.52`, порт `10051`

---

## 4. COM‑порт

```bash
pio device list
```

Запомните порт (Windows: `COMx`). Если PlatformIO стабильно находит плату, **не** прописывайте `upload_port` в `platformio.ini`.

---

## 5. Этапы прошивки (как в ТЗ)

### B — только DHT22

```bash
pio run -e hw-test -t upload
pio device monitor
```

Ожидание: `[BOOT] ESP32 Thermometer hardware test`, каждые ~3 с `[DHT] Temperature: …` / `Humidity: …`.

Выход из monitor: `Ctrl+C`.

### C–G — Wi‑Fi + Zabbix

```bash
pio run -e release -t upload
pio device monitor
```

Ожидание: `[WIFI] Connected`, `[WIFI] IP: …`, раз в ~60 с `[ZABBIX] … processed: N failed: 0`.

---

## 6. Zabbix 5.0

1. **Configuration → Templates → Import** → `zabbix/Template_ESP32_Thermometer.xml`
2. **Configuration → Hosts → Create host**
   - **Host name:** `thermo-01` (как `DEVICE_NAME` в `config.h`)
3. Привязать шаблон **Template ESP32 Thermometer**
4. Данные: **Monitoring → Latest data**

---

## 7. Диагностика

| Симптом | Что проверить |
|---------|----------------|
| COM не виден | Кабель в **COM**, драйвер USB‑UART, другой USB‑порт |
| Serial пустой | Скорость **115200**; не включать native USB вместо COM |
| DHT read failed | GPIO4, 3V3, пауза 2+ с между опросами |
| Wi‑Fi не коннект | `secrets.h`, SSID/пароль, 2.4 GHz |
| Zabbix failed=6 | Host name ≠ `DEVICE_NAME`, шаблон не привязан, firewall 10051 |
| processed OK, нет данных в UI | Неверный host, item keys не из шаблона |

---

## 8. Связь с Project в Cursor

Можно продолжать чат Project в облаке: вы прошиваете **локально**, присылаете фрагменты Serial (без паролей) — правки кода идут через **Git pull**.

Облачная прошивка вашей платы возможна только при **private workers** и `cursor worker start` на этом ПК; для этого проекта **локальный режим — основной**.
