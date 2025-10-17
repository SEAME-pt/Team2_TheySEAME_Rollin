In a new fresh Raspberry Pi OS always do
  - sudo apt update
  - sudo apt full-upgrade
  - sudo raspi-config
  - Enable I2C interface
  - sudo reboot

Display:
  - Add this to the config.txt in /boot/firmware directory
      dtoverlay=vc4-kms-v3d
      #DSI1 Use
      dtoverlay=vc4-kms-dsi-waveshare-panel,7_9_inch
      #DSI0 Use
      #dtoverlay=vc4-kms-dsi-waveshare-panel,7_9_inch,dsi0
  - Need to power display with either USB-C or 5V/GND (0V) pins
  - Links: https://www.waveshare.com/wiki/7.9inch_DSI_LCD

Hailo AI:
  - sudo apt install hailo-all
  - Run "rpicam-hello -t 0 --post-process-file /usr/share/rpi-camera-assets/hailo_yolov6_inference.json"
  - Links: https://www.raspberrypi.com/documentation/accessories/ai-kit.html, https://www.raspberrypi.com/documentation/computers/ai.html

Move Car with Gamepad:
  - Create a python3 -m venv venv --system-site-packages
  - Run pip install piracer-py
  - Create a gamepad.py file with the Github example
  - Links: https://github.com/SEA-ME/piracer_py

