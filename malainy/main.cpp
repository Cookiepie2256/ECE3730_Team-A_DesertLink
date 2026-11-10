#include <iostream>
#include <cassert>
#include <cmath>
#include "PocketCodec.h"

// Helper function to compare floats with a small tolerance, 
// since our compression intentionally loses some decimal precision.
bool isClose(float a, float b, float tolerance = 0.11) {
    return std::abs(a - b) < tolerance;
}

void runCompressionTest(
    const std::string& testName,
    uint8_t id, float lat, float lon, float alt, 
    float temp, float hum, float batt, 
    uint8_t status, uint32_t uptimeSec
) {
    std::cout << "Running Test: " << testName << "...\n";

    // ==========================================
    // 1. NODE: PACK THE DATA
    // ==========================================
    PocketCodec node(id);
    node.packLocation(lat, lon, alt);
    node.packEnvironment(temp, hum);
    node.packSystemState(batt, status, uptimeSec);

    // ==========================================
    // 2. SIMULATE LoRa TRANSMISSION
    // ==========================================
    uint8_t* txBuffer = node.getEncodedBytes();
    size_t packetSize = node.getPayloadSize();
    
    // Ensure we hit exactly 18 bytes
    assert(packetSize == 18); 

    // ==========================================
    // 3. BASE STATION: UNPACK THE DATA
    // ==========================================
    PocketCodec baseStation;
    baseStation.loadPayload(txBuffer);

    // ==========================================
    // 4. VERIFY EDGE CASES AND COMPRESSION
    // ==========================================
    
    // Exact matches (Data that shouldn't lose any precision)
    assert(baseStation.unpackNodeId() == id);
    assert(baseStation.unpackLatitude() == lat);
    assert(baseStation.unpackLongitude() == lon);
    assert(baseStation.unpackStatusFlag() == status);

    // Altitude: Cast from float to int16_t (Loses decimals)
    assert(baseStation.unpackAltitude() == (int16_t)alt);

    // Temperature: Stored as int16_t * 10 (Loses hundreds/thousands precision)
    assert(isClose(baseStation.unpackTemperature(), temp));

    // Humidity & Battery: Cast to uint8_t (Drops all decimals)
    assert(baseStation.unpackHumidity() == (uint8_t)hum);
    assert(baseStation.unpackBattery() == (uint8_t)batt);

    // Uptime: Converted from seconds to minutes
    assert(baseStation.unpackUptimeMinutes() == (uint16_t)(uptimeSec / 60));

    std::cout << "  -> PASSED. JSON Output: " << baseStation.toJSON() << "\n\n";
}

int main() {
    std::cout << "--- Starting PocketCodec Compression Tests ---\n\n";

    // TEST 1: Standard St. George Summer Day
    // High heat, low humidity, standard hiking altitude (Snow Canyon State Park)
    runCompressionTest(
        "Standard St. George Summer",
        1, 
        37.2144f, -113.6402f, 960.5f,  // Snow Canyon Lat, Lon, Alt (meters)
        106.34f, 12.8f,                // 106.3°F, ~12% humidity (Decimals will be truncated)
        85.5f, 0,                      // 85% battery, status OK
        3600                           // 1 hour uptime (3600 seconds = 60 mins)
    );

    // TEST 2: Winter Night Edge Case (Negative Temp, High Altitude)
    // Testing if int16_t correctly handles negative temperatures and high elevations
    runCompressionTest(
        "Pine Valley Mountain Winter Night (Negative Temp)",
        2, 
        37.3855f, -113.4842f, 3144.0f, // Pine Valley peak (over 10,000 ft)
        -4.5f, 45.2f,                  // -4.5°F
        15.2f, 1,                      // 15% battery, status 1 (Warning)
        86400                          // 24 hours uptime (1440 mins)
    );

    // TEST 3: Extreme Limits & Zeros (Data boundaries)
    // Testing 0 values, exact limits of uint8_t (255), and max uptime
    runCompressionTest(
        "Zeroes and Max Limits",
        255,                           // Max uint8_t node ID
        0.0f, 0.0f, -86.0f,            // 0 Lat/Lon, Death Valley elevation (-86m)
        0.0f, 0.0f,                    // 0°F, 0% humidity
        100.0f, 255,                   // 100% battery, Max status flag
        3932100                        // ~45 days uptime (65535 minutes - Max uint16_t)
    );

    std::cout << "--- All Tests Passed Successfully! ---\n";
    return 0;
}