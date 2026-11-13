#include "PocketCodec.h"

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
String PocketCodec::toJSON() {
  String json = "{";
  json += "\"node_id\":" + String(unpackNodeId()) + ",";
  json += "\"latitude\":" + String(unpackLatitude(), 5) + ",";
  json += "\"longitude\":" + String(unpackLongitude(), 5) + ",";
  json += "\"altitude_m\":" + String(unpackAltitude()) + ",";
  json += "\"temperature_f\":" + String(unpackTemperature(), 1) + ",";
  json += "\"humidity_percent\":" + String(unpackHumidity()) + ",";
  json += "\"battery_percent\":" + String(unpackBattery()) + ",";
  json += "\"status_flag\":" + String(unpackStatusFlag()) + ",";
  json += "\"uptime_minutes\":" + String(unpackUptimeMinutes());
  json += "}";
  
  return json;
}