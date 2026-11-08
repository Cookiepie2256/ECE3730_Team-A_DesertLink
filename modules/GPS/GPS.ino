#include "GPS.h"

// Instantiate the GPS object using standard Serial1
GPS myGPS(Serial1, 9600);

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    if (myGPS.begin()) {
        Serial.println("GPS module initialized successfully!");
    } else {
        Serial.println("Failed to initialize GPS module.");
    }
}

void loop() {
    double lat = 0.0;
    double lon = 0.0;
    float alt = 0.0;
    uint8_t sats = 0;

    if (myGPS.getGPS(lat, lon, alt, sats)) {
        Serial.print("Latitude: "); Serial.println(lat, 6);
        Serial.print("Longitude: "); Serial.println(lon, 6);
        Serial.print("Altitude (m): "); Serial.println(alt);
        Serial.print("Satellites: "); Serial.println(sats);
    }

    delay(1000);
}