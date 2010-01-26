#!/usr/bin/perl

sub skip
{
    # do nothing
}


#
# "bom" populates the following global variable:
#
# $cmp{component-reference}[0] = value
# $cmp{component-reference}[1] = footprint
# $cmp{component-reference}[2] = field1
# ...
#

sub bom
{
    if (/^#End Cmp/) {
	$mode = *skip;
	return;
    }
    die unless /^\|\s+(\S+)\s+/;
    my $ref = $1;
    my @f = split(/\s*;\s*/, $');
    next if $f[0] eq "NC";
    $cmp{$ref} = [ @f ];
}


#
# "equ" populates the following global variables:
#
# $id{item-number} = "namespace item-number"
#   This is used for heuristics that look up parts commonly referred to by
#   their part number.
#
# $eq{"namespace0 item-number0"}[] = ("namespace1 item-number1", ...)
#   List of all parts a given part is equivalent to.
#

sub equ
{
    my @f = split(/\s+/);
    my $a = "$f[0] $f[1]";
    my $b = "$f[2] $f[3]";
    $id{$f[1]} = $a;
    $id{$f[3]} = $b;
    push @{ $eq{$a} }, $b;
    push @{ $eq{$b} }, $a;
}


#
# "inv" populates the following global variables:
#
# $id{item-number} = "namespace item-number"
#   This is used for heuristics that look up parts commonly referred to by
#   their part number.
#
# $inv{"namespace item-number"}[0] = items-in-stock
# $inv{"namespace item-number"}[1] = currency
# $inv{"namespace item-number"}[2] = order-quantity
# $inv{"namespace item-number"}[3] = unit-price
#   [2] and [3] may repeat.
#

sub inv
{
    my @f = split(/\s+/);
    my $id = "$f[0] $f[1]";
    shift @f;
    my $ref = shift @f;
    die "duplicate inventory entry for \"$id\"" if defined $inv{$id};
    $id{$ref} = $id;
    $inv{$id} = [ @f ];
}


#
# "par" populates the following global variables:
#
# $parts{component-ref}[0] = namespace
# $parts{component-ref}[1] = item-number
# [0] and [1] may repeat
#
# $want{"namespace item"} = number of times we may use the part. If multiple
#   parts are eligible for a component, each of them is counted as desirable
#   for each component.
#
# $comps{"namespace item"}{component-ref} = 1
#   Set of components a part may be used for.
#

sub par
{
    my @f = split(/\s+/);
    my $ref = shift @f;
    $parts{$ref} = [ @f ];
    while (@f) {
	my @id = splice(@f, 0, 2);
	my $id = "$id[0] $id[1]";
	$want{$id}++;
	$comps{$id}{$ref} = 1;
    }
}


sub parse
{
    $mode = *skip;
    while (<>) {
	chop;
	if (/^#Cmp/) {
	    $mode = *bom;
	    next;
	}
	if (/^#EQU\b/) {
	    $mode = *equ;
	    next;
	}
	if (/^#INV\b/) {
	    $mode = *inv;
	    next;
	}
	if (/^#PAR\b/) {
	    $mode = *par;
	    next;
	}
	s/#.*//;
	next if /^\s*$/;
	&$mode($_);
    }
}

return 1;
