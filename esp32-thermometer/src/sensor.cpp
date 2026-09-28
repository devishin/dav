#include "sensor.h"

#include <DHTesp.h>

#include "config.h"

namespace {
DHTesp dht;
}  // namespace

void sensorBegin() {
  dht.setup(DHT_PIN, static_cast<DHTesp::DHT_MODEL_t>(DHT_TYPE));
}

SensorSnapshot sensorReadOnce(const SensorSnapshot &previous) {
  SensorSnapshot out = previous;
  TempAndHumidity th = dht.getTempAndHumidity();

  if (dht.getStatus() == 0 && !isnan(th.temperature) && !isnan(th.humidity)) {
    out.valid = true;
    out.temperatureC = th.temperature;
    out.humidityPct = th.humidity;
    out.consecutiveFailures = 0;
  } else {
    out.valid = false;
    if (out.consecutiveFailures < 255) {
      out.consecutiveFailures++;
    }
  }
  return out;
}

bool sensorIsOk(const SensorSnapshot &s) {
  return s.consecutiveFailures < DHT_FAILURE_LIMIT;
}

void sensorLogReading(const SensorSnapshot &s) {
  if (s.valid) {
    Serial.printf("[DHT] Temperature: %.1f C\n", s.temperatureC);
    Serial.printf("[DHT] Humidity: %.1f %%\n", s.humidityPct);
  } else {
    Serial.printf("[DHT] Read failed %u/%u\n", s.consecutiveFailures,
                  DHT_FAILURE_LIMIT);
  }
}
