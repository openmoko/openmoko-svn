DESCRIPTION = "Settings-daemon provides a bridge between gconf and xsettings"
LICENSE = "GPL"
DEPENDS = "gconf glib-2.0"
RDEPENDS = "xrdb"
SECTION = "x11"
PV = "0.0+svn${SRCDATE}"
PR = "r4"

SRC_URI = "svn://svn.o-hand.com/repos/matchbox/trunk;module=${PN};proto=http \
           file://70settings-daemon "

S = "${WORKDIR}/${PN}"

inherit autotools pkgconfig gettext gconf

do_configure_append() {
	if [ "${DISTRO}" = "openmoko" ]; then
		sed -i -e "s,poky,openmoko," settings-daemon.c
		sed -i -e "s,poky,openmoko," settings-daemon.schemas
		sed -i -e "s,Clearlooks,openmoko-standard," settings-daemon.schemas
		sed -i -e "s,Sato,openmoko-standard," settings-daemon.schemas
	fi
}

do_install_append () {
	install -d ${D}/${sysconfdir}/X11/Xsession.d
	install -m 755 ${WORKDIR}/70settings-daemon ${D}/${sysconfdir}/X11/Xsession.d/
}

FILES_${PN} = "${bindir}/* ${sysconfdir}"
