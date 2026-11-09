// EnvSensor.ino
// Standalone test for the TrailLink Environmental Sensor Module.
// Board: WisBlock RAK4631 (RAKwireless nRF52 BSP). Serial Monitor: 115200 baud.
//
// What it shows:
//   1. Sensor detected at startup
//   2. Live readings every 2 s, raw scaled ints + human units
//   3. Error flag when the sensor is unplugged or fails
//   4. A stand-in for Malainy's DataPacket::updateEnvironment() so the
//      exact values handed to the packet module are visible
#include "EnvSensor.h"

EnvSensor env;

// Temporary stand-in for DataPacket::updateEnvironment() until integration.
void updateEnvironment(int16_t tempC100, uint16_t hum100, bool valid) {
  Serial.print("  -> DataPacket gets: temp=");
  Serial.print(tempC100);
  Serial.print(" hum=");
  Serial.print(hum100);
  Serial.print(" flagBit1=");
  Serial.println(valid ? 1 : 0);
}

void printError(EnvSensor::Error e) {
  switch (e) {
    case EnvSensor::ENV_NOT_FOUND:    Serial.println("ERROR: sensor not found"); break;
    case EnvSensor::ENV_READ_FAILED:  Serial.println("ERROR: read failed (check wiring)"); break;
    case EnvSensor::ENV_OUT_OF_RANGE: Serial.println("ERROR: value out of range"); break;
    default: break;
  }
}

void runConversionSelfTest() {
  Serial.println("--- Conversion self test ---");
  struct { float in; int16_t expect; bool ok; } t[] = {
    {23.45f, 2345, true}, {-10.5f, -1050, true}, {125.0f, 12500, true}, {150.0f, 0, false}
  };
  for (auto &c : t) {
    int16_t out = 0;
    bool ok = EnvSensor::scaleTemp(c.in, out);
    bool pass = (ok == c.ok) && (!ok || out == c.expect);
    Serial.print(c.in); Serial.print(" C -> "); Serial.print(ok ? String(out) : String("rejected"));
    Serial.println(pass ? "  PASS" : "  FAIL");
  }
  uint16_t h = 0;
  bool okH = EnvSensor::scaleHumidity(35.2f, h);
  Serial.print("35.20 % -> "); Serial.print(h);
  Serial.println((okH && h == 3520) ? "  PASS" : "  FAIL");
  Serial.println("----------------------------");
}

void setup() {
  Serial.begin(115200);
  unsigned long t0 = millis();
  while (!Serial && millis() - t0 < 5000) delay(10);

  Serial.println("TrailLink EnvSensor test");
  runConversionSelfTest();

  if (env.begin()) Serial.println("Sensor found");
  else             printError(env.getError());
}

void loop() {
  // Retry startup if the sensor was plugged in late
  if (env.getError() == EnvSensor::ENV_NOT_FOUND) env.begin();

  if (env.read()) {
    int16_t  t = env.getTempC100();
    uint16_t h = env.getHumidity100();
    Serial.print("Temp: ");     Serial.print(t / 100.0, 2);
    Serial.print(" C (");       Serial.print(t);
    Serial.print(")  Hum: ");   Serial.print(h / 100.0, 2);
    Serial.print(" % (");       Serial.print(h);
    Serial.println(")");
  } else {
    printError(env.getError());
  }

  updateEnvironment(env.getTempC100(), env.getHumidity100(), env.isValid());
  delay(2000);
}
