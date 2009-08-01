#!/usr/bin/perl
use strict;
use Carp;
use Kicad::Netlist;
use Kicad::Project;

(defined $ARGV[0] && defined $ARGV[1]) or 
    do { print "Usage: $0 projectfile netlistfile\n"; exit 0; };

sub padprint
{
    my ($size,$str) = @_;
    my $diff = $size - length($str);
    return $diff>0 ? $str.' 'x$diff : substr($str,0,$size);
}

my $prj = new Kicad::Project($ARGV[0]);
my $net = new Kicad::Netlist($ARGV[1]);
my $components = $net->get_components();

print "Unconnected pin report\n\n";
print "COMPONENT TYPE                PIN   NAME\n";
print "--------- ------------------- ----- ------------------------\n";

foreach my $comp (keys %{$components}) {
    my $type = $components->{$comp}->{'library'};
    my $cref = $prj->find_component_in_libraries($type);
    croak "Component $comp not found in libraries (of type $type)" unless defined $cref;
    while ( my ($pin_number,$pin_name) = each %{$components->{$comp}->{'pins'}}) {
        $pin_name eq '?' && print padprint(10,$comp), padprint(20,$type), 
            padprint(6,$pin_number), $cref->{pins}->{$pin_number}->{name}, "\n";
    }
}
