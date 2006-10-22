DESCRIPTION = "Common files for the OpenMoko distribution"
HOMEPAGE = "http://www.openmoko.org"
SECTION = "openmoko/base"
LICENSE = "GPL"

SRC_URI = "file://pixmaps"

FILESPATH += ":/local/pkg/openmoko/OM-2007/artwork/images"

dirs = "pixmaps"

do_install() {
	find ${WORKDIR} -name ".svn" | xargs rm -rf
	install -d ${D}${datadir}/openmoko/
	for i in ${dirs}; do
		cp -fpPR ${WORKDIR}/$i ${D}${datadir}/openmoko/
	done
}

FILES_${PN} = "${datadir}"

