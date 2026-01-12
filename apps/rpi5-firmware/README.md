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

### 3. Compilation
1. ```source <sdk-env-path>```
2. ```mkdir build/ && cd build/```
3. ```cmake ..```
4. ```make```
5. scp or other method to pass the binary to the RPI5
6. Run the binary

