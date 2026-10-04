# System Architecture

## 1. Introduction

The Harare Office and Street Parking Management System is an IoT-based parking-bay monitoring prototype designed to detect and display the occupancy status of four parking bays.

The architecture consists of four main functional layers:

1. Sensing layer
2. Edge processing and control layer
3. Communication layer
4. Application and output layer

The system uses four HC-SR04 ultrasonic sensors connected to an ESP32 development board. The ESP32 processes the sensor measurements and determines whether each parking bay is occupied or available. The resulting information is displayed locally on a 16×2 I²C LCD and through a web-based dashboard accessed over the ESP32's local Wi-Fi network.

---

## 2. High-Level Architecture

```text
                  PARKING ENVIRONMENT
                         │
                         ▼
              ┌─────────────────────┐
              │   SENSING LAYER     │
              │                     │
              │ HC-SR04 Sensor 1    │
              │ HC-SR04 Sensor 2    │
              │ HC-SR04 Sensor 3    │
              │ HC-SR04 Sensor 4    │
              └──────────┬──────────┘
                         │
                  Distance Readings
                         │
                         ▼
              ┌─────────────────────┐
              │   EDGE PROCESSING   │
              │                     │
              │       ESP32         │
              │                     │
              │ Sensor Acquisition  │
              │ Reading Validation  │
              │ Median Processing   │
              │ Occupancy Decision  │
              └──────────┬──────────┘
                         │
                Parking Status Data
                         │
              ┌──────────┴──────────┐
              │                     │
              ▼                     ▼
     ┌─────────────────┐    ┌──────────────────┐
     │ LOCAL OUTPUT    │    │ COMMUNICATION    │
     │                 │    │                  │
     │ 16×2 I²C LCD    │    │ ESP32 Wi-Fi AP   │
     └─────────────────┘    └────────┬─────────┘
                                    │
                              Local Wi-Fi
                                    │
                                    ▼
                         ┌────────────────────┐
                         │ APPLICATION LAYER │
                         │                    │
                         │ Web Dashboard      │
                         │                    │
                         │ Bay 1 Status       │
                         │ Bay 2 Status       │
                         │ Bay 3 Status       │
                         │ Bay 4 Status       │
                         │ Available Bays     │
                         │ Occupied Bays      │
                         └────────────────────┘