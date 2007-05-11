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

most_recent () {
	${echo} > .list
	for name in $files; do
		if [[ "$name" == $1 ]]; then
			${echo} "$name" > .list
		fi
	done
	export $2=`sort -n .list | tail -n 1`
	rm -rf .list
	[ -z "${!2}" ] && ( ${echo} not found; exit -1 )
	${echo} ${!2}
}

${echo} -n "    "Kernel is...\ 
most_recent "$kernel_wildcard" kernel_image || exit -1
${echo} -n "    "Root filesystem is...\ 
most_recent "$rootfs_wildcard" rootfs_image || exit -1
${echo} -n "    "U-boot is...\ 
most_recent "$uboot_wildcard" uboot_image || exit -1

sleep 2

download () {
	[ -s "$1" ] && return
	rm -rf "$1"
	wget "$download_dir/$1"
}

download "$kernel_image" || exit -1
download "$rootfs_image" || exit -1
download "$uboot_image" || exit -1

echo
echo "    "Now use openmoko/flash.sh to install OpenMoko to NAND Flash.
echo
