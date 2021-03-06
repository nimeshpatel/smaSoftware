#!/usr/bin/perl

#
# script to go through random quasars for baseline calibration.
#

# correlator integration time for each scan in seconds (usually 30 or 60)
 $inttime=30;    
#
# This script finishes if cal1 is below $ElLowerLimit.

$ncal1=7;   

# number of sources (other than $cal1) to be observed in each loop; number of
# scans on each source.
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

$numberInSourceList=$#sourceList; 

$nflux = 20;

# Limits (in degrees)
$sunDistanceLimit=30;  
$ElLowerLimit=17;
$ElUpperLimit=86;

#########################################################################
# initialization


# load the sma library of functions used in this observing script.
#do 'sma.pl';
$mypid = $$;
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

GetOptions('restart','subcompact','calibrator=s'=>\$cal1,'help');

if ($opt_help) {
    &Usage; die "\n";
}

if($cal1 eq '')
{
    die "Plase specify a calibrator using the -c command\n";
}

if($opt_subcompact)
{
   $ElLowerLimit=33; 
}

#find the currently active receiver
for($d = 0; $d <= $#sma; $d++)
{
    @frequency = `tune -a $sma[$d] -c active`;
#    $freq = substr $frequency[3], 20, 1;
    $freq = substr $frequency[1], 20, 1;
    if($freq eq '0')
    {
	$freq = '1mm';
    }
    elsif($freq eq '2')
    {	
	$freq = '850';
    }
    else
    {
	$freq = '';
    }
    if($freq ne '')
    {	
	last;
    }
}

#check for a + in the calibrator name and put a \ before it
$calcheck = $cal1;
if($calcheck =~ /\+/)
{
    @plus = split('\+',$calcheck);
    $calcheck = $plus[0] . '\+' . $plus[1];
}
#get the list of sources over 1.5 Jy.
getSources();

$check = 0;
for($c = 0; $c <= $#cal; $c++)
{
    if($cal[$c] =~ /$calcheck/)
    {
	$check = 1;
	$j2000 = $cal[$c];
    }
    elsif($common[$c] =~ /$calcheck/)
    {
	$check = 1;
	$j2000 = $cal[$c];
    }
}

if($check == 0)
{
    die "Please specify a bright calibrator.\n";
}

for($g = 0; $g <= $#sourceList; $g++)  
{
    if($j2000 eq $sourceList[$g])
    {
	splice(@sourceList,$g,1);
	last;
    }
}

#this part is to fix the tsys problem with 2, but it needs more work...
@tsys = @sma;
$tsysant = 0; #set to 0 if everything is normal.
if($tsysant != 0)
{
    for($w = 0; $w <= $#tsys; $w++)
    {
	if($tsysant == ($w + 1))
	{
	    splice(@tsys,$w,1);
	}
    }
}

$foundflux = 0;

unless($opt_restart)
{
    print "Searching for an available flux calibrator\n";
    
# Uranus
    $sourceCoordinates=`lookup -s uranus`;
    chomp($sourceCoordinates);
    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
    if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	print "Uranus is available, and will be observed right now.\n";
	command("observe -s uranus");  
	command("sleep 5");
	tsys();
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
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Neptune is available, and will be observed right now.\n";
	    command("observe -s neptune");  
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else {
	    print "Neptune is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Callisto
	$sourceCoordinates=`lookup -s callisto`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Callisto is available, and will be observed right now.\n";
	    command("observe -s callisto");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Callisto is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Ganymede
	$sourceCoordinates=`lookup -s ganymede`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Ganymede is available, and will be observed right now.\n";
	    command("observe -s ganymede");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Ganymede is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Titan
	$sourceCoordinates=`lookup -s titan`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Titan is now available and will be observed.\n";
	    command("observe -s titan");
	    command("sleep 5");
	    tsys();
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
   	   command("observe -s $cal1");  tsys();
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
      command("observe -s $source");  tsys();
      command("integrate -s $nscan -t $inttime -w");
   }

}  ### end of the while(1) loop

# last calibration on cal1
if (defined($cal1) and checkEl($cal1) > $ElLowerLimit) {
   command("observe -s $cal1");  tsys();
   command("integrate -s $ncal1 -t $inttime -w");
}

if(!$opt_restart && $foundflux == 0)
{
    print "Searching for an available flux calibrator\n";
    
# Uranus
    $sourceCoordinates=`lookup -s uranus`;
    chomp($sourceCoordinates);
    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
    if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	print "Uranus is available, and will be observed right now.\n";
	command("observe -s uranus");  
	command("sleep 5");
	tsys();
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
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Neptune is available, and will be observed right now.\n";
	    command("observe -s neptune");  
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else {
	    print "Neptune is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Callisto
	$sourceCoordinates=`lookup -s callisto`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Callisto is available, and will be observed right now.\n";
	    command("observe -s callisto");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Callisto is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Ganymede
	$sourceCoordinates=`lookup -s ganymede`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Ganymede is available, and will be observed right now.\n";
	    command("observe -s ganymede");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else {
	    print "Ganymede is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Titan
	$sourceCoordinates=`lookup -s titan`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ElLowerLimit<$sourceElDegrees && $sourceElDegrees<$ElUpperLimit && $sunDistance>$sunDistanceLimit) {
	    print "Titan is now available and will be observed.\n";
	    command("observe -s titan");
	    command("sleep 5");
	    tsys();
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
if($foundflux == 0)
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

sub Usage()
{
printf "Usage: flux.pl -c (calibrator name)
options:
-r (restart, skips the flux calibrators.  Be sure to use the same gain calibrator)
-s (subcompact, sets the lower elevation limit to 33, for use in subcompact configuration)
-c (calibrator, sets the calibrator to be used, must be over 1.5 Jy)\n";
die "\n";
}

sub getSources 
{
    @fluxdata = ();  #each line in the file split on commas
    @fluxdate = ();  #the date part of the file split on a space
    $fsource = 'hi';
    $num = 0;
    $calnum = 0;
    @newsourcelist = ();
    open(DATA, "</global/catalogs/callist/recent_flux_measurements.dat");
    $line = $_;
    while(<DATA>)
    {
	unless($line =~ /^\#/)  #skips anyline that starts with #
	{
	    @fluxdata = split(",", $line);
	    $uselessnum = split(" ", $fluxdata[2]); #not actually useless
	    if($uselessnum == 1)  #this is zero if there's whitespace, so it carries the J2000 name to lines that are only white
	    {
		$fsource = $fluxdata[2];
		$fcommon = $fluxdata[0];
	    }
#	print "$fsource\n";
#	print "$uselessnum\n";
	    if($fluxdata[7] =~ /SMA/)  #only sources observed at the sma
	    {
#	    print "the above source is an SMA source!\n";
		if($freq eq '')
		{
		    print "The current frequency couldn't be found, using 230.\n";
		    $freq = "1mm";
		}
		if($fluxdata[5] eq $freq) #only look at sources in the requested band
		{
		    if($fluxdata[8] > 1.5)
		    {
			$cal[$calnum] = $fsource;
#		    print "$fsource is bright enough.  It's common name is: $fluxdata[0]\n";
			$common[$calnum] = $fcommon;
			$calnum++;
		    }
		}
	    }
	}
	$line = $_;
    }
    close(DATA);
    return 0;
}

#both of these were stolen from sma.pl, to eliminate the conflict with it's getOptions.
sub printPID {
  print "The process ID of this $0 script is $mypid\n";
}

sub checkANT {
	my ($i,$exist);
   print "Checking antenna status ... \n";
   for ($i = 1; $i <=8; $i++) {
      if($simulateMode==0) { 
         $exist = `value -a $i -v antenna_status`;
         chomp($exist);
      } else {
         $exist=1;
      }
      if ($exist) {
         print "Found antenna ",$i," in the array!\n";
         @sma = (@sma,$i);
      }
   }
   print "Antennas @sma are going to be used.\n";
# write out a copy of this script in the data area
  $thisfile=${0};
  $hour=(gmtime)[2];
  $minute=(gmtime)[1];
  $seconds=(gmtime)[0];
  $day=(gmtime)[3];
  $month=(gmtime)[4]+1;
  $year=(gmtime)[5];
  $year+=1900;
  $timeStamp=sprintf("%4d%02d%02d_%02d%02d%02d",
		$year,$month,$day,$hour,$minute,$seconds);
  $scriptFileName=$thisfile."_".$timeStamp;
  $directoryName=`readDSM -v DSM_AS_FILE_NAME_C80 -m m5 -l`;
  chomp($directoryName);
  $scriptFileNameWithPath=$directoryName."/".$scriptFileName;

  print "Copy command:
  $thisfile\n
  cp $thisfile $scriptFileNameWithPath\n";

  $cpresponse=`cp $thisfile $scriptFileNameWithPath`;
	if($cpresponse ne "") {
	print "Copying this script to data area...\n $cpresponse";
	}
}

sub command {
   print "@_\n";       				# print the command
   if($simulateMode==0) {system("@_");}	# execute the command
   sleep 1;                             	# sleep 1 sec
}

sub LST {
	my ($LST);

   if (defined($givenLST)) {
      $LST=$givenLST;
   } else {
		if($simulateMode) {$LST=simlst();}
  		else {$LST= `value -a $sma[0] -v lst_hours`;}
	  	chomp($LST);
   }
	if ($_[0]) {printf("LST [hr]= %6.2f\n",$LST);}

	return $LST;
}

sub checkEl{
	my ($sourcename)=$_[0];
   my ($silent)    =(defined($_[1]) and $_[1]);
	my ($sourceCoordinates,$sourceAz,$sourceEl,$sunDistance);

   if((not $simulateMode) or ($thisMachine eq "hal9000")) {
		$sourceCoordinates=`lookup -s $sourcename`;
   	chomp($sourceCoordinates);
   	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
		if ($sourceAz eq 'error_flag') {
			print "##########################################\n";
			print "######## WARNING WARNING WARNING #########\n";
			print "##### source $sourcename not found. ######\n";
			print "##########################################\n";
			for ($i=0; $i<7;$i++) {print "\a";sleep 1;}
			die   " quiting from the script \n";
		}
   } else {
   	$sourceEl=30.;
   }
   if (not $silent) {
      printf("%s is at %4.1f degrees elevation\n",$sourcename,$sourceEl);
   }
   return $sourceEl;
}

sub tsys
{
    if($tsysant != 0)
    {
	print "tsys -a 1,4,5,6,7\n";
	if($simulateMode==0) {system("tsys -a 1,4,5,6,7");}
	sleep(1);
    }
    else
    {
	print "tsys\n";
	if($simulateMode==0) {system("tsys");}
	sleep(1);
    }
}
