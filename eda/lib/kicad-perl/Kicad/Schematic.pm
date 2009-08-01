#
#    Kicad Schematic Parser
#    Copyright (C) 2009 Alvaro Lopes <alvieboy@alvie.com>
#
#  This library is free software; you can redistribute it and/or
#  modify it under the same terms as Perl itself.
#

package Kicad::Schematic;

use strict;

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

sub parse_connection_descr
{
    my ($self,$fh,$line) = @_;
    
    my @v = split(/\s+/,$line);
    print STDERR "Connection $v[1] at $v[2],$v[3]\n";
}

sub parse_schema_descr
{
    my ($self,$fh,$line) = @_;
    while(<$fh>) {
        chomp;
        return 1 if /\$EndDescr/;
    }
    die "Invalid descr";
    return undef;
}
sub parse_body
{
    my ($self,$fh) = @_;
    
    while (my $line =<$fh>) {
        chomp $line;
        my $st = substr($line,0,1);
        print STDERR "'$st' ($line)\n";
        ( $st eq '$' && $self->parse_block($fh,$line) ) ||
            ( $st eq 'T' && $self->parse_text_descr($fh,$line) ) ||
            ( $st eq 'L' && $self->parse_part_descr($fh,$line) ) ||
            ( $st eq 'W' && $self->parse_segment_descr($fh,$line) ) ||
            ( $st eq 'E' && $self->parse_record_descr($fh,$line) ) ||
            ( $st eq 'P' && $self->parse_polyline_descr($fh,$line) ) ||
            ( $st eq 'C' && $self->parse_connection_descr($fh,$line) ) ||
            ( $st eq 'C' && $self->parse_noconnection_descr($fh,$line) || die "Invalid start line $st\n");

    }
}

sub parse_header
{
    my ($self,$fh) = @_;
    while (<$fh>) {
        print;

        return 1 if (/^EELAYER\sEND/);
    }
    return undef;
}

1;
