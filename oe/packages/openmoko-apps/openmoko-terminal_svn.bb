DESCRIPTION = "The OpenMoko Application Manager"
SECTION = "openmoko/applications"
DEPENDS += "ipkg"
RDEPENDS += "gtkterm2"
PV = "1.0.0+svn${SRCDATE}"
PR = "r0"

inherit openmoko

SRC_URI = "file://openmoko-terminal.png \
           file://openmoko-terminal.desktop"

do_install() {
        install -d ${D}/${datadir}/pixmaps
        install -d ${D}/${datadir}/applications
        install -m 0644 ${WORKDIR}/openmoko-terminal.png ${D}/${datadir}/pixmaps/
        install -m 0644 ${WORKDIR}/openmoko-terminal.desktop ${D}${datadir}/applications/
}

