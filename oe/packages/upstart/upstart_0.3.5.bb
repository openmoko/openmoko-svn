DESCRIPTION = "Upstart: An event-based replacement for the /sbin/init daemon"
HOMEPAGE = "http://upstart.ubuntu.com/"
SECTION = "base"
LICENSE = "GPL"
PR = "r2"

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/files"

SRC_URI = "http://upstart.ubuntu.com/download/upstart-0.3.5.tar.bz2 \
	   file://upstart-0.3.5-pwrokfail.patch;patch=1 \
	  "
inherit autotools update-alternatives

ALTERNATIVE_NAME = "init"
ALTERNATIVE_LINK = "${base_sbindir}/init"
ALTERNATIVE_PATH = "${base_sbindir}/init.upstart"
ALTERNATIVE_PRIORITY = 60

EXTRA_OECONF = "--enable-compat=sysv --sbindir=${base_sbindir} --libdir=${base_libdir}"
#EXTRA_OECONF = "--enable-compat=sysv"

do_configure() {
	gnu-configize
	oe_runconf
}

do_install() {
	oe_runmake 'DESTDIR=${D}' install
	mv ${D}${base_sbindir}/init ${D}${base_sbindir}/init.${PN}
	# initctl
	mv ${D}${base_sbindir}/reboot ${D}${base_sbindir}/reboot.${PN}
	# runlevel
	mv ${D}${base_sbindir}/shutdown ${D}${base_sbindir}/shutdown.${PN}
	# telinit
	mv ${D}${base_sbindir}/halt ${D}${base_sbindir}/halt.${PN}
	# poweroff
	# logd
}

pkg_postinst_${PN} () {
	update-alternatives --install ${base_sbindir}/halt halt halt.${PN} 200
	update-alternatives --install ${base_sbindir}/reboot reboot reboot.${PN} 200
	update-alternatives --install ${base_sbindir}/shutdown shutdown shutdown.${PN} 200
}

pkg_prerm_${PN} () {
	update-alternatives --remove halt halt.${PN}
	update-alternatives --remove reboot reboot.${PN}
	update-alternatives --remove shutdown shutdown.${PN}
}

