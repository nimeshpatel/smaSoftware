#!/usr/bin/perl -w

#
# script to go through random quasars for baseline calibration.
#
# MODIFIED BASELINE SCRIPT FOR VEX BY Mark Gurwell/David Wilner 05/31/2007

# correlator integration time for each scan in seconds
 $inttime=15;    
# 
# This script finishes if cal1 and cal2 are both below $ElLowerLimit.
#

$cal1='1751+096';
$cal2='1642+398';

$nscan=8;   

# number of sources (other than $calx's) to be observed in each loop
$nsrc=2;   

@sourceList=('1337-129','1517-243','1642+398',
	     '1743-038','1751+096','1924-292');

$numberInSourceList=@sourceList; 

# Limits (in degrees)
$sunDistanceLimit=30;  
$ElLowerLimit=18;
$ElUpperLimit=87;

#########################################################################
# initialization

print "\n\n";

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();

# just in case antennas are left in optical mode from opointing.
command("radio");
#command('setFeedOffset -f 230');

# set default integration time for the correlator.
command("integrate -t $inttime");

print "----- initialization done, starting the main loop -----\n";

## The Main Loop
while(1) {
    printPID();
    $nloop=$nloop+1;
    print "--- Loop No.= $nloop ---\n";
    
    LST();
    # observe cal1 and/or cal2
    $cal1El=checkEl($cal1); $cal2El=checkEl($cal2);
    if ($cal1El < $ElLowerLimit && $cal2El < $ElLowerLimit) {
	print "$cal1 and $cal2 too low. quitting from the main loop\n";
	last;                # break out of the main while loop
    } 
    if ($cal1El >= $ElLowerLimit && $cal1El < $ElUpperLimit) {
	command("observe -s $cal1");  command("tsys");
	command("integrate -s $nscan -t $inttime -w");
    } 
    if ($cal2El >= $ElLowerLimit && $cal2El < $ElUpperLimit) {
	command("observe -s $cal2");  command("tsys");
	command("integrate -s $nscan -t $inttime -w");
    }
    
    # observe other sources
    @sourcesInThisLoop=();
    for $n (1..$nsrc) {
	LST();
	# source picking 
	while(1) {
	    # pick up a random source from the sourcelist
	    $i=rand($numberInSourceList); # generates random number, 0<= $i < $arg
	    $i=int($i);                   # truncates floating points. 
	    $source=$sourceList[$i];
	    # pick up again if $cal1 is picked
	    if ($source eq $cal1 || $source eq $cal2) {next;}
	    # pick up again if the source has been already observed in this loop.
	    $same = 0;
	    foreach $obj (@sourcesInThisLoop) {if ($obj eq $source) {$same=1;}}
	    if ($same) {next;}
	    # pick up again if the source is somehow unknown to the system.
#	    if ($simulateMode) {
#		$lookupTime="$mn $d $year $hour:$min";
#		$lookupOutput=`lookup -s $source -t "$lookupTime"`;
#	    } else {
		$lookupOutput=`lookup -s $source`;
#	    }
	    chomp($lookupOutput);
	    ($sourceAz,$sourceEl,$sunDistance)=split(' ',$lookupOutput);
	    if ($sourceAz eq 'Source') {next;}  # for 'Source catalog is corrupted' 
	    # or  'Source not found'
	    # pick up again if the source's elevation or sun distance is inappropriate
	    if ($sunDistance < $sunDistanceLimit) {next;}
	    if ($sourceEl < $ElLowerLimit or $sourceEl > $ElUpperLimit) {next;}
	    # if nothing else, a good source is chosen.
	    @sourcesInThisLoop=($source,@sourcesInThisLoop);
	    printf " %d/%d: %s, az=%6.1f, el=%4.1f, sunDist=%6.1f \n",
	    $n, $nsrc, $source, $sourceAz, $sourceEl, $sunDistance;
	    last;     # break out of the source picking loop.
	}
	
	# observe the source
	command("observe -s $source");  command("tsys");
	command("integrate -s $nscan -t $inttime -w");
    }
    
}  ### end of the while(1) loop

# last calibration on cal1
if (checkEl($cal1) > 16) {
    command("observe -s $cal1");  command("tsys");
    command("integrate -s $nscan -t $inttime -w");
}
if (checkEl($cal2) > 16) {
    command("observe -s $cal2");  command("tsys");
    command("integrate -s $nscan -t $inttime -w");
}



# bye-bye message
print "----- Congratulations! This script is done. -----\n";
print "######################################################\n";
print "\n\n";

