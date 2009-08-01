use strict;
use Data::Dumper;

use Kicad::Library;
use Kicad::Netlist;

my $sch = new Kicad::Netlist;

defined $ARGV[0] or do { print "Usage: $0 netlistfile\n"; exit 0; };

$sch->parse($ARGV[0]);

my %libs;
my %uses;

my $components = $sch->get_components();

while( my ($cname,$comp) = each %{$components}) {
    $libs{ $comp->{'library'} }->{$comp->{'value'}}||=0;
    $libs{ $comp->{'library'} }->{$comp->{'value'}}++;

    $uses{ $comp->{'library'} }||=0;
    $uses{ $comp->{'library'} }+= 1;
}

while (my ($lib,$vals) = each %libs) {
    print "Component $lib ($uses{$lib} components)\n";
    foreach my $v( keys %$vals) {
        printf " [%03d] %s\n", $vals->{$v},$v;
    }
    print "\n";
}
