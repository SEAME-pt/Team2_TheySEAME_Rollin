SUMMARY = "Adding overlays to the rpi5"
DESCRIPTION = "Overlays needed for rpi5"

RPI_KERNEL_DEVICETREE_OVERLAYS:append = " \
	overlays/vc4-kms-dsi-waveshare-panel.dtbo \
	"
