#include "GPS.h"

GPS myGPS(Serial1, 9600);

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);

    myGPS.begin();
    Serial.println("M5Stamp S3 Software-Only GPS Test Initialized.");
}

void loop() {
    // Feed mock sentences that have valid, matching NMEA checksums (*63 and *47)
    myGPS.feedData("$GPRMC,173206.00,A,0615.95887,N,07535.95863,W,0.047,,310118,,,D*63\r\n");
    myGPS.feedData("$GPGGA,173206.00,0615.95887,N,07535.95863,W,2,07,1.24,1572.1,M,2.5,M,,0000*47\r\n");

    double lat = 0.0;
    double lon = 0.0;
    float alt = 0.0;
    uint8_t sats = 0;

    // Check if your class successfully parses the valid fed data
    if (myGPS.getGPS(lat, lon, alt, sats)) {
        Serial.println("--- GPS Fix Parsed Successfully via Software! ---");
        Serial.print("Latitude: "); Serial.println(lat, 6);
        Serial.print("Longitude: "); Serial.println(lon, 6);
        Serial.print("Altitude: "); Serial.print(alt); Serial.println(" m");
        Serial.print("Satellites: "); Serial.println(sats);
    } else {
        Serial.println("Waiting for fix...");
    }

    delay(2000);
}