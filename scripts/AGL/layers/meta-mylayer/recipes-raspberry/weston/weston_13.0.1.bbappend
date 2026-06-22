SUMMARY = "Weston custom config"
DESCRIPTION = "Install a custom weston configuration"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://weston.ini"

do_install:append() {
    install -d ${D}/${sysconfdir}/xdg/weston
    install -m 0644 ${WORKDIR}/weston.ini ${D}/${sysconfdir}/xdg/weston/weston.ini
}
