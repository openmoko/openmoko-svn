DESCRIPTION = "OpenMoko Sound System"
SECTION = "openmoko/base"
RDEPENDS = "\
  libpulse-bin \
  libpulse-module-alsa-sink \
  libpulse-module-alsa-source \
  libpulse-module-cli \
  libpulse-module-esound-protocol-unix \
  libpulse-module-simple-protocol-tcp \
  libpulse-module-native-protocol-unix \
  libpulse-module-cli-protocol-unix \
"
PR = "r0"

inherit openmoko-base

SRC_URI = "file://session"
S = "${WORKDIR}"

do_install() {
    install -d ${D}/${sysconfdir}/pulseaudio
	install -m 0755 ${WORKDIR}/session ${D}/${sysconfdir}/pulseaudio/session
}

PACKAGE_ARCH = "all"
