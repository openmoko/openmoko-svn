DESCRIPTION = "The OpenMoko On-Screen-Keyboard"
SECTION = "openmoko/inputmethods"
DEPENDS = "libfakekey expat libxft"
PV = "0.0.1+svn${SRCDATE}"
PR = "r1"

inherit openmoko pkgconfig gettext

EXTRA_OECONF = "--disable-cairo"

FILES_${PN} = "${bindir}/* ${datadir}"
