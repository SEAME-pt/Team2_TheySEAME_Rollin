SUMMARY = "Weston custom config"
DESCRIPTION = "Install a custom weston configuration"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

SRC_URI = "file://weston.ini.default"

do_install:append() {
    install -d ${D}${weston_ini_dir}
    install -m 0644 ${WORKDIR}/weston.ini.default ${D}${weston_ini_dir}
}
