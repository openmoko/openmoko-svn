DESCRIPTION = "Free and Open On-Chip Debugging, In-System Programming and Boundary-Scan Testing"
HOMEPAGE = "http://openocd.berlios.de/"
LICENSE = "GPL"
PV = "141"

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/openocd"

inherit autotools

SRC_URI = "svn://svn.berlios.de/;module=openocd;rev=${PV} \
	   http://svn.openmoko.org/developers/werner/openocd-wait-patiently.patch;patch=1 \
	   file://openocd-link-static.patch;patch=1"
S = "${WORKDIR}/openocd/trunk"

EXTRA_OECONF = "  --disable-ftdi2232 --disable-ftd2xx"  
