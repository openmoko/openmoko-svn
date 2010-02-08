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


#
# When looking for a description, we also consider equivalent parts.
#
# Furthermore, some descriptions may just be pointers to other descriptions.
# Users can add regular expressions that are used to extract references from
# a description, which are then looked up as well.
#

sub __dsc_lookup
{
    local ($id) = @_;

    for ($id, &eq($id)) {
        return $dsc{$_} if defined $dsc{$_};
    }
    return undef;
}


sub dsc_find
{
    my $id = $_[0];
    LOOKUP: while (1) {
	my $dsc = &__dsc_lookup($id);
	return undef unless defined $dsc;
	for (my $i = 0; $i <= $#xlat_from; $i++) {
# @@@ this is UUUUHHHGLLEEEEE !!! Why can't I just expand $to[$i] ?
	    next
	      unless ($id = $dsc) =~ s/^.*$xlat_from[$i].*$/$xlat_to[$i] $1/;
	    next LOOKUP if defined &__dsc_lookup($id);
	}
	return $dsc;
    }
    return undef;
}


sub dsc_xlat
{
    local ($from, $to) = @_;
    push(@xlat_from, $from);
    push(@xlat_to, $to);
}


sub dsc_xlat_arg
{
    return undef unless $_[0] =~ /^(.)([^\1]*)\1([^\1]*)\1$/;
    &dsc_xlat($2, $3);
    return 1;
}


return 1;
