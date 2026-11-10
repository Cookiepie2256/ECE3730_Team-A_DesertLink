// EnvSensor.ino
// Test sketch for the DesertLink Environmental Sensor Module.
// Board: WisBlock RAK4631. Serial Monitor: 115200 baud.
//
// Demo: you should see "Sensor found", then a reading every 2 s.
// Breathe on the sensor -> humidity goes up. Unplug it -> "Valid: 0".
#include "EnvSensor.h"

EnvSensor env;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10);   // wait for Serial Monitor

  Serial.println("DesertLink EnvSensor test");
  if (env.begin()) Serial.println("Sensor found");
  else             Serial.println("ERROR: sensor not found");
}

void loop() {
  // These three values are what DataPacket::updateEnvironment() receives
  if (env.read()) {
    Serial.print("Temp: ");       Serial.print(env.getTempC100());     // 2345 = 23.45 C
    Serial.print("  Humidity: "); Serial.print(env.getHumidity100());  // 3520 = 35.20 %
    Serial.println("  Valid: 1");
  } else {
    Serial.println("ERROR: bad reading  Valid: 0");
  }

  delay(2000);
}
