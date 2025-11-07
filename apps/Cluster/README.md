# Qt Dashboard Application for Raspberry Pi

## 1. Introduction

This document explains how to build, deploy, and automatically run a Qt application on a Raspberry Pi using cross-compilation from a Linux PC.

The workflow consists of:
1. Building the application on the host (PC)
2. Deploying the compiled binaries to the Raspberry Pi
3. Configuring the Raspberry Pi to run the application on boot

---

## 2. Development Environment

| Component | Description |
|------------|--------------|
| **Host Machine** | Ubuntu 22.04 (x86_64) |
| **Target Device** | Raspberry Pi 5 with Raspberry Pi OS |
| **Toolchain** | arm-linux-gnueabihf-g++ |

---

## 3. Cross-Compilation Setup in Qt Creator

### 3.1 Install Qt and Tools on Host

Install Qt 5.13 and required toolchains:

```bash
sudo apt update
sudo apt install build-essential qt5-default qtbase5-dev qtchooser qt5-qmake g++-arm-linux-gnueabihf rsync ssh
```

### 3.2 Prepare the Raspberry Pi

On the Raspberry Pi, install the required Qt runtime libraries:

```bash
sudo apt update
sudo apt install qt5-default qtbase5-dev openssh-server
```
Copy sysroot from Raspberry Pi to Host

From the host machine, copy the system root (libraries and includes) from the Pi:
```bash
mkdir -p ~/rpi/sysroot
rsync -avz --rsync-path="sudo rsync" pi@<raspberry_ip>:/lib ~/rpi/sysroot
rsync -avz --rsync-path="sudo rsync" pi@<raspberry_ip>:/usr/include ~/rpi/sysroot/usr
rsync -avz --rsync-path="sudo rsync" pi@<raspberry_ip>:/usr/lib ~/rpi/sysroot/usr
```

### 3.3 Configure Qt Creator for Raspberry Pi

1. Open **Qt Creator → Tools → Options → Devices → Devices**
   - Add a **Boot to Qt Device**
   - Hostname: `<raspberry_ip>`
   - User: `username`
   - Test Connection ✅

2. Open **Qt Creator → Tools → Options → Kits**
   - Add a new **Kit** for the Raspberry Pi
   - Compiler: `arm-linux-gnueabihf-g++`
   - Qt Version: Qt 5.13 for Embedded Linux (ARM)
   - Device type: **Boot to Qt Device**
   - Device: your Raspberry Pi

3. In your project, select this **Kit** (Raspberry Pi Kit).

Now you can build and deploy directly from Qt Creator using the **Run** button.

---

## 4. Deployment via Qt Creator

Qt Creator will:
- Build your app on the host.
- Transfer the binary via SSH to `/opt/<ProjectName>` (or another path you define).
  
You can verify with:
```bash
ssh pi@<raspberry_ip>
ls /opt/<ProjectName>
```

---

## 5. Auto-Start on Boot

To automatically launch the app when the Raspberry Pi boots:

Create autostart dir and command file if not exists:
```bash
mkdir -p ~/.config/autostartnano
nano ~/.config/autostart/qtapp.desktop
```
Add this:
```
[Desktop Entry]
Type=Application
Name=Qt App
Exec=qmlscene app dir
X-GNOME-Autostart-enabled=true
```
And then:
```bash
sudo reboot
```

## 6. Fullscreen Configuration (Hide Taskbar)

To remove the panel (taskbar) and make the Qt application fullscreen on Raspberry Pi OS (Wayfire environment):

1. Edit the panel configuration file:
   ```bash
   nano ~/.config/wf-panel-pi.ini
   ```

2. Add (or modify) the following lines:
   ```
   autohide=true
   autohide_duration=500
   ```

3. Save and reboot:
   ```bash
   sudo reboot
   ```

After reboot, the taskbar will automatically hide, and the Qt application will appear fullscreen.
