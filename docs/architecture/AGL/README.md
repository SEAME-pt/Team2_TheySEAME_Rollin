# AGL

AGL (Automotive Grade Linux) is the OS used in this project.

## Overview

This document serves as a record of the choices regarding the AGL OS.

## Why

The AGL OS is an open source project specialized in the automotive industry.
As this project is aimed at the automotive industry

## Installation

This project followed the [AGL docs](https://docs.automotivelinux.org/en/trout/#) in the trout branch

The image chosen was the ```agl-image-minimal-crosssdk```

As of features choosen:
- agl-all-features
- agl-devel
- agl-ic

After the build, the image was extracted to a 128GB MicroSD card
