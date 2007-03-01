#!/usr/bin/perl
#
# openocdcmd.pl - Send commands to OpenOCD through the telnet interface
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


sub usage
{
    print STDERR "usage: $0 host port command ...\n";
    exit(1);
}


&usage unless $#ARGV >= 2;

$host = shift @ARGV;
$port = shift @ARGV;

open(PIPE,"| telnet $host $port") || die "telnet: $!";

# make the pipe unbuffered

$old = select PIPE;
$| = 1;
select $old;

while (defined $ARGV[0]) {
    print PIPE "$ARGV[0]\n" || die "telnet: $!";
    shift @ARGV;
}

# wait until telnet closes the pipe

vec($ein, fileno(PIPE), 1) = 1;
$n = select($ein, undef, $ein, undef);
if ($n < 0) {
    print "select: $!";
    exit(1);
}

close PIPE || die "telnet: $!";
