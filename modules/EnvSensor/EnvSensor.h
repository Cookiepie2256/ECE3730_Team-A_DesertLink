// EnvSensor.h
// DesertLink (ECE 3730 Team A) - Environmental Sensor Module
// Owner: Andy Holt
//
// Reads temperature (F) and humidity (%) from an SHT3x (SHT30/SHT31/SHT35)
// sensor on the I2C bus.
//
// How to use:
//   EnvSensor env;
//   env.begin();    // in setup()
//   env.read();     // in loop()
//   codec.packEnvironment(env.getTempF(), env.getHumidity());
//
// Needs the "Adafruit SHT31 Library" (works for all SHT3x sensors).
#ifndef ENV_SENSOR_H
#define ENV_SENSOR_H

#include <Arduino.h>
#include <Adafruit_SHT31.h>

class EnvSensor {
public:
  bool  begin();         // true = sensor found
  bool  read();          // true = good reading
  float getTempF();      // temperature in F
  float getHumidity();   // humidity in %
  bool  isValid();       // true if the last read() worked

private:
  Adafruit_SHT31 sensor;
  bool  found = false;
  bool  valid = false;
  float tempF = 0;
  float hum   = 0;
};

#endif
