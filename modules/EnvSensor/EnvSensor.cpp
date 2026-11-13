// EnvSensor.cpp
// DesertLink (ECE 3730 Team A) - Environmental Sensor Module
// Owner: Andy Holt
#include "EnvSensor.h"

bool EnvSensor::begin() {
#ifdef WB_IO2
  pinMode(WB_IO2, OUTPUT);     // turn on power to the sensor slot
  digitalWrite(WB_IO2, HIGH);
  delay(10);
#endif
  found = sensor.begin();
  return found;
}

bool EnvSensor::read() {
  valid = false;
  if (!found && !begin()) return false;   // retry if sensor was missing

  sensors_event_t h, t;
  if (!sensor.getEvent(&h, &t)) {
    found = false;
    return false;
  }

  float c = t.temperature;
  float rh = h.relative_humidity;
  if (isnan(c) || isnan(rh) || c < -40 || c > 125 || rh < 0 || rh > 100) return false;

  tempF = c * 9.0 / 5.0 + 32.0;
  hum   = rh;
  valid = true;
  return true;
}

float EnvSensor::getTempF()    { return tempF; }
float EnvSensor::getHumidity() { return hum; }
bool  EnvSensor::isValid()     { return valid; }
