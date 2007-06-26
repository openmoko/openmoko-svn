DESCRIPTION = "The OpenMoko Dialer"
SECTION = "openmoko/applications"
PV = "0.0.1+svn${SRCDATE}"

DEPENDS += "libgsmd eds-dbus"

inherit openmoko

FILES_${PN} += "${datadir}/dbus-1/services"
