#include "zabbix_sender.h"

#include "config.h"

#include <ArduinoJson.h>
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
  json.reserve(256 + count * 80);
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

bool readBytesWithTimeout(WiFiClient &client, uint8_t *buf, size_t len,
                          uint32_t timeoutMs) {
  size_t got = 0;
  const uint32_t start = millis();
  while (got < len) {
    if (client.available() > 0) {
      const int n = client.read(buf + got, len - got);
      if (n > 0) {
        got += static_cast<size_t>(n);
      }
    } else if (!client.connected() && client.available() == 0) {
      break;
    }
    if (millis() - start > timeoutMs) {
      return false;
    }
    delay(1);
  }
  return got == len;
}

bool readZabbixJsonResponse(WiFiClient &client, String &outJson,
                            uint32_t timeoutMs) {
  uint8_t header[13];
  if (!readBytesWithTimeout(client, header, sizeof(header), timeoutMs)) {
    return false;
  }
  if (header[0] != 'Z' || header[1] != 'B' || header[2] != 'X' ||
      header[3] != 'D') {
    return false;
  }

  uint64_t datalen = 0;
  for (int i = 0; i < 8; ++i) {
    datalen |= static_cast<uint64_t>(header[5 + i]) << (8 * i);
  }

  if (datalen == 0 || datalen > 8192) {
    return false;
  }

  outJson = "";
  outJson.reserve(static_cast<unsigned>(datalen) + 1);
  const uint32_t start = millis();
  while (outJson.length() < datalen) {
    while (client.available() > 0 &&
           outJson.length() < static_cast<size_t>(datalen)) {
      const int c = client.read();
      if (c < 0) {
        break;
      }
      outJson += static_cast<char>(c);
    }
    if (millis() - start > timeoutMs) {
      return false;
    }
    if (!client.connected() && client.available() == 0 &&
        outJson.length() < static_cast<size_t>(datalen)) {
      return false;
    }
    delay(1);
  }
  return outJson.length() == static_cast<size_t>(datalen);
}

void parseInfoProcessedFailed(const char *info, int &processed, int &failed) {
  processed = -1;
  failed = -1;
  if (info == nullptr) {
    return;
  }
  const char *p = strstr(info, "processed:");
  if (p) {
    processed = atoi(p + 10);
  }
  p = strstr(info, "failed:");
  if (p) {
    failed = atoi(p + 7);
  }
}

}  // namespace

ZabbixSendResult ZabbixSender::sendBatch(const char *host, const char *server,
                                         uint16_t port,
                                         const ZabbixMetric *metrics,
                                         size_t count) {
  ZabbixSendResult result;

  if (host == nullptr || server == nullptr || count == 0) {
    return result;
  }

  const String payload = buildPayloadJson(host, metrics, count);
  const uint64_t datalen = static_cast<uint64_t>(payload.length());

  WiFiClient client;
  client.setTimeout(ZABBIX_TCP_IO_TIMEOUT_MS / 1000);

  if (!client.connect(server, port, ZABBIX_TCP_CONNECT_TIMEOUT_MS)) {
    Serial.println(F("[ZABBIX] Connection timeout"));
    return result;
  }
  result.transportOk = true;

  const uint8_t header[13] = {'Z', 'B', 'X', 'D', '\x01'};
  if (!writeAll(client, header, sizeof(header))) {
    client.stop();
    result.transportOk = false;
    return result;
  }

  for (int i = 0; i < 8; ++i) {
    const uint8_t b = static_cast<uint8_t>((datalen >> (8 * i)) & 0xFF);
    if (client.write(&b, 1) != 1) {
      client.stop();
      result.transportOk = false;
      return result;
    }
  }

  const uint8_t reserved[8] = {0};
  if (!writeAll(client, reserved, sizeof(reserved))) {
    client.stop();
    result.transportOk = false;
    return result;
  }

  if (!writeAll(client, reinterpret_cast<const uint8_t *>(payload.c_str()),
                payload.length())) {
    client.stop();
    result.transportOk = false;
    return result;
  }

  String responseJson;
  if (!readZabbixJsonResponse(client, responseJson, ZABBIX_TCP_IO_TIMEOUT_MS)) {
    Serial.println(F("[ZABBIX] Invalid or incomplete response"));
    client.stop();
    return result;
  }
  client.stop();

  StaticJsonDocument<512> doc;
  if (deserializeJson(doc, responseJson) != DeserializationError::Ok) {
    Serial.println(F("[ZABBIX] JSON parse error"));
    return result;
  }

  const char *response = doc["response"] | "";
  strncpy(result.response, response, sizeof(result.response) - 1);

  const char *info = doc["info"] | "";
  strncpy(result.info, info, sizeof(result.info) - 1);

  parseInfoProcessedFailed(info, result.processed, result.failed);

  result.zabbixOk = (strcmp(response, "success") == 0 && result.failed == 0 &&
                     result.processed == static_cast<int>(count));

  Serial.printf("[ZABBIX] response: %s\n", result.response);
  if (result.processed >= 0 && result.failed >= 0) {
    Serial.printf("[ZABBIX] processed: %d\n", result.processed);
    Serial.printf("[ZABBIX] failed: %d\n", result.failed);
  }
  if (result.zabbixOk) {
    Serial.println(F("[ZABBIX] OK"));
  } else if (result.failed > 0) {
    Serial.println(F("[ZABBIX] Partial failure (failed > 0)"));
  }

  return result;
}
