#!/usr/bin/perl
#
# envcpp.pl - U-Boot environment editor
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

#
# Preprocessing operations:
#
# CPP-like conditionals:
#
# #ifdef VAR
# #ifndef VAR
# #else
# #endif
#
# Macro expansion:
#
# #define MACRO
# #define MACRO TEXT ...
# MACRO
# MACRO##TOKEN
#
# Note that #ifdef/#ifndef use both existing environment variables (i.e., those
# present in the environment at the time of processing) and  preprocessing
# variables (i.e., those set with -D), while macro expansion only uses
# preprocessing variables.
#


sub usage
{
    print STDERR
"usage: $0 [-D var[=value]] [env_file]\n".
"  -D var[=value] define a variable for env_file preprocessing only\n";
    exit(1);
}


sub expand
{
    local ($s, @key) = @_;
    local ($tmp, $pre, $exp, $post);
    local ($i, @tmp);

    $tmp = "";
    while (length $s) {
	$pre = $s;
	$exp = "";
	$post = "";
	for ($i = 0; $i <= $#key; $i++) {
	    @tmp = @key;
	    splice(@tmp, $i, 1);
	    if ($s =~ /(##)?\b$key[$i]\b(##)?/) {
		if (length $` < length $pre) {
		    $pre = $`;
		    $post = $';
		    $exp = &expand($def{$key[$i]}, @tmp);
		}
	    }
	}
	$tmp .= $pre.$exp;
	$s = $post;
    }
    return $tmp;
}


if ($0 =~ m#/[^/]*$#) {
    push(@INC, $`);
}

while (@ARGV) {
    if ($ARGV[0] =~ /^-D/) {
	shift @ARGV;
	if ($' ne "") {
		$def = $';
	}
	else {
		&usage unless defined $ARGV[1];
		$def = shift @ARGV;
	}
	if ($def =~ /=/) {
	    $def{$`} = $';
	}
	else {
	    $def{$def} = 1;
	}
    }
    elsif ($ARGV[0] =~ /^-.+/) {
	&usage;
    }
    else {
	&usage if defined $ARGV[1];
	last;
    }
}

$file = shift(@ARGV);
undef $line;
if (defined $file) {
    open(FILE, $file) || die "$file: $!";
}
else {
    *FILE = *STDIN;
}
while (<FILE>) {
    chop;
    if (/^\s*#if(n?)def\s+(\S+)/) {
	if (!$false &&
	  (defined $env{$2} || defined $def{$2}) == ($1 ne "n")) {
	    $true++;
	}
	else {
	    $false++;
	}
    }
    elsif (/^\s*#else\b/) {
	if (!$false && !$true) {
	    print STDERR "$file:$.: #else without #if...\n";
	    exit(1);
	}
	if (!$false) {
	    $true--;
	    $false++;
	}
	elsif ($false == 1) {
	    $false--;
	    $true++;
	}
    }
    elsif (/^\s*#endif\b/) {
	if (!$false && !$true) {
	    print STDERR "$file:$.: #endif without #if...\n";
	    exit(1);
	}
	if ($false) {
	    $false--;
	}
	else {
	    $true--;
	}
    }
    next if $false;

    if (/^\s*#define\s+(\S+)(\s*(.*?))?\s*$/) {
	if (defined $def{$1} && $def{$1} ne $3) {
	    print STDERR "$file:$.: redefinition of macro \"$1\"\n";
	    exit(1);
	}
	$def{$1} = $3;
    }

    $_ = &expand($_, keys %def);

    s/#.*//;
    s/\s*$//;
    next if /^\s*$/;
    if (/^\s+/) {
	if (!defined $line) {
	    print STDERR "first line cannot be a continuation\n";
	    exit(1);
	}
	$line .= " ".$';
    }
    else {
	print "$line\n" || die $! if defined $line;
	$line = $_;
    }
}
close FILE || die $!;
print "$line\n" || die $! if defined $line;
