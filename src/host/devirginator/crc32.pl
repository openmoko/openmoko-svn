#!/usr/bin/perl
#
# crc32.pl - Straightforward and very slow CRC32 implementation
#
# Copyright (C) 2006-2007 by OpenMoko, Inc.
# Written by Werner Almesberger <werner@openmoko.org>
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

#
# For details, see
# http://en.wikipedia.org/w/index.php?title=Cyclic_redundancy_check&oldid=110823112
#

sub crc32
{
    local ($s) = @_;
    local ($poly, $crc) = (0xedb88320, ~0);
    local ($i, $j);

    for ($i = 0; $i != length $s; $i++) {
	for ($j = 0; $j != 8; $j++) {
	    if (($crc ^ (unpack("C", substr($s, $i, 1)) >> $j)) & 1) {
		$crc = ($crc >> 1) ^ $poly;
	    }
	    else {
		$crc = $crc >> 1;
	    }
	}
    }
    return ~$crc;
}
