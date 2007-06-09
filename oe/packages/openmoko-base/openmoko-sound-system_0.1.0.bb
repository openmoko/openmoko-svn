DESCRIPTION = "OpenMoko Sound System"
SECTION = "openmoko/base"
RDEPENDS = "\
  pulseaudio-server \
  pulseaudio-module-alsa-sink \
  pulseaudio-module-alsa-source \
  pulseaudio-module-cli \
  pulseaudio-module-esound-protocol-unix \
  pulseaudio-module-simple-protocol-tcp \
  pulseaudio-module-native-protocol-unix \
  pulseaudio-module-cli-protocol-unix \
"
PR = "r1"

inherit openmoko-base

SRC_URI = "file://session"
S = "${WORKDIR}"

do_install() {
    install -d ${D}/${sysconfdir}/pulseaudio
	install -m 0755 ${WORKDIR}/session ${D}/${sysconfdir}/pulseaudio/session
}

PACKAGE_ARCH = "all"
