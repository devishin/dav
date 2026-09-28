#include <Arduino.h>
#include <esp_timer.h>

#include "config.h"
#include "sensor.h"

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#warning "Building with secrets.example.h — copy to secrets.h for device upload"
#endif

#ifndef PHASE_HW_TEST
#include "wifi_manager.h"
#include "zabbix_sender.h"
#endif

namespace {

#ifdef PHASE_HW_TEST

void runHwTestLoop() {
  static uint32_t lastReadMs = 0;
  static SensorSnapshot sensor;

  const uint32_t now = millis();
  if (now - lastReadMs >= DHT_READ_INTERVAL_MS) {
    lastReadMs = now;
    sensor = sensorReadOnce(sensor);
    sensorLogReading(sensor);
  }
}

#else
WifiManager wifi;
ZabbixSender zabbix;

uint32_t deviceUptimeSeconds() {
  return static_cast<uint32_t>(esp_timer_get_time() / 1000000ULL);
}

void logHardwareInfo() {
  Serial.println(F("[HW] ESP32-S3"));
  Serial.printf("[HW] Flash: %u MB\n",
                static_cast<unsigned>(ESP.getFlashChipSize() / (1024U * 1024U)));
  if (ESP.getPsramSize() > 0) {
    Serial.printf("[HW] PSRAM: %u MB\n",
                  static_cast<unsigned>(ESP.getPsramSize() / (1024U * 1024U)));
  } else {
    Serial.println(F("[HW] PSRAM: not detected"));
  }
  Serial.printf("[DHT] GPIO: %u\n", static_cast<unsigned>(DHT_PIN));
}

void sendZabbixBatch(const SensorSnapshot &sensor) {
  if (!wifi.isConnected()) {
    Serial.println(F("[ZABBIX] skip — WiFi down"));
    return;
  }

  ZabbixMetric metrics[6];
  size_t n = 0;

  char bufTemp[16];
  char bufHum[16];
  char bufRssi[12];
  char bufUptime[16];
  char bufSensorOk[4];
  char bufFirmware[16];

  const bool ok = sensorIsOk(sensor);
  snprintf(bufSensorOk, sizeof(bufSensorOk), "%d", ok ? 1 : 0);
  metrics[n++] = {ZBX_KEY_SENSOR_OK, bufSensorOk};

  if (sensor.valid) {
    snprintf(bufTemp, sizeof(bufTemp), "%.1f", sensor.temperatureC);
    snprintf(bufHum, sizeof(bufHum), "%.1f", sensor.humidityPct);
    metrics[n++] = {ZBX_KEY_TEMPERATURE, bufTemp};
    metrics[n++] = {ZBX_KEY_HUMIDITY, bufHum};
  }

  if (wifi.hasRssi()) {
    snprintf(bufRssi, sizeof(bufRssi), "%d", wifi.rssi());
    metrics[n++] = {ZBX_KEY_RSSI, bufRssi};
  }

  snprintf(bufUptime, sizeof(bufUptime), "%lu",
           static_cast<unsigned long>(deviceUptimeSeconds()));
  metrics[n++] = {ZBX_KEY_UPTIME, bufUptime};

  snprintf(bufFirmware, sizeof(bufFirmware), "%s", FIRMWARE_VERSION);
  metrics[n++] = {ZBX_KEY_FIRMWARE, bufFirmware};

  Serial.printf("[ZABBIX] Sending %u metrics to %s:%u\n", static_cast<unsigned>(n),
                ZABBIX_SERVER, static_cast<unsigned>(ZABBIX_PORT));

  zabbix.sendBatch(DEVICE_NAME, ZABBIX_SERVER, ZABBIX_PORT, metrics, n);
}

void runFullAppLoop() {
  static uint32_t lastReadMs = 0;
  static uint32_t lastSendMs = 0;
  static SensorSnapshot sensor;

  wifi.loop();

  const uint32_t now = millis();

  if (now - lastReadMs >= DHT_READ_INTERVAL_MS) {
    lastReadMs = now;
    sensor = sensorReadOnce(sensor);
    sensorLogReading(sensor);
  }

  if (now - lastSendMs >= TELEMETRY_INTERVAL_MS) {
    lastSendMs = now;
    sendZabbixBatch(sensor);
  }
}
#endif

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();

#ifdef PHASE_HW_TEST
  Serial.println(F("[BOOT] ESP32 Thermometer hardware test"));
#else
  Serial.printf("[BOOT] ESP32 Thermometer %s\n", FIRMWARE_VERSION);
  logHardwareInfo();
#endif

  sensorBegin();

#ifndef PHASE_HW_TEST
  Serial.printf("[DHT] read every %lu s; Zabbix every %lu s\n",
                static_cast<unsigned long>(DHT_READ_INTERVAL_MS / 1000),
                static_cast<unsigned long>(TELEMETRY_INTERVAL_MS / 1000));
  wifi.begin(WIFI_SSID, WIFI_PASSWORD);
#endif
}

void loop() {
#ifdef PHASE_HW_TEST
  runHwTestLoop();
#else
  runFullAppLoop();
#endif
}
