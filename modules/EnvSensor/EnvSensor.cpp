// EnvSensor.cpp
// DesertLink (ECE 3730 Team A) - Environmental Sensor Module
// Owner: Andy Holt
#include "EnvSensor.h"

bool EnvSensor::begin() {
#ifdef WB_IO2
  // RAK19007: WB_IO2 turns on power to the sensor slots
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(10);
#endif
  found = sensor.begin();   // also starts I2C (Wire)
  return found;
}

bool EnvSensor::read() {
  valid = false;

  // If the sensor was missing, try to find it again (plugged in late)
  if (!found && !begin()) return false;

  sensors_event_t humEvent, tempEvent;
  if (!sensor.getEvent(&humEvent, &tempEvent)) {
    found = false;            // lost the sensor, retry next time
    return false;
  }

  float t = tempEvent.temperature;
  float h = humEvent.relative_humidity;

  // Reject impossible values (SHTC3 range is -40..125 C, 0..100 %)
  if (isnan(t) || isnan(h) || t < -40 || t > 125 || h < 0 || h > 100) return false;

  tempC100 = (int16_t)lroundf(t * 100);
  hum100   = (uint16_t)lroundf(h * 100);
  valid = true;
  return true;
}

int16_t  EnvSensor::getTempC100()    { return tempC100; }
uint16_t EnvSensor::getHumidity100() { return hum100; }
bool     EnvSensor::isValid()        { return valid; }
