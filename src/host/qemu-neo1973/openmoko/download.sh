#! /usr/bin/env bash
# Chooses and downloads some OpenMoko image snapshots for flash.sh to use.
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

echo "    "Retrieving available builds list...
files=`lynx -dump $download_dir | grep http | sed "s,[0-9 \t\.]*$download_dir\([a-zA-Z0-9_\.-]\)[ \t]*,\1,"`
dev_files=`lynx -dump $dev_download_dir | grep http | sed "s,[0-9 \t\.]*$dev_download_dir\([a-zA-Z0-9_\.-]\)[ \t]*,\1,"`

if [[ "${files}" == "" ]]; then
	echo "    "Trying alternative sources
	download_dir="$backup_download_dir"
	dev_download_dir="$backup_download_dir"
	files=`lynx -dump $download_dir | grep http | sed "s,[0-9 \t\.]*$download_dir\([a-zA-Z0-9_\.-]\)[ \t]*,\1,"`
	dev_files="$files"
fi

most_recent () {
	${echo} > .list
	f=$3files; for name in ${!f}; do
		if [[ "$name" == $1 ]]; then
			${echo} "$name" >> .list
		fi
	done
	export $2=`sort -n .list | tail -n 1`
	rm -rf .list
	[ -z "${!2}" ] && ${echo} not found && exit -1
	${echo} ${!2}
}

${echo} -n "    "Kernel is...\ 
most_recent "$kernel_wildcard" kernel_image "" || exit -1
${echo} -n "    "Root filesystem is...\ 
most_recent "$rootfs_wildcard" rootfs_image "" || exit -1
${echo} -n "    "U-boot is...\ 
most_recent "$uboot_wildcard" uboot_image dev_ || exit -1

sleep 2

download () {
	[ -s "$1" ] && return
	rm -rf "$1"
	dir=$2download_dir; wget "${!dir}$1"
}

download "$kernel_image" "" || exit -1
download "$rootfs_image" "" || exit -1
download "$uboot_image" dev_ || exit -1

# Get this computer's aproximated geographic latitude/longitude for
# QEMU GPS emulation start position, taking advantage of a working
# internet connection.  If this fails or isn't accurate, we don't
# really care, QEMU will then use some defaults.
#
# If the "position" file contains no position data or wrong data,
# please consider contributing to the hostip.info database by
# correcting the information at http://www.hostip.info/correct.html
echo Retrieving position
lynx -dump http://api.hostip.info/rough.php?position=true > position

echo
echo "    "Now use openmoko/flash.sh to install OpenMoko to NAND Flash.
echo
