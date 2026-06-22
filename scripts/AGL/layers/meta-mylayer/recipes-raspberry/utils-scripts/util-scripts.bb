SUMMARY = "Utils scripts"
DESCRIPTION = "Utility scripts used in the RPI5 AGL"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RDEPENDS:${PN} = "bash"

SRC_URI = "file://wifi.sh file://release.sh file://wifi.service file://CarControl.service"

S = "${WORKDIR}"

SYSTEMD_SERVICE:${PN} = "wifi.service CarControl.service"
FILES:${PN} = "/root/scripts ${systemd_unitdir}/system"

do_install() {
    install -d ${D}/root/scripts
    install -m 0775 wifi.sh ${D}/root/scripts/wifi.sh
    install -m 0775 release.sh ${D}/root/scripts/release.sh

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 wifi.service ${D}${systemd_unitdir}/system
    install -m 0644 CarControl.service ${D}${systemd_unitdir}/system
}
