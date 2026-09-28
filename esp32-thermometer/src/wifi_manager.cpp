#include "wifi_manager.h"

#include "config.h"

void WifiManager::begin(const char *ssid, const char *password) {
  ssid_ = ssid;
  password_ = password;
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(false);
  loggedConnected_ = false;
  Serial.printf("[WIFI] Connecting to %s\n", ssid_);
  startConnectAttempt();
}

void WifiManager::startConnectAttempt() {
  if (ssid_ == nullptr) {
    return;
  }
  connecting_ = true;
  connected_ = false;
  loggedConnected_ = false;
  connectStartedMs_ = millis();
  WiFi.disconnect(true, false);
  WiFi.begin(ssid_, password_);
}

void WifiManager::loop() {
  const uint32_t now = millis();

  if (WiFi.status() == WL_CONNECTED) {
    if (!loggedConnected_) {
      connected_ = true;
      connecting_ = false;
      loggedConnected_ = true;
      Serial.println(F("[WIFI] Connected"));
      Serial.printf("[WIFI] IP: %s\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WIFI] RSSI: %d dBm\n", WiFi.RSSI());
    }
    connected_ = true;
    connecting_ = false;
    return;
  }

  if (loggedConnected_) {
    Serial.println(F("[WIFI] Disconnected"));
    loggedConnected_ = false;
  }

  connected_ = false;

  if (connecting_) {
    if (now - connectStartedMs_ >= WIFI_CONNECT_ATTEMPT_TIMEOUT_MS) {
      connecting_ = false;
      lastReconnectMs_ = now;
      Serial.println(F("[WIFI] Connect attempt timed out, will retry"));
    }
    return;
  }

  if (now - lastReconnectMs_ >= WIFI_RECONNECT_INTERVAL_MS) {
    Serial.printf("[WIFI] Connecting to %s\n", ssid_);
    startConnectAttempt();
  }
}

int WifiManager::rssi() const {
  if (!connected_ || WiFi.status() != WL_CONNECTED) {
    return 0;
  }
  return WiFi.RSSI();
}

bool WifiManager::hasRssi() const {
  return connected_ && WiFi.status() == WL_CONNECTED;
}
