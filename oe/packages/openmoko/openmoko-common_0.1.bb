DESCRIPTION = "Common files for the OpenMoko distribution"
HOMEPAGE = "http://www.openmoko.org"
SECTION = "openmoko/base"
LICENSE = "GPL"

SRC_URI = "file://pixmaps"

dirs = "pixmaps"

do_install() {
	install -d ${D}${datadir}/openmoko/
	for i in ${dirs}; do
		cp -fpPR ${WORKDIR}/$i ${D}${datadir}/openmoko/
	done
	find ${D}${datadir} -name ".svn" -exec rm -rf {} \;
}

FILES_${PN} = "${datadir}"

