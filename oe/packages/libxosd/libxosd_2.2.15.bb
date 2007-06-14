DESCRIPTION = "A library for displaying a TV-like on-screen display in X."
SECTION = "libs/x11"
DEPENDS = "virtual/libx11 libxext"
LICENSE = "GPL"
PR = "r0"

SRC_URI = "svn://libxosd.svn.sourceforge.net/svnroot/libxosd/source;module=${PV};proto=https"
S = "${WORKDIR}/${PV}"

inherit autotools binconfig

do_stage() {
	autotools_stage_all
}

