#include "zabbix_sender.h"

#include "config.h"

#include <WiFiClient.h>

namespace {

bool writeAll(WiFiClient &client, const uint8_t *data, size_t len) {
  size_t sent = 0;
  while (sent < len) {
    const int n = client.write(data + sent, len - sent);
    if (n <= 0) {
      return false;
    }
    sent += static_cast<size_t>(n);
  }
  return true;
}

String buildPayloadJson(const char *host, const ZabbixMetric *metrics,
                        size_t count) {
  String json;
  json.reserve(256 + count * 64);
  json += F("{\"request\":\"sender data\",\"data\":[");
  for (size_t i = 0; i < count; ++i) {
    if (i > 0) {
      json += ',';
    }
    json += F("{\"host\":\"");
    json += host;
    json += F("\",\"key\":\"");
    json += metrics[i].key;
    json += F("\",\"value\":\"");
    json += metrics[i].value;
    json += F("\"}");
  }
  json += F("]}");
  return json;
}

}  // namespace

bool ZabbixSender::sendBatch(const char *host, const char *server, uint16_t port,
                             const ZabbixMetric *metrics, size_t count) {
  if (host == nullptr || server == nullptr || count == 0) {
    return false;
  }

  const String payload = buildPayloadJson(host, metrics, count);
  const uint64_t datalen = static_cast<uint64_t>(payload.length());

  WiFiClient client;
  client.setTimeout(ZABBIX_TCP_IO_TIMEOUT_MS);

  if (!client.connect(server, port, ZABBIX_TCP_CONNECT_TIMEOUT_MS)) {
    return false;
  }

  const uint8_t header[13] = {'Z', 'B', 'X', 'D', '\x01'};
  if (!writeAll(client, header, sizeof(header))) {
    client.stop();
    return false;
  }

  for (int i = 0; i < 8; ++i) {
    const uint8_t b = static_cast<uint8_t>((datalen >> (8 * i)) & 0xFF);
    if (client.write(&b, 1) != 1) {
      client.stop();
      return false;
    }
  }

  const uint8_t reserved[8] = {0};
  if (!writeAll(client, reserved, sizeof(reserved))) {
    client.stop();
    return false;
  }

  if (!writeAll(client, reinterpret_cast<const uint8_t *>(payload.c_str()),
                payload.length())) {
    client.stop();
    return false;
  }

  uint32_t waitStart = millis();
  while (client.available() == 0 && client.connected()) {
    if (millis() - waitStart > ZABBIX_TCP_IO_TIMEOUT_MS) {
      break;
    }
    delay(10);
  }

  bool ok = client.available() > 0;
  client.stop();
  return ok;
}
