DESCRIPTION = "JTAG utility to interface w/ a S3C2410 device"
SECTION = "devel"
AUTHOR = "Harald Welte"
LICENSE = "GPL"
PV = "0.1+svn${SRCDATE}"
PR = "r0"

SRC_URI = "svn://svn.openmoko.org/trunk/src/host/;module=sjf2410-linux;proto=https"
S = "${WORKDIR}/sjf2410-linux"

inherit native

CFLAGS += "-DLINUX_PPDEV"

do_compile() {
	oe_runmake
}

do_deploy() {
        install -d ${DEPLOY_DIR_IMAGE}
        install -m 0755 sjf2410 ${DEPLOY_DIR_IMAGE}/sjf2410
}

do_stage() {
	:
}

do_install() {
	:
}

addtask deploy before do_build after do_compile
