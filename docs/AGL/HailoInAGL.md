# Hailo in AGL

This document describes the hailo integration in AGL

## Meta-hailo

Get the [meta-hailo](https://github.com/hailo-ai/meta-hailo/tree/hailo8-scarthgap) layer in the hailo8-scarthgap branch

Then, add the following recipes to the AGL **local.conf** or **site.conf** file

```bash
IMAGE_INSTALL:append = " hailortcli hailo-firmware libhailort packagegroup-hailo-hailort"
```

The **hailo-pci** recipe is necesserary but it has a problem. Below is a fix and how to install it

## Problems

Theres an overlap problem with the **hailo-pci** and **linux-raspberrypi_6.12.bb** recipes.
They try to add files with the same name to the same directorie resulting in an error.
When this happens, run the following commands

Before everything, set up a http server for the AGL rpm packages
If you don't know how to do it, go to this [file](./HttpRPM.md)
After this set up remove the current hailo driver in the Target PC

```bash
dnf remove kernel-module-hailo-pci
```

Now, go back to your AGL installation enviroment (Host PC) and run this commands

```bash
bitbake -c cleanall linux-raspberrypi
bitbake -c install hailo-pci
bitbake package-index
```

Now, go back to the Target PC and run this

```bash
dnf makecache
dnf install kernel-module-hailo-pci
```

To check if everything is OK, you can use this command and check the hailo driver version.
It should be **4.23** instead of 4.20

```bash
modinfo hailo-pci
```

