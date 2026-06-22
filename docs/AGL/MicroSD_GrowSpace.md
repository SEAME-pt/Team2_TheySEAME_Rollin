# MicroSD increase rootfs space

This document describes how to increase microSD space in AGL

# Steps

If the microSD is the same as before, it already has a 90GB partition,
so just running this ```resize2fs /dev/mmcblk0p2``` increases it's size

If it's a different microSD, i don't know how to do it
