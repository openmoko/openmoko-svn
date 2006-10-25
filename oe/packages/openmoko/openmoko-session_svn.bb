DESCRIPTION = "Matchbox session files for OpenMoko"
HOMEPAGE = "http://www.openmoko.org"
SECTION = "openmoko/base"
LICENSE = "GPL"
RDEPENDS = "matchbox matchbox-applet-startup-monitor gtk-theme-clearlooks"
PV = "0.0+svn${SRCDATE}"

SRC_URI = "${OPENMOKO_MIRROR};module=etc;proto=http"
S = "${WORKDIR}"

do_install() {
	cp -R ${S}/etc ${D}/etc
	rm -fR ${D}/etc/.svn
	rm -fR ${D}/etc/matchbox/.svn
	chmod -R 755 ${D}/etc
}

pkg_postinst_openmoko-session () {
#!/bin/sh -e
if [ "x$D" != "x" ]; then
    exit 1
fi

gconftool-2 --config-source=xml::$D${sysconfdir}/gconf/gconf.xml.defaults --direct --type string --set /desktop/poky/interface/theme Clearlooks
}
