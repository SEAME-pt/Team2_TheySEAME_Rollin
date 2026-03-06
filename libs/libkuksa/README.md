# Kuksa Library Documentation

## Overview

The **Kuksa Library** is a C++ wrapper around the **KUKSA VAL gRPC
API**. It provides a simple interface to:

-   subscribe to vehicle signals from a KUKSA server
-   publish signal values back to the KUKSA server

This library is intended for applications that need to read and write
vehicle data such as throttle, steering, gear, battery state of charge,
speed, and driving mode.

------------------------------------------------------------------------

## Features

-   gRPC-based communication with a KUKSA VAL server
-   thread-safe storage of vehicle state using `std::atomic`
-   support for signal subscription
-   support for publishing values to KUKSA
-   generic type conversion from KUKSA `Value` messages to C++ types
-   lightweight static library build using CMake

------------------------------------------------------------------------

## Architecture

The library is implemented through the `kuksaLib` class.

### Main responsibilities

-   Create a gRPC channel and KUKSA VAL stub
-   Subscribe to predefined vehicle signal paths
-   Update internal state when new values are received
-   Provide getter methods for the latest signal values
-   Publish values to selected KUKSA paths

------------------------------------------------------------------------

## Class Reference

### `class kuksaLib`

#### Internal state

These variables store the latest signal values received from KUKSA:

-   `std::atomic<float> throttle`
-   `std::atomic<float> steering`
-   `std::atomic<int> gear`
-   `std::atomic<float> battery`
-   `std::atomic<float> speed`
-   `std::atomic<int> drivingMode`

Using atomics ensures thread-safe read and write access.

#### Connection settings

-   `std::string _server = "kuksa databroker ip:port"` -- Address of the KUKSA
    server.
-   `std::shared_ptr<grpc::Channel> channel` -- gRPC communication
    channel.
-   `std::unique_ptr<VAL::Stub> stub` -- Stub used to call the KUKSA VAL
    service.

------------------------------------------------------------------------

## Constructor

``` cpp
kuksaLib::kuksaLib()
{
    channel = grpc::CreateChannel(_server, grpc::InsecureChannelCredentials());
    stub = VAL::NewStub(channel);
}
```

The constructor creates a gRPC channel using insecure credentials and
initializes the KUKSA VAL stub.

------------------------------------------------------------------------

## subscribeFromKuksa()

Subscribes to a set of vehicle signal paths and continuously reads
updates from the KUKSA server.

### Subscribed signals

-   Vehicle.Control.Throttle.Value
-   Vehicle.Control.Steering.Angle
-   Vehicle.Control.Gear.Value
-   Vehicle.Control.Brake
-   Vehicle.Powertrain.Battery.StateOfCharge
-   Vehicle.Speed
-   Vehicle.Control.Mode.DrivingMode

### Behavior

1.  Creates a SubscribeRequest
2.  Opens a streaming gRPC subscription
3.  Reads incoming updates in a loop
4.  Converts received values to numeric types
5.  Updates the corresponding internal atomic variables

This function blocks while the stream is active, so it is recommended to
run it in a separate thread.

------------------------------------------------------------------------

## sendValueToKuksa()

Publishes a value to a specific KUKSA signal path.

### Supported types

-   float
-   int32_t
-   bool
-   std::string

### Example

``` cpp
lib.sendValueToKuksa<float>("Vehicle.Control.Throttle.Value", 0.75f);
lib.sendValueToKuksa<int32_t>("Vehicle.Control.Gear.Value", 3);
```

Returns `true` if the value was successfully sent.

------------------------------------------------------------------------

## valueToType()

Converts a KUKSA `Value` object into a C++ variable.

### Supported KUKSA types

-   int32
-   int64
-   uint32
-   uint64
-   float
-   double
-   bool
-   string

Returns `true` if conversion was successful.

------------------------------------------------------------------------

## Getter Methods

The library provides access to the latest subscribed values:

``` cpp
float getThrottle() const;
float getSteering() const;
int getGear() const;
float getBattery() const;
float getSpeed() const;
int getDrivingMode() const;
```

These return the latest values received from the KUKSA server.

------------------------------------------------------------------------

## Example Usage

``` cpp
#include "KuksaLib.hpp"
#include <iostream>
#include <thread>

int main() {
    kuksaLib lib;

    std::thread subThread([&lib]() {
        lib.subscribeFromKuksa();
    });

    lib.sendValueToKuksa<float>("Vehicle.Control.Throttle.Value", 0.5f);

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Throttle: " << lib.getThrottle() << std::endl;
    std::cout << "Speed: " << lib.getSpeed() << std::endl;

    subThread.join();
}
```

------------------------------------------------------------------------

## Build System

The project uses **CMake** and builds the library as a static library.

### Dependencies

-   Protobuf
-   gRPC
-   Abseil

### Build

``` bash
mkdir build
cd build
cmake ..
make
```

Make sure the following tools are installed:

-   protoc
-   grpc_cpp_plugin
-   gRPC development libraries
-   Protobuf development libraries
-   Abseil

------------------------------------------------------------------------

## Design Notes

### Thread safety

Signal values are stored using `std::atomic` so multiple threads can
safely read and update them.

### Blocking subscription

`subscribeFromKuksa()` runs in a blocking loop and should normally be
executed in a worker thread.

### Hardcoded configuration

The current implementation uses a fixed server address and fixed signal
paths.
------------------------------------------------------------------------

## Summary

The `kuksaLib` class provides a simple interface for interacting with a
KUKSA VAL server in C++. It abstracts the gRPC communication layer and
allows applications to easily:

-   receive vehicle signal updates
-   read the latest values
-   publish new signal values
