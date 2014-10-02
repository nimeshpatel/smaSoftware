#!/usr/bin/perl -w

#
# script to go through random quasars for baseline calibration.
#

# correlator integration time for each scan in seconds (usually 30 or 60)
 $inttime=30;    
#
# This script finishes if cal1 is below $ElLowerLimit.
#
#  CHOOSE ONE CALIBRATOR FOR THE ENTIRE BASELINE TRACK
#
# !!! Note that using the script only recognizes J2000 XXXX+XXX style names.
# !!! Using the "3cXXX" style names will cause BOTH names to be observed!!!
# 3c111=0418+380,3c273=1229+020,3c84=0319+415,3c279=1256-057,3c454.3=2253+161
#bllac=2202+422,3c345=1642+398
#$cal1='1337-129';
#$cal1='2148+069';
#$cal1='0418+380';
#$cal1='0530+135';
#$cal1='1229+020';   
#$cal1='1256-057';   
#$cal1='0854+201';   
#$cal1='1924-292';   
#$cal1='1229+020';   
#$cal1='1058+015';
#$cal1='1751+096';
$cal1='2253+161';
#$cal1='2232+117';
#$cal1='0319+415';
#$cal1='2225-049';

$ncal1=5;   

# number of sources (other than $cal1) to be observed in each loop; number of
# scans on each source.
$nsrc=3;      $nscan=7;
#$nsrc=4;      $nscan=5;
#$nsrc=4;    $nscan=6;

# J2000 names of the target sources
## > 2Jy @ 1 mm, or 1.4Jy @ 0.85mm, last updated Jan 2006.
@sourceList=('0136+478','0319+415','0359+509','0418+380','0522-364','0530+135','0721+713','1058+015','1229+020','1230+123','1256-057','1337-129','1642+398','1751+096','1849+670','1924-292','2015+371','2202+422','2225-049','2229-085','2253+161');
#the last line were included to fill out declinations >50 degrees
# The following are in the catalog but are not in the simulator catalog.
# '0455-462'

$numberInSourceList=@sourceList; 

# Limits (in degrees)
$sunDistanceLimit=30;  
$ElLowerLimit=33;
$ElUpperLimit=87;

#########################################################################
# initialization

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
   # observe cal1 if it is defined
   if (defined($cal1)) {
	   $cal1El=checkEl($cal1);
   	if ($cal1El < $ElLowerLimit) {
   	   print "$cal1 too low. quitting from the main loop\n";
   	   last;                # break out of the main while loop
   	} 
   	if ($cal1El < $ElUpperLimit) {
   	   command("observe -s $cal1"); command("antennaWait"); 
   	   command("ipoint -i 30 -r 1");
   	} else {
   	   print "$cal1 too high. skipped.\n";
   	}
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
         if (defined($cal1) and $source eq $cal1) {next;}
         # pick up again if the source has been already observed in this loop.
         $same = 0;
         foreach $obj (@sourcesInThisLoop) {if ($obj eq $source) {$same=1;}}
         if ($same) {next;}
         # pick up again if the source is somehow unknown to the system.
         $lookupOutput=`lookup -s $source`;
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
      command("observe -s $source");  command("antennaWait");
      command("ipoint -i 30 -r 1");
   }

}  ### end of the while(1) loop

# last calibration on cal1
if (defined($cal1) and checkEl($cal1) > 16) {
   command("observe -s $cal1");  command("antennaWait");
   command("ipoint -i 30 -r 1");
}

# bye-bye message
print "----- Congratulations! This script is done. -----\n";
print "\n\n";

