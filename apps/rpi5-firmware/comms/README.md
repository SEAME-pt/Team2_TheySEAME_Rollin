# Documentation for the CAN communication on RPI5

## 1. Overview

This module implements the **CAN communication** that runs inside the **Raspberry Pi 5**
It can send and receive CAN frames in non-blocking mode

---

## 2. Requirements
### 2.1 Hardware
- Raspberry Pi 5
- HW-184

### 2.2 Software
- SocketCAN
- Automative Grade Linux(ARM64)
---
## 3. Application Features
### 3.1 Send and Receive CAN frames
- Send and Receive CAN frames wihtout blocking
---

## 4. System Architecture
### 4.1 Main Components
- **CAN class**: Class that controls the operations in the CAN socket

---

### 5 Compilation

CMake is the tool used to build this module.
The AGL sdk is also used to compile the binary to the correct architecture

```bash
mkdir -p build && cd build
source <sdk-path>
cmake ..
make
scp CAN <user>@<ip-address>:<file-path>
./<file-path>/CAN
```
---
## 6. Conclusion
This document serves as mini guide to the CAN module
