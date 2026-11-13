// EnvSensor.cpp
// DesertLink (ECE 3730 Team A) - Environmental Sensor Module
// Owner: Andy Holt
#include "EnvSensor.h"

bool EnvSensor::begin() {
#ifdef WB_IO2
  pinMode(WB_IO2, OUTPUT);     // turn on 3V3_S power (sensor slots and header)
  digitalWrite(WB_IO2, HIGH);
  delay(10);
#endif
  // SHT3x is at 0x44 (ADDR pin low, most boards) or 0x45 (ADDR pin high)
  found = sensor.begin(0x44) || sensor.begin(0x45);
  return found;
}

bool EnvSensor::read() {
  valid = false;
  if (!found && !begin()) return false;   // retry if sensor was missing

  float c, rh;
  if (!sensor.readBoth(&c, &rh)) {
    found = false;
    return false;
  }

  if (isnan(c) || isnan(rh) || c < -40 || c > 125 || rh < 0 || rh > 100) return false;

  tempF = c * 9.0 / 5.0 + 32.0;
  hum   = rh;
  valid = true;
  return true;
}

float EnvSensor::getTempF()    { return tempF; }
float EnvSensor::getHumidity() { return hum; }
bool  EnvSensor::isValid()     { return valid; }
