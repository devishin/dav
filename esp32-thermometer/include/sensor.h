#pragma once

#include <stdint.h>

struct SensorSnapshot {
  bool valid = false;
  float temperatureC = 0.0f;
  float humidityPct = 0.0f;
  uint8_t consecutiveFailures = 0;
};

void sensorBegin();
SensorSnapshot sensorReadOnce(const SensorSnapshot &previous);
bool sensorIsOk(const SensorSnapshot &s);
void sensorLogReading(const SensorSnapshot &s);
