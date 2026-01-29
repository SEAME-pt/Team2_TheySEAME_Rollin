SUMMARY = "Boot files for RPI5"
DESCRIPTION = "Boot files for RPI5"

FILESEXTRAPATHS:prepend := "${THISDIR}/files:"

SRC_URI += "file://cmdline.txt"

do_deploy() {
    install -d ${DEPLOYDIR}/${BOOTFILES_DIR_NAME}
    install -m 0644 ${WORKDIR}/cmdline.txt ${DEPLOYDIR}/${BOOTFILES_DIR_NAME}/cmdline.txt
}
