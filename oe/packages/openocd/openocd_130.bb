DESCRIPTION = "Free and Open On-Chip Debugging, In-System Programming and Boundary-Scan Testing"
HOMEPAGE = "http://openocd.berlios.de/"
LICENSE = "GPL"
PV = "130"

inherit autotools

SRC_URI = "svn://svn.berlios.de/;module=openocd;revision=130 \
	   http://svn.openmoko.org/developers/werner/openocd-wait-patiently.patch;patch=1"
S = "${WORKDIR}/openocd/trunk"

EXTRA_OECONF = "  --disable-ftdi2232 --disable-ftd2xx"  
