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


#
# "chr" populates the following global variable:
#
# $chr{"namespace item-number"}{parameter} = value
#
# $last is used internally for continuation lines.
#

sub chr
{
    if (/^\s+/) {
	my @f = split(/\s+/, $');
    } else {
	my @f = split(/\s+/);
	my $ref = shift @f;
	my $num = shift @f;
	$last = "$ref $num";
    }
    for (@f) {
	die unless /=/;
	$chr{$last}{uc($1)} = $2;
    }
}


#
# "sub" populates the following global variables:
#
# $end[rule-number] = 0 / 1
# $match[rule-number]{field} = pattern
# $action[rule-number]{field} = value
#
# $match_stack[depth]{field} = pattern
# $action_stack[depth]{field} = value
# $may_cont = 0 / 1
# $last
#
# to do:
# - test this
# - unit canonicalization
# - glob to RE rewriting for pattern
# - $n expansion for value
#

sub sub
{
    /^\s*/;
    my $indent = $&;
    my @f = split(/\s+/, $');
    my $in = 0;		# indentation level
    while (/^./ =~ $indent) {
	if ($& eq " ") {
	    $in++;
	} elsif ($& eq "\t") {
	    $in = ($in+8) & ~7;
	} else {
	    die;
	}
    }
    if ($may_cont && $in > $last) {
	pop(@match);
	pop(@action);
    } else {
	$match_stack[0] = undef;
	$action_stack[0] = undef;
    }
    $last = $in;
    while (@f) {
	my $f = shift @f;
	last if $f eq "->" || $f eq "{" || $f eq "}" || $f eq "!";
	if ($f =~ /=/) {
	    $match_stack[0]{"REF"} = $f;
	} else {
	    $match_stack[0]{uc($`)} = $';
	}
    }
    if ($f eq "->") {
	while (@f) {
	    my $f = shift @f;
	    last if $f eq "{" || $f eq "!";
	}
	die unless /=/;
	$action_stack[0]{uc($`)} = $';
    }
    $may_cont = 0;
    if ($f eq "{") {
	unshift(@match_stack, undef);
	unshift(@action_stack, undef);
    } elsif ($f eq "}") {
	shift @match_stack;
	shift @action_stack;
    } else {
	push(@end, $f eq "!");
	$may_cont = $f ne "!";
	my $n = $#end;
	for $m (@match_stack) {
	    for (keys %{ $_ }) {
		$match[$n]{$_} = $m{$_};
	    }
	}
	for $a (@action_stack) {
	    for (keys %{ $_ }) {
		$action[$n]{$_} = $m{$_};
	    }
	}
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
	if (/^#CHR\b/) {
	    $mode = *chr;
	    undef $last;
	    next;
	}
	if (/^#SUB\b/) {
	    $mode = *sub;
	    undef $last;
	    undef $may_cont;
	    next;
	}
	s/#.*//;
	next if /^\s*$/;
	&$mode($_);
    }
}

return 1;
