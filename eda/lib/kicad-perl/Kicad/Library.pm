#
#    Kicad Library Parser
#    Copyright (C) 2009 Alvaro Lopes <alvieboy@alvie.com>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#

package Kicad::Library;

use strict;
use Carp;
use warnings;
use Data::Dumper;

sub new
{
    my $self = {};
    bless($self,shift);
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

sub parse_block
{
    my ($self,$fh,$line) = @_;

    my $st = substr($line,1,1);
    
    ( $st eq 'C' && $self->parse_part_descr($fh,$line) ) ||
        ( $st eq 'S' && $self->parse_sheet_descr($fh,$line) ) ||
        ( $st eq 'D' && $self->parse_schema_descr($fh,$line) ) || die;
    
}

sub parse_draw
{
    my ($self,$fh,$comp) = @_;
    
    while (my $line=<$fh>) {
        chomp $line;
        next if $line=~/^#/;
        my @key=qw/name num posx posy length direction name_text_size num_text_size unit convert electrical_type pin_type/;
        if ($line=~/^X\s+(.*)/) {
            my @val = split(/\s+/,$1);
            my $i=0;
            map { $comp->{'pins'}->{$val[1]}->{$_} = $val[$i++] } (@key);
        }
        if ($line=~/^ENDDRAW/) {
            last;
        }
    }    
}

sub parse_def
{
    my ($self,$fh,$line,$name,$sym,@v) = @_;
    
    my $comp ={};
    
    while (my $line=<$fh>) {
        chomp $line;
        next if $line=~/^#/;
        if ($line=~/^ENDDEF/) {
            last;
        }
        if ($line=~/^(F\d)\s+(.*)$/) {
            # Reference and name
            $comp->{$1}=$2;
            next;
        }
        if ($line=~/^ALIAS\s+(.*)/) {
            $comp->{'alias'} = $1;
            next;
        }
        if ($line=~/^DRAW/) {
            $self->parse_draw($fh,$comp);
        }
        
        
    }
    $self->{'components'}->{$name} = $comp;
    
    # Add alias
    if (defined ($comp->{'alias'})) {
        foreach (split(/\s+/,$comp->{'alias'})) {
            $self->{'components'}->{$_} = $comp;
        }
    }
}

sub find_component
{
    my ($self,$name) = @_;
    #    print Dumper($self);
    #print join(' ', keys(%{$self->{'components'}}));
    return $self->{'components'}->{$name}
}

sub parse_body
{
    my ($self,$fh) = @_;
    
    while (my $line=<$fh>) {
        chomp $line;
        next if $line=~/^#/;
        if ($line=~/^DEF\s+([\S]+)\s+([\S]+)\s+(\d+)\s+(.*)$/)
        {
            $self->parse_def($fh,$line,$1,$2);
        } else {
            croak "Invalid line $line";
        }
    }
}

sub parse_header
{
    my ($self,$fh) = @_;
    while (<$fh>) {
        next if /^#/;
        return 1 if (/^EESchema-LIBRARY/);
    }
    return undef;
}

1;
