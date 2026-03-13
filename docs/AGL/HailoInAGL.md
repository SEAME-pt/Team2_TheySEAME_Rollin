# Hailo in AGL

This document describes the hailo integration in AGL

## Meta-hailo

Get the [meta-hailo](https://github.com/hailo-ai/meta-hailo/tree/hailo8-scarthgap) layer in the hailo8-scarthgap branch

Then, add the following recipes to the AGL **local.conf** or **site.conf** file

```bash
IMAGE_INSTALL:append = " hailo-pci hailortcli hailo-firmware libhailort packagegroup-hailo-hailort"
```

The **hailo-pci** recipe is necesserary but it has a problem. Below is a fix and how to build it

## Common Problems

Theres an overlap problem with the **hailo-pci** and **linux-raspberrypi_6.12.bb** recipes.
They try to add files with the same name to the same directorie resulting in an error.

One solution to this problem is to don't split the kernel modules.
So the **linux-raspberrypi** will produce a single package, which makes the conflict disappear

1. Edit local.conf

This line ensures the kernel modules won't split and will only produce a single package

```bash
KERNEL_MODULES_SPLIT = "0"
```

2. Build the image

If already had built **linux-raspberrypi** or **hailo-pci**, you need to clear the sstate-cache before building the image again

```bash
bitbake -c cleansstate linux-raspberrypi
bitbake -c cleansstate hailo-pci
```

3. Set up a RPM Http server

Look [here](./HttpRPM.md) on how to set up

4. Reinstall hailo-pci

Inside the AGL (TARGET PC) reinstall the **hailo-pci** package

```bash
dnf remove hailo-pci
dnf install hailo-pci
```

To check if everything is OK, you can use this command and check the hailo driver version.
It should be **4.23** instead of 4.20

```bash
modinfo hailo-pci
```

