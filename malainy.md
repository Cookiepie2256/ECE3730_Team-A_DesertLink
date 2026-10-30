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

## Individual Contribution: Malainy

### My Responsibility
I will develop the data compression and data unpacking sections of the code. 

### My Files
- `data_handling.h` -The class definions for data handling 
- `data_handling.cpp` -The functions for data handling

### My Inputs and Outputs
| Direction | Data | Type / Units / Format | Source or Destination |
|-----------|------|-----------------------|-----------------------|
| Input | temp | uint16 | sensor reading |
| Output | temp_formatted | uint8 | antenna for communication |

### My Test Plan

I will define realistic numbers for each sensor, call the functions to format it, then update the data. I'll create a dummy function to unpack the data and ensure it matches. 

Input: uint16 temp
Output: uint8 temp_formatted