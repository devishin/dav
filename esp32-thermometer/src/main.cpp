#include <Arduino.h>
#include <DHTesp.h>

#include "config.h"

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

DHTesp dht;

struct SensorSnapshot {
  bool valid = false;
  float temperatureC = NAN;
  float humidityPct = NAN;
  uint8_t consecutiveFailures = 0;
};

SensorSnapshot readDhtOnce(SensorSnapshot prev) {
  SensorSnapshot out = prev;
  TempAndHumidity th = dht.getTempAndHumidity();

  if (dht.getStatus() == 0 && !isnan(th.temperature) && !isnan(th.humidity)) {
    out.valid = true;
    out.temperatureC = th.temperature;
    out.humidityPct = th.humidity;
    out.consecutiveFailures = 0;
  } else {
    out.valid = false;
    if (out.consecutiveFailures < 255) {
      out.consecutiveFailures++;
    }
  }
  return out;
}

void logSensor(const SensorSnapshot &s) {
  if (s.valid) {
    Serial.printf("[DHT] T=%.1f C  RH=%.1f %%  fails=%u\n", s.temperatureC,
                  s.humidityPct, s.consecutiveFailures);
  } else {
    Serial.printf("[DHT] read failed (consecutive=%u, limit=%u)\n",
                  s.consecutiveFailures, DHT_FAILURE_LIMIT);
  }
}

#ifdef PHASE_HW_TEST

void runHwTestLoop() {
  static uint32_t lastReadMs = 0;
  static SensorSnapshot sensor;

  const uint32_t now = millis();
  if (now - lastReadMs >= DHT_READ_INTERVAL_MS) {
    lastReadMs = now;
    sensor = readDhtOnce(sensor);
    logSensor(sensor);
  }
}

#else

WifiManager wifi;
ZabbixSender zabbix;

char bufTemp[16];
char bufHum[16];
char bufRssi[8];
char bufDhtStatus[4];
char bufUptime[16];

void sendZabbixBatch(const SensorSnapshot &sensor) {
  if (!wifi.isConnected()) {
    Serial.println(F("[ZBX] skip send — WiFi down"));
    return;
  }

  if (sensor.valid) {
    snprintf(bufTemp, sizeof(bufTemp), "%.1f", sensor.temperatureC);
    snprintf(bufHum, sizeof(bufHum), "%.1f", sensor.humidityPct);
  } else {
    snprintf(bufTemp, sizeof(bufTemp), "0");
    snprintf(bufHum, sizeof(bufHum), "0");
  }

  snprintf(bufRssi, sizeof(bufRssi), "%d", wifi.rssi());

  const int dhtStatus =
      (sensor.consecutiveFailures >= DHT_FAILURE_LIMIT) ? ZBX_DHT_STATUS_FAILED
                                                        : ZBX_DHT_STATUS_OK;
  snprintf(bufDhtStatus, sizeof(bufDhtStatus), "%d", dhtStatus);
  snprintf(bufUptime, sizeof(bufUptime), "%lu", millis() / 1000UL);

  const ZabbixMetric metrics[] = {
      {ZBX_KEY_TEMPERATURE, bufTemp},
      {ZBX_KEY_HUMIDITY, bufHum},
      {ZBX_KEY_WIFI_RSSI, bufRssi},
      {ZBX_KEY_DHT_STATUS, bufDhtStatus},
      {ZBX_KEY_UPTIME, bufUptime},
  };

  const bool ok = zabbix.sendBatch(ZABBIX_HOST, ZABBIX_SERVER, ZABBIX_PORT,
                                   metrics, sizeof(metrics) / sizeof(metrics[0]));
  Serial.printf("[ZBX] batch %s -> %s:%u\n", ok ? "OK" : "FAIL", ZABBIX_SERVER,
                static_cast<unsigned>(ZABBIX_PORT));
}

void runFullAppLoop() {
  static uint32_t lastReadMs = 0;
  static uint32_t lastSendMs = 0;
  static SensorSnapshot sensor;

  wifi.loop();

  const uint32_t now = millis();

  if (now - lastReadMs >= DHT_READ_INTERVAL_MS) {
    lastReadMs = now;
    sensor = readDhtOnce(sensor);
    logSensor(sensor);
  }

  if (now - lastSendMs >= ZABBIX_SEND_INTERVAL_MS) {
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
  Serial.println(F("=== ESP32 Thermometer — PHASE HW TEST (DHT GPIO4) ==="));
#else
  Serial.println(F("=== ESP32 Thermometer — full app (WiFi + Zabbix ZBXD) ==="));
#endif

  dht.setup(DHT_PIN, static_cast<DHTesp::DHT_MODEL_t>(DHT_TYPE));

#ifdef PHASE_HW_TEST
  Serial.printf("DHT on GPIO%u every %lu ms (serial only)\n", DHT_PIN,
                static_cast<unsigned long>(DHT_READ_INTERVAL_MS));
#else
  Serial.printf("DHT every %lu s; Zabbix batch every %lu s\n",
                static_cast<unsigned long>(DHT_READ_INTERVAL_MS / 1000),
                static_cast<unsigned long>(ZABBIX_SEND_INTERVAL_MS / 1000));
  wifi.begin(WIFI_SSID, WIFI_PASS);
#endif
}

void loop() {
#ifdef PHASE_HW_TEST
  runHwTestLoop();
#else
  runFullAppLoop();
#endif
}
