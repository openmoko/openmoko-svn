#!/usr/bin/perl

sub rows
{
    local $s = $_[0];
    my @res = ();

    while ($s =~ m#.*?<tr>(.*?)</tr>#) {
	push(@res, $1);
	$s = $';
    }
    return @res;
}


sub cols
{
    local $s = $_[0];
    my @res = ();

    while ($s =~ m#.*?<td[^>]*>(.*?)</td>#) {
	push(@res, $1);
	$s = $';
    }
    return @res;
}


sub usage
{
    print STDERR "usage: $0 (query [-i cache_file] | dsc | inv) [file ...]\n";
    exit(1);
}


$mode = shift @ARGV;
&usage unless $mode eq "query" || $mode eq "dsc" || $mode eq "inv";

if ($mode eq "query") {
    if ($ARGV[0] eq "-i") {
	shift @ARGV;
	$name = shift @ARGV;
	open(OLD, $name) || die "$name: $!";
	$q = join("", <OLD>);
	($old = $q) =~ tr/\r\n//d;
	close OLD;
    }

    while (<>) {
	chop;
	s/#.*//;
	next if /^\s*$/;
	next if /^\s/;
	s/\s.*//;
	next if $old =~ m#align=right>Digi-Key Part Number</th><td>$_</td#;
	push(@pn, $_);
    }

    if (0+@pn) {
	$cmd = "wget -nv -O - ".join(" ",
	  map
	  "http://search.digikey.com/scripts/DkSearch/dksus.dll?Detail\\&name=$_",
	  @pn);
	$q .= `$cmd`;
    }

    print $q;
    exit;
}


$q = join("", <>);
$q =~ tr/\r\n//d;

print "#DSC\n" if $mode eq "dsc";
print "#INV\n" if $mode eq "inv";
print "# MACHINE-GENERATED. DO NOT EDIT !\n";
print "# ", `date -u`;

for (split(/<!DOCTYPE HTML/, $q)) {
    next unless m#align=right>Digi-Key Part Number</th><td>([^<]+)</td#;
    $pn = $1;
    $qty = 0;
    if (m#align=right>Quantity Available</th><td[^>]*>([0-9,]+)<#) {
	($qty = $1) =~ tr/,//d;
    }
    next unless m#align=right>Description</th><td>(.*?)</td#;
    $dsc = $1;
    next unless m#<table.*<th>Price Break<(.*?)</table>#;
    if ($mode eq "dsc") {
	print "DIGI-KEY $pn $dsc\n";
	next;
    }
    print "DIGI-KEY $pn $qty USD";
    for (&rows($1)) {
	@c = &cols($_);
	next unless $c[0] =~ /^[0-9,]+$/;
	next unless $c[1] =~ /^[0-9.]+$/;
	$c[0] =~ tr/,//d;
	$c[1] =~ tr/,//d;	# let's hope we don't need this one often :)
	$c[1] =~ s/0+$// if $c[1] =~ /\./;
	print " $c[0] $c[1]";
    }
    print "\n";
}
