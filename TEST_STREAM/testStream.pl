#!/usr/bin/perl 

use strict;
use Ivy;
use warnings;
use Device::SerialPort;
use Modern::Perl '2014';

no warnings 'experimental::smartmatch';

sub fletcher16 ($$);
sub statusFunc ($$$$$$$);
sub congestionCallback ($$$);
sub tachyMessageCb ($);
sub initSerial ($);
sub serialCb();
sub getSerial();
sub serialWatchdog();

# for each application gives the number of running instances
my %connected_applications;

# for each couple appli:host gives the number of application running on host
my %where_applications;


$| = 1;

my $appliname = "rpm2ivy";
my $bus ;

my $serialName = $ARGV[0] // "/dev/ttyACMx";
my $serial;


$serialName = getSerial () if $serialName eq "/dev/ttyACMx";

say "DBG> serialName = $serialName";

do {
    $serial = initSerial ($serialName);
    sleep (1) unless $serial;
} unless defined $serial;


Ivy->init (-ivyBus => (defined $bus) ? $bus : undef,
	   -appName => $appliname,
	   -loopMode => 'LOCAL',
	   -messWhenReady => "$appliname READY",
    );




my $Ivyobj = Ivy->new(-statusFunc => \&statusFunc,
		      -slowAgentFunc=> \&congestionCallback,
		      -blockOnSlowAgent => 0,
    );


$Ivyobj->start;



$Ivyobj->fileEvent(*FHD, \&serialCb);
$Ivyobj->repeat (1000, \&serialWatchdog);
$Ivyobj->mainLoop();



# this function has 3 additionnal parameters till Ivy Version 4.6
# and now getting the new/dying applications is straightforward.
# The first 3 parameters are kept only for upward compatibility! 
sub statusFunc ($$$$$$$)  {
    my ($ref_ready, $ref_nonReady, $ref_hashReady, $appname, $status, $host, $regexp) = @_;

    if ($status eq "new") {
	print  "$appname connected from $host\n";
	$where_applications{"$appname:$host"}++;
    }
    elsif ($status eq "died") {
	print  "$appname disconnected from $host\n";
	$where_applications{"$appname:$host"}--;
    }
    elsif ($status eq 'subscribing') {
	print  "$appname subscribed to '$regexp'\n";
    }
    elsif ($status eq 'unsubscribing') {
	print  "$appname unsubscribed to '$regexp'\n";
    }
    elsif ($status eq 'filtered') {
	print  "$appname subscribed to *FILTERED* '$regexp'\n";
    }
    else {
	print  "Bug: unkown status; $status in &statusFunc\n";
    }

    %connected_applications = %$ref_hashReady;
}


sub congestionCallback ($$$)
{
  my ($name, $addr, $state) = @_;

  printf ("\033[1m $name [$addr] %s\033[m\n", $state ? "CONGESTION" : "OK");
}


sub initSerial ($)
{
    my $dev = shift;
 
    my $port = tie (*FHD, 'Device::SerialPort', $dev);
    unless ($port) {
	warn "Can't tie: $! .. still trying\n"; 
	return undef;
    }

#port configuration 230400/8/N/1
    $port->databits(8);
    $port->baudrate(230400);
    $port->parity("none");
    $port->stopbits(1);
    $port->handshake("none");
    $port->buffers(1, 1); #1 byte or it buffers everything forever
    $port->write_settings           || undef $port; #set
    unless ($port)                  { die "couldn't write_settings"; }

    return $port;
}

use constant  WAIT_FOR_SYNC => 0;
use constant  WAIT_FOR_LEN => 1;
use constant  WAIT_FOR_PAYLOAD => 2;
use constant  WAIT_FOR_CHECKSUM => 3;

sub serialCb()
{
    state $state = WAIT_FOR_SYNC;
    state @sync;
    state $buffer;
    state $len;
    state $crcBuf;
    state @crc;
    state $calculatedCrc;
    state $receivedCrc;
    my $totLen;

    @sync = (0,0) unless @sync;
    for ($state) {
	when  (WAIT_FOR_SYNC) {
#	    say ("notsync");
	    $sync[0] = $sync[1];
	    sysread (FHD, $buffer, 1);
	    $sync[1] = unpack ('C', $buffer);
	    if (($sync[0] == 0xFE) && ($sync[1] == 0xED)) {
		$state = WAIT_FOR_LEN;
	    } 
	}

	when  (WAIT_FOR_LEN) {
	    sysread (FHD, $buffer, 1);
	    ($len) = unpack ('C', $buffer);
	    $state = WAIT_FOR_PAYLOAD;  
	}
	
	when  (WAIT_FOR_PAYLOAD) {
	    say ("len is $len");
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $buffer, $len-$totLen, $totLen);
	    } while ($totLen != $len);
	    
	    $calculatedCrc = fletcher16 (\$buffer, undef);
	    $state = WAIT_FOR_CHECKSUM;
	}
	
	when  (WAIT_FOR_CHECKSUM) {
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $crcBuf, 2-$totLen, $totLen);
	    } while ($totLen != 2);
	    
	    @crc = unpack ('CC', $crcBuf);
	    $receivedCrc =  ($crc[1] << 8)  | $crc[0];
	    if ($calculatedCrc == $receivedCrc) {
		tachyMessageCb (\$buffer);
	    } else {
		printf ("CRC DIFFER C:0x%x != R:0x%x (len=%d)\n", 
			$calculatedCrc, $receivedCrc, $len);
	    }
	    $state = WAIT_FOR_SYNC;
	    $buffer=undef;
	}
    }
}


sub fletcher16 ($$) 
{
    my ($bufferRef, $msgIdRef) = @_;
    my $sum1 = 0; # 8 bits
    my $sum2 = 0; # 8 bits
    my $index;
    
    my @buffer = unpack ('C*', $$bufferRef);
    my $count = scalar(@buffer);
    
    $sum1 = ($sum1 + $count) % 0xff;
    $sum2 = $sum1;

    for($index = 0; $index < $count; $index++)  {
#	say "B[$index]=$buffer[$index]";
	$sum1 = ($sum1 + $buffer[$index]) % 0xff;
	$sum2 = ($sum2 + $sum1) % 0xff;
    }
    
    $$msgIdRef=$buffer[0] if defined $msgIdRef; # msgId
    return (($sum2 << 8) | $sum1);
}

 use Time::HiRes qw( clock_gettime clock_getres clock_nanosleep
                             ITIMER_REAL ITIMER_VIRTUAL ITIMER_PROF ITIMER_REALPROF );
my @messagesId = ('rpm', 'error');


# TACHY
# payload : ID(1) DYNSIZE(1) RPM(2) x DYNSIZE
#
#
sub tachyMessageCb ($)
{
    my ($bufferRef) = @_;
 
    my ($id) = unpack('C', $$bufferRef);
    substr($$bufferRef, 0, 1, '');
    
    if ($id == 0) {
	my ($nb) = unpack('C', $$bufferRef);
	substr($$bufferRef, 0, 1, '');
	my $format =  'C' x $nb;
    	my @fields = unpack($format, $$bufferRef);
    	printf ("ERRs = %s\n", join (':', @fields));
    } elsif ($id == 1) {
	my ($nb) = unpack('C', $$bufferRef);
	substr($$bufferRef, 0, 1, '');
    	my $format =  'S' x $nb;
    	my @fields = unpack($format, $$bufferRef);
	printf ("RPMs = %s\n", join (':', @fields));
    }
}



sub simpleMsgSend ($)
{
    my $bufferRef = shift;
    my $len = length $$bufferRef;
    my $crc = fletcher16 ($bufferRef, undef);
    my @sync = (0xFE, 0xED);

    my $msgHeader = pack ('C3', @sync, $len);
    my $msgTrailer = pack ('S', $crc);
    unless (syswrite (FHD, $msgHeader)) {
	say "serial port write error";
	exit;
    }
    syswrite (FHD, $$bufferRef);
    syswrite (FHD, $msgTrailer);
}

sub serialWatchdog ()
{
    unless (syswrite (FHD, ' ')) {
	say "watchdog : serial port write error";
	exit;
    }
}

sub getSerial()
{
    opendir (my $dhd, "/dev") || die "cannot opendir /dev\n";
    my @acm;

    while (my $fn = readdir ($dhd)) {
	push (@acm, $fn) if $fn =~ m|^stm_acm_\d+|;
    }
    closedir ($dhd);

    die "no ACM device\n" unless scalar @acm;
    return '/dev/' . (reverse sort (@acm))[0];
}
