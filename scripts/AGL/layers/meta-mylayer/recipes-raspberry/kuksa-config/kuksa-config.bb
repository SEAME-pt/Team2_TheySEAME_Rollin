SUMMARY = "Kuksa Config Files"
DESCRIPTION = "Kuksa Config Files in the RaspberryPi5"
LICENSE = "CLOSED"
LIC_FILES_CHKSUM = ""

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

RDEPENDS:${PN} = "bash"

SRC_URI += "file://CAN.dbc file://vss.json file://config.ini file://kuksa-can-provider.default file://kuksa-databroker"

S = "${WORKDIR}"

do_install() {
    install -d ${D}/${sysconfdir}/kuksa
    install -d ${D}/${sysconfdir}/kuksa-can-provider
    install -d ${D}/${sysconfdir}/default
    install -m 0644 CAN.dbc ${D}/${sysconfdir}/kuksa/CAN.dbc
    install -m 0644 vss.json ${D}/${sysconfdir}/kuksa/vss.json
    install -m 0644 config.ini ${D}/${sysconfdir}/kuksa-can-provider/config.ini
    install -m 0644 kuksa-can-provider.default ${D}/${sysconfdir}/default/kuksa-can-provider.default
    install -m 0644 kuksa-databroker ${D}/${sysconfdir}/default/kuksa-databroker
}
