#!/bin/bash

AGL_TOP=$HOME/$BUILD_DIR

# Init the build enviroment
source meta-agl/scripts/aglsetup.sh -f -m raspberrypi5 -b $BUILD_DIR $AGL_FEATURES

# Add meta-qt6 layer
bitbake-layers add-layer ../external/meta-qt6
bitbake-layers add-layer ../meta-mylayer

# Symlink the site.conf
ln -sf $HOME/site.conf $AGL_TOP/conf/.

time bitbake agl-image-weston
