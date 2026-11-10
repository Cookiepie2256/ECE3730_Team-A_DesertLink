// EnvSensor.h
// DesertLink (ECE 3730 Team A) - Environmental Sensor Module
// Owner: Andy Holt
//
// Reads temperature (F) and humidity (%) from the RAK1901 sensor.
//
// How to use:
//   EnvSensor env;
//   env.begin();    // in setup()
//   env.read();     // in loop()
//   codec.packEnvironment(env.getTempF(), env.getHumidity());
//
// Needs the "Adafruit SHTC3" library.
#ifndef ENV_SENSOR_H
#define ENV_SENSOR_H

#include <Arduino.h>
#include <Adafruit_SHTC3.h>

class EnvSensor {
public:
  bool  begin();         // true = sensor found
  bool  read();          // true = good reading
  float getTempF();      // temperature in F
  float getHumidity();   // humidity in %
  bool  isValid();       // true if the last read() worked

private:
  Adafruit_SHTC3 sensor;
  bool  found = false;
  bool  valid = false;
  float tempF = 0;
  float hum   = 0;
};

#endif
