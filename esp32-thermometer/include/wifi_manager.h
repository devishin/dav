#pragma once

#include <WiFi.h>

class WifiManager {
 public:
  void begin(const char *ssid, const char *password);
  void loop();

  bool isConnected() const { return connected_; }
  int rssi() const { return connected_ ? WiFi.RSSI() : 0; }

 private:
  void startConnectAttempt();

  const char *ssid_ = nullptr;
  const char *password_ = nullptr;
  bool connected_ = false;
  bool connecting_ = false;
  uint32_t lastReconnectMs_ = 0;
  uint32_t connectStartedMs_ = 0;
};
