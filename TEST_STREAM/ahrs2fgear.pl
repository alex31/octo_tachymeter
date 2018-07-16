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
sub initSerial ($);
sub serialCb();
sub flightGearMessageCb($$);
sub imuParamCb($$$$$$$$);
sub ahrsParamCb($$);
sub getSerial();
sub serialWatchdog();

# for each application gives the number of running instances
my %connected_applications;

# for each couple appli:host gives the number of application running on host
my %where_applications;


$| = 1;

my $appliname = "ahrs2ggear";
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


$Ivyobj->bindRegexp('^ImuParam sample=(\d+) gyroLpf=(\d+) gyroFsr=(\d+) accelLpf=(\d+) ' . 
		    'accelFsr=(\d+) magSampleRate=(\d+) ahrs=(\w)',
		    [\&imuParamCb]);
$Ivyobj->bindRegexp('^ImuParam ahrs=(\w)',
		    [\&ahrsParamCb]);
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

#port configuration 115200/8/N/1
    $port->databits(8);
    $port->baudrate(115200);
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
use constant  DEG_BY_RAD => 180/3.1415927;

use constant  FG_EULER => 1; 
use constant  FG_ACCEL => 2;
use constant  FG_GYRO => 3;
use constant  FG_MAG => 4;
use constant  FG_IMUPARAM => 5;
use constant  FG_STAT_ACCEL => 6;
use constant  FG_STAT_GYRO => 7;
use constant  FG_STAT_MAG => 8;
use constant  FG_AHRSPARAM => 9;
use constant  FG_CPU_COMP => 10;
use constant  FG_CPU_INV => 11;

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
    state $msgId;
    my $totLen;

    @sync = (0,0) unless @sync;
    for ($state) {
	when  (WAIT_FOR_SYNC) {
	    $sync[0] = $sync[1];
	    sysread (FHD, $buffer, 1);
	    $sync[1] = unpack ('C', $buffer);
	    if (($sync[0] == 0xED) && ($sync[1] == 0xFE)) {
		$state = WAIT_FOR_LEN;
	    } 
	}

	when  (WAIT_FOR_LEN) {
	    sysread (FHD, $buffer, 1);
	    ($len) = unpack ('C', $buffer);
	    $state = WAIT_FOR_PAYLOAD;  
	}
	
	when  (WAIT_FOR_PAYLOAD) {
#	    say ("len is $len");
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $buffer, $len-$totLen, $totLen);
	    } while ($totLen != $len);
	    
	    $receivedCrc = fletcher16 (\$buffer, \$msgId);
	    $state = WAIT_FOR_CHECKSUM;
	}
	
	when  (WAIT_FOR_CHECKSUM) {
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $crcBuf, 2-$totLen, $totLen);
	    } while ($totLen != 2);
	    
	    @crc = unpack ('CC', $crcBuf);
	    $calculatedCrc =  ($crc[1] << 8)  | $crc[0];
	    if ($calculatedCrc == $receivedCrc) {
		flightGearMessageCb ($msgId, \$buffer);
	    } else {
		printf ("CRC DIFFER id=$msgId C:0x%x != R:0x%x\n", $calculatedCrc, $receivedCrc);
	    }
	    $state = WAIT_FOR_SYNC;
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

sub flightGearMessageCb ($$)
{
    my ($msgId, $bufferRef) = @_;
    state $acc_x = 0.;
    state $acc_y = 0.;
    state $acc_z = 0.;
    state $gyro_p = 0.;
    state $gyro_q = 0.;
    state $gyro_r = 0.;
    state $mag_x = 0.;
    state $mag_y = 0.;
    state $mag_z = 0.;

    given ($msgId) {
	when (FG_EULER) {
	    # euler mesg;
	    my ($id, $phi, $tetha, $psi) = unpack ('Cf[3]', $$bufferRef);
	    
	    $phi *= -(DEG_BY_RAD);
	    $phi += 180;
	    $tetha *= (DEG_BY_RAD);
	    $psi *= DEG_BY_RAD;
	    ($phi, $tetha) = ($tetha, $phi);

	    # $phi *= (DEG_BY_RAD);
	    # $tetha *= (DEG_BY_RAD);
	    # $psi *= DEG_BY_RAD;


#	    printf ("EULER id=%d phi=%.3f, tetha=%.3f, psi=%.3f\n", $id, $phi, $tetha, $psi);
	    my $s = $Ivyobj->sendMsgs ("ImuMsg Accel_x=0. Accel_y=0. Accel_z=0. " .
				       "Roll=$phi Pitch=$tetha Yaw=$psi Heading=$psi");
	    if ($s == 0) {
		printf ('.');
#		warn "ImuMsg Accel_x=0 Accel_y=0 Accel_z=0 " .
#				       "Roll=$phi Pitch=$tetha Yaw=$psi Heading=$psi";
	    }

            # for now, send IMU data together with EULER angles
            $Ivyobj->sendMsgs ("ImuMsg Raw Ax=$acc_x Ay=$acc_y Az=$acc_z " .
                                "Gp=$gyro_p Gq=$gyro_q Gr=$gyro_r ".
                                "Mx=$mag_x My=$mag_y Mz=$mag_z");
	}

	when (FG_ACCEL) {
	    # accel mesg;
	    my $id;
	    ($id, $acc_x, $acc_y, $acc_z) = unpack ('Cf[3]', $$bufferRef);
	    
#	    printf ("ACCEL id=%d acc_x=%.3f, acc_y=%.3f, acc_z=%.3f\n", $id, $acc_x, $acc_y, $acc_z);

	}

	when (FG_GYRO) {
	    # gyro mesg;
	    my $id;
	    ($id, $gyro_p, $gyro_q, $gyro_r) = unpack ('Cf[3]', $$bufferRef);
	    
#	    printf ("GYRO id=%d gyro_p=%.3f, gyro_q=%.3f, gyro_r=%.3f\n", $id, $gyro_p, $gyro_q, $gyro_r);

	}

	when (FG_MAG) {
	    # mag mesg;
	    my $id;
	    ($id, $mag_x, $mag_y, $mag_z) = unpack ('Cf[3]', $$bufferRef);
	    
#	    printf ("MAG id=%d mag_x=%.3f, mag_y=%.3f, mag_z=%.3f\n", $id, $mag_x, $mag_y, $mag_z);

	}

	when ([FG_STAT_ACCEL .. FG_STAT_MAG]) {
	    # stat mesg;
	    my ($id, $stat_x, $stat_y, $stat_z) = unpack ('Cf[3]', $$bufferRef);
	    #say ("ImuMsg Stat Sensors=$id Stat_x=$stat_x Stat_y=$stat_y Stat_z=$stat_z");
	    $Ivyobj->sendMsgs ("ImuMsg Stat Sensors=$id Stat_x=$stat_x Stat_y=$stat_y Stat_z=$stat_z");
	}

	when ([FG_CPU_COMP .. FG_CPU_INV]) {
	    # cpu load mesg;
	    my ($id, $cpuLoad) = unpack ('CL[1]', $$bufferRef);
#	    say ("ImuMsg Cpu Load Ahrs=$id cpu=$cpuLoad");
	    $Ivyobj->sendMsgs ("ImuMsg Cpu Ahrs=$id Load=$cpuLoad");
	}

    }
}

sub imuParamCb($$$$$$$$)
{
    my ($app, $sampleRate, $gyroLpf, $gyroFsr, $accelLpf, $accelFsr, $magSampleRate, $ahrs) = @_;
    my $buffer = pack ('CS6C', FG_IMUPARAM, $sampleRate, $gyroLpf, $gyroFsr, $accelLpf, $accelFsr,
	$magSampleRate, ord uc $ahrs);

    simpleMsgSend (\$buffer);

    # say ("sampleRate = $sampleRate");
    # say ("gyroLpf = $gyroLpf");
    # say ("gyroFsr = $gyroFsr");
    # say ("accelLpf = $accelLpf");
    # say ("accelFsr = $accelFsr");
    # say ("magSampleRate = $magSampleRate");
    # say ("ahrs = $ahrs");
}

sub ahrsParamCb($$)
{
    my ($app, $ahrs) = @_;
    my $buffer = pack ('CC', FG_AHRSPARAM, ord uc $ahrs);

    simpleMsgSend (\$buffer);
}



sub simpleMsgSend ($)
{
    my $bufferRef = shift;
    my $len = length $$bufferRef;
    my $crc = fletcher16 ($bufferRef, undef);
    my @sync = (0xED, 0xFE);

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
