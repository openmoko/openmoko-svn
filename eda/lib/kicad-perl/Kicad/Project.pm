#
#    Kicad Project
#    Copyright (C) 2009 Alvaro Lopes <alvieboy@alvie.com>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#

package Kicad::Project;

use Kicad::Library;

use strict;
use Carp;
use warnings;

my @paths = qw,/usr/share/kicad/library /usr/local/share/kicad/library,;

sub new
{
    my $self = { 'libs' => [] };
    bless($self,shift);
    $self->parse($_[0]) if defined ($_[0]);
    return $self;
}

sub add_library
{
    my ($self,$lib) = @_;
    my $libinstance = new Kicad::Library;
    $libinstance->parse($lib);
    push(@{$self->{'libs'}},$libinstance);
}

sub find_component_in_libraries
{
    my ($self,$cname) = @_;
    for my $lib (@{$self->{'libs'}}) {
        my $c = $lib->find_component($cname);
        return $c if defined $c;
    }
    return undef;
}

sub parse
{
    my ($self,$file) = @_;
    my $fh;

    if (ref($file) ne 'GLOB') {
        open($fh,'<',$file) or croak "Cannot open $file: $!";
    } else {
        $fh = $file;
    }

    # scan for [eeschema/libraries]
    while (my $line=<$fh>) {
        chomp $line;
        if ($line=~/^\[eeschema\/libraries\]/) {
            $self->parse_libraries($fh);
        }
    }
}

sub load_lib
{
    my ($self,$libname) = @_;
    $libname =~ s/\.lib$//;
    for my $path(@paths,'.','..') {
        my $name = "${path}/${libname}.lib";
        if (-f $name) {
            $self->add_library($name);
            return;
        }
    }
    croak "Cannot load library $libname";
}

sub parse_libraries
{
    my ($self,$fh) = @_;
    while (my $line=<$fh>) {
        chomp $line;
        if ($line=~/^LibName\d+=(.*)$/) {
            $self->load_lib($1);
            next;
        } 
        last;
    }
}

1;
