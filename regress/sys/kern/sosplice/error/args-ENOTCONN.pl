# test ENOTCONN for splicing from unconnected socket

use strict;
use warnings;
use IO::Socket;
use BSD::Socket::Splice "SO_SPLICE";

our %args = (
    errno => 'ENOTCONN',
    func => sub {
	my $s = IO::Socket::INET->new(
	    Proto => "tcp",
	) or die "socket failed: $!";

	my $ss = IO::Socket::INET->new(
	    Proto => "tcp",
	) or die "socket splice failed: $!";

	$s->setsockopt(SOL_SOCKET, SO_SPLICE, pack('i', $ss->fileno()))
	    and die "splice from unconnected socket succeeded";
    },
);
