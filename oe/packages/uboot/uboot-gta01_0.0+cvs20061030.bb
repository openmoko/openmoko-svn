DESCRIPTION = "U-boot bootloader w/ gta01 support"
SECTION = "bootloader"
PRIORITY = "optional"
LICENSE = "GPL"
SRCDATE := "${PV}"
PR = "r3"

PROVIDES = "virtual/bootloader"
S = "${WORKDIR}/git"

SRC_URI = "git://www.denx.de/git/u-boot.git/;protocol=git \
	   file://u-boot-20061030-qt2410-gta01.patch;patch=1 \
	   file://u-boot-20061030-gta01v4.patch;patch=1 \
	   file://u-boot-20061030-gta01bv2.patch;patch=1 \
	   file://u-boot-20061030-ext2load_hex.patch;patch=1"
# file://gta01_*.h"

EXTRA_OEMAKE = "CROSS_COMPILE=${TARGET_PREFIX}"
TARGET_LDFLAGS = ""
UBOOT_MACHINES = "gta01v3 gta01v4 gta01bv2"

inherit base

do_compile () {
	for type in nand 
	do
		for machine in ${UBOOT_MACHINES}
		do
			#install -m 0644 ${WORKDIR}/${machine}_${type}.h include/configs/gta01.h
			chmod +x board/gta01/split_by_variant.sh
			oe_runmake ${machine}_config
			oe_runmake all
			mv u-boot.bin u-boot_${machine}_${type}.bin
		done
	done
}

do_deploy () {
	install -d ${DEPLOY_DIR_IMAGE}
	for type in nand
	do
		for machine in ${UBOOT_MACHINES}
		do
			install ${S}/u-boot_${machine}_${type}.bin ${DEPLOY_DIR_IMAGE}/u-boot_${type}-${machine}-${DATETIME}.bin
		done
	done
	install -m 0755 tools/mkimage ${STAGING_BINDIR}/uboot-mkimage
}

do_deploy[dirs] = "${S}"
addtask deploy before do_build after do_compile
