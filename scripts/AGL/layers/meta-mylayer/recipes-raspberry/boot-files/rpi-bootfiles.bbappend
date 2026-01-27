SUMMARY = "Boot files for RPI5"
DESCRIPTION = "Boot files for RPI5"

FILESEXTRAPATHS:prepend := "${THISDIR}"

SRC_URI:remove = "file://config.txt"
SRC_URI:append = "file://config.txt file://cmdline.txt"
