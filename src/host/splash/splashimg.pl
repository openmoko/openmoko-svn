#!/usr/bin/perl
#
# splashimg.pl - Convert a 480x640 PNG to a splash screen raw dump 
#
# Copyright (C) 2006-2008 by OpenMoko, Inc.
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


$bpp = 16;
$swap_pix = 0;


sub usage
{
    print STDERR "usage: $0 [-32] [-pnm] [-swap] [System_boot.png]\n";
    exit(1);
}


while ($ARGV[0] =~ /^-[^-]/) {
    if ($ARGV[0] eq "-32") {
	$bpp = 32;
	shift @ARGV;
    }
    elsif ($ARGV[0] eq "-pnm") {
	$pnm = 1;
	shift @ARGV;
    }
    elsif ($ARGV[0] eq "-swap") {
	$swap_pix = 1;
	shift @ARGV;
    }
    else {
	die "unrecognized option: \"$ARGV[0]\"";
    }
}

&usage unless $#ARGV < 2;
if ($ARGV[0] eq "") {
    $file = "System_boot.png";
}
else {
    $file = @ARGV[0];
}

if ($pnm) {
    $cmd = "cat '$file' | ppmtorgb3";
}
else {
    $cmd = "pngtopnm '$file' | ppmtorgb3";
}

system($cmd) && die "system \"$cmd\": $?";

for ("red", "grn", "blu") {
    open(FILE, "noname.$_") || die "noname.$_";
    $f = join("", <FILE>);
    close FILE;
    unlink("noname.$_");
    $f =~ s/^P5\s+(\d+)\s+(\d+)\s+(\d+)\s//s;
    ($w, $h, $p) = ($1, $2, $3);
    $p{$_} = $f;
}

print STDERR "$w x $h ($p)\n";

for ($i = 0; $i != $w*$h; $i++) {
    $r = unpack("C", substr($p{"red"}, $i ^ $swap_pix, 1));
    $g = unpack("C", substr($p{"grn"}, $i ^ $swap_pix, 1));
    $b = unpack("C", substr($p{"blu"}, $i ^ $swap_pix, 1));
    if ($bpp == 16) {
	$v = ($r >> 3) << 11 | ($g >> 2) << 5 | ($b >> 3);
	print pack("v", $v) || die "print: $!";
    }
    else {
	$v = ($r) << 16 | ($g) << 8 | $b;
	print pack("V", $v) || die "print: $!";
    }
}
