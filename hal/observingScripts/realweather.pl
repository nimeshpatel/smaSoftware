#!/usr/bin/perl -w

$inttime=30;    
$cal1='1256-057';  # earlier part of the track
$cal2='2253+161';  # later part of the track
$ncal1=7;   
$ncal2=7;   

$nsrc = 4;
$nscan = 8;

# J2000 names of the target sources (F > 2 Jy at 1.3mm)
@sourceList=('0102+584','0319+415','0418+380','1159+292','1517-243',
	     '1626-298','1642+398','1751+096','1911-201','1924-292',
	     '2202+422','2232+117','2225-049');

$numberInSourceList=@sourceList; 

# Limits (in degrees)
$sunDistanceLimit=30;  
$ElLowerLimit=22;
$ElUpperLimit=86;

# load the sma library of functions used in this observing script.
do 'sma.pl';
printPID();

# check participating antennas.
checkANT();

# just in case antennas are left in optical mode from opointing.
command("radio");
# set default integration time for the correlator.
command("integrate -t $inttime");
print "----- initialization done, starting the main loop -----\n";

## The Main Loop
$realweather = 1;
command("/application/bin/dDSUseRealWeather");
while(1) {
    printPID();
    $nloop=$nloop+1;
    print "--- Loop No.= $nloop ---\n";
    LST();
    # observe cal1 if it is defined
    if (defined($cal1)) {
	$cal1El=checkEl($cal1);
	if ($cal1El < $ElLowerLimit) {
	    print "$cal1 too low. skipped.\n";
   	} else { 
	    if ($cal1El < $ElUpperLimit) {
		if ($realweather == 1) {
		    command("observe -s $cal1 -t bandpass");
		} else {
		    command("observe -s $cal1");
		}
		command("tsys");
		command("integrate -s $ncal1 -t $inttime -w");
	    } else {
		print "$cal1 too high. skipped.\n";
	    }
   	}
    }
    # observe cal2 if it is defined
    if (defined($cal2)) {
	$cal2El=checkEl($cal2);
	if ($cal2El < $ElLowerLimit) {
	    print "$cal2 too low. skipped.\n";
   	}  else {
	    if ($cal2El < $ElUpperLimit) {
		if ($realweather == 1) {
		    command("observe -s $cal2 -t bandpass");
		} else {
		    command("observe -s $cal2");
		}
		command("tsys");
		command("integrate -s $ncal2 -t $inttime -w");
	    } else {
		print "$cal2 too high. skipped.\n";
	    }
	}
    }
    # observe other sources
    @sourcesInThisLoop=();
    for $n (1..$nsrc) {
	LST();
	# source picking 
	while(1) {
	    # pick up a random source from the sourcelist
	    $i=rand($numberInSourceList); # generates random number
	    $i=int($i);                   # truncates floating points. 
	    $source=$sourceList[$i];
	    # pick up again if $cal1 is picked
	    if (defined($cal1) and $source eq $cal1) {
		next;
	    }
	    # pick up again if source has been already observed in this loop.
	    $same = 0;
	    foreach $obj (@sourcesInThisLoop) {
		if ($obj eq $source) {
		    $same=1;
		}
	    }
	    if ($same) {
		next;
	    }
	    # pick up again if the source is somehow unknown to the system.
	    $lookupOutput=`lookup -s $source`;
	    chomp($lookupOutput);
	    ($sourceAz,$sourceEl,$sunDistance)=split(' ',$lookupOutput);
	    if ($sourceAz eq 'Source') {
		next;
	    }  # for 'Source catalog is corrupted' 
	    # or  'Source not found'
	    # pick up again if source elev. or sun distance is inappropriate
	    if ($sunDistance < $sunDistanceLimit) {
		next;
	    }
	    if ($sourceEl < $ElLowerLimit or $sourceEl > $ElUpperLimit) {
		next;
	    }
	    # if nothing else, a good source is chosen.
	    @sourcesInThisLoop=($source,@sourcesInThisLoop);
	    printf " %d/%d: %s, az=%6.1f, el=%4.1f, sunDist=%6.1f \n",
              $n, $nsrc, $source, $sourceAz, $sourceEl, $sunDistance;
	    last;     # break out of the source picking loop.
	}
	if ($realweather == 1) {
	    command("observe -s $source -t bandpass");  command("tsys");
	} else {
	    command("observe -s $source");  command("tsys");
	}
	command("integrate -s $nscan -t $inttime -w");
    }
    if ($realweather == 1) {
	command("/application/bin/dDSUseDefaultWeather");
	$realweather = 0;
    } else {
	command("/application/bin/dDSUseRealWeather");
	$realweather = 1;
    }
}  ### end of the while(1) loop
