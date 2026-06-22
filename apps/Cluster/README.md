# Documentation for Qt Application on Raspberry Pi 5
<img width="1280" height="400" alt="Screenshot from 2025-12-09 10-41-42" src="https://github.com/user-attachments/assets/e56a72ff-da19-4a23-82c0-ebcc9298e8c2" />


## 1. Overview
This application, developed using **Qt**, will run on a **Raspberry Pi 5** and is designed to display real-time information on the displayer screen:
- Battery level
- Speed (hm/h)
- weather
- Date and time

The interface is optimized for performance on the Raspberry Pi 5 and suitable for small displays or integrated dashboards.

---

## 2. Requirements
### 2.1 Hardware
- Raspberry Pi 5
- Compatible displayer
- Required sensors:
  - Speed sensor
  - Battery monitoring module
- Reliable power supply

### 2.2 Software
- Qt 6.7.3
- Automative Grade Linux(ARM64)
---
## 3. Application Features
### 3.1 Battery Monitoring
- Displays battery percentage
- The UI includes a battery level indicator bar

### 3.2 Speed Display
- Shows current speed in hm/h
- Receives data from a external microcontroller

### 3.4 Weather Monitoring
- The temperature section includes a weather status icon
- Displays temperature in °C

### 3.5 Date and Time
- Uses system clock to show current date and time
---

## 4. System Architecture
### 4.1 Data Flow
1. Sensors provide data via microcontrollers
2. Application reads sensor values through a backend module
3. UI updates in real time using Qt signals and slots

### 4.2 Main Components
- **SystemInfo (Vehicle Data Manager)**: Central module responsible for managing all vehicle-related data.
- **generalInfo**: Supplies general data such as the current time, date, and other non-vehicle-related information.
- **Screen01**: Visual representation of all data from SystemInfo and InfoProvider.

---

### 5 Compilation

- **Native build (Linux host / dev machine)**  
  - Create a build folder inside the project:  
     ```bash
     mkdir -p build
     cd build
     ```
  - Run Qt CMake to generate Makefiles:  
     ```bash
     /path/to/qt/bin/qt-cmake ..
     ```
     > Replace `/path/to/qt/bin/qt-cmake` with your Qt installation path.  
  - Build the project:  
     ```bash
     cmake --build .
     ```
  - Run the executable:  
     ```bash
     ./qtAppExec
     ```

- **Cross-compiling for Raspberry Pi (ARM target)**  
  For instructions on cross-compiling, see [this README](Cross_Compile/README.md)

---

## 6. Running the Application

### 6.1 Prerequisites
Before running the application, ensure:
- **KUKSA Databroker** is running and accessible from the machine where qtAppExec will run
- The application connects to KUKSA on startup to fetch vehicle data (battery, speed, etc.)
- If KUKSA is unavailable, the application will show: `Failed to communicate with kuksa`

### 6.2 Local Execution
Run directly on the development machine:
```bash
cd build
./qtAppExec
```

This will start the Qt GUI application with the default display (requires X11/Wayland).

### 6.3 Remote Execution on Headless Machine

If running on a remote machine without a display (e.g., Raspberry Pi or headless server), configure the platform plugin:

#### Option 1: Offscreen Mode (Background)
Runs the application in offscreen mode (no visual output, useful for testing):
```bash
ssh team2@100.103.187.55 'QT_QPA_PLATFORM=offscreen /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/build/qtAppExec'
```

#### Option 2: VNC Server
Stream the GUI over VNC to view remotely:
```bash
ssh team2@100.103.187.55 'QT_QPA_PLATFORM=vnc /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/build/qtAppExec'
```

Then connect with a VNC client to `100.103.187.55:1` and port `5900`.

#### Option 3: X11 Forwarding (from local machine with X11)
Forward the display from remote to local machine:
```bash
ssh -X team2@100.103.187.55 'DISPLAY=:0 /home/team2/Documents/hugo-folder/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/build/qtAppExec'
```

**Note:** Requires X11 server running on local machine and SSH X11 forwarding enabled.

### 6.4 Display Configuration Troubleshooting

If you encounter: `could not connect to display` or `Could not load the Qt platform plugin "xcb"`

**Available platform plugins:** wayland, eglfs, xcb, vnc, minimal, vkkhrdisplay, linuxfb, wayland-egl, offscreen, minimalegl

Set the appropriate one:
```bash
export QT_QPA_PLATFORM=offscreen  # For background execution
export QT_QPA_PLATFORM=vnc        # For remote GUI viewing
export QT_QPA_PLATFORM=wayland    # If Wayland is available
export QT_QPA_PLATFORM=eglfs      # For embedded systems with GPU
```

---

## 6. Conclusion
This document outlines the structure, requirements, and features of the Qt application designed for the Raspberry Pi 5. It serves as a guide for development, deployment, and future enhancements.
