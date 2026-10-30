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

## Individual Contribution: Gage

### My Responsibility
GPS Module

### My Files
gps.h
gps.cpp

### My Inputs and Outputs
Input: NMEA serial stream
Output: GPS Coordinates

### My Test Plan

I can connect the module directly to my computer, and use a library on Arduino IDE (TinyGPSPlus) to create test code to give me my current location