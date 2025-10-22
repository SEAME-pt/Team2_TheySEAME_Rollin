## Raspberry Pi 5 OS Debian Bookworm
# Core
 - In a new fresh Raspberry Pi OS always do
```bash
  sudo apt update
  sudo apt full-upgrade
```
 - Then enable I2C and SPI interface with raspi-config
```bash
  sudo raspi-config
  // After config
  sudo reboot
```

# PiRacer Display:
 - Add this to the config.txt in /boot/firmware directory
```bash
      dtoverlay=vc4-kms-v3d
      #DSI1 Use
      dtoverlay=vc4-kms-dsi-waveshare-panel,7_9_inch
      #DSI0 Use
      #dtoverlay=vc4-kms-dsi-waveshare-panel,7_9_inch,dsi0
```
  - Power display with either USB-C or 5V and GND (0V) pins
  - Useful Links: https://www.waveshare.com/wiki/7.9inch_DSI_LCD

# Hailo AI:
 - Install the default Hailo AI package
```bash
  sudo apt install hailo-all
```
  - Test the Hailo AI with this command
```bash
  rpicam-hello -t 0 --post-process-file /usr/share/rpi-camera-assets/hailo_yolov6_inference.json
```
  - Useful Links: https://www.raspberrypi.com/documentation/accessories/ai-kit.html, https://www.raspberrypi.com/documentation/computers/ai.html

# Move Car with Gamepad:
  - Create a python venv
```bash
  cd piracer/
  python3 -m venv venv --system-site-packages
```
  - Follow SEA-ME piracer github README
  - Create a gamepad.py file with the Github example
  - SEA-ME PiRacer github: https://github.com/SEA-ME/piracer_py

# Microcontroller:
  - Install the can-utils to test the CANBUS communication
 ```bash
  sudo apt install can-utils
```
  - Edit the /boot/firmware/config.txt with
```bash
  dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25
  dtoverlay=spi-bcm2835
```
  - Useful Links: https://wiki.seeedstudio.com/2-Channel-CAN-BUS-FD-Shield-for-Raspberry-Pi/ 
