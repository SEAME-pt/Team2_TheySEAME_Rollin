# Intro to AGL

AGL (Automotive Grade Linux) is an open source project, from the Linux Foundation, which purpose is to build open source software platforms for the automotive industry

AGL can address all software in a vehicle like in-vehicle-infotainment (ivi), instrument cluster (ic), advanced driver assistance systems (ADAS) and more 

## Why AGL

The project required an Operating System capable of running an Instrument Cluster and supporting ADAS features for our car.
AGL can be tailored to our specific requirements and it's open source software, hence this choice

## Building AGL Image

This project followed the [AGL docs](https://docs.automotivelinux.org/en/trout/#) in the trout

The target machine is a ```raspberrypi5``` and the image built is ```agl-image-compositor```

### Features

This image contains:
- agl-all-features
- agl-devel
- agl-ic

```bash
source meta-agl/scripts/aglsetup.sh -m raspberrypi5 -d <build-name> agl-all-features agl-devel agl-ic
```

### Layers

The ```meta-qt6``` layer is added to the bblayers.conf file

```bash
bitbake-layers add-layer meta-qt6
```

In the local.conf file, install the following recipes:
- QT: "qtbase qtmultimedia qtquick3d qtdeclarative"
- DEV: "cmake"

```bash
IMAGE_INSTALL:append = " qtbase qtmultimedia qtquick3d qtdeclarative"
IMAGE_INSTALL:append = " cmake"
```

Then build the image

```bash
time bitbake agl-image-compositor
```

### SDK

This project followed the [AGL_SDK](https://docs.automotivelinux.org/en/trout/#04_Developer_Guides/01_Setting_Up_AGL_SDK/) to set up the SDK

SDK mirrors the enviroment of the created AGL image.
With it, apps can be developed and tested in a enviroment close to the real one without messing with physical hardware

To create the SDK run this command
```bash
bitbake agl-image-compositor -c do_populate_sdk
```

