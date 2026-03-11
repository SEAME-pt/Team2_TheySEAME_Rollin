DESVRIPTION = "LibPisp for AGL"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""
SRC_URI = "git://github.com/raspberrypi/libpisp.git;protocol=https;branch=main;"

SRCREV = "3312c3ba6d76f12d5163d7b85a9c16307e9f57e8"

DEPENDS = "nlohmann-json"

S = "${WORKDIR}/git"

inherit meson pkgconfig
