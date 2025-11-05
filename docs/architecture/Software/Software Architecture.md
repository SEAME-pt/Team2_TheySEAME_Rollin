# SOFTWARE ARCHITECTURE

This section describes the software used in the PiRacer Instrument Cluster system.  
It focuses on the main components, their purposes, and how they interact.

## WHAT WE USED
- [ThreadX RTOS](#THREADX-RTOS)
- [Qt Framework](#QT-FRAMEWORK)
- [COVESA AND UPROTOCOL](#COVESA-AND-UPROTOCOL)

## THREADX RTOS
ThreadX is a Real-Time Operating System running on the STM32 microcontroller.
It manages the car control tasks and handles sensor data in real time.

## QT FRAMEWORK
Qt is used to build the instrument cluster UI on the Raspberry Pi 5.
It displays real-time vehicle information like speed and system status.

Main responsibilities:
- Developed in C++ and QML
- Real-time data updates

## COVESA AND UPROTOCOL
COVESA defines communication standards for connected vehicles.
uProtocol is used to structure and send messages between devices through CAN.

## SUMMARY
This software architecture defines a modular system with separate layers for real-time processing, communication, UI, and continuous integration.
It supports real-time performance, modular updates, and future scalability.
