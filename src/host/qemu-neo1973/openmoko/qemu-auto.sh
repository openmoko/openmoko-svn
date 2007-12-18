#! /usr/bin/env bash
# Basically just runs qemu, in a way that qemu-cmd.pl works.
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

if [ -e "$qemu_monitor" ]; then
	echo A QEMU session appears to be already active
	exit -1
fi

trap "rm -rf \"$qemu_monitor\" \"$dump_dir\"; stty sane" INT EXIT

$qemu -mtdblock "$script_dir/$flash_image"		\
	-kernel "$script_dir/openmoko-kernel.bin"	\
	-snapshot -usb -show-cursor -parallel none	\
	-usbdevice keyboard -usbgadget -serial stdio	\
	-monitor unix:"$qemu_monitor",server,nowait	\
	"$@"
