DESCRIPTION = "Free and Open On-Chip Debugging, In-System Programming and Boundary-Scan Testing"
HOMEPAGE = "http://openocd.berlios.de/"
LICENSE = "GPL"
PV = "0.0+svn${SRCDATE}"

inherit autotools

SRC_URI = "svn://svn.berlios.de/;module=openocd"
S = "${WORKDIR}/openocd/trunk"

EXTRA_OECONF = "  --disable-ftdi2232 --disable-ftd2xx"  
