#pragma once

#include <Arduino.h>

struct ZabbixMetric {
  const char *key;
  const char *value;
};

class ZabbixSender {
 public:
  bool sendBatch(const char *host, const char *server, uint16_t port,
                 const ZabbixMetric *metrics, size_t count);
};
