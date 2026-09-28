#pragma once

#include <stdint.h>

// --- Hardware ---
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = 22;  // DHT22 (AM2302)

// --- Telemetry intervals (non-blocking loop) ---
constexpr uint32_t DHT_READ_INTERVAL_MS = 10000;
constexpr uint32_t ZABBIX_SEND_INTERVAL_MS = 60000;

// Consecutive DHT read failures before status = failed
constexpr uint8_t DHT_FAILURE_LIMIT = 3;

// --- Zabbix item keys (must match Template_ESP32_Thermometer.xml) ---
constexpr const char *ZBX_KEY_TEMPERATURE = "esp32.temperature";
constexpr const char *ZBX_KEY_HUMIDITY = "esp32.humidity";
constexpr const char *ZBX_KEY_WIFI_RSSI = "esp32.wifi.rssi";
constexpr const char *ZBX_KEY_DHT_STATUS = "esp32.dht.status";
constexpr const char *ZBX_KEY_UPTIME = "esp32.uptime";

// DHT status valuemap: 0 = OK, 1 = FAILED
constexpr int ZBX_DHT_STATUS_OK = 0;
constexpr int ZBX_DHT_STATUS_FAILED = 1;

// --- WiFi reconnect (non-blocking) ---
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 15000;
constexpr uint32_t WIFI_CONNECT_ATTEMPT_TIMEOUT_MS = 12000;

// --- Zabbix sender ---
constexpr uint16_t ZABBIX_TRAP_PORT = 10051;
constexpr uint32_t ZABBIX_TCP_CONNECT_TIMEOUT_MS = 5000;
constexpr uint32_t ZABBIX_TCP_IO_TIMEOUT_MS = 8000;
