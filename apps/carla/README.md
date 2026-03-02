# PiRacer Simulation Integration with CARLA

## Overview

This project integrates a custom **PiRacer simulation module** with the
**CARLA simulator** using the CARLA C++ API and KUKSA middleware for
vehicle signal communication.

The goal is to validate and test vehicle control logic in a simulated
environment before deploying it to a physical PiRacer platform. This
reduces development risk and enables deterministic, repeatable testing.

------------------------------------------------------------------------

## What is CARLA?

CARLA (Car Learning to Act) is an open-source autonomous driving
simulator built on Unreal Engine.

It provides:

-   Realistic vehicle physics
-   High-fidelity 3D environments
-   Configurable sensors (Camera, LiDAR, IMU, GNSS)
-   Python and C++ APIs
-   Deterministic simulation through synchronous mode

In this project, the **CARLA C++ Client API** is used to:

1.  Connect to a running CARLA server\
2.  Spawn a vehicle actor in the simulation\
3.  Apply throttle, steering, and gear commands\
4.  Run the simulation in synchronous mode with a fixed timestep

------------------------------------------------------------------------

## Purpose of `piracer_config.hpp`

The `piracer_config.hpp` file centralizes all simulation parameters to
ensure maintainability and consistent behavior.

It defines three main parameter groups:

### Physical Parameters

-   `MASS_KG`
-   `WHEELBASE_M`
-   `MAX_STEER_DEG`
-   `MAX_SPEED_MPS`

These parameters approximate the dynamic behavior of the PiRacer within
CARLA.

> **Note:** CARLA vehicle blueprints represent full-scale vehicles.\
> The configuration approximates RC-scale behavior but does not modify
> the visual mesh scale.

------------------------------------------------------------------------

### Timing Parameters

-   `WORLD_DT_S`
-   `CAM_TICK_S`
-   `IMU_TICK_S`
-   `GNSS_TICK_S`

These define deterministic update rates for the simulation world and
sensors.

------------------------------------------------------------------------

### Control Parameters

-   Steering slew rate limits\
-   Throttle slew rate limits\
-   Simulated control latency

These improve simulation-to-real consistency by modeling realistic
actuator response and smoothing command transitions.

------------------------------------------------------------------------

# Setup Instructions

## 1. Install CARLA

Follow the official CARLA installation guide:

https://carla.readthedocs.io/en/latest/

------------------------------------------------------------------------

## 2. Copy the PiRacer Simulation Module

``` sh
cp -r PiracerSim CarlaRoot/
```

------------------------------------------------------------------------

## 3. Copy the KUKSA Library

``` sh
cp -r libs/libkuksa CarlaRoot/PiracerSim/
```

------------------------------------------------------------------------

# Running the Simulation

## 1. Start CARLA

Launch the CARLA server:

``` sh
make launch
```

Once the CARLA window opens:

-   Click **Start** in the simulator interface.
-   Wait until the world is fully loaded.

------------------------------------------------------------------------

## 2. Build and Run the PiRacer Controller

The PiRacerSim project includes a Makefile to simplify building and
running the C++ controller.

------------------------------------------------------------------------

## Build & Run (Makefile)

### Available Targets

-   **build**\
    Compiles the C++ client and its dependencies.\
    Output binary:

        bin/PiracerSim

-   **run**\
    Builds (if necessary) and executes the client.

-   **run.only**\
    Executes the existing binary without rebuilding.

-   **clean**\
    Removes all build artifacts.

-   **re**\
    Cleans and rebuilds the project from scratch.

-   **build_libcarla**\
    Builds LibCarla using `cmake` and `ninja`.\
    Requires a properly configured CARLA environment.

-   **ToolChain.cmake**\
    Generates the CMake toolchain configuration file.

------------------------------------------------------------------------

### Usage Examples

Build the project:

``` sh
make build
```

Build and run:

``` sh
make run
```

Run without rebuilding:

``` sh
make run.only
```

Clean:

``` sh
make clean
```

Rebuild:

``` sh
make re
```

------------------------------------------------------------------------

These commands streamline the development workflow and simplify
validation of the PiRacer controller inside the CARLA simulation
environment.
