// Arduino.h (SIMULATION STUB)
// Minimal stand-in for the Arduino core so Wireless.cpp and Wireless.ino
// can be compiled and run on a Linux PC. Not used on real hardware.
#pragma once
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <chrono>
#include <thread>

#define OUTPUT 1
#define HIGH 1
#define LOW 0
#define HEX 16
#define LED_GREEN 35
#define LED_BLUE 36

inline uint32_t millis() {
  static const auto t0 = std::chrono::steady_clock::now();
  return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - t0).count();
}

void sim_poll();  // services the fake radio (defined in SX126x-Arduino.h)

inline void delay(uint32_t ms) {
  uint32_t start = millis();
  do {
    sim_poll();
    std::this_thread::sleep_for(std::chrono::microseconds(200));
  } while (millis() - start < ms);
}

inline void pinMode(int, int) {}
inline void digitalWrite(int, int) {}

struct SimSerial {
  void begin(unsigned long) {}
  explicit operator bool() const { return true; }
  void print(const char* s) { fputs(s, stdout); fflush(stdout); }
  void print(char c) { putchar(c); fflush(stdout); }
  void print(unsigned v, int base) { printf(base == HEX ? "%X" : "%u", v); fflush(stdout); }
  void println(const char* s = "") { puts(s); fflush(stdout); }
  void printf(const char* fmt, ...) {
    va_list ap; va_start(ap, fmt); vprintf(fmt, ap); va_end(ap); fflush(stdout);
  }
};
inline SimSerial Serial;
