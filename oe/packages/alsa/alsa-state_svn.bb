DESCRIPTION = "ALSA state files"
LICENSE = "MIT"
SECTION = "base"
PR = "r2"

SRC_URI = "file://capturehandset.state \
           file://captureheadset.state \
           file://gsmbluetooth.state \
           file://gsmhandset.state \
           file://gsmheadset.state \
           file://stereoout.state"

do_install () {
	install -d ${D}${sysconfdir}/alsa
	install -m 0644 ${WORKDIR}/*.state ${D}${sysconfdir}/alsa
}

PACKAGE_ARCH = "all"
FILES_${PN} += "${sysconfdir}/alsa/*"
