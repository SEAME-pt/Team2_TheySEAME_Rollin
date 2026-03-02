# PiRacer Control in CARLA (C++)

## Overview

This project implements a C++ controller that interfaces with the CARLA simulator and applies vehicle control commands

The system allows simulation-based testing of control logic before deployment on a real PiRacer platform.

---

## What is CARLA?

CARLA (Car Learning to Act) is an open-source autonomous driving simulator built on Unreal Engine.

It provides:

- High-fidelity 3D environments
- Realistic vehicle physics
- Configurable sensors (camera, LiDAR, IMU, GNSS)
- Python and C++ APIs for vehicle control

In this project, the CARLA C++ API is used to:

1. Connect to a running CARLA server
2. Spawn a vehicle in the simulation world
3. Enable synchronous simulation mode
4. Apply throttle, steering, and gear commands each frame

---

## Purpose of `piracer_config.hpp`

The configuration file centralizes all tunable parameters, including:

### Physical Parameters
- Vehicle mass
- Wheelbase
- Maximum steering angle
- Maximum desired speed

These parameters are used to approximate PiRacer behavior within CARLA.

### Timing Parameters
- Fixed simulation timestep
- Sensor update rates

This ensures deterministic simulation behavior.

### Control Parameters
- Slew rate limits
- Control latency

These help simulate realistic actuation dynamics and smooth command transitions.

---

## System Architecture

1. The CARLA server runs independently.
2. The C++ controller connects via TCP.
3. KUKSA provides control inputs.
4. The controller normalizes inputs and applies them via `VehicleControl`.
5. The world advances using synchronous ticks.

---

## Integration Steps

### 1. Launch CARLA

Start the CARLA server:

Linux: