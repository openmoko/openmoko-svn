DESCRIPTION = "matchbox-panel-2 is a lightweight dock (system tray) application based on Gtk+"
LICENSE = "GPL"
SECTION = "x11/panels"
DEPENDS = "gtk+"
PV = "0.1+svn${SRCDATE}"
PR = "r0"

SRC_URI = "svn://svn.o-hand.com/repos/matchbox/trunk;module=${PN};proto=http \
           file://glib-2.8-backport.patch;patch=1"
S = "${WORKDIR}/${PN}"

inherit autotools pkgconfig

EXTRA_OECONF = "--disable-startup-notification --disable-libnotify"

do_stage() {
    autotools_stage_all
}

PACKAGES += "${PN}-applets"
FILES_${PN}-applets = "${libdir}/matchbox-panel/lib*.so* ${datadir}/*"

