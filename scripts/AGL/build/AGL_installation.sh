#!/bin/bash

LOCAL_CONF=./conf/local.conf
AGL_FEATURES="agl-app-framework agl-buildstats agl-devel agl-drm-lease agl-ic \
	agl-netboot agl-package-management agl-pipewire agl-selinux"
DEVEL_RECIPES=" cmake"
QT_RECIPES=" qtbase qtmultimedia qtquick3d qtdeclarative"

repo init -b trout -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo && \
repo sync

# Init the build enviroment
source meta-agl/scripts/aglsetup.sh -f -m raspberrypi5 -b build $AGL_FEATURES

# Add meta-qt6 layer
bitbake-layers add-layer ../external/meta-qt6
bitbake-layers add-layer ../meta-mylayer

# Add recipes to local.conf
echo "IMAGE_INSTALL:append = \"$QT_RECIPES\"" >> $LOCAL_CONF
echo "IMAGE_INSTALL:append = \"$DEVEL_RECIPES\"" >> $LOCAL_CONF

# Edit local.conf
echo "SSTATE_DIR = \"../sstate-cache/\"" >> $LOCAL_CONF
echo "DL_DIR = \"../downloads/\"" >> $LOCAL_CONF
echo "INHERIT += \"rm_work\"" >> $LOCAL_CONF
echo "RM_OLD_IMAGE = \"1\"" >> $LOCAL_CONF

# Start creating the image and sdk
time bitbake agl-image-weston
time bitbake agl-image-weston -c do_populate_sdk

