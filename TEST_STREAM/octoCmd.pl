#!/usr/bin/perl 

#
#
#

use strict;
use warnings;
use feature ':5.22';
use Device::SerialPort;
use Modern::Perl '2015';
use Tk;
use Tk::ProgressBar;
use Tk::LabFrame;
use Tk::ROText;
use Carp qw/longmess cluck confess/;
use POSIX;
use File::Temp qw/ :mktemp  /;


no warnings 'experimental::smartmatch';

sub fletcher16 ($$);
sub statusFunc ($$$$$$$);
sub initSerial ($);
sub serialCb();
sub sendMotorParameters();
sub octoMessageCB($);
sub messagePollingCB();
sub getSerial();
sub generateGui();
sub generatePanel ();
sub generateOneServoFrame ($$);
sub labelLabelFrame ($$$$;$);
sub labelEntryFrame ($$$$;$);
sub fhbits(@);
sub startLog();
sub stopLog();

my $mw;
my $mwf;

my %options;
my %tkObject = (
    "clink0" => 0,
    "clink1" => 0,
    "clinkOn0" => 1,
    "clinkOn1" => 1,
    );


my @varDataIn = ({
    'rpm' => 0,		
    'errors' => 0
		 });

my $tachoErrMsg='';

foreach my $i (1 .. 7) {
    %{$varDataIn[$i]} = %{$varDataIn[0]};
}

my %varDataOut = (
    'rpmMin' => 0,		
    'rpmMax' => 0,
    'motorMagnets' => 0,
    'nbMotors' => 0,
    'nbMessPerSec' => 0,
    'sensorType' => 0,
    'runState' => 0,
    'logState' => 0,
);

my $rotext;

my ($logFd, $logFdCount);

my $serialName = $ARGV[0] // "/dev/ttyACMx";
#my $serialHandle;

$serialName = getSerial () if $serialName eq "/dev/ttyACMx";

say "DBG> serialName = $serialName";
#sysopen ($serialHandle,  $serialName, O_RDWR);
my $serial;

do {
    $serial = initSerial ($serialName);
    sleep (1) unless $serial;
} unless defined $serial;

my $selectReadBits = fhbits(*FHD);

generateGui();

my $dbgBuf;


#$mw->fileevent(\*FHD, 'readable', \&serialCb) ;
$mw->repeat(100, \&messagePollingCB);
$mw->after(200, sub {
    # demande l'état du tachymetre
    my $buffer = pack ('cc', (6,0));
    simpleMsgSend(\$buffer);
});
Tk::MainLoop;






#                 _____                  _    _______   _            
#                |  __ \                | |  |__   __| | |           
#                | |__) |   ___   _ __  | |     | |    | | _         
#                |  ___/   / _ \ | '__| | |     | |    | |/ /        
#                | |      |  __/ | |    | |     | |    |   <         
#                |_|       \___| |_|    |_|     |_|    |_|\_\        
sub generateGui()
{
    $mw = MainWindow->new;
    $mw->wm (title => "octo tachy");
    my $w = $mw->screenwidth;
    my $h = $mw->screenheight;

    $mw->MoveToplevelWindow (0,0);

    $mwf =  $mw->Frame ()->pack(-side => 'left', -anchor => 'w');
    generatePanel ();
}


sub generatePanel ()
{
    my $outerFrame = $mwf->Frame ();
    $outerFrame->pack(-side => 'left', -anchor => 'w');
    
    my $entriesFrame = $mwf->Frame ();
    $entriesFrame->pack(-side => 'left', -anchor => 'w');

    my $specialOrderFrame = $entriesFrame->Frame (-bd => '1m', -relief => 'sunken');
    $specialOrderFrame->pack(-side => 'top', -anchor => 'w');

    my $rb1Frame = $specialOrderFrame->Frame (-bd => '1m', -relief => 'sunken');
    $rb1Frame->pack(-side => 'top', -anchor => 'w');
    my @pk = (-side => 'left', -pady => '2m', -padx => '0m', 
	      -anchor => 'w', -fill => 'both', -expand => 'true');
    
    $rb1Frame->Radiobutton (-text => "Stop",
				     -variable => \ ($varDataOut{'runState'}),
				     -value => 0,
				     -command => sub {
					 $tachoErrMsg='';
					 my $buffer = pack ('cc', (3, 0));
					 simpleMsgSend(\$buffer);
					 $buffer = pack ('cc', (6,0));
					 simpleMsgSend(\$buffer);
				     }
	)->pack(@pk);
    
    $rb1Frame->Radiobutton (-text => "Run",
				     -variable => \ ($varDataOut{'runState'}),
				     -value => 1,
				     -command => sub {
					 $tachoErrMsg='';
					 sendMotorParameters();
					 my $buffer = pack ('cc', (3, 1));
					 simpleMsgSend(\$buffer);
					 $buffer = pack ('cc', (6,0));
					 simpleMsgSend(\$buffer);
				     }
	)->pack(@pk);

    my $rb2Frame = $specialOrderFrame->Frame (-bd => '1m', -relief => 'sunken');
    $rb2Frame->pack(-side => 'top', -anchor => 'w');
    
    $rb2Frame->Radiobutton (-text => "Log Disable",
				     -variable => \ ($varDataOut{'logState'}),
				     -value => 0,
				     -command => sub {
					 stopLog();
				     }
	)->pack(@pk);
    
    $rb2Frame->Radiobutton (-text => "Log Enable",
				     -variable => \ ($varDataOut{'logState'}),
				     -value => 1,
				     -command => sub {
					 startLog();
				     }
	)->pack(@pk);


    
    
    labelEntryFrame($specialOrderFrame, "Rpm Min", \ ($varDataOut{'rpmMin'}), 'top', 10); 
    labelEntryFrame($specialOrderFrame, "Rpm Max", \ ($varDataOut{'rpmMax'}), 'top', 10); 
    labelEntryFrame($specialOrderFrame, "Nb Magnets", \ ($varDataOut{'motorMagnets'}), 'top', 10); 
    labelEntryFrame($specialOrderFrame, "Mb Motors", \ ($varDataOut{'nbMotors'}), 'top', 10); 
    labelEntryFrame($specialOrderFrame, "Mb Mess / Sec", \ ($varDataOut{'nbMessPerSec'}), 'top', 10); 
    labelLabelFrame($specialOrderFrame, "lastErr = ", \ ($tachoErrMsg), 'left', 48);

    my @pl = qw/-side left -expand 1 -padx .5c -pady .5c/;
    my $rbf  = $specialOrderFrame->LabFrame(-label => 'sensor')->pack(@pl);
    $rbf->Radiobutton(
	-text     => "Hall effect",
	-variable => \ ($varDataOut{'sensorType'}),
	-relief   => 'flat',
	-value    => 0,
        )->pack(@pl);
    $rbf->Radiobutton(
	-text     => "Esc opto coupler",
	-variable => \ ($varDataOut{'sensorType'}),
	-relief   => 'flat',
	-value    => 1,
        )->pack(@pl);
    
    $rotext =  $specialOrderFrame->ROText(
	-height => 2,
	-width => 30,
	)->pack(@pk);
    
#  __    __   __   ____    _____   _____   ______   _____          
# |  |/\|  | |  | |    \  /  ___\ |  ___| |_    _| /  ___>         
# |        | |  | |  |  | |  \_ \ |  ___|   |  |   \___  \         
#  \__/\__/  |__| |____/  \_____/ |_____|   |__|   <_____/         
    my $dmxsFrame = $mwf->Frame ();
    $dmxsFrame->pack(-side => 'left', -anchor => 'w');
    generateOneServoFrame($dmxsFrame, 0);
    generateOneServoFrame($dmxsFrame, 1);
}

sub generateOneServoFrame ($$) {
    my ($frame, $escIdx) =@_;
 
    my $dataFrame = $frame->Frame (-bd => '1m', -relief => 'sunken');
    $dataFrame->pack(-side => 'left', -anchor => 'w');

    foreach my $varName (sort keys %{$varDataIn[$escIdx]}) {
	labelLabelFrame($dataFrame, "$varName = ", \ ($varDataIn[$escIdx]->{$varName}), 'left', 10);
    }

}




sub labelLabelFrame ($$$$;$)
{
    my ($ef, $labelText, $textVar, $packDir, $width) = @_ ;
    
    my (
	$label,
	$entry,
	$frame,
	$frameDir
	) = ();
    
    $frameDir = ($packDir eq 'top') ? 'left' : 'top' ; 
    
    $width = 15 unless defined $width ;
    $frame = $ef->Frame ();
    $frame->pack (-side => $frameDir, -pady => '2m', -padx => '0m', 
		  -anchor => 'w', -fill => 'both', -expand => 'true');
    
    $label = $frame->Label (-text => $labelText);
    $label->pack (-side =>$packDir, -padx => '0m', -fill => 'y');
    
    $entry = $frame->Label (-width => $width, -relief => 'sunken',
			    -bd => 2, -textvariable => $textVar,
			    -font => "-adobe-courier-medium-r-*-*-14-*-*-*-*-*-iso8859-15") ;
    
    $entry->pack (-side =>'right', -padx => '0m', -anchor => 'e');
    
    return $entry ;
}

sub labelEntryFrame ($$$$;$)
{
    my ($ef, $labelText, $textVar, $packDir, $width) = @_ ;
    
    my (
	$label,
	$entry,
	$frame,
	$frameDir
	) = ();
    
    $frameDir =  'top' ;
    
    $width = 15 unless defined $width ;
    $frame = $ef->Frame ();
    $frame->pack (-side => $frameDir, -pady => '2m', -padx => '0m', 
		  -fill => 'y', -anchor => 'w');
    
    $label = $frame->Label (-text => $labelText);
    $label->pack (-side =>$packDir, -padx => '0m', -fill => 'y');
    
    $entry = $frame->Entry (-width => $width, -relief => 'sunken',
			    -bd => 2, -textvariable => $textVar,
			    -font => "-adobe-courier-medium-r-*-*-14-*-*-*-*-*-iso8859-15",
			    -exportselection => 'false') ;

    $entry->pack (-side =>$packDir, -padx => '0m', -anchor => 'w');
    
    return $entry ;
}


#                 ______                 _            _          
#                /  ____|               (_)          | |         
#                | (___     ___   _ __   _     __ _  | |         
#                 \___ \   / _ \ | '__| | |   / _` | | |         
#                .____) | |  __/ | |    | |  | (_| | | |         
#                \_____/   \___| |_|    |_|   \__,_| |_|         



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
#	    say ("len is $len");
	    $totLen = 0;
	    do {
		$totLen += sysread (FHD, $buffer, $len-$totLen, $totLen);
	    } while ($totLen != $len);
	    
	    $receivedCrc = fletcher16 (\$buffer, undef);
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
		octoMessageCB (\$buffer);
	    } else {
		printf ("CRC DIFFER C:0x%x != R:0x%x\n", $calculatedCrc, $receivedCrc);
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

sub octoMessageCB ($)
{
    my ($bufferRef) = @_;
    state $afterId;
 
    my ($id) = unpack('C', $$bufferRef);
    substr($$bufferRef, 0, 1, '');
    
    if ($id == 0) {
	my ($nb) = unpack('C', $$bufferRef);
	substr($$bufferRef, 0, 1, '');
	my $format =  'C' x $nb;
    	my @fields = unpack($format, $$bufferRef);
#    	printf ("ERRs = %s\n", join (':', @fields));
	for (my $c=0; $c<$nb; $c++) {
	    $varDataIn[$c]->{'errors'} = sprintf ("%.0f", $fields[$c]); 
	}
	$afterId->cancel() if defined $afterId;
	$afterId = $mw->after(5000, sub {
	    for (my $c=0; $c<$nb; $c++) {
		$varDataIn[$c]->{'errors'} = 0;
	    }});
    } elsif ($id == 1) {
	my ($nb) = unpack('C', $$bufferRef);
	substr($$bufferRef, 0, 1, '');
    	my $format =  'S' x $nb;
    	my @fields = unpack($format, $$bufferRef);
	#	printf ("RPMs = %s\n", join (':', @fields));
	print $logFd "$logFdCount\t" if $logFd;
	$logFdCount++;
	for (my $c=0; $c<$nb; $c++) {
	    $varDataIn[$c]->{'rpm'} = sprintf ("%.0f", $fields[$c]);
	    print $logFd "$varDataIn[$c]->{'rpm'}\t" if $logFd;
	}
	print $logFd "\n" if $logFd;
    } elsif ($id == 5) {
	$tachoErrMsg = unpack('Z*', $$bufferRef);
    } elsif ($id == 7) {
	my ($minRpm, $maxRpm, $motorNbMagnets, $nbMotors, $sensorType, 
	    $widthOneRpm, $timDivider, $nbMessPerSec, $runningState) = 
		unpack('VVCCCVVVC', $$bufferRef);
	$varDataOut{rpmMin} = $minRpm;
	$varDataOut{rpmMax} = $maxRpm;
	$varDataOut{motorMagnets} = $motorNbMagnets;
	$varDataOut{nbMotors} = $nbMotors;
	$varDataOut{nbMessPerSec} = $nbMessPerSec;
	$varDataOut{sensorType} = $sensorType;
	$varDataOut{'runState'} = $runningState;
	$varDataOut{'sensorType'} = $sensorType;
	$rotext->delete('1.0', 'end');
	$rotext->insert('1.0', "timDivider = $timDivider\n");
	$rotext->insert('2.0', sprintf ("width rpmMin = %d\n", $widthOneRpm/$varDataOut{rpmMin}));
	$rotext->insert('3.0', sprintf ("width rpmMax = %d\n", $widthOneRpm/$varDataOut{rpmMax}));
	$rotext->insert('4.0', sprintf ("resolution rpmMax = %d bits\n", 
					log($widthOneRpm/$varDataOut{rpmMax}) / log(2))) if $widthOneRpm;
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


sub messagePollingCB()
{
    foreach my $escIdx (0,1) {
	my $rout;
	while (select($rout=$selectReadBits, undef, undef, 0.001)) {
	    serialCb();
	}
    }
}


sub sendMotorParameters()
{
    foreach my $k (keys %varDataOut) {
	$varDataOut{$k} = int($varDataOut{$k});
    }

    my $buffer = pack ('cVVccc', (
			   4, # id
			   $varDataOut{'rpmMin'},
			   $varDataOut{'rpmMax'},
			   $varDataOut{'motorMagnets'},
			   $varDataOut{'nbMotors'},
			   $varDataOut{'sensorType'}
		       ));
    simpleMsgSend(\$buffer);

    $buffer = pack ('cS', (2, $varDataOut{'nbMessPerSec'}));
    simpleMsgSend(\$buffer);
}

sub startLog()
{
    my $file;
    $logFdCount = 0;
    ($logFd, $file) = mkstemps('/tmp/octoTacho_XXXXX', '.tsv');
}

sub stopLog()
{
    close ($logFd);
    $logFd = undef;
}

sub fhbits(@) 
{
    my @fhlist = @_;
    my $bits = "";
    for my $fh (@fhlist) {
	vec($bits, fileno($fh), 1) = 1;
    }
    return $bits;
}

