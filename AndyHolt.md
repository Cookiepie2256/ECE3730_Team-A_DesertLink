## Team Members
| Name | GitHub Username | Responsibility |
|------|-----------------|----------------|
| Andy Holt | Andy-Holt | Environmental Sensor |

## Individual Contribution: Andy Holt

### My Responsibility
I built the Environmental Sensor Module (`EnvSensor`). It reads temperature and humidity from the RAK1901 (SHTC3) sensor over I2C on the RAK4631, checks that the values are good, and passes them to the data packet module as scaled integers.

**Code:** [modules/EnvSensor](https://github.com/Cookiepie2256/ECE3730_Team-A_DesertLink/tree/main/modules/EnvSensor)

### My Files
- `EnvSensor.h`: class declaration
- `EnvSensor.cpp`: sensor startup, reading, and range checks
- `EnvSensor.ino`: test sketch (Serial Monitor at 115200)

Needs the **Adafruit SHTC3** library.

### How to Use
```cpp
EnvSensor env;
env.begin();   // in setup()
env.read();    // in loop()
packet.updateEnvironment(env.getTempC100(), env.getHumidity100(), env.isValid());
```

### My Inputs and Outputs
| Direction | Data | Type / Units | Source or Destination |
|-----------|------|--------------|-----------------------|
| Input | Temperature and humidity | I2C, address 0x70 | RAK1901 sensor |
| Input | Read request | `env.read()` | Main loop |
| Output | Temperature | `int16_t`, °C × 100 (23.45 °C = 2345) | Malainy's `updateEnvironment()` |
| Output | Humidity | `uint16_t`, % × 100 (35.20 % = 3520) | Malainy's `updateEnvironment()` |
| Output | Valid flag | `bool` | Malainy's `updateEnvironment()` (packet flag bit 1) |

### My Test Results
| Test | Expected | Observed |
|------|----------|----------|
| Startup | "Sensor found" | _TODO_ |
| Room reading | Close to room temp and humidity | _TODO_ |
| Breathe on sensor | Humidity goes up | _TODO_ |
| Unplug sensor | "Valid: 0" | _TODO_ |

**Screenshot:** _TODO_
