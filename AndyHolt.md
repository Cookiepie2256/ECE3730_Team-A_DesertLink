## Team Members
| Name | GitHub Username | Responsibility |
|------|-----------------|----------------|
| Andy Holt | Andy-Holt | Environmental Sensor |

## Individual Contribution: Andy Holt

### My Responsibility
I am developing the Environmental Sensor Module. It is a C++ class that initializes the RAK1901 (SHTC3) temperature and humidity sensor over I2C on the RAK4631, takes readings, checks that they are valid, and passes them to the data packet module for transmission.

### My Files
- `EnvSensor.h`: Class declaration for the environmental sensor (init, read, getters, status flag).
- `EnvSensor.cpp`: Implements sensor startup, I2C reads, range checking, and error handling.

### My Inputs and Outputs
| Direction | Data | Type / Units / Format | Source or Destination |
|-----------|------|-----------------------|-----------------------|
| Input | Raw temperature and humidity | I2C data from SHTC3 (address 0x70) | RAK1901 sensor |
| Input | Read request | Function call | Main loop |
| Output | Temperature | `float`, °C | Data packet module |
| Output | Relative humidity | `float`, %RH | Data packet module |
| Output | Sensor status | `bool` (true = valid reading) | Data packet module / main loop |

### My Test Plan
1. **Room reading:** Read the sensor at room temperature and compare to a reference thermometer. Expected: temperature within ±1 °C, humidity within ±5 %RH.
2. **Response test:** Breathe on the sensor. Expected: humidity and temperature rise within a few seconds, then return to baseline.
3. **Fault test:** Unplug the sensor and call read. Expected: status returns false and no bad values are sent.