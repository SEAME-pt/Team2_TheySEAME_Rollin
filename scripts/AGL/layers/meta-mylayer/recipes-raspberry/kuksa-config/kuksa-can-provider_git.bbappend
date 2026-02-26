SUMMARY = "Kuksa Config Files"
DESCRIPTION = "Kuksa Config Files in the RaspberryPi5"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RDEPENDS:${PN} = "bash"

SRC_URI:append = " file://kuksa-can-provider.default"

S = "${WORKDIR}"

do_install:append() {
    install -d ${D}/${sysconfdir}/default
    install -d ${D}/${sysconfdir}/kuksa-can-provider

    install -m 0644 ${S}/files/config.ini ${D}/${sysconfdir}/kuksa-can-provider/config.ini
    install -m 0644 kuksa-can-provider.default ${D}/${sysconfdir}/default/kuksa-can-provider.default
}
