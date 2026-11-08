#ifndef GPS_H
#define GPS_H

#include <Arduino.h>
#include <TinyGPSPlus.h>

class GPS {
public:
    // Constructor allows passing a custom hardware serial port and baud rate
    GPS(HardwareSerial &serialPort = Serial1, uint32_t baudRate = 9600);

    // Initialize the module and underlying serial communication
    bool begin();

    // Must be called frequently in loop() to ingest incoming serial data
    void update();

    // Main requested method: returns true if fresh valid GPS data is parsed
    bool getGPS(double &latitude, double &longitude, float &altitude, uint8_t &satellites);

    // Optional getters for individual data points
    bool isLocationValid();
    uint32_t getAge();

private:
    HardwareSerial &_serialGPS;
    uint32_t _baudRate;
    TinyGPSPlus _gps;
};

#endif // GPS_H