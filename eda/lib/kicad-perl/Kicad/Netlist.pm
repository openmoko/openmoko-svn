#
#    Kicad Netlist Parser
#    Copyright (C) 2009 Alvaro Lopes <alvieboy@alvie.com>
#
#
#  This library is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#

package Kicad::Netlist;

use strict;
use Carp;
use warnings;

sub new
{
    my $self = {};
    bless($self,shift);
    $self->parse($_[0]) if defined ($_[0]);
    return $self;
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
    $self->parse_header($fh) or return undef;
    $self->parse_body($fh);
}

sub get_components
{
    my ($self) =@_;
    return $self->{'components'};
}

sub parse_component
{
    my ($self,$fh,$cdef) = @_;
    my @v = split(/\s+/,$cdef);
    my $comp = { 'pins' => {} };
    my $i=0;
    map { $comp->{$_} = $v[$i++] } (qw/id footprint name value library/);
    $comp->{'library'} =~ s/\{Lib=([^}]+)\}/$1/e;
    while (my $line=<$fh>) {
        chomp $line;
        next if $line=~/^#/;
        if ($line=~/^\s+\(\s+(\S+)\s+(\S+)\s+\)$/) {
            # Pin entry
            $comp->{'pins'}->{$1}=$2;
        }
        if ($line=~/^\s+\)/) {
            last;
        }
    }
    $self->{components}->{$comp->{name}}=$comp;
}

sub parse_footprints
{
    my ($self,$fh) = @_;
    while (my $line=<$fh>) {
        chomp $line;
        # Nothing to do yet
        if ($line=~/^\}/) {
            last;
        }
    }
}

sub parse_nets
{
    my ($self,$fh) = @_;
    
    my %nets;
    my $current_net;
    
    while (my $line=<$fh>) {
        chomp $line;
        #Net 2 "" ""
        # R1519 2
        # TP1511 1
        # U1501 R25

        if ($line=~/^Net\s+(\S*)\s+(\S+)\s+(\S+)$/) {
            croak "NET $1 already exists!!!" if exists $nets{$1};
            $current_net = $1;
            $nets{$current_net}->{'name_full'} = $2;
            $nets{$current_net}->{'name'} = $3;
            next;
        }
        if ($line=~/^\}/) {
            last;
        }
        croak "Net definition without net" unless defined $current_net;
        $line=~s/^\s+//;
        my ($component,$pin) = split(/\s/,$line);
        
        # Find component instance
        
        croak "Cannot find component instance $component" unless defined $self->find_component_instance($component);
        $nets{$current_net}->{'nodes'} ||= [];
        
        push(@{$nets{$current_net}->{'nodes'}}, {'comp' => $component, 'pin' => $pin});
    }
    $self->{'nets'} = {%nets};
}

sub get_nets
{
    my ($self) = @_;
    return $self->{'nets'};
}

sub find_component_instance
{
    my ($self,$name) = @_;
    return $self->{'components'}->{$name};
}

sub parse_netlist
{
    my ($self,$fh) = @_;
    while (my $line=<$fh>) {
        chomp $line;
        next if $line=~/^#/;
        if ($line=~/\s+\(\s+(.*)$/) {
            $self->parse_component($fh,$1);
            next;
        } 
        if ($line=~/^\*/) {
            $self->parse_footprints($fh);
            next;
        }
        if ($line=~/^\{/) {
            $self->parse_nets($fh);
        }
    }
}

sub parse_body
{
    my ($self,$fh) = @_;
    
    while (my $line=<$fh>) {
        chomp $line;
        next if $line=~/^#/;
        if ($line=~/^\(/)
        {
            $self->parse_netlist($fh);
        } else {
            croak "Invalid line $line";
        }
    }
}

sub parse_header
{
    my ($self,$fh) = @_;
    while (<$fh>) {
        return 1 if (/^# EESchema Netlist/);
    }
    croak "Invalid netlist";
    return undef;
}

1;
