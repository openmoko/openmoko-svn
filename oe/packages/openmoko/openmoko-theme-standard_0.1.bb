DESCRIPTION = "Standard Gtk+ theme for the OpenMoko distribution"
HOMEPAGE = "http://www.openmoko.org"
SECTION = "openmoko/base"
LICENSE = "GPL"

SRC_URI = "${OPENMOKO_MIRROR}/artwork;module=themes;proto=http"
S = "${WORKDIR}"

dirs = "themes/openmoko-standard"

do_install() {
	find ${WORKDIR} -name ".svn" | xargs rm -rf
	install -d ${D}${datadir}/themes/
	for i in ${dirs}; do
		cp -fpPR ${WORKDIR}/$i ${D}${datadir}/themes/
	done
}

FILES_${PN} = "${datadir}"
