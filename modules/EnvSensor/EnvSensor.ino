// EnvSensor.ino
// Test sketch for the DesertLink Environmental Sensor Module.
// Board: WisBlock RAK4631. Serial Monitor: 115200 baud.
#include "EnvSensor.h"

EnvSensor env;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10);

  Serial.println("DesertLink EnvSensor test");
  if (env.begin()) Serial.println("Sensor found");
  else             Serial.println("ERROR: sensor not found");
}

void loop() {
  if (env.read()) {
    Serial.print("Temp: ");       Serial.print(env.getTempF(), 2);    Serial.print(" F");
    Serial.print("  Humidity: "); Serial.print(env.getHumidity(), 2); Serial.println(" %");
  } else {
    Serial.println("ERROR: bad reading");
  }
  delay(2000);
}
