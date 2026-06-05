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
The next step is only needed if the **purePursuit** files were altered. If not, the next step is not needed

6. For the LKA (Lane-keep-Assist) to work, it is needed to copy the **purePursuit/purePursuit.so** file
into the TARGET (RPI5) in the same directory where the python command is run

```scp purePursuit/purePursuit.so <user>@<ip>:<target-path>```

The LKA is run throught the python script

7. ```python3 run_ai_pipeline.py```

## Python Bindings

We use boost.python to call C++ classes, functions in python

Here's the boost.python [docs](https://www.boost.org/doc/libs/1_45_0/libs/python/doc/tutorial/doc/html/index.html)

The **purePursuit** python API in under the **pythonPurePursuit.cpp** file.
If anyone changes the **purePursuit.cpp** and **purePursuit.hpp** also needs to update the **pythonPurePursuit.cpp** file

### How to use the bindings in python

Here's an example on how to use the C++ python bindings

```python
from purePursuit import PurePursuit
from purePursuit import quadFunc

# Example frame dimensions
frameHeight = 640
frameWidth = 640

# Class constructor
pp = PurePursuit()

# Left Lane Polynomial
leftquad = quadFunc()

# Right Lane Polynomial
rightquad = quadFunc()

# Fill the Polynomial coefficients
leftquad.a = 2
leftquad.b = 2
leftquad.c = 2
rightquad.a = 2
rightquad.b = 2
rightquad.c = 2

# Call the Pure Pursuit
pp.control(leftquad, rightquad, frameHeight, frameWidth)
```


