SUMMARY = "Utils scripts"
DESCRIPTION = "Utility scripts used in the RPI5 AGL"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RDEPENDS:${PN} = "bash"

SRC_URI = "file://wifi.sh file://release.sh"

S = "${WORKDIR}"

FILES:${PN} = "/root/scripts"

do_install() {
    install -d ${D}/root/scripts
    install -m 0775 wifi.sh ${D}/root/scripts/wifi.sh
    install -m 0775 release.sh ${D}/root/scripts/release.sh
}
