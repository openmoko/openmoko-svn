DESCRIPTION = "Unix to Unix Copy"
HOMEPAGE = "http://www.airs.com/ian/uucp.html"
SECTION = "console/utils"
LICENSE = "GPL"
PR = "r1"

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/files"

SRC_URI = "ftp://ftp.gnu.org/pub/gnu/uucp/uucp-${PV}.tar.gz \
	   file://policy.patch;patch=1"

S = "${WORKDIR}/uucp-${PV}"

inherit autotools

do_configure() {
	gnu-configize
	oe_runconf
}
