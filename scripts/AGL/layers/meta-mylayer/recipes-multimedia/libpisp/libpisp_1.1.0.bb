DESVRIPTION = "LibPisp for AGL"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""
SRC_URI = "git://github.com/raspberrypi/libpisp.git;protocol=https;branch=main;"
SRCREV = "e7974a156008bcd7bf077e1315afba6d54c2bb4a"

DEPENDS = "nlohmann-json"

inherit meson pkgconfig
