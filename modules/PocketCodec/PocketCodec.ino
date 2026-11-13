#include <PocketCodec.h>

// Helper function to compare floats with a small tolerance
bool isClose(float a, float b, float tolerance = 0.11) {
  return abs(a - b) < tolerance;
}

// Custom test runner for the Arduino Serial Monitor
void runCompressionTest(
    String testName,
    uint8_t id, float lat, float lon, float alt, 
    float temp, float hum, float batt, 
    uint8_t status, uint32_t uptimeSec
) {
    Serial.print("Running Test: ");
    Serial.println(testName);

    // ==========================================
    // 1. NODE: PACK THE DATA
    // ==========================================
    PocketCodec node(id);
    node.packLocation(lat, lon, alt);
    node.packEnvironment(temp, hum);
    node.packSystemState(batt, status, uptimeSec);

    // ==========================================
    // 2. SIMULATE TRANSMISSION
    // ==========================================
    uint8_t* txBuffer = node.getEncodedBytes();
    size_t packetSize = node.getPayloadSize();
    
    if (packetSize != 18) {
      Serial.println("  -> FAIL: Packet size is not 18 bytes.");
      return;
    }

    // ==========================================
    // 3. BASE STATION: UNPACK THE DATA
    // ==========================================
    PocketCodec baseStation;
    baseStation.loadPayload(txBuffer);

    // ==========================================
    // 4. VERIFY EDGE CASES AND COMPRESSION
    // ==========================================
    bool passed = true;
    
    if (baseStation.unpackNodeId() != id) passed = false;
    if (baseStation.unpackLatitude() != lat) passed = false;
    if (baseStation.unpackLongitude() != lon) passed = false;
    if (baseStation.unpackStatusFlag() != status) passed = false;
    
    // Check truncated conversions
    if (baseStation.unpackAltitude() != (int16_t)alt) passed = false;
    if (!isClose(baseStation.unpackTemperature(), temp)) passed = false;
    if (baseStation.unpackHumidity() != (uint8_t)hum) passed = false;
    if (baseStation.unpackBattery() != (uint8_t)batt) passed = false;
    if (baseStation.unpackUptimeMinutes() != (uint16_t)(uptimeSec / 60)) passed = false;

    if (passed) {
      Serial.print("  -> PASSED. JSON Output: ");
      Serial.println(baseStation.toJSON());
    } else {
      Serial.println("  -> FAILED: Data mismatch during unpacking.");
    }
    Serial.println();
}

void setup() {
  // Start the serial monitor at a high baud rate
  Serial.begin(115200);
  
  // Wait a moment for the Serial Monitor to connect 
  // (Crucial for ESP32-S3/M5Stamp boards with native USB)
  delay(3000); 
  
  Serial.println("\n--- Starting PocketCodec Hardware Tests ---\n");

  // TEST 1: Standard St. George Summer Day
  runCompressionTest(
      "Standard St. George Summer",
      1, 37.2144, -113.6402, 960.5, 106.34, 12.8, 85.5, 0, 3600
  );

  // TEST 2: Winter Night Edge Case
  runCompressionTest(
      "Pine Valley Mountain Winter Night",
      2, 37.3855, -113.4842, 3144.0, -4.5, 45.2, 15.2, 1, 86400
  );

  // TEST 3: Zeroes and Max Limits
  runCompressionTest(
      "Zeroes and Max Limits",
      255, 0.0, 0.0, -86.0, 0.0, 0.0, 100.0, 255, 3932100
  );

  Serial.println("--- All Tests Completed ---");
}

void loop() {
  // Tests run exactly once on boot. 
  // We leave the loop empty so it doesn't spam the serial monitor.
}