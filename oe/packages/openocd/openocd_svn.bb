DESCRIPTION = "Free and Open On-Chip Debugging, In-System Programming and Boundary-Scan Testing"
HOMEPAGE = "http://openocd.berlios.de/"
LICENSE = "GPL"
PV = "0.0+svn${SRCDATE}"
PR = "r1"

inherit autotools

SRC_URI = "svn://svn.berlios.de/openocd;module=trunk \
           file://openocd-link-static.patch;patch=1"
S = "${WORKDIR}/trunk"

EXTRA_OECONF = "  --disable-ftdi2232 --disable-ftd2xx"  