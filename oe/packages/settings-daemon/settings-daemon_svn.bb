DESCRIPTION = "Settings-daemon is a bridge between xst/gpe-confd and gconf"
LICENSE = "GPL"
DEPENDS = "gconf xst glib-2.0"
SECTION = "x11"
PR = "r2"

SRC_URI = "svn://svn.o-hand.com/repos/matchbox/trunk;module=${PN};proto=http \
           file://70settings-daemon "

S = "${WORKDIR}/${PN}"

inherit autotools pkgconfig gettext

do_configure_append() {
	if [ "${DISTRO}" = "openmoko" ]; then
		sed -i -e "s,poky,openmoko," settings-daemon.c
		sed -i -e "s,poky,openmoko," settings-daemon.schemas
		sed -i -e "s,Clearlooks,openmoko-standard," settings-daemon.schemas
	fi
}

do_install_append () {
	install -d ${D}/${sysconfdir}/X11/Xsession.d
	install -m 755 ${WORKDIR}/70settings-daemon ${D}/${sysconfdir}/X11/Xsession.d/
}

FILES_${PN} = "${bindir}/* ${sysconfdir}"
