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
3. ```cmake ../.. -DSDK_PATH=<absolute-path-to-SDK-sysroots>```
4. ```make install```

The next step depends on which binary you want to run

There's the **Manual Driving** (Remote Control) and the **Automatic Driving** (LKA) binaries

### Manual Driving Binary
5. Pass the binaries to the RPI5 ```scp bin/CarControl <user>@<ip>:<target-path>```
6. Run the binary in the RPI5 ```./<target-path>/<binary-file>```

### Automatic Driving Binary (LKA)
The next step is only needed if the **lkaControl** and **actuator** files were altered or it is the first time doing this.
If not, the next step is not needed

6. For the LKA (Lane-keep-Assist) to work, it is needed to copy the **lkaControl/lkaControl.so** and 
**actuator/actuator.so** file into the TARGET (RPI5) to the **perception/** directory
This files are found under the CMake build directory after compilation

```bash
scp lkaControl/lkaControl.so actuator/actuator.so <user>@<ip>:<target-path>/perception
```

7. Go to the perception directory and run the python script

```bash
cd perception/
python3 run_ai_pipeline.py --enable-lka
```

8. Also run this command to make the car move foward

```bash
cansend can0 101#03
cansend can0 100#0F
```

## Python Bindings

We use boost.python to call C++ classes, functions in python

Here's the boost.python [docs](https://www.boost.org/doc/libs/1_45_0/libs/python/doc/tutorial/doc/html/index.html)

The **lkaControl** and **actuator** python API in under the **pythonLkaControl.cpp** and **pythonActuator.cpp** file respectively.
If anyone changes files under these directories, also needs to update the respectively python bindings file

### How to use the bindings in python

Here's an example on how to use the C++ python bindings

```python
from lkaControl import LkaControl, Debug
from actuator import Bridge

lkaControl = LkaControl() # Create the C++ class responsible for the Lka module
bridge = Bridge(lkaControl) # This starts up the C++ classes for the LkaControl
debug = Debug() # This is just variables from the LkaControl

lkaControl.control(...)
```


