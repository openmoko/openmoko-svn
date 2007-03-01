DESCRIPTION = "Free and Open On-Chip Debugging, In-System Programming and Boundary-Scan Testing"
HOMEPAGE = "http://openocd.berlios.de/"
LICENSE = "GPL"
PV = "130"

inherit autotools

SRC_URI = "svn://svn.berlios.de/;module=openocd;rev=130 \
	   http://svn.openmoko.org/developers/werner/openocd-wait-patiently.patch;patch=1 \
	   file://openocd-link-static.patch;patch=1"
S = "${WORKDIR}/openocd/trunk"

EXTRA_OECONF = "  --disable-ftdi2232 --disable-ftd2xx"  

do_deploy() {
	install -d ${DEPLOY_DIR_IMAGE}
	install -m 0755 src/openocd ${DEPLOY_DIR_IMAGE}/openocd
}

addtask deploy before do_package after do_install
