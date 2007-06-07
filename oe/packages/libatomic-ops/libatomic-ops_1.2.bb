DESCRIPTION = "libatomic-ops is a library for atomic integer operations"
SECTION = "devel"
LICENSE = "MIT"

SRC_URI = "${DEBIAN_MIRROR}/main/liba/libatomic-ops/libatomic-ops_${PV}.orig.tar.gz"
S = "${WORKDIR}/libatomic_ops-${PV}"

inherit autotools pkgconfig

do_stage() {
	autotools_stage_all
}


