// EnvSensor.h
// TrailLink (ECE 3735 Team 1) - Environmental Sensor Module
// Owner: Andy Holt
//
// Reads temperature and humidity on the RAK4631 over I2C and reports them
// as scaled integers so no floats cross module boundaries:
//   temperature = degC x 100   (int16_t)   23.45 C -> 2345
//   humidity    = %RH  x 100   (uint16_t)  35.20 % -> 3520
//
// Supported sensors (pick one below):
//   ENV_SENSOR_RAK1901 : RAK1901 WisBlock (Sensirion SHTC3), plugs into Slot B
//   ENV_SENSOR_SHT31   : SHT31 breakout wired to the RAK19007 I2C header
#ifndef ENV_SENSOR_H
#define ENV_SENSOR_H

#include <Arduino.h>
#include <Wire.h>

// ---- Select the sensor your node uses ----
#define ENV_SENSOR_RAK1901
// #define ENV_SENSOR_SHT31

#if defined(ENV_SENSOR_RAK1901)
  #include <Adafruit_SHTC3.h>
#elif defined(ENV_SENSOR_SHT31)
  #include <Adafruit_SHT31.h>
#else
  #error "Define ENV_SENSOR_RAK1901 or ENV_SENSOR_SHT31 in EnvSensor.h"
#endif

class EnvSensor {
public:
  // Valid physical limits (scaled). Readings outside these are rejected.
  static const int16_t  TEMP_MIN_C100 = -4000;   // -40.00 C
  static const int16_t  TEMP_MAX_C100 = 12500;   // 125.00 C
  static const uint16_t HUM_MAX_100   = 10000;   // 100.00 %

  // Error codes returned by getError()
  enum Error : uint8_t {
    ENV_OK = 0,
    ENV_NOT_FOUND,     // begin() could not find the sensor on I2C
    ENV_READ_FAILED,   // sensor did not answer during read()
    ENV_OUT_OF_RANGE   // sensor answered but value is impossible (NaN / out of range)
  };

  EnvSensor();

  // Starts I2C and the sensor. Returns true if the sensor was found.
  bool begin(TwoWire &wire = Wire);

  // Takes a new measurement. Returns true if the reading is valid.
  // On failure the last good values are kept and isValid() returns false.
  bool read();

  // Last reading, scaled. Used by DataPacket::updateEnvironment().
  int16_t  getTempC100() const;      // degC x 100
  uint16_t getHumidity100() const;   // %RH  x 100

  // True if the most recent read() succeeded. Sets packet flag bit 1.
  bool isValid() const;

  // Why the last read failed (ENV_OK when valid).
  Error getError() const;

  // Pure conversion helpers (public so they can be unit tested).
  static bool     scaleTemp(float tempC, int16_t &out);
  static bool     scaleHumidity(float hum, uint16_t &out);

private:
#if defined(ENV_SENSOR_RAK1901)
  Adafruit_SHTC3 _sensor;
#else
  Adafruit_SHT31 _sensor;
#endif
  bool     _found;
  bool     _valid;
  Error    _error;
  int16_t  _tempC100;
  uint16_t _hum100;
};

#endif
