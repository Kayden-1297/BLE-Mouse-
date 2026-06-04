# BLE Mouse

## Description

This project implements a wireless Bluetooth Low Energy (BLE) Air Mouse using an ESP32 microcontroller, MPU6050 motion sensor, and Flex Sensor. The system allows users to control the computer cursor through hand movements and perform mouse click operations using finger bending gestures.
The project demonstrates embedded systems concepts such as sensor interfacing, motion tracking, Bluetooth communication, real-time data processing, and human-computer interaction.

## Hardware Components

* ESP32 Development Board
* MPU6050 Accelerometer and Gyroscope Sensor
* Flex Sensor
* Jumper Wires
* Breadboard
* USB Cable
* Pcb
* 20k ohm Resistor - 2

## Software Used

* Arduino IDE
* ESP32 Board Package
* BLE Mouse Library
* Wire Library (I2C Communication)

## Features

* Wireless Bluetooth Low Energy Mouse
* Cursor movement using hand gestures
* Mouse click detection using Flex Sensor
* Real-time motion tracking
* Low power consumption
* Portable and user-friendly design

## Working Principle

1. MPU6050 detects hand orientation and movement.
2. ESP32 processes accelerometer and gyroscope data.
3. Cursor movement is generated based on sensor readings.
4. Flex Sensor detects finger bending.
5. Finger bending triggers mouse click events.
6. ESP32 communicates with the computer via BLE and acts as a wireless mouse.

## Folder Structure

/project-root
│
├── README.md
├── src/
│   └── ble_air_mouse.ino
│
├── docs/
│   ├── circuit_diagram.png
│   ├── block_diagram.png
│   └── project_report.pdf
│
└── images/
└── prototype.jpg

## Installation

1. Install Arduino IDE.
2. Install ESP32 Board Package.
3. Install BLE Mouse Library.
4. Connect ESP32 via USB.
5. Upload the source code.
6. Pair the ESP32 with your computer through Bluetooth.
7. Use hand gestures to control the cursor.

## Applications

* Touchless Human Computer Interaction
* Smart Presentation Control
* Accessibility Solutions
* Gaming and Virtual Reality Interfaces
* IoT-based Input Devices

## Future Enhancements

* Gesture recognition for multiple commands
* Scroll and drag functionality
* Rechargeable battery integration
* Machine learning-based gesture classification

## Author

Shivam Devkar 
Electronics and Computer Engineering
