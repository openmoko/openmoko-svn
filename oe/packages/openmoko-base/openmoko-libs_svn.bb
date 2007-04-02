DESCRIPTION = "openmoko-libs is a set of libraries implementing an \
application framework for mobile communications applications \
based on Gtk+ 2.x"
SECTION = "openmoko/libs"
LICENSE = "LGPL"
DEPENDS += "gtk+ eds-dbus"
PV = "0.0.1+svn${SRCDATE}"
PR = "r3"

inherit openmoko

do_configure_prepend() {
        touch libmokocore/Makefile.in
        touch libmokopim/Makefile.in
        touch libmokonet/Makefile.in
}

do_stage() {
        autotools_stage_all
}

PACKAGES =+ "libmokojournal libmokojournal-dev libmokojournal-dbg"

FILES_libmokojournal = "${libdir}/libmokojournal.so.*"
FILES_libmokojournal-dev = "${libdir}/libmokojournal.so ${libdir}/libmokojournal.*a ${includedir}/${PN}/libmokojournal"
FILES_libmokojournal-dbg = "${libdir}/.debug/libmokojournal.so.*"
