DESCRIPTION = "ALSA state files"
SECTION = "base"
PR = "r0"

SRC_URI = "http://opensource.wolfsonmicro.com/~gg/neo1973/capturehandset.state \
           http://opensource.wolfsonmicro.com/~gg/neo1973/captureheadset.state \
           http://opensource.wolfsonmicro.com/~gg/neo1973/gsmbluetooth.state \
           http://opensource.wolfsonmicro.com/~gg/neo1973/gsmhandset.state \
           http://opensource.wolfsonmicro.com/~gg/neo1973/gsmheadset.state \
           http://opensource.wolfsonmicro.com/~gg/neo1973/stereoout.state"

FILES_${PN} += "${sysconfdir}/alsa/*"

do_install () {
	install -d ${D}${sysconfdir} \
		   ${D}${sysconfdir}/alsa
	install -m 0644 ${WORKDIR}/capturehandset.state ${D}${sysconfdir}/alsa/
	install -m 0644 ${WORKDIR}/captureheadset.state ${D}${sysconfdir}/alsa/
	install -m 0644 ${WORKDIR}/gsmbluetooth.state ${D}${sysconfdir}/alsa/
	install -m 0644 ${WORKDIR}/gsmhandset.state ${D}${sysconfdir}/alsa/
	install -m 0644 ${WORKDIR}/gsmheadset.state ${D}${sysconfdir}/alsa/
	install -m 0644 ${WORKDIR}/stereoout.state ${D}${sysconfdir}/alsa/
}
