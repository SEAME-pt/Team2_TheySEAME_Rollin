# Hardware 

This is the list of hardware components and their respective responsabilities in this project.

There's also a [diagram](https://github.com/SEAME-pt/Team2_TheySEAME_Rollin/blob/develop/docs/architecture/Hardware%20Architecture.pdf)
which shows how we organized and connected each component.

## What we used

- [PiRacer Kit](#PiRacer)
- [Raspberry Pi 5](#Raspberry-Pi-5)
- [Raspberry Pi AI HAT+](#Raspberry-Pi-AI-HAT)
- [Raspberry Pi Camera](#Raspberry-Pi-Camera)
- [Microncontroller](#Microcontroller)
- [CANBUS](#Can-Controller-and-Transceiver)
- [Cluster Display](#Cluster-Display)
- [Sensors](#Sensors)

## PiRacer

The Waveshare PiRacer Kit is the base to build our Car

It allows the communication with the servo and motors through the expansion board via I2C

- Expansion Board:
    - 3 Battery slots 3.6V/4.2V
    - 2 5V, 1 3.3V and 1 GND pins
    - 1 SDA and SCL I2C pins
- 2 Gearmotors
- MG996R servo
- Car Chassi

## Raspberry Pi 5

The Raspberry Pi 5 is the central unit between devices

It passes the measures to the [Cluster Displaye](#Cluster-Display), process the [Raspberry Pi Camera](#Raspberry-Pi-Camera) images
and pass Car controls to the [Microncontroller](#Microcontroller)
and process the autonomous driving with the help of the [Raspberry Pi AI HAT+](#Raspberry-Pi-AI-HAT)

Datasheet/Manual:
- Manuals: https://www.raspberrypi.com/documentation/

## Raspberry Pi AI HAT

The Raspberry Pi AI HAT+ is the heavy AI processor.
It has a Hailo-8 AI acceleration chip with 26 TOPS of performance

It indentifies the road signals and other road objects.
It passes that information to the [Raspberry Pi 5](#Raspberry-Pi-5)

Datasheet/Manual:
- Manuals: https://www.raspberrypi.com/documentation/computers/ai.html

## Raspberry Pi Camera

The Raspberry Pi Camera Module 3 is responsible for streaming the road.
This stream will have AI processing in real-time by the [Raspberry Pi AI HAT](#Raspberry-Pi-AI-HAT).
Then this information is passed to the [Raspberry Pi 5](#Raspberry-Pi-5)

It is connected to the [Raspberry Pi 5](#Raspberry-Pi-5) via 22-way FPC connector

Datasheet/Manual:
- Manuals: https://www.raspberrypi.com/documentation/computers/camera_software.html

## Microcontroller

The Microcontroller is a STM32 B-U585I-IOT02A.
This Micro runs a RTOS (Real Time Operating System) and is responsible for controlling the Car movement and gathering sensor data.

This device has 2 MB FLASH, 786 KB RAM, Wi-Fi, Bluetooth and a lot of built-in sensors

Datasheet/Manual:
- Manual: https://www.st.com/resource/en/user_manual/um2839-discovery-kit-for-iot-node-with-stm32u5-series-stmicroelectronics.pdf
- MCU datasheet: https://www.st.com/resource/en/datasheet/stm32u585ai.pdf 

## Can Controller and Transceiver

The HW-184 board creates the CANBUS.
This enables communication via CAN between the [Microcontroller](#Microcontroller) and the [Raspberry Pi 5](#Raspberry-Pi-5)

It implments CAN V2.0B at 1 Mb/s:
- 0 to 8-byte length in data field
- Standard and extended data and remote
frames 
- MCP2515: CAN Controller
- TJA1050: CAN Transceiver

This connects the [STM32 MCU](#Microcontroller) and the [Raspberry Pi 5](#Raspberry-Pi-5) via CANBUS

Datasheet/Manual:
- MCP2515 product page: https://www.microchip.com/en-us/product/MCP2515
- TJA1050 datasheet: https://www.nxp.com/docs/en/data-sheet/TJA1050.pdf

## Cluster Display

The display is a 7.9 DSI LCD Waveshare Display.
It is responsible to show a QT app with information about the Car

It has a resolution of 400x1480 pixels, a touch-screen and 60Hz refresh rate
It is connected via a DSI cable to the [Raspberry Pi 5](#Raspberry-Pi-5)

## Sensors

This projects uses some sensors to gather information about the Car

- Speedsensor: LM393 Sen-Speed

Datasheet/Manual:
- LM393 datasheet: https://www.onsemi.com/download/data-sheet/pdf/lm393-d.pdf
