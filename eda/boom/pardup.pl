#!/usr/bin/perl
while (<>) {
    @f = split(/\s+/);
    $ref = shift @f;
    for ($i = 0; $i != @f; $i++) {
	next unless $f[$i] eq "FIC" || $f[$i] eq "MISSING" ||
	  $f[$i] eq "DIGI-KEY";
	splice(@f, $i, 2);
	$i--;
    }
    next if @f < 3;
    push(@{ $multi{join(" ", @f)} }, $ref);
}
for (sort keys %multi) {
    print "$_ -- ", join(" ", @{ $multi{$_} }), "\n";
}
