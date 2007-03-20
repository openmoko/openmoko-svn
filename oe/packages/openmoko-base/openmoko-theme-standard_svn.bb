DESCRIPTION = "Standard Gtk+ theme for the OpenMoko distribution"
SECTION = "openmoko/base"
PV = "0.0+svn${SRCDATE}"
PR = "r4"

inherit openmoko-base

SRC_URI = "${OPENMOKO_MIRROR}/src/target/${OPENMOKO_RELEASE}/artwork;module=themes;proto=http"
S = "${WORKDIR}"

dirs = "themes/openmoko-standard"

do_install() {
	find ${WORKDIR} -name ".svn" | xargs rm -rf
	install -d ${D}${datadir}/themes/
	for i in ${dirs}; do
		cp -fpPR ${WORKDIR}/$i ${D}${datadir}/themes/
	done
	
	install -d ${D}${sysconfdir}/gtk-2.0
	echo 'include "${datadir}/themes/openmoko-standard/gtk-2.0/gtkrc"' > ${D}${sysconfdir}/gtk-2.0/gtkrc
}

# yes, i know... we're going to have a more sane method to do that
# to respect all kinds of gtk-theme packages
pkg_postinst() {
	echo overriding current theme to openmoko-standard...
	mkdir -p ${sysconfdir}/gtk-2.0
	echo 'include "${datadir}/themes/openmoko-standard/gtk-2.0/gtkrc"' > ${sysconfdir}/gtk-2.0/gtkrc
	echo done
}

FILES_${PN} = "${datadir} ${sysconfdir}"
