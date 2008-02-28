#!/usr/bin/perl
#
# envedit.pl - U-Boot environment editor
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

$ENV_SIZE = 0x4000;


sub usage
{
    print STDERR
"usage: $0 [-I dir] [-s size] [-c] [-i file] [-o file|-p] [-f env_file]\n".
"                  [-D var[=value]] [var=[value] ...]\n".
"  -c             ignore CRC errors in input environment\n".
"  -f env_file    read changes from env_file\n".
"  -i file        read environment from file (default: use empty ".
  "environment)\n".
"  -o file        write environment to file (default: write to stdout)\n".
"  -p             print environment in human-readable form to stdout\n".
"  -s bytes       environment size in bytes (default: 16384)\n".
"  -D var[=value] define a variable for env_file preprocessing only\n".
"  -I dir         add directory to INC path (to find crc32.pl)\n".
"  var=           remove the specified variable\n".
"  var=value      set the specified variable\n".
"The options -I, -c, and -s, if present, must precede all other options.\n";
"The options -D, if present, must precede -f.\n";
    exit(1);
}



sub do_crc32
{
    if (!defined $have_crc) {
	do 'crc32.pl';
	$have_crc = 1;
    }
    return &crc32(@_);
}


sub readenv
{
    local ($file) = @_;
    local ($crc, $env, $want);

    open(FILE, $file) || die "$file: $!";
    $env = join("", <FILE>);
    close FILE;
    if (length $env < 6) {	# CRC plus \0\0
	print STDERR sprintf("Environment is too small (%d bytes < 6)\n",
	  length $env);
	exit(1);
    }
    if (length $env != $ENV_SIZE) {
	print STDERR sprintf("warning: environment is %d bytes, expected %d\n",
	  length $env, $ENV_SIZE);
	$env = substr($env, 0, $ENV_SIZE);
	$env .= '\000' x ($ENV_SIZE-length $env);
    }
    ($crc, $env) = unpack("Va*", $env);
    $want = &do_crc32($env);
    if ($crc != $want) {
	print STDERR sprintf("CRC error: expected 0x%08x, got 0x%08x\n",
	  $want, $crc);
	exit(1) unless $warn_crc;
    }
    foreach (split(/\000/, $env)) {
	last if $_ eq "";
	if (/^([^=]+)=/) {
	    if ($' eq "") {
		print STDERR "warning: skipping empty entry for \"$_\"\n";
	    }
	    else {
		$env{$1} = $';
	    }
	}
	else {
	    print STDERR
	      "warning: ignoring invalid environment entry \"$_\"\n";
	}
    }
}


sub set
{
    return 0 unless $_[0] =~ /^([^=]+)=\s*(.+)?$/;

    local ($var, $value) = ($1, $2);

    if ($value ne "") {
	$env{$var} = $value;
    }
    else {
	undef $env{$var};
    }
    print STDERR
      "warning: variable name \"$var\" contains invalid characters\n"
      unless $var =~ /^\w+$/;
    return 1;
}


sub set_err
{
    if (!&set($_[0])) {
	print STDERR "invalid setting \"$_[0]\"\n";
	exit(1);
    }
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
    if ($ARGV[0] eq "-i") {
	&usage unless defined $ARGV[1];
	&readenv($ARGV[1]);
	shift(@ARGV);
	shift(@ARGV);
    }
    elsif ($ARGV[0] eq "-o") {
	&usage if defined $printenv;
	&usage unless defined $ARGV[1];
	shift(@ARGV);
	$outfile = shift(@ARGV);
    }
    elsif ($ARGV[0] eq "-p") {
	&usage if defined $outfile;
	$printenv = 1;
	shift(@ARGV);
    }
    elsif ($ARGV[0] eq "-s") {
	&usage if $have_crc;
	&usage unless defined $ARGV[1];
	shift(@ARGV);
	$ENV_SIZE = eval shift(@ARGV);
    }
    elsif ($ARGV[0] eq "-c") {
	&usage if $have_crc;
	shift(@ARGV);
	$warn_crc = 1;
    }
    elsif ($ARGV[0] eq "-I") {
	&usage if $have_crc;
	&usage unless defined $ARGV[1];
	shift(@ARGV);
	push(@INC, shift @ARGV);
    }
    elsif ($ARGV[0] eq "-D") {
	&usage unless defined $ARGV[1];
	shift(@ARGV);
	if ($ARGV[0] =~ /=/) {
	    $def{$`} = $';
	}
	else {
	    $def{$ARGV[0]} = 1;
	}
	shift @ARGV;
    }
    elsif ($ARGV[0] eq "-f") {
	&usage unless defined $ARGV[1];
	shift(@ARGV);
	$file = shift(@ARGV);
	undef $line;
	open(FILE, $file) || die "$file: $!";
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
		&set_err($line) if defined $line;
		$line = $_;
	    }
	}
	close FILE;
	&set_err($line) if defined $line;
    }
    elsif (&set($ARGV[0])) {
	shift(@ARGV);
    }
    else {
	&usage;
    }
}


if ($printenv) {
    foreach (sort { $a cmp $b } keys %env) {
	print "$_=$env{$_}\n" if defined $env{$_};
    }
}
else {
    foreach (sort { $a cmp $b } keys %env) {
	$env .= "$_=$env{$_}\0" if defined $env{$_};
    }
    if (length $env > $ENV_SIZE-5) {
	# leave room for CRC and terminating \000
	print STDERR sprintf("environment too big (%d > %d)\n",
	  length $env, $ENV_SIZE-5);
	exit(1);
    }
    $env .= "\000" x ($ENV_SIZE-4-length $env);
    $crc = &do_crc32($env);
    $env = pack("V", $crc).$env;
    if (defined $outfile) {
	open(FILE, ">$outfile") || die "$outfile: $!";
	print FILE $env || die "$outfile: $!";
	close(FILE) || die "$outfile: $!";
    }
    else {
	print $env || die "print: $!";
    }
}
