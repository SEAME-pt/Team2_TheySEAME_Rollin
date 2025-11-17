# Intro to AGL

AGL (Automotive Grade Linux) is an open source project, from the Linux Foundation, which purpose is to build open source software platforms for the automotive industry
AGL can address all software in a vehicle, infotainment, instrument cluster (ic), advanced driver assistance systems (ADAS) and more 

## Why AGL

The project needed any OS that could run an Instrument Cluster for our car.
AGL is an OS that can be tailored to our needs (ic) and it's open source software hence this choice

## Building AGL Image

This project followed the [AGL docs](https://docs.automotivelinux.org/en/trout/#) in the trout

The target machine is a ```raspberrypi5``` and the image built is ```agl-image-minimal-crosssdk```

Features chosen:
- agl-all-features :(agl-demo agl-netboot)
- agl-devel :(agl-package-management)
- agl-ic

```bash
source meta-agl/scripts/aglsetup.sh -m raspberrypi5 -d <build-name> agl-all-features agl-devel agl-ic
time bitbake agl-image-minimal-crosssdk
```

After the build, the image is extracted to a 128GB MicroSD card

### SDK

This project followed the [AGL_SDK](https://docs.automotivelinux.org/en/trout/#04_Developer_Guides/01_Setting_Up_AGL_SDK/) to set up the SDK

SDK mirrors the enviroment of the created AGL image.
With it, apps can be developed and tested in a enviroment close to the real one without messing with physical hardware

