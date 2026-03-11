# Camera Module 3 in AGL

This document describes the [Camera Module 3](https://www.raspberrypi.com/documentation/accessories/camera.html#camera-module-3) integration in AGL

1. Edit the AGL local.conf or site.conf files

You need to add this configurations

```bash
ENABLE_I2C = "1"
KERNEL_MODULE_AUTOLOAD:rpi:append = " i2c-dev i2c-bcm2708"
VIDEO_CAMERA = "1"
RASPBERRYPI_CAMERA_V3 = "1"
```

2. Create 

