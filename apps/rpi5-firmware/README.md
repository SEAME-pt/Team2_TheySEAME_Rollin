# RaspberryPi5 App
This is the code that runs in the RaspberryPi5

## 1. Overview
This application let's you send CAN frames to control a **Car** whenever a **Gamepad** input is registered.
This is intended to run in the **RaspberryPi5** with a **ShanWan Gamepad**.

## 2. Requirements
### 2.1 Hardware
- 2 MCP2515
- A bluetooth Gamepad (You can modify the code to suit your Gamepad)

### 2.2 Software
- Cross-compilation tools (sdk from AGL installation)

## 3. Compilation
### 3.1 Host
1. ```mkdir build/host && cd build/host```
2. ```cmake ../..```
3. ```make install```
4. Run the binaries inside bin ```./bin/unit_tests```

### 3.2 Target (RPI5)
1. ```source <sdk-env-path>```
2. ```mkdir build/target && cd build/target```
3. ```cmake ../..```
4. ```make install```
5. Pass the binaries to the RPI5 ```scp bin/CarControl <user>@<ip>:<target-path>```
6. Run the binary in the RPI5 ```./<target-path>/CarControl```

