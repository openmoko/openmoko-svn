DESCRIPTION = "Amiga Modules of Michael Lauer"
SECTION = "openmoko/media"
PV = "1"
PR = "r0"

inherit openmoko-base

SRC_URI = "http://people.openmoko.org/laforge/audio/mickeys_modules.tar.bz2"
S = "${WORKDIR}"

dirs = "mickeys_modules"

do_install() {
	find ${WORKDIR} -name ".svn" | xargs rm -rf
	install -d ${D}${datadir}/sounds/
	for i in ${dirs}; do
		cp -fpPR ${WORKDIR}/$i ${D}${datadir}/sounds/
	done
}

FILES_${PN} = "${datadir}"
