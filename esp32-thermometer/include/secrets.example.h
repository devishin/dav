#pragma once

// Copy this file to secrets.h and fill in real values (secrets.h is gitignored).
// Example:
//   cp include/secrets.example.h include/secrets.h

#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASS "your-wifi-password"

// Zabbix trapper target (e.g. server 192.168.11.52 on LAN)
#define ZABBIX_SERVER "192.168.11.52"
#define ZABBIX_PORT 10051

// Host name as configured in Zabbix (template macro {$ESP32_HOST})
#define ZABBIX_HOST "ESP32-Thermometer"
