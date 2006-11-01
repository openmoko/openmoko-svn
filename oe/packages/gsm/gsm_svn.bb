DESCRIPTION = "GSM libraries and daemons implementing the 07.10 specification"
HOMEPAGE = ""
LICENSE = "GPL"
SECTION = "libs/gsm"
PV = "0.0+svn${SRCDATE}"

SRC_URI = "svn://svn.gta01.hmw-consulting.de/trunk/src/target;module=gsm;proto=http"
S = "${WORKDIR}/gsm"

inherit autotools

PACKAGES =+ "${PN}-tools ${PN}-daemon"
FILES_${PN}-tools = "${bindir}/*tool*"
RPROVIDES_${PN}-tools = "gsm-tools"
FILES_${PN}-daemon = "${bindir}/gsmd"
RPROVIDES_${PN}-daemon = "gsm-daemon"

