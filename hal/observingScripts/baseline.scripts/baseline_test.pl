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
#
#$cal1='1337-129';
#$cal1='2148+069';
#$cal1='0418+380';
#$cal1='0530+135';
#$cal1='0825+031';
#$cal1='0927+390';
#$cal1='1229+020';   
#$cal1='1256-057';   
#$cal1='0854+201'; # avoid! goes too close to zenith, causing a gap in gaincal  
#$cal1='1924-292';   
#$cal1='1229+020';   
#$cal1='1058+015';
$cal1='1751+096';
#$cal1='2253+161'; 
#$cal1='nrao530';
#$cal1='1733-130';
#$cal1='2232+117';
#$cal1='0319+415';
#$cal1='2225-049';


$ncal1=7;   

# number of sources (other than $cal1) to be observed in each loop; number of
# scans on each source.
#$nsrc=3;      $nscan=5;
#$nsrc=4;      $nscan=5;
#$nsrc=4;    $nscan=6;
#$nsrc=3;  $nscan=9;
$nsrc=3;  $nscan=6;

# J2000 names of the target sources
## > 2Jy @ 1 mm, or 1.4Jy @ 0.85mm, last updated Jan 2006.
#@sourceList=('0010+109','0102+584','0319+415','0359+509','0418+380','0455-462',
# '0522-364','0530+135','0538-440','0721+713','0854+201','0927+390','1058+015',
# '1229+020','1256-057','1316-336','1337-129','1517-243','1642+689','1642+398',
# '1924-292','2148+069','2202+422','2225-049','2229-085','2232+117','2253+161');

# Generally > 1.25 Jy at 1 mm, last updated Feb 16, 2006.
#@sourceList=('0006-063','0102+584','0106-405','0238+166','0237+288','0319+415','0359+509',
# '0418+380','0423-013','0428-379','0455-462','0457-234',
# '0522-364','0530+135','0538-440','0825+031','0854+201','0927+390', 
# '0958+655','1058+015','1147-382','1159+292','1229+020','1230+123',
# '1256-057','1316-336','1337-129','1517-243','1625-254','1626-298',
# '1642+689','1642+398','1733-130','1743-038','1751+096','1801+440',
# '1849+670','1911-201','1923-210','1924-292','1957-387','2000-178',
# '2011-157','2056-472','2148+069','2202+422','2225-049','2229-085',
# '2232+117','2246-121','2253+161','2258-279','2348-165',
# '1419+543','0841+708');

#New source lists January 2008

## > 1.5Jy @ 1 mm, or 1.0Jy @ 0.85mm, last updated Jan 2008.
#@sourceList=('0010+109','0102+584','0319+415','0359+509','0418+380','0455-462',
# '0522-364','0530+135','0538-440','0721+713','0854+201','0927+390','1058+015',
# '1229+020','1256-057','1316-336','1337-129','1517-243','1642+689','1642+398',
# '1924-292','2148+069','2202+422','2225-049','2229-085','2232+117','2253+161');

# Generally > 1 Jy at 1 mm, last updated Jan, 2008.
@sourceList=( '0006-063','0106-405','0136+478','0205+322','0237+288',
 '0319+415','0359+509','0418+380','0423-013','0428-379','0433+053','0455-462',
 '0522-364','0530+135','0538-440','0721+713','0730-116','0739+016','0750+125',
 '0825+031','0854+201','0920+446','0927+390','1037-295','1058+015','1127-189',
 '1147-382','1229+020','1230+123','1256-057','1310+323','1337-129','1419+543',
 '1427-421','1504+104','1512-090','1517-243','1549+026','1550+054','1625-254',
 '1626-298','1635+381','1638+573','1642+689','1642+398','1658+076','1727+455',
 '1733-130','1751+096','1800+784','1801+440','1842+681','1849+670','1911-201',
 '1924-292','1925+211','1927+739','1937-399','1957-387','2000-178','2015+371',
 '2025+337','2056-472','2134-018','2148+069','2202+422','2225-049','2229-085',
 '2232+117','2246-121','2253+161','2258-279','2327+096','2333-237');

# The following are in the catalog but are not in the simulator catalog.
# '0455-462'

$numberInSourceList=@sourceList; 

$nflux = 20;

# Limits (in degrees)
$sunDistanceLimit=35;  
$ElLowerLimit=20;
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

use Getopt::Long;
$Getopt::Long::autoabbrev=1;

GetOptions('restart');

$foundflux = 0;

unless($opt_restart)
{
    print "Searching for an available flux calibrator\n";
    
# Uranus
    $sourceCoordinates=`lookup -s uranus`;
    chomp($sourceCoordinates);
    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	print "Uranus is available, and will be observed right now.\n";
	command("observe -s uranus");  
	command("sleep 5");
	command("tsys");
	command("integrate -s $nflux -t $inttime -w");
	$foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
    }
    else {
	print "Uranus is not available now.\n";
    }
    if($foundflux == 0)
    {
# Neptune
	$sourceCoordinates=`lookup -s neptune`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Neptune is available, and will be observed right now.\n";
	    command("observe -s neptune");  
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else {
	    print "Uranus is not available now.\n";
	}
    }
    elsif($foundflux == 0)
    {
# Callisto
	$sourceCoordinates=`lookup -s callisto`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Callisto is available, and will be observed right now.\n";
	    command("observe -s callisto");
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Callisto is not available now.\n";
	}
    }
    elsif($foundflux == 0)
    {
# Ganymede
	$sourceCoordinates=`lookup -s ganymede`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Ganymede is available, and will be observed right now.\n";
	    command("observe -s ganymede");
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Ganymede is not available now.\n";
	}
    }
    elsif($foundflux == 0)
    {
# Titan
	$sourceCoordinates=`lookup -s titan`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Titan is now available and will be observed.\n";
	    command("observe -s titan");
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else {
	    print "Titan is not available now.\n";
	}
    }
}


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
   	   command("observe -s $cal1");  command("tsys");
   	   command("integrate -s $ncal1 -t $inttime -w");
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
      command("observe -s $source");  command("tsys");
      command("integrate -s $nscan -t $inttime -w");
   }

}  ### end of the while(1) loop

# last calibration on cal1
if (defined($cal1) and checkEl($cal1) > 16) {
   command("observe -s $cal1");  command("tsys");
   command("integrate -s $ncal1 -t $inttime -w");
}

if(!$opt_restart && $foundflux == 0)
{
    print "Searching for an available flux calibrator\n";
    
# Uranus
    $sourceCoordinates=`lookup -s uranus`;
    chomp($sourceCoordinates);
    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	print "Uranus is available, and will be observed right now.\n";
	command("observe -s uranus");  
	command("sleep 5");
	command("tsys");
	command("integrate -s $nflux -t $inttime -w");
	$foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
    }
    else {
	print "Uranus is not available now.\n";
    }
    if($foundflux == 0)
    {
# Neptune
	$sourceCoordinates=`lookup -s neptune`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Neptune is available, and will be observed right now.\n";
	    command("observe -s neptune");  
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else {
	    print "Uranus is not available now.\n";
	}
    }
    elsif($foundflux == 0)
    {
# Callisto
	$sourceCoordinates=`lookup -s callisto`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Callisto is available, and will be observed right now.\n";
	    command("observe -s callisto");
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Callisto is not available now.\n";
	}
    }
    elsif($foundflux == 0)
    {
# Ganymede
	$sourceCoordinates=`lookup -s ganymede`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Ganymede is available, and will be observed right now.\n";
	    command("observe -s ganymede");
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Ganymede is not available now.\n";
	}
    }
    elsif($foundflux == 0)
    {
# Titan
	$sourceCoordinates=`lookup -s titan`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) {
	    print "Titan is now available and will be observed.\n";
	    command("observe -s titan");
	    command("sleep 5");
	    command("tsys");
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else {
	    print "Titan is not available now.\n";
	}
    }
}

# bye-bye message
print "----- Congratulations! This script is done. -----\n";
if($foundflux == 0);
{
    print "\n\n";
    print "######################################################\n";
    print "######################################################\n";
    print "##                                                  ##\n";
    print "## WARNING   WARNING   WARNING   WARNING   WARNING  ##\n";
    print "##                                                  ##\n";
    print "##   Please remember to observe an appropriate flux ##\n";
    print "## calibrator, so this data can be calibrated!!!    ##\n";
    print "##                                                  ##\n";
    print "##   Please remember to observe an appropriate flux ##\n";
    print "## calibrator, so this data can be calibrated!!!    ##\n";
    print "##                                                  ##\n";
    print "## WARNING   WARNING   WARNING   WARNING   WARNING  ##\n";
    print "##                                                  ##\n";
    print "######################################################\n";
    print "######################################################\n";
    print "\n\n";
}
