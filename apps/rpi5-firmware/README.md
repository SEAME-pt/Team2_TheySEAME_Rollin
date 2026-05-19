# RaspberryPi5 App
This is the code that runs in the RaspberryPi5

## 1. Overview
This application encompasses the algorithms for the ADAS part of the **Car** and a manual control

## 2. Requirements
### 2.1 Software
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
5. Pass the binaries to the RPI5 ```scp bin/<binary-file> <user>@<ip>:<target-path>```
6. Run the binary in the RPI5 ```./<target-path>/<binary-file>```

For the LKA (Lane-keep-Assist) to work, it needs to be integrated with the AI pipeline script.
This is a python script that returns a binary mask produced by the **Hailo8**.
It's in the ```perception``` directory, inside the RPI5

To use run these commands in the following order:

In one tty, run the python program
- ```python3 run_ai_pipeline.py --named-pipe NamedPipe```

In another, run the LKA binary
- ```./<LKA-binary>```
