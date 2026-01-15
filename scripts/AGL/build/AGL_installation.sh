#!/bin/bash

# Download AGL with repo command
repo init -b trout -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo && repo sync

# Init the build enviroment
source meta-agl/scripts/aglsetup.sh -f -m raspberrypi5 -b build $AGL_FEATURES

# Add meta-qt6 layer
bitbake-layers add-layer ../external/meta-qt6
bitbake-layers add-layer ../meta-mylayer

# Symlink the site.conf
ln -sf $HOME/agl/site.conf $AGL_TOP/conf/.

# Start creating the image and sdk
time bitbake agl-image-weston
time bitbake agl-image-weston -c do_populate_sdk

