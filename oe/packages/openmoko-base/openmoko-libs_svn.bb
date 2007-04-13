DESCRIPTION = "Openmoko-libs are a set of libraries implementing an \
application framework for mobile communications applications \
based on Gtk+ 2.x"
SECTION = "openmoko/libs"
LICENSE = "LGPL"
DEPENDS += "gtk+ eds-dbus libgsmd"
PV = "0.0.1+svn${SRCDATE}"
PR = "r4"

inherit openmoko

do_configure_prepend() {
        touch libmokocore/Makefile.in
        touch libmokopim/Makefile.in
        touch libmokonet/Makefile.in
}

do_stage() {
        autotools_stage_all
}

PACKAGES = "\
  libmokocore libmokocore-dev libmokocore-dbg \
  libmokogsmd libmokogsmd-dev libmokogsmd-dbg \
  libmokoui libmokoui-dev libmokoui-dbg \
  libmokojournal libmokojournal-dev libmokojournal-dbg"

FILES_libmokocore = "${libdir}/libmokocore.so.*"
FILES_libmokocore-dev = "${libdir}/libmokocore.so ${libdir}/libmokocore.*a ${includedir}/${PN}/libmokocore"
FILES_libmokocore-dbg = "${libdir}/.debug/libmokocore.so.*"

FILES_libmokogsmd = "${libdir}/libmokogsmd.so.*"
FILES_libmokogsmd-dev = "${libdir}/libmokogsmd.so ${libdir}/libmokogsmd.*a ${includedir}/${PN}/libmokogsmd"
FILES_libmokogsmd-dbg = "${libdir}/.debug/libmokogsmd.so.*"

FILES_libmokoui = "${libdir}/libmokoui.so.*"
FILES_libmokoui-dev = "${libdir}/libmokoui.so ${libdir}/libmokoui.*a ${includedir}/${PN}/libmokoui"
FILES_libmokoui-dbg = "${libdir}/.debug/libmokoui.so.*"

FILES_libmokojournal = "${libdir}/libmokojournal.so.*"
FILES_libmokojournal-dev = "${libdir}/libmokojournal.so ${libdir}/libmokojournal.*a ${includedir}/${PN}/libmokojournal"
FILES_libmokojournal-dbg = "${libdir}/.debug/libmokojournal.so.*"
