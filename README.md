# DESIGNING AND DEVELOPING OFFICE, STREET PARKING MANAGEMENT SYSTEM OF HARARE

## Project Overview

The **Harare Office and Street Parking Management System** is an IoT-based parking-bay monitoring prototype designed to improve the visibility of parking-bay occupancy in selected office and street parking environments in Harare, Zimbabwe.

The system uses an **ESP32 development board**, four **HC-SR04 ultrasonic sensors**, a **16×2 I²C LCD**, Wi-Fi communication and a web-based dashboard to detect and display the occupancy status of individual parking bays.

The prototype provides real-time parking information without requiring users to manually inspect each parking bay. The ESP32 operates as the edge controller, processes ultrasonic sensor readings and provides the resulting parking information through a local Wi-Fi network.

The project is intended as an academic prototype demonstrating the application of **Internet of Things (IoT)** technologies to an urban parking-management problem.

---

## Project Aim

The aim of the project is:

> **To design and develop an IoT-based office and street parking management system for Harare using ultrasonic sensors, an ESP32 development board, Wi-Fi communication and a web-based dashboard to provide real-time parking-bay occupancy information.**

---

## Project Objectives

The project has three main objectives:

1. **To design and develop an IoT-based parking-bay monitoring prototype using ultrasonic sensors and an ESP32 development board to detect the occupancy status of individual parking bays.**

2. **To develop a web-based dashboard that receives and displays real-time parking-bay occupancy information transmitted by the ESP32 through Wi-Fi.**

3. **To evaluate the accuracy, reliability, communication performance and basic cybersecurity of the developed IoT parking-bay management system under controlled testing conditions.**

---

## System Architecture

The system follows a simple IoT architecture consisting of sensing, edge processing, communication and application/output layers.

```text
        PARKING BAYS
             │
             ▼
     ┌─────────────────┐
     │  HC-SR04        │
     │  Ultrasonic     │
     │  Sensors × 4    │
     └────────┬────────┘
              │
              ▼
     ┌─────────────────┐
     │      ESP32      │
     │                 │
     │ Sensor Reading  │
     │ Processing      │
     │ Occupancy       │
     │ Classification  │
     └───────┬─────────┘
             │
       ┌─────┴─────┐
       │           │
       ▼           ▼
 ┌──────────┐   ┌──────────────┐
 │ 16×2 LCD │   │ Local Wi-Fi  │
 │ Display  │   │ Access Point │
 └──────────┘   └───────┬──────┘
                        │
                        ▼
               ┌─────────────────┐
               │ Web Dashboard    │
               │                 │
               │ Bay Status      │
               │ Available Bays  │
               │ Occupied Bays   │
               └─────────────────┘# harare-parking-management-system
source code for the harare smart street parking
