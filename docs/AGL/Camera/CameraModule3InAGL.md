# Camera Module 3 in AGL

This document describes the [Camera Module 3](https://www.raspberrypi.com/documentation/accessories/camera.html#camera-module-3) integration in AGL

1. Enable the camera in the config.txt file

We do this by adding this lines in the local.conf file.
You need to add this configurations

```bash
ENABLE_I2C = "1"
KERNEL_MODULE_AUTOLOAD:rpi:append = " i2c-dev i2c-bcm2708"
VIDEO_CAMERA = "1"
RASPBERRYPI_CAMERA_V3 = "1" #This will add the camera to the default DSI port, which is DSI-1
```

2. Create **libcamera** and **libcamera-apps** recipes 

We need to override this recipes, because they can't build the rpi/pisp pipeline, which is necessary in the RPI5

In this [PR](https://github.com/agherzan/meta-raspberrypi/pull/1517) they have the fixed recipes.
Copy the **libpisp**, **rpi-libcamera** and **rpicam-apps**.
Put it inside a layer of your own

3. Install the new recipes

Now, in the local.conf or site.conf, add the new recipes.

```bash
IMAGE_INSTALL:appends = " rpi-libcamera rpicam-apps"
```

4. Check if rpi/pisp was created

After building the image, do a quick check for this file **ipa_rpi_pisp**.
If it doesn't exist, something went wrong

```bash
find <build-dir>/tmp -name "*ipa_rpi_pisp*"
```

5. Change the DSI port if necessary

After flashing the sd card, find the **config.txt** file and change the DSI port if isn't right
Format: ```dtoverlay=imx708,<DSI-Port>```

Available ports in RPI5:

- ```cam0``` -> DSI-0
- ```cam1``` -> DSI-1

