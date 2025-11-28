# CAN

This module implements the CAN communication that runs inside the raspberry pi 5

## Build

CMake is the tool used to build this module. Don't forget to source the sdk to be in the same enviroment as the rpi5

```bash
mkdir -p build && cd build
cmake ..
make
... pass the CAN binary to the rpi5 ...
./CAN
```
