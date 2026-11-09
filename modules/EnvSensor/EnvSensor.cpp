// EnvSensor.cpp
// TrailLink (ECE 3735 Team 1) - Environmental Sensor Module
// Owner: Andy Holt
#include "EnvSensor.h"

#define SHT31_I2C_ADDR 0x44   // SHT31 default (ADR pin low). SHTC3 address is fixed at 0x70.

EnvSensor::EnvSensor()
  : _found(false), _valid(false), _error(ENV_NOT_FOUND), _tempC100(0), _hum100(0) {}

bool EnvSensor::begin(TwoWire &wire) {
  // RAK19007: WB_IO2 switches the 3V3_S rail that powers the sensor slots.
#ifdef WB_IO2
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(10);
#endif
  wire.begin();

#if defined(ENV_SENSOR_RAK1901)
  _found = _sensor.begin(&wire);
#else
  _found = _sensor.begin(SHT31_I2C_ADDR);
#endif

  _valid = false;
  _error = _found ? ENV_OK : ENV_NOT_FOUND;
  return _found;
}

bool EnvSensor::read() {
  if (!_found) {
    _valid = false;
    _error = ENV_NOT_FOUND;
    return false;
  }

  float tempC = NAN;
  float hum   = NAN;

#if defined(ENV_SENSOR_RAK1901)
  sensors_event_t humEvent, tempEvent;
  if (!_sensor.getEvent(&humEvent, &tempEvent)) {
    _valid = false;
    _error = ENV_READ_FAILED;
    return false;
  }
  tempC = tempEvent.temperature;
  hum   = humEvent.relative_humidity;
#else
  if (!_sensor.readBoth(&tempC, &hum)) {
    _valid = false;
    _error = ENV_READ_FAILED;
    return false;
  }
#endif

  int16_t  t;
  uint16_t h;
  if (!scaleTemp(tempC, t) || !scaleHumidity(hum, h)) {
    _valid = false;
    _error = ENV_OUT_OF_RANGE;
    return false;
  }

  _tempC100 = t;
  _hum100   = h;
  _valid    = true;
  _error    = ENV_OK;
  return true;
}

int16_t  EnvSensor::getTempC100() const    { return _tempC100; }
uint16_t EnvSensor::getHumidity100() const { return _hum100; }
bool     EnvSensor::isValid() const        { return _valid; }
EnvSensor::Error EnvSensor::getError() const { return _error; }

// temp_out = round(T_C x 100), rejected if NaN or outside -40..125 C
bool EnvSensor::scaleTemp(float tempC, int16_t &out) {
  if (isnan(tempC)) return false;
  long v = lroundf(tempC * 100.0f);
  if (v < TEMP_MIN_C100 || v > TEMP_MAX_C100) return false;
  out = (int16_t)v;
  return true;
}

// hum_out = round(RH x 100), rejected if NaN or outside 0..100 %
bool EnvSensor::scaleHumidity(float hum, uint16_t &out) {
  if (isnan(hum)) return false;
  long v = lroundf(hum * 100.0f);
  if (v < 0 || v > HUM_MAX_100) return false;
  out = (uint16_t)v;
  return true;
}
