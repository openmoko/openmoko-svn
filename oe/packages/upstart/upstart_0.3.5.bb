DESCRIPTION = "Upstart: An event-based replacement for the /sbin/init daemon"
HOMEPAGE = "http://upstart.ubuntu.com/"
SECTION = "base"
LICENSE = "GPL"
PR = "r1"

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/files"

SRC_URI = "http://upstart.ubuntu.com/download/upstart-0.3.5.tar.bz2 \
	   file://upstart-0.3.5-pwrokfail.patch;patch=1 \
	  "
inherit autotools

EXTRA_OECONF = "--enable-compat=sysv"

do_configure() {
	gnu-configize
	oe_runconf
}
