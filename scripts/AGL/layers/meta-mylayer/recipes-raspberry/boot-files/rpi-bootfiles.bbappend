SUMMARY = "Boot files for RPI5"
DESCRIPTION = "Boot files for RPI5"

FILESEXTRAPATHS:prepend := "${THISDIR}:"

SRC_URI:append = "file://config.txt file://cmdline.txt"

do_deploy:append() {
    install -m 0751 ${WORKDIR}/config.txt ${DELPOYDIR}/config.txt
}
