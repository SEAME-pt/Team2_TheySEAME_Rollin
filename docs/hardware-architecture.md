# Hardware 

## What we used

- [PiRacer Kit](#PiRacer)
- [Raspberry Pi 5](#Raspberry-Pi-5)
- [Raspberry Pi AI HAT+](#Raspberry-Pi-AI-HAT+)
- [7.9 inch Waveshare Display](#Cluster-Display)
- [CANBUS](#Can-Controller-and-Transceiver)
- [Microncontroller](#Microcontroller)
- [Sensors](#Sensors)

## PiRacer

This project uses the Waveshare PiRacer Kit as a base to build our Car

This is the base of our Car and allows the communication with the servo and motors through the expansion board via I2C

- Expansion Board
- 2 Gearmotors
- MG996R servo
- 3 Battery slots 3.6V/4.2V
- 2 5V, 1 3.3V and 1 GND pins
- 1 SDA and SCL I2C pins

## Raspberry Pi 5

This project uses the Raspberry Pi 5 as the central unit between devices

It has to pass the measures to show in the display, pass car controls to the microcontroller 
and process the autonomous driving with the help of the AI HAT

## Raspberry Pi AI HAT+

This project uses the Raspberry Pi AI HAT+ as the heavy AI processor
It has a Hailo-8 AI acceleration chip with 26 TOPS

It indentifies the road signals and other road objects

## Microcontroller

This project uses the STM32 board B-U585I-IOT02A

This device is responsible for controlling the Car movement and gathering sensor data.
This tasks are done under a RTOS (Real Time Operating System)

This device has 2 MB FLASH, 786 KB RAM, Wi-Fi, Bluetooth and a lot of built-in sensors

## Can Controller and Transceiver

This project uses the MCP2515 board to create CANBUS communication

It implments CAN V2.0B at 1 Mb/s:
- 0 to 8-byte length in data field
- Standard and extended data and remote
frames 

- MCP2515: CAN Controller
- TJA1050: CAN Transceiver

This connects the [STM32 MCU](#Microcontroller) and the [Raspberry Pi 5](#Raspberry-Pi-5) via CANBUS

## Cluster Display

This project uses a 7.9 DSI LCD Waveshare Display to show a QT app with information about the Car

It has a resolution of 400x1480 pixels, a touch-screen and 60Hz refresh rate
It is connected via a DSI cable to the [Raspberry Pi 5](#Raspberry-Pi-5)

## Sensors

This projects uses some sensors to gather information about the Car

- Speedsensor: LM393 Sen-Speed

