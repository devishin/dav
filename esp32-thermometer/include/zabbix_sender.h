#pragma once

#include <Arduino.h>

struct ZabbixMetric {
  const char *key;
  const char *value;
};

struct ZabbixSendResult {
  bool transportOk = false;
  bool zabbixOk = false;
  int processed = -1;
  int failed = -1;
  char response[16] = {};
  char info[128] = {};
};

class ZabbixSender {
 public:
  ZabbixSendResult sendBatch(const char *host, const char *server, uint16_t port,
                             const ZabbixMetric *metrics, size_t count);
};
