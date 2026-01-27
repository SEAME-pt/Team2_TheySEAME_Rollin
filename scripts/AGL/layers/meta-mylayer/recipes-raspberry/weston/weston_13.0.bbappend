SUMMARY = "Weston custom config"
DESCRIPTION = "Install a custom weston configuration"

SRC_URI = "file://weston.ini.default"

S = "${WORKDIR}"

do_install() {
    install -d ${D}${sysconfdir}/xdg/weston
    install -m 0644 weston.ini.default ${D}${sysconfdir}/xdg/weston
}
