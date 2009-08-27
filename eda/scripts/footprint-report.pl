#!/usr/bin/perl

use strict;
use warnings;
use Carp;

my %footprints;

my ($filename,$reqsheet) = @ARGV;

sub usage
{
    print STDERR "Usage: $0 bom.lst [SHEETNAME]\n";
    exit;
}

usage() unless defined $filename;

open(FILE,$filename)
    or croak "Cannot open $filename: $!";

while(<FILE>) {
    last if /order = Value/;
    next unless /^\|/;
    chomp;
    my ($ref, $component, $sheet, $location, $footprint)=
        m/\|\s+(\S+)\s+([^\(]+)\(Sheet\s\/([^\/]+)\/\)\s+\(([^\)]+)\);\s*(.*)$/;
    croak "Invalid line '$_'" unless defined $ref;
    next unless defined $sheet;
    # Filter if needed
    if (defined $reqsheet) {
        next unless $reqsheet eq $sheet;
    }
    $footprint=~s/\s+$//;
    $component=~s/\s+$//;
    $footprint = "* UNKNOWN FOOTPRINT *" if $footprint eq '';
    
    $footprints{$footprint}||=[];
    
    push(@{$footprints{$footprint}}, { 'ref'=>$ref,'comp'=>$component});
}

print "*** FOOTPRINT REPORT ***\n";
print "**> Sheet $reqsheet only <**\n" if defined $reqsheet;
print "\n\n";
foreach my $foot (sort keys %footprints) {
    print "'${foot}' used by:\n";
    foreach my $inst(@{$footprints{$foot}}) {
        print "  $inst->{'ref'} [ $inst->{'comp'} ]\n";
    }
    print "\n";
}
