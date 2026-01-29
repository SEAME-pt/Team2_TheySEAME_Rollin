SUMMARY = "Boot files for RPI5"
DESCRIPTION = "Boot files for RPI5"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://config.txt"

do_deploy() {
    install -d ${DEPLOYDIR}/${BOOTFILES_DIR_NAME}
    install -m 0644 ${WORKDIR}/config.txt ${DEPLOYDIR}/${BOOTFILES_DIR_NAME}/config.txt
}
