#! /usr/bin/perl -w
use strict;
use Socket;
if ($#ARGV > 0) {
	socket(SOCK, PF_UNIX, SOCK_STREAM, 0)	|| die "socket: $!";
	connect(SOCK, sockaddr_un("$ARGV[0]"))	|| die "connect: $!";
	print SOCK "$ARGV[1]\n";
	close SOCK;
}
