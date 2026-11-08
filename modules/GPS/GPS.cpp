#include "GPS.h"

GPS::GPS(HardwareSerial &serialPort, uint32_t baudRate)
    : _serialGPS(serialPort), _baudRate(baudRate) {}

bool GPS::begin() {
    // Start the hardware serial port for the RAK12501 module on Serial1
    _serialGPS.begin(_baudRate);
    delay(500);
    return true;
}

void GPS::update() {
    // Continuously read available bytes from the Quectel L76K module and feed TinyGPS++
    while (_serialGPS.available() > 0) {
        _gps.encode(_serialGPS.read());
    }
}

bool GPS::getGPS(double &latitude, double &longitude, float &altitude, uint8_t &satellites) {
    update();

    if (_gps.location.isUpdated() && _gps.location.isValid()) {
        latitude = _gps.location.lat();
        longitude = _gps.location.lng();
        altitude = _gps.altitude.isValid() ? _gps.altitude.meters() : 0.0;
        satellites = _gps.satellites.isValid() ? _gps.satellites.value() : 0;
        return true;
    }

    return false;
}

bool GPS::isLocationValid() {
    update();
    return _gps.location.isValid();
}

uint32_t GPS::getAge() {
    return _gps.location.age();
}