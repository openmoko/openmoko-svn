DESCRIPTION = "GSM libraries and daemons implementing the 07.10 specification"
HOMEPAGE = ""
LICENSE = "GPL"
SECTION = "libs/gsm"
PROVIDES = "libgsmd libgsmd-tools gsmd"
PV = "0.0+svn${SRCDATE}"

SRC_URI = "svn://svn.gta01.hmw-consulting.de/trunk/src/target;module=gsm;proto=http"
S = "${WORKDIR}/gsm"

inherit autotools

PACKAGES =+ "libgsmd-tools gsmd"
FILES_libgsmd-tools = "${bindir}/*tool*"
FILES_gsmd = "${bindir}/gsmd"

