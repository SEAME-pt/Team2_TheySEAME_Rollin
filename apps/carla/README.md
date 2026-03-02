# PiRacer Simulation Integration with CARLA

## Overview

This project integrates a custom **PiRacer simulation module** into the
CARLA simulator using the C++ API and KUKSA middleware for vehicle
signal communication.

The system allows testing vehicle control logic inside CARLA before
deploying it to a real PiRacer platform.

------------------------------------------------------------------------

# What is CARLA?

CARLA (Car Learning to Act) is an open-source autonomous driving
simulator built on Unreal Engine.

It provides:

-   Realistic vehicle physics
-   High-fidelity 3D environments
-   Configurable sensors (Camera, LiDAR, IMU, GNSS)
-   Python and C++ APIs
-   Deterministic simulation via synchronous mode

In this project, we use the **C++ client API** to:

1.  Connect to a running CARLA server
2.  Spawn a vehicle
3.  Apply throttle, steering, and gear commands
4.  Run in synchronous mode with a fixed timestep

------------------------------------------------------------------------

# Purpose of `piracer_config.hpp`

The `piracer_config.hpp` file centralizes all tunable parameters used in
the simulation.

It defines:

## Physical Parameters

-   MASS_KG
-   WHEELBASE_M
-   MAX_STEER_DEG
-   MAX_SPEED_MPS

These parameters approximate the physical behavior of the PiRacer inside
CARLA.

Note: CARLA vehicle blueprints are full-scale real vehicles. The
configuration approximates RC behavior but does not change the mesh
scale.

## Timing Parameters

-   WORLD_DT_S
-   CAM_TICK_S
-   IMU_TICK_S
-   GNSS_TICK_S

These define deterministic update frequencies.

## Control Parameters

-   Steering and throttle slew rate limits
-   Simulated control latency

This improves simulation-to-real consistency.
------------------------------------------------------------------------

# Setup Instructions

## 1. Setup CARLA

Follow the instructions in the [official CARLA documentation](https://carla.readthedocs.io/en/latest/).

------------------------------------------------------------------------

## 2. Copy the PiRacer Simulation Module

cp -r PiracerSim CarlaRoot/

------------------------------------------------------------------------

## 3. Copy the KUKSA Middleware

cp -r kuksa CarlaRoot/

------------------------------------------------------------------------

# Running the Simulation

## 1. Start CARLA

make launch

------------------------------------------------------------------------


## 2. Build and Run the PiRacer Controller

To build and run the PiRacer simulation client, use the following Makefile targets:

## Build & Run (Makefile)

The PiRacerSim project includes a Makefile to simplify building and
running the C++ controller.

### Available Targets

-   **build**\
    Compiles the C++ client and all required dependencies.\
    The generated binary is placed in:

        bin/cpp_client

-   **run**\
    Builds the project (if necessary) and executes the C++ client.

-   **run.only**\
    Executes the already compiled binary without triggering a rebuild.

-   **clean**\
    Removes all build artifacts (object files, binaries, and temporary
    files).

-   **re**\
    Cleans the project and rebuilds everything from scratch.

-   **build_libcarla**\
    Builds LibCarla using `cmake` and `ninja` (requires a properly
    configured CARLA environment).

-   **ToolChain.cmake**\
    Generates the CMake toolchain configuration file with the
    appropriate compiler settings.

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

Clean build artifacts:

``` sh
make clean
```

Rebuild from scratch:

``` sh
make re
```

------------------------------------------------------------------------

These commands streamline the development workflow and simplify testing
the PiRacer controller within the CARLA simulation environment.