DESCRIPTION = "The OpenMoko Dates Application"
SECTION = "openmoko/pim"
LICENSE = "GPL"
DEPENDS = "glib-2.0 gtk+ libglade eds-dbus openmoko-libs"
RDEPENDS = "libedata-cal"
PV = "0.1+svn${SRCDATE}"
PR = "r7"

inherit gnome autotools pkgconfig gtk-icon-cache

SRC_URI = "svn://svn.o-hand.com/repos/dates/branches/;module=openmoko;proto=http \
           file://Dates.png \
           file://openmoko-dates.desktop"

S = "${WORKDIR}/openmoko"

EXTRA_OECONF = "--enable-omoko"

do_install_append () {
	install -d ${D}/${datadir}/pixmaps
	install -m 0644 ${WORKDIR}/Dates.png ${D}/${datadir}/pixmaps/
	rm -rf ${D}${datadir}/icons
	rm -rf ${D}${datadir}/applications/dates.desktop
	install -m 0644 ${WORKDIR}/openmoko-dates.desktop ${D}${datadir}/applications/
}

FILES_${PN} += "${datadir}/pixmaps \
                ${datadir}/dates/"

