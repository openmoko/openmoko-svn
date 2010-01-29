#!/usr/bin/perl

use re 'eval';


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
    my @f;
    if (/^\s+/) {
	@f = split(/\s+/, $');
    } else {
	@f = split(/\s+/);
	my $ref = shift @f;
	my $num = shift @f;
	$last = "$ref $num";
    }
    for (@f) {
	die unless /=/;
	$chr{$last}{uc($`)} = $';
    }
}


#
# "sub" populates the following global variables:
#
# $end[rule-number] = 0 / 1
# $match[rule-number]{field}[0] = original-pattern
# $match[rule-number]{field}[1] = RE1 
# $match[rule-number]{field}[2] = RE2
# $action[rule-number]{field} = value
#
# $match_stack[depth]{field}[0] = original-pattern
# $match_stack[depth]{field}[1] = RE1
# $match_stack[depth]{field}[2] = RE2
# $action_stack[depth]{field} = value
# $may_cont = 0 / 1
# $last
# $last_action
#

#
# $cvn_from{internal-handle} = index
# $cvn_to{internal-handle} = index
# $cvn_unit{internal-handle} = unit-name
# $cvn_num = internal-handle
# $found{field-or-subfield} = string


sub sub_pattern
{
    local ($field, $p) = @_;
    my $n = 0;
    $p =~ s/\./\./g;
    $p =~ s/\+/\\+/g;
    $p =~ s/\?/./g;
    $p =~ s/\*/.*/g;
    my $tmp = "";
    while ($p =~ /^([^\(]*)\(/) {
	$n++;
	$tmp .= "$1(?'${field}__$n'";
	$p = $';
    }
    $p = "^".$tmp.$p."\$";
    my $q = $p;
    while ($p =~ /^([^\$]*)\$(.)/) {
	$p = "$1(\\d+$2\\d*|\\d+[GMkmunpf$2]\\d*)(?{ &__cvn($cvn_num); })$'";
	$cvn_unit{$cvn_num} = $2;
	die unless $q =~ /^([^\$]*)\$(.)/;
	$q = "$1(\\d+(\.\\d+)?[GMkmunpf]?$2)$'";
	$cvn_num++;
    }
    return ($p, $q);
}


sub sub_value
{
    return $_[0];
}


sub sub
{
    /^(\s*)/;
    my $indent = $1;
    my @f = split(/\s+/, $');
    my $f;
    my $in = 0;		# indentation level
    while (length $indent) {
	my $c = substr($indent, 0, 1, "");
	if ($c eq " ") {
	    $in++;
	} elsif ($c eq "\t") {
	    $in = ($in+8) & ~7;
	} else {
	    die;
	}
    }
    if ($may_cont && $in > $last) {
	pop(@match);
	pop(@action);
	pop(@end);
    } else {
	$match_stack[0] = undef;
	$action_stack[0] = undef;
	$last_action = 0;
	$last = $in;
    }
    if (!$last_action) {
	while (@f) {
	    $f = shift @f;
	    last if $f eq "->" || $f eq "{" || $f eq "}" || $f eq "!";
	    if ($f =~ /=/) {
		$match_stack[0]{uc($`)} = [ $', &sub_pattern(uc($`), $') ];
	    } else {
		$match_stack[0]{"REF"} = [ &sub_pattern("REF", $f) ];
	    }
	}
	$last_action = 1 if $f eq "->";
    }
    if ($last_action) {
	while (@f) {
	    $f = shift @f;
	    last if $f eq "{" || $f eq "!";
	    die unless $f =~ /=/;
	    $action_stack[0]{uc($`)} = &sub_value($');
	}
    }
    $may_cont = 0;
    if ($f eq "{") {
	unshift(@match_stack, undef);
	unshift(@action_stack, undef);
	die "items following {" if @f;
    } elsif ($f eq "}") {
	shift @match_stack;
	shift @action_stack;
	die "items following }" if @f;
    } else {
	die "items following !" if @f && $f eq "!";
	push(@end, $f eq "!");
	$may_cont = $f ne "!";
	my $n = $#end;
	push(@match, undef);
	push(@action, undef);
	for my $m (reverse @match_stack) {
	    for (keys %{ $m }) {
		$match[$n]{$_} = $m->{$_};
	    }
	}
	for my $a (reverse @action_stack) {
	    for (keys %{ $a }) {
		$action[$n]{$_} = $a->{$_};
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
	if (/^#(SUB|GEN)\b/) {
	    $mode = *sub;
	    undef $last;
	    undef $last_action;
	    undef $may_cont;
	    next;
	}
	s/#.*//;
	next if /^\s*$/;
	&$mode($_);
    }
}

return 1;
