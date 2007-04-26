DESCRIPTION = "The OpenMoko Tasks Application"
SECTION = "openmoko/pim"
LICENSE = "GPL"
DEPENDS = "glib-2.0 gtk+ libglade eds-dbus openmoko-libs"
RDEPENDS = "libedata-cal"
PV = "0.1+svn${SRCDATE}"
PR = "r8"

inherit gnome autotools pkgconfig gtk-icon-cache

SRC_URI = "svn://svn.o-hand.com/repos/tasks/;module=trunk;proto=http

S = "${WORKDIR}/openmoko"

EXTRA_OECONF = "--enable-omoko --disable-gtk"

