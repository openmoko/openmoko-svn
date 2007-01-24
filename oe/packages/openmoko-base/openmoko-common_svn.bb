DESCRIPTION = "Common files for the OpenMoko distribution"
SECTION = "openmoko/base"
PV = "0.0+svn${SRCDATE}"
PR = "r1"

inherit openmoko-base

SRC_URI = "${OPENMOKO_MIRROR}/src/target/${OPENMOKO_RELEASE}/artwork;module=images;proto=https"
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
