#!/usr/bin/perl


#
# determine the equivalent parts, taking into account that %eq is transitive
#

sub eq
{
    my %seen;
    my @p = @_;	# parts to consider
    my @r = ();	# new equivalences we've found
    my $skip = @p;

    while (@p) {
	my $p = shift @p;
	next if $seen{$p};
	$seen{$p} = 1;
	push(@r, $p) if $skip-- <= 0;
	push(@p, @{ $eq{$p} });
    }
    return @r;
}


return 1;
