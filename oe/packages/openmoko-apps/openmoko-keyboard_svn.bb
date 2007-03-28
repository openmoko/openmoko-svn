DESCRIPTION = "The OpenMoko Keyboard Application"
SECTION = "openmoko/applications"
DEPENDS = "libfakekey expat libxft"

PV = "0.0.1+svn${SRCDATE}"
PR = "r1"

inherit openmoko

FILES_${PN} = "${bindir}/* \
               ${datadir}/applications \
               ${datadir}/pixmaps \
               ${datadir}/openmoko-keyboard "

