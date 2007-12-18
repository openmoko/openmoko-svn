#! /usr/bin/env bash
# Passes a file to the system in the emulator.
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

if [ ! -e "$qemu_monitor" ]; then
	echo No applicable QEMU session found
	exit -1
fi

if [[ "$1" == "" ]]; then
	qemu_cmd eject
else
	rm -rf "$dump_dir"; mkdir -p "$dump_dir"
	cp -fR "$@" "$dump_dir"	# TODO: Use symlinks perhaps
	qemu_cmd change sd0 "fat:$dump_dir"
fi
