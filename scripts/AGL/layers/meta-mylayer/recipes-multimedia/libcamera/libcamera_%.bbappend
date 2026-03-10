LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""
SRC_URI = "git://github.com/raspberrypi/libcamera;protocol=https;branch=main"

PACKAGECONFIG:append[raspberrypi] = " -Dpipelines=rpi/pisp -Dipas=rpi/pisp"

SRCREV = "83cb81014528a66f81131a4067a6de29550e3667"
