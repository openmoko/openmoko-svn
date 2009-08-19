#!/usr/bin/perl

$Vol_max = 0.4;
$Voh_min = 2.4;
$Voh_max = 3.3;

$Rs = 82e3;
$Rp = 1000e3;

$Ii = 1e-6;


sub v
{
    local ($v, $rs, $rp, $i) = @_;

    return ($v-$rs*$i)/(1+$rs/$rp);
}

print "Vil(max) = ", &v($Vol_max, $Rs*0.95, $Rp*1.05, -$Ii), " V\n";
print "Vih(min) = ", &v($Voh_min, $Rs*1.05, $Rp*0.95, $Ii), " V\n";
print "Vih(max) = ", &v($Voh_max, $Rs*0.95, $Rp*1.05, $Ii), " V\n";
print "Ileak(max) = ", $Voh_max/($Rs*0.95+$Rp*0.95)*1e6, " uA\n";
