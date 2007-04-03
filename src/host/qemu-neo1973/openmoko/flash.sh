#! /bin/bash
# Generates a ready to use OpenMoko NAND flash image.  Vaguely based
# on devirginator and http://wiki.openmoko.org/wiki/NAND_bad_blocks.
#
# Copyright (C) 2007 OpenMoko, Inc.
# Written by Andrzej Zaborowski <andrew@openedhand.com>
# All Rights Reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#

. openmoko/env

cd $script_dir

if ! which pngtopnm || ! which ppmtorgb3; then
	echo Splash needs \'pngtopnm\' and \'ppmtorgb3\' - make \
		sure that they\'re in \$PATH.
	exit -1
fi

${make} splash.gz || exit -1

# Find the most recent OpenMoko images in the current directory.
# We assume they have numeric date or some build number in their names.
most_recent () {
	export $2="`ls -1 $1 | sort | tail -n 1`"
	export $3="`python -c \"print hex(\`stat -c %s ${!2}\`)\"`"
	[ -n "${!3}" ]
}

most_recent "$kernel_wildcard" kernel_image kernel_size || exit -1
most_recent "$rootfs_wildcard" rootfs_image rootfs_size || exit -1
most_recent "$uboot_wildcard" uboot_image uboot_size || exit -1

echo Using \'$kernel_image\' as the kernel image.
echo Using \'$rootfs_image\' as the root filesystem image.
echo Using \'$uboot_image\' as bootloader.

# Currently we just make the u-boot image accessible under u-boot.bin
# and qemu will load it from the working directory.  This is a
# temporary solution.
rm -rf $uboot_symlink
ln -s $script_dir_relative/$uboot_image $uboot_symlink

rm -rf $flash_image
${make} $flash_image || exit -1

# Launch the emulator assuming that u-boot is now functional enough
# for us to be able to issue u-boot commands.
# This is also an example of how you *shouldn't* write scripts.
emu () {
	$qemu -mtdblock "$script_dir/$flash_image" -kernel "$script_dir/$1" \
		-serial stdio -nographic -usb -monitor null <&0 & pid=$!
}
uboot () {
	cd $src_dir
	emu $1 <<< "$3"
	echo Please wait, programming the NAND flash...
	sleep $2
	kill $pid # Ugly, use the qemu monitor instead
	sleep 1
	kill -9 $pid
	sleep 1
	cd $script_dir
}

# Set up BBT, u-boot environment, boot menu and program u-boot binary.
uboot $uboot_image 20 "

setenv dontask y
nand createbbt
setenv bootcmd 'setenv bootargs \${bootargs_base} \${mtdparts}; bootm $kernel_addr'
setenv menu_1 'Set console to USB: setenv stdin usbtty; setenv stdout usbtty; setenv stderr usbtty'
setenv menu_2 'Set console to serial: setenv stdin serial; setenv stdout serial; setenv stderr serial'
setenv menu_3 'Power off: neo1973 power-off'
setenv splashimage 'nand read.e $splash_addr splash $splash_size; unzip $splash_addr 0x33d00000 0x96000'
dynpart
nand write.e $kernel_addr u-boot $uboot_size
dynenv set u-boot_env
saveenv"

# Program bootsplash.
uboot splash.gz 10 "

nand write.e $kernel_addr splash $splash_size"

# Program the kernel binary.
uboot $kernel_image 10 "

nand write.e $kernel_addr kernel $kernel_size"

# Program the root filesystem.
uboot $rootfs_image 15 "

nand write.jffs2 $kernel_addr rootfs $rootfs_size"

echo
echo "    "All done.
echo
echo "    "Read the qemu manual and use a commandline like the following to boot:
echo \ \$ $qemu_relative -mtdblock $script_dir_relative/$flash_image -kernel $script_dir_relative/$kernel_image -usb -show-cursor
echo
echo "    "Append \'-snapshot\' to make the flash image read-only so that every
echo "    "time emulation starts in the original unmodified state.
echo "    "[Space] for AUX button, [Enter] for POWER.
echo
