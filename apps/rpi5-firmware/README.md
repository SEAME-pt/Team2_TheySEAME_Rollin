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

This is only needed if the **purePursuit** files were altered. If not the next step is not needed

For the LKA (Lane-keep-Assist) to work, it is needed to copy the **purePursuit/purePursuit.so** file
into the TARGET (RPI5) **/usr/lib/** directory.

The LKA is run throught the python script

- ```python3 run_ai_pipeline.py```

## Python Bindings

We use boost.python to call C++ classes, functions in python

Here's the boost.python [docs](https://www.boost.org/doc/libs/1_45_0/libs/python/doc/tutorial/doc/html/index.html)

Only the purePursuit is callable in python so, only alter the **pythonPurePursuit.cpp** file if you want to make changes


