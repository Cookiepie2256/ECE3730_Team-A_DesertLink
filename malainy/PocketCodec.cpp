#include "PocketCodec.h"
#include <sstream>
#include <iomanip>

// ==========================================
//               CONSTRUCTORS
// ==========================================

// Use this constructor on the NODE (Sender)
PocketCodec::PocketCodec(uint8_t nodeId) {
  memset(&payload, 0, sizeof(RescuerData));
  payload.nodeId = nodeId;
}

// Use this constructor on the BASE STATION (Receiver)
PocketCodec::PocketCodec() {
  memset(&payload, 0, sizeof(RescuerData));
}

// ==========================================
//         NODE FUNCTIONS (PACKING)
// ==========================================

void PocketCodec::packLocation(float latitude, float longitude, float altitudeMeters) {
  payload.latitude = latitude;
  payload.longitude = longitude;
  payload.altitude = (int16_t)altitudeMeters;
}

void PocketCodec::packEnvironment(float tempFahrenheit, float humidityPercent) {
  payload.temperature = (int16_t)(tempFahrenheit * 10.0);
  payload.humidity = (uint8_t)humidityPercent;
}

void PocketCodec::packSystemState(float batteryPercent, uint8_t statusFlag, uint32_t uptimeSeconds) {
  payload.batteryLevel = (uint8_t)batteryPercent;
  payload.statusFlag = statusFlag;
  payload.uptimeMinutes = (uint16_t)(uptimeSeconds / 60);
}

uint8_t* PocketCodec::getEncodedBytes() {
  return (uint8_t*)&payload;
}

size_t PocketCodec::getPayloadSize() {
  return sizeof(RescuerData);
}

// ==========================================
//      BASE STATION FUNCTIONS (UNPACKING)
// ==========================================

void PocketCodec::loadPayload(uint8_t* rawLoRaData) {
  // Safely copy the incoming bytes into our struct structure
  memcpy(&payload, rawLoRaData, sizeof(RescuerData));
}

uint8_t PocketCodec::unpackNodeId() {
  return payload.nodeId;
}

float PocketCodec::unpackLatitude() {
  return payload.latitude;
}

float PocketCodec::unpackLongitude() {
  return payload.longitude;
}

int16_t PocketCodec::unpackAltitude() {
  return payload.altitude;
}

float PocketCodec::unpackTemperature() {
  // Restore the decimal point
  return (float)payload.temperature / 10.0;
}

uint8_t PocketCodec::unpackHumidity() {
  return payload.humidity;
}

uint8_t PocketCodec::unpackBattery() {
  return payload.batteryLevel;
}

uint8_t PocketCodec::unpackStatusFlag() {
  return payload.statusFlag;
}

uint16_t PocketCodec::unpackUptimeMinutes() {
  return payload.uptimeMinutes;
}

// Formats the unpacked data into a JSON string for serial output

std::string PocketCodec::toJSON() {
  std::ostringstream json;
  
  json << "{";
  
  // Cast uint8_t to int so g++ prints numbers, not ASCII characters
  json << "\"node_id\":" << (int)unpackNodeId() << ",";
  
  // Lock floats to standard decimal notation instead of scientific
  json << std::fixed;
  
  json << "\"latitude\":" << std::setprecision(5) << unpackLatitude() << ",";
  json << "\"longitude\":" << std::setprecision(5) << unpackLongitude() << ",";
  json << "\"altitude_m\":" << unpackAltitude() << ",";
  json << "\"temperature_f\":" << std::setprecision(1) << unpackTemperature() << ",";
  
  // Cast the remaining uint8_t fields to int
  json << "\"humidity_percent\":" << (int)unpackHumidity() << ",";
  json << "\"battery_percent\":" << (int)unpackBattery() << ",";
  json << "\"status_flag\":" << (int)unpackStatusFlag() << ",";
  
  // uint16_t prints as a number automatically, no cast needed
  json << "\"uptime_minutes\":" << unpackUptimeMinutes(); 
  
  json << "}";
  
  return json.str();
}