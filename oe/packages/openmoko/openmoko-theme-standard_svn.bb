DESCRIPTION = "Standard Gtk+ theme for the OpenMoko distribution"
HOMEPAGE = "http://www.openmoko.org"
SECTION = "openmoko/base"
LICENSE = "GPL"
PV = "0.0+svn${SRCDATE}"
PR = "r1"

SRC_URI = "${OPENMOKO_MIRROR}/artwork;module=themes;proto=http"
S = "${WORKDIR}"

dirs = "themes/openmoko-standard"

do_install() {
	find ${WORKDIR} -name ".svn" | xargs rm -rf
	install -d ${D}${datadir}/themes/
	for i in ${dirs}; do
		cp -fpPR ${WORKDIR}/$i ${D}${datadir}/themes/
	done
	
	install -d ${D}${sysconfdir}/gtk-2.0
	echo 'include "${datadir}/themes/openmoko-standard/gtkrc"' >> ${D}${sysconfdir}/gtk-2.0/gtkrc
}

FILES_${PN} = "${datadir} ${sysconfdir}"
