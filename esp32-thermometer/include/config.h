#pragma once

#include <stdint.h>

// --- Identity (must match Zabbix Host name exactly) ---
#define DEVICE_NAME "thermo-01"
#define FIRMWARE_VERSION "1.0.0"

// --- Hardware ---
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = 22;  // DHT22 (AM2302)

// --- Telemetry intervals (non-blocking loop) ---
#ifndef DHT_READ_INTERVAL_MS
constexpr uint32_t DHT_READ_INTERVAL_MS = 10000;
#endif

constexpr uint32_t TELEMETRY_INTERVAL_MS = 60000;

constexpr uint8_t DHT_FAILURE_LIMIT = 3;

// --- Zabbix (trapper push) ---
constexpr const char *ZABBIX_SERVER = "192.168.11.52";
constexpr uint16_t ZABBIX_PORT = 10051;

constexpr const char *ZBX_KEY_TEMPERATURE = "thermometer.temperature";
constexpr const char *ZBX_KEY_HUMIDITY = "thermometer.humidity";
constexpr const char *ZBX_KEY_RSSI = "thermometer.rssi";
constexpr const char *ZBX_KEY_UPTIME = "thermometer.uptime";
constexpr const char *ZBX_KEY_SENSOR_OK = "thermometer.sensor_ok";
constexpr const char *ZBX_KEY_FIRMWARE = "thermometer.firmware";

// --- WiFi reconnect (non-blocking) ---
constexpr uint32_t WIFI_RECONNECT_INTERVAL_MS = 15000;
constexpr uint32_t WIFI_CONNECT_ATTEMPT_TIMEOUT_MS = 12000;

// --- Zabbix sender TCP ---
constexpr uint32_t ZABBIX_TCP_CONNECT_TIMEOUT_MS = 5000;
constexpr uint32_t ZABBIX_TCP_IO_TIMEOUT_MS = 8000;
