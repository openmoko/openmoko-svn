DESCRIPTION = "A library for displaying a TV-like on-screen display in X."
SECTION = "libs/x11"
DEPENDS = "virtual/libx11 libxext"
LICENSE = "GPL"
PR = "r0"

SRC_URI = "${SOURCEFORGE_MIRROR}/libxosd/libxosd-${PV}.tar.gz"

inherit autotools binconfig

do_stage() {
	autotools_stage_all
}

