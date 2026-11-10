#ifndef POCKET_CODEC_H
#define POCKET_CODEC_H

#include <cstdint>
#include <cstring>
#include <string>

// 1. The 18-byte packed struct
struct __attribute__((packed)) RescuerData {
  uint8_t nodeId;
  float latitude;
  float longitude;
  int16_t altitude;
  int16_t temperature;
  uint8_t humidity;
  uint8_t batteryLevel;
  uint8_t statusFlag;
  uint16_t uptimeMinutes;
};

// 2. The Codec Class Declaration
class PocketCodec {
  private:
    RescuerData payload;

  public:
    // --- CONSTRUCTORS ---
    PocketCodec(uint8_t nodeId); // For the Node (Sender)
    PocketCodec();               // For the Base Station (Receiver)

    // --- NODE FUNCTIONS (PACKING) ---
    void packLocation(float latitude, float longitude, float altitudeMeters);
    void packEnvironment(float tempFahrenheit, float humidityPercent);
    void packSystemState(float batteryPercent, uint8_t statusFlag, uint32_t uptimeSeconds);
    
    uint8_t* getEncodedBytes();
    size_t getPayloadSize();

    // --- BASE STATION FUNCTIONS (UNPACKING) ---
    void loadPayload(uint8_t* rawLoRaData);
    
    uint8_t unpackNodeId();
    float unpackLatitude();
    float unpackLongitude();
    int16_t unpackAltitude();
    float unpackTemperature();
    uint8_t unpackHumidity();
    uint8_t unpackBattery();
    uint8_t unpackStatusFlag();
    uint16_t unpackUptimeMinutes();
    
    std::string toJSON();
};

#endif