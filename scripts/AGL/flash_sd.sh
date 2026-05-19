#!/bin/bash

set -x

IMAGE_NAME=$(find $PWD/agl/tmp/deploy/images -name "*rootfs.wic.xz")

sudo umount /dev/sda1
sudo umount /dev/sda2
xzcat ${IMAGE_NAME} | sudo dd of=/dev/sda bs=4M status=progress
sync
