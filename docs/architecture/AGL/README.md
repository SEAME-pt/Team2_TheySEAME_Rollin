## Intro to AGL

AGL (Automotive Grade Linux) is an open source project, from the Linux Foundation, which purpose is to build open source software platforms for the automotive industry
AGL can address all software in a vehicle, infotainment, instrument cluster (ic), advanced driver assistance systems (ADAS) and more 

## Overview

This document serves as a record of the choices regarding the AGL OS.

## Why AGL

The project needed any OS that could run an Instrument Cluster for our car.
AGL is an OS that can be tailored to our needs (ic) and it's open source software

## Installation

This project followed the [AGL docs](https://docs.automotivelinux.org/en/trout/#) in the trout branch

The image chosen was the ```agl-image-minimal-crosssdk```

As of features choosen:
- agl-all-features :(agl-demo agl-netboot)
- agl-devel :(agl-package-management)
- agl-ic

After the build, the image is extracted to a 128GB MicroSD card
