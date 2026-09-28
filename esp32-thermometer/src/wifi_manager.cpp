#include "wifi_manager.h"

#include "config.h"

void WifiManager::begin(const char *ssid, const char *password) {
  ssid_ = ssid;
  password_ = password;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  startConnectAttempt();
}

void WifiManager::startConnectAttempt() {
  if (ssid_ == nullptr) {
    return;
  }
  connecting_ = true;
  connected_ = false;
  connectStartedMs_ = millis();
  WiFi.disconnect(true, false);
  WiFi.begin(ssid_, password_);
}

void WifiManager::loop() {
  const uint32_t now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    connected_ = true;
    connecting_ = false;
    return;
  }

  connected_ = false;

  if (connecting_) {
    if (now - connectStartedMs_ >= WIFI_CONNECT_ATTEMPT_TIMEOUT_MS) {
      connecting_ = false;
      lastReconnectMs_ = now;
    }
    return;
  }

  if (now - lastReconnectMs_ >= WIFI_RECONNECT_INTERVAL_MS) {
    startConnectAttempt();
  }
}
