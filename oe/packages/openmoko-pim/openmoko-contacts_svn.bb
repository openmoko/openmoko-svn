DESCRIPTION = "Contacts is an address-book application."
LICENSE = "GPL"
SECTION = "openmoko/pim"
DEPENDS = "glib-2.0 gtk+ libglade eds-dbus gnome-vfs openmoko-libs"
RDEPENDS = "gnome-vfs-plugin-file"
RRECOMMENDS = "gnome-vfs-plugin-http"
PV = "0.1+svn${SRCDATE}"
PR = "r0"

SRC_URI = "svn://svn.o-hand.com/repos/contacts/branches/private;module=omoko;proto=https \
	   file://stock_contact.png \
	   file://stock_person.png"
S = "${WORKDIR}/omoko"

inherit autotools pkgconfig

EXTRA_OECONF = "--enable-gnome-vfs"

do_install_append () {
	install -d ${D}/${datadir}/pixmaps
	install -m 0644 ${WORKDIR}/stock_contact.png ${D}/${datadir}/pixmaps
	install -m 0644 ${WORKDIR}/stock_person.png ${D}/${datadir}/pixmaps
}

FILES_${PN} += "${datadir}/pixmaps/stock_contact.png \
		${datadir}/pixmaps/stock_person.png"

