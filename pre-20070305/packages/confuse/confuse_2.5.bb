SRC_URI = "http://download.savannah.gnu.org/releases/confuse/confuse-${PV}.tar.gz \
           file://build-only-library.patch;patch=1"
S = "${WORKDIR}/confuse-${PV}"

inherit autotools binconfig pkgconfig

EXTRA_OECONF = "--enable-shared"

do_stage() {
    autotools_stage_all
}

