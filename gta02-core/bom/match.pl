#!/usr/bin/perl

use re 'eval';


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


#
# We convert each input pattern into two regular expressions: the first matches
# units in the nXn notation, e.g., 4u7 or 100R. The second matches them in SI
# notation (sans space).
#
# When matching (sub_match), we first apply the first expression. Each time we
# encounter a unit ($R, $F, etc.), __cvn is called. __cvn stores the index of
# the unit in %cvn_from and %cvn_to.
#
# We then pick these substrings from the input string and convert the units to
# SI notation. At the same time, we normalize the mantissa. Once done, we run
# the second expression. This one always matches (hopefully :-)
#
# All (...) ranges in the original pattern have been replaced with named
# capture buffers in the second expression, so all these subfields are now
# gathered in the $+ array. (The same also happened in the first pass, but we
# ignore it.)
#
# Finally, when expanding a value (sub_expand), we look for $field and
# $field:index, and expand accordingly.
#


sub __cvn
{
    local ($num) = @_;

    $cvn_from{$num} = $-[$#-];
    $cvn_to{$num} = $+[$#+];
}


sub sub_match
{
    local ($s, $field, $m1, $m2) = @_;

    #
    # Perform the first match and record where we saw $<unit> patterns.
    #
    undef %cvn_from;
    undef %cvn_to;
    return undef unless $s =~ $m1;

    #
    # Convert the unit patterns to almost-SI notation. (We don't put a space
    # after the number, but the rest is SI-compliant.)
    #
    my $off = 0;
    for (keys %cvn_from) {
	my $unit = $cvn_unit{$_};
	my $from = $cvn_from{$_}+$off;
	my $len = $cvn_to{$_}-$cvn_from{$_};
	die unless substr($s, $from, $len) =~
	    /(\d+)$unit(\d*)|(\d+)([GMkmunpf])(\d*)/;

	#
	# Normalize to \d+.\d*
	#
	my $v = "$1$3.$2$5";
	my $exp = $4 eq "" ? " " : $4;

	#
	# Mantissa must be < 1000.
	# Do the math as string operation to avoid rounding errors.
	#
	while ($v =~ /(\d+)(\d{3})\./) {
	    $v = "$1.$2$'";
	    $exp =~ tr/GMk munpf/TGMk munp/;
	}

	#
	# Mantissa must be >= 1.
	#
	while ($v =~ /\b0\.(\d+)/) {
	    if (length $1 < 3) {
		$v = $1.("0" x (3-length $1)).".";
	    } else {
		$v = substr($1, 0, 3).".".substr($1, 3);
	    }
	    $exp =~ tr/GMk munpf/Mk munpa/;
	}
	$exp =~ s/ //;
	$v =~ s/\.$//;
	$v = $v.$exp.$unit;
	$off += length($v)-$len;
	substr($s, $from, $len, $v);
    }

    #
    # Run the second match on the string to process any (...) patterns
    #
    $found{$field} = $s;
    die $m2 unless $s =~ $m2;
    for (keys %+) {
	$found{$_} = $+{$_};
    }
    return $s;
}


sub sub_expand
{
    local ($s) = @_;

    while ($s =~ /^([^\$]*)\$([A-Za-z_]\w*)(:(\d+))?|^([^\$]*)\${([A-Za-z_]\w*)(:(\d+))?}/) {
	my $name = "$2$6";
	$name .= "__$4$8" if defined($4) || defined($8);
	die "don't know \"$name\"" unless defined $found{$name};
	$s = $1.$5.$found{$name}.$';
    }
    return $s;
}


#
# return 0 if all rules have been exhausted, 1 if there was an explicit halt.
#

sub apply_rules
{
    RULE: for (my $i = 0; $i <= $#match; $i++) {
	print STDERR "RULE #$i\n" if $debug;
	%found = %field;
	for (keys %{ $match[$i] }) {
	    print STDERR "  MATCH $_=$match[$i]{$_}[0] " if $debug;
	    if (!defined $found{$_}) {
		print STDERR "NO FIELD\n" if $debug;
		next RULE;
	    }
	    print STDERR "FIELD $found{$_} " if $debug;
	    if (!defined &sub_match($found{$_}, $_,
	      $match[$i]{$_}[1], $match[$i]{$_}[2])) {
		print STDERR "MISS\n" if $debug;
		next RULE;
	    }
	    print STDERR "MATCH\n" if $debug;
	}
	for (keys %{ $action[$i] }) {
	    my $s = &sub_expand($action[$i]{$_});
	    print STDERR "  SET $_=$action[$i]{$_} => $s\n" if $debug;
	    $field{$_} = $s;
	}
	if ($end[$i]) {
	    print STDERR "  END\n" if $debug;
	    return 1;
	}
    }
    return 0;
}


return 1;
