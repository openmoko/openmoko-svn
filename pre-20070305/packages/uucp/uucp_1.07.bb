DESCRIPTION = "Unix to Unix Copy"
HOMEPAGE = "http://www.airs.com/ian/uucp.html"
SECTION = "console/utils"
LICENSE = "GPL"
PR = "r0"

SRC_URI = "ftp://ftp.gnu.org/pub/gnu/uucp/uucp-${PV}.tar.gz"
S = "${WORKDIR}/uucp-${PV}"

inherit autotools

do_configure() {
	gnu-configize
	oe_runconf
}
