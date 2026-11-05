# ECE3730_Team A_DesertLink
TrailLink uses LoRa wireless communication to send GPS location, temperature, and humidity from portable field nodes to a central station. 

## Project Overview
A fun team project with a search-and-rescue connection

## Team Members
| Name | GitHub Username | Responsibility |
|------|-----------------|----------------|
| Malainy | malainyl | data packing |
| Gage | gbo012 | gps module |
| Kevin | EliteDarkLord35 | multi-node monitoring |
| Tanner | tanjeffris | wireless communication |
| Andy | Andy-Holt | environmental sensor |

## Individual Contribution: Kevin

### My Responsibility
multi-node monitoring

### My Files
* `gps_driver.py` (or `gps_parser.cpp` / `.h`) – Interfacing logic for hardware UART communication, checksum verification, and NMEA sentence parsing (`$GPRMC`, `$GGGA`).
* `gps_test.py` – Standalone test script to verify serial output and satellite lock status.

### My Inputs and Outputs
* **Input:** Raw NMEA 0183 serial stream (UART via RX/TX pins at 9600 baud).
* **Output:** Formatted floating-point GPS coordinates (latitude, longitude, altitude) and fix/satellite status flag.

### My Test Plan
1. **Serial Communication Test:** Connect the GPS module to the microcontroller and verify raw ASCII NMEA sentence streams (`$GPRMC`, `$GGGA`) over the serial interface.
2. **Satellite Lock & Fix Test:** Power on the module outdoors and verify that valid fix flags (`fix_quality > 0`) are set once satellite lock is acquired.
3. **Coordinate Parsing Accuracy Test:** Verify that parsed latitude and longitude output matches known control coordinates (e.g., tested location) and correctly converts NMEA raw formats to standard decimal degrees.
4. **Signal Degraded Recovery Test:** Obstruct the antenna to simulate signal loss, verifying that invalid fix flags are caught cleanly and coordinate reporting resumes automatically upon re-acquiring lock.
