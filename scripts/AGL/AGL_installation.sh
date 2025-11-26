#!/bin/bash

# Init the build enviroment
source meta-agl/scripts/aglsetup.sh -m raspberrypi5 -b $BUILD_DIR $AGL_FEATURES

# Add meta-qt6 layer
bitbake-layers add-layer $AGL_TOP/external/meta-qt6

# Add recipes to local.conf
echo "IMAGE_INSTALL:append = \"$QT_RECIPES\"" >> $LOCAL_CONF
echo "IMAGE_INSTALL:append = \"$DEVEL_RECIPES\"" >> $LOCAL_CONF

# Edit local.conf
echo "SSTATE_DIR = \"$AGL_TOP/sstate-cache/\"" >> $LOCAL_CONF
echo "DL_DIR = \"$HOME/downloads/\"" >> $LOCAL_CONF
echo "INHERIT += \"rm_work\"" >> $LOCAL_CONF
echo "RM_OLD_IMAGE = \"1\"" >> $LOCAL_CONF

# Start creating the image
time bitbake agl-image-compositor

