// EnvSensor.h
// DesertLink (ECE 3730 Team A) - Environmental Sensor Module
// Owner: Andy Holt
//
// Reads temperature and humidity from a RAK1901 (SHTC3) over I2C.
// Values are scaled integers (team standard, no floats between modules):
//   getTempC100()     23.45 C  -> 2345   (int16_t)
//   getHumidity100()  35.20 %  -> 3520   (uint16_t)
//
// How to use (3 steps):
//   EnvSensor env;                    // 1. make one object
//   env.begin();                      // 2. call once in setup()
//   env.read();                       // 3. call in loop(), then use the getters
//   packet.updateEnvironment(env.getTempC100(), env.getHumidity100(), env.isValid());
//
// Needs the "Adafruit SHTC3 Library" (Arduino Library Manager).
#ifndef ENV_SENSOR_H
#define ENV_SENSOR_H

#include <Arduino.h>
#include <Adafruit_SHTC3.h>

class EnvSensor {
public:
  bool     begin();           // start the sensor. true = sensor found
  bool     read();            // take a new reading. true = reading is good
  int16_t  getTempC100();     // temperature in C x 100
  uint16_t getHumidity100();  // humidity in % x 100
  bool     isValid();         // true if the last read() worked (packet flag bit 1)

private:
  Adafruit_SHTC3 sensor;
  bool     found    = false;
  bool     valid    = false;
  int16_t  tempC100 = 0;
  uint16_t hum100   = 0;
};

#endif
