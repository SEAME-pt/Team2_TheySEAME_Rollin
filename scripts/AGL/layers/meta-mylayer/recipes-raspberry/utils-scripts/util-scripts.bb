SUMMARY = "Utils scripts"
DESCRIPTION = "Utility scripts used in the RPI5 AGL"

FILESEXTRAPATHS:prepend := "${THISDIR}/files"

SRC_URI = "file://wifi.sh file:release.sh"

do_install() {
    install -d ${D}/root/scripts
    install -m 0775 ${WORKDIR}/wifi.sh ${D}/root/scripts
    install -m 0775 ${WORKDIR}/release.sh ${D}/root/scripts
}
