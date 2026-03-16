#!/bin/bash

set -x

IMAGE_NAME=$(find $HOME/Documents/Repo_Joao/scripts/AGL/agl/tmp/deploy/images -name "*rootfs.wic.xz")

sudo umount /dev/sda1
sudo umount /dev/sda2
xzcat ${IMAGE_NAME} | sudo dd of=/dev/sda bs=4M
sync
