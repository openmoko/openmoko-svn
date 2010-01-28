#!/usr/bin/perl
require "parser.pl";
&parse;

for $k (sort keys %chr) {
    for $p (sort keys %{ $chr{$k} }) {
	print "chr{$k}{$p} = $chr{$k}{$p}\n";
    }
}
for ($i = 0; $i != @end; $i++) {
    for (sort keys %{ $match[$i] }) {
	print "$_=$match[$i]{$_} ";
    }
    print "->";
    for (sort keys %{ $action[$i] }) {
	print " $_=$action[$i]{$_}";
    }
    print $end[$i] ? " !\n" : "\n";
}
