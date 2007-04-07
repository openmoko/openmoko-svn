DESCRIPTION = "The OpenMoko Media Player"
SECTION = "openmoko/applications"
DEPENDS += "alsa-lib dbus-glib libid3 libvorbis"
PV = "0.0.1+svn${SRCDATE}"
PR = "r1"

inherit openmoko

FILES_${PN} = "${bindir}/* \
	       ${libdir}/* \
	       ${datadir}/applications \
	       ${datadir}/images \
	      "
