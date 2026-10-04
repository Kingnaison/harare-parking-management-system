# ESP32 Parking Management System – GPIO Mapping

## System Overview

The parking management prototype uses an ESP32 development board to monitor four individual parking bays using HC-SR04 ultrasonic sensors. The ESP32 processes the sensor readings and provides parking-bay occupancy information through a web-based dashboard over its local Wi-Fi access point.

A 16×2 I²C LCD is also connected to the ESP32 to provide a local indication of the number of available parking bays.

## Ultrasonic Sensor Connections

| Parking Bay | Sensor | TRIG GPIO | ECHO GPIO |
|-------------|--------|-----------|-----------|
| Bay 1 | HC-SR04 | GPIO 13 | GPIO 35 |
| Bay 2 | HC-SR04 | GPIO 14 | GPIO 34 |
| Bay 3 | HC-SR04 | GPIO 25 | GPIO 32 |
| Bay 4 | HC-SR04 | GPIO 26 | GPIO 33 |

## LCD Connections

| LCD Pin | ESP32 GPIO |
|---------|------------|
| SDA | GPIO 21 |
| SCL | GPIO 22 |
| I²C Address | 0x27 |

## Parking Occupancy Logic

The system classifies a parking bay as occupied when a valid ultrasonic measurement is between 3 cm and 9 cm inclusive.

- 3–9 cm: OCCUPIED
- Outside this range: AVAILABLE
- Invalid/no-echo reading: previous occupancy state is retained

Each sensor is sampled three times to improve measurement reliability. Where three valid readings are available, the median value is used for occupancy classification.

## Communication

The ESP32 operates as a local Wi-Fi access point.

**SSID:** `Sulindika_Parking`

**Password:** `Parking123`

**Dashboard IP address:** `192.168.4.1`

The web dashboard displays the occupancy status of each parking bay, the total number of bays, the number of available bays and the number of occupied bays.

## Local Display

The 16×2 I²C LCD displays:

`PARKING BAYS`

`LEFT: X`

where `X` represents the current number of available parking bays.

## System Data Flow

HC-SR04 Sensors
        ↓
ESP32
        ↓
Occupancy Classification
        ↓
Local LCD

ESP32
        ↓
Wi-Fi Access Point
        ↓
Web Dashboard
        ↓
Parking Bay Status