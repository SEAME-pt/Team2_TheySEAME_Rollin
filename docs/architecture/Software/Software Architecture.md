# SOFTWARE ARCHITECTURE

This section describes the software used in the PiRacer Instrument Cluster system.  
It focuses on the main components, their purposes, and how they interact.

## WHAT WE USED
- [Overview](#Overview)
- [ThreadX RTOS](#THREADX-RTOS)
- [Qt Framework](#QT-FRAMEWORK)
- [COVESA AND UPROTOCOL](#COVESA-AND-UPROTOCOL)

## Overview
This project primarily uses C/C++ and Python programming languages.
It also heavily relies in Open Source software to build our Applications

## THREADX RTOS
ThreadX is a Real-Time Operating System running on the STM32 microcontroller.
It manages the car control tasks and handles sensor data in real time.

This project uses ThreadX because it's an Open Source RTOS certified for Safety-Critical Applications which is ideal for automotive systems

## QT FRAMEWORK
Qt is used to build the instrument cluster UI on the Raspberry Pi 5.
It displays real-time vehicle information like speed and system status.

Main responsibilities:
- Developed in C++ and QML
- Real-time data updates

## COVESA AND UPROTOCOL
COVESA defines communication standards for connected vehicles.
uProtocol is used to structure processed data. Then transmit this data between devices over a CAN or I2C bus.

## SUMMARY
This software architecture defines a modular system with separate layers for real-time processing, communication, UI, and continuous integration.
It supports real-time performance, modular updates, and future scalability.
