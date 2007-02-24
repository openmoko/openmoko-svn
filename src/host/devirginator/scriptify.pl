#!/usr/bin/perl
#
# scriptify.pl - Convert a text file to a script image for u-boot's "autoscr"
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

use Archive::Zip;

while (<>) {
    s/#.*//;
    next if /^\s*$/;
    $cmd .= $_;
}

$cmd .= pack("c",0);
$cmd = pack("NN",length $cmd,0).$cmd;

$crc = Archive::Zip::computeCRC32($cmd,0);

$hdr = pack("NNNNNNNcccca32",
  0x27051956,	# ih_magic (IH_MAGIC)
  0,		# ih_crc
  time, 	# ih_time
  length $cmd,	# ih_size
  0,		# ih_load
  0,		# ih_ep 
  $crc,		# ih_dcrc
  17,		# ih_os (IH_OS_U_BOOT)
  2,		# ih_arch (IH_CPU_ARM)
  6,		# ih_type (IH_TYPE_SCRIPT)
  0,		# ih_comp (IH_COMP_NONE)
  "script");	# ih_name

$crc = Archive::Zip::computeCRC32($hdr,0);

substr($hdr,4,4) = pack("N",$crc);

print $hdr.$cmd;
