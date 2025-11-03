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
| **Toolchain** | arm-linux-gnueabihf-g++ or aarch64-linux-gnu-g++ |

---

## 3. Cross-Compilation Setup in Qt Creator

### 3.1 Install Qt and Tools on Host

Install Qt 5.13 and required toolchains:

```bash
sudo apt update
sudo apt install build-essential qt5-default qtbase5-dev qtchooser qt5-qmake g++-arm-linux-gnueabihf rsync ssh
```

### 3.2 Configure Qt Creator for Raspberry Pi

1. Open **Qt Creator → Tools → Options → Devices → Devices**
   - Add a **Generic Linux Device**
   - Hostname: `<raspberry_ip>`
   - User: `pi`
   - Test Connection ✅

2. Open **Qt Creator → Tools → Options → Kits**
   - Add a new **Kit** for the Raspberry Pi
   - Compiler: `arm-linux-gnueabihf-g++`
   - Qt Version: Qt 5.13 for Embedded Linux (ARM)
   - Device type: **Generic Linux Device**
   - Device: your Raspberry Pi

3. In your project, select this **Kit** (Raspberry Pi Kit).

Now you can build and deploy directly from Qt Creator using the **Run** button.

---

## 4. Deployment via Qt Creator

Qt Creator will:
- Build your app on the host.
- Transfer the binary via SSH to `/opt/<ProjectName>` (or another path you define).
- Execute the app on the Pi.

You can verify with:
```bash
ssh pi@<raspberry_ip>
ls /opt/<ProjectName>
```

---

## 5. Auto-Start on Boot

To automatically launch the app when the Raspberry Pi boots:

### 6.1 GUI Mode (LXDE)
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
