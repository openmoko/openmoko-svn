DESCRIPTION = "Common files for the OpenMoko distribution"
HOMEPAGE = "http://www.openmoko.org"
SECTION = "openmoko/base"
LICENSE = "GPL"

SRC_URI = "${OPENMOKO_MIRROR}/artwork;module=images;proto=http"
S = "${WORKDIR}"

dirs = "images/pixmaps"

do_install() {
	find ${WORKDIR} -name ".svn" | xargs rm -rf
	install -d ${D}${datadir}/openmoko/
	for i in ${dirs}; do
		cp -fpPR ${S}/$i ${D}${datadir}/openmoko/
	done
}

FILES_${PN} = "${datadir}"
