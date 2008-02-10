#!/usr/bin/perl
#
# blink.pl - Binary Linker
#
# Copyright (C) 2008 by OpenMoko, Inc.
# Written by Werner Almesberger <werner@openmoko.org>
# All Rights Reserved
#

#
# usage: blink.pl item ...
#
# Build an image containing the items specified on the command line and print
# the resulting data to stdout. Any gaps in the data are filled with zero
# bytes.
#
# "item" is
#
# object[@[=]address]
#
# - if no address is specified, the new object the places after the previous
#   one
# - the address can be decimal, hex, or octal, following the usual C
#   conventions
# - if the address is lower than the current position, it must be prefixed with
#   an equal sign
#
# "object" is one of
#
# filename
# "string"
# value[/bytes]
#
# - the default value size is four bytes
# - note that the double quotes of a string must be protected from the shell
#
# E.g.,
# blink.pl foo.bin /tmp/bar.bin@0x1000 0x1000/4@=0x10 '"hello"'@0x2000
#


$addr = 0;


sub put
{
    $data .= "\000" x ($addr-length $data) if $addr > length $data;
    substr($data, $addr, length $_[0], $_[0]);
    $addr += length $_[0];
}


for (@ARGV) {
    if (/@(=?)(0[Xx][0-9a-fA-F]+|[0-9]+)$/) {
	$obj = $`;
	$new = eval $2;
	die "new address $2 before current address"
	  if $new < $addr && $1 ne "=";
	$addr = $new;
    }
    else {
	$obj = $_;
    }
    if ($obj =~ m#^(0[xX][0-9a-fA-F]+|0-9+)(/[124])?$#) {
	$val = pack(!defined $2 || $2 == 4 ? "V" : $2 == 2 ? "v" : "c",
	  eval $1);
	&put($val);
    }
    elsif ($obj =~ /^"(.*)"$/) {
	($in, $out) = ($1, "");
	while ($in =~ /\\([0-7]{1,3}|[^0-7])/) {
	    $out .= $`;
	    if (index("01234567", substr($1, 0, 1)) == -1) {
		$n = index("bnrt", $1);
		if ($n == -1) {
		    $out .= $1;
		}
		else {
		    $out .= substr("\b\n\r\t", $n, 1);
		}
	    }
	    else {
		$out .= pack("c", oct $1);
	    }
	    $in = $';
	}
	&put($out.$in."\000");
    }
    else {
	open(FILE, $obj) || die "$obj: $!";
	defined read(FILE, $data, -s FILE, $addr) || die "$obj: $!";
	close FILE;
	$addr += -s FILE;
    }
}

die "print: $!" unless print $data;
