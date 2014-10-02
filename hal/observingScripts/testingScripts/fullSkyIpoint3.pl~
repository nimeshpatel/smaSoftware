#!/usr/bin/perl
#Things not working:
#Observes calibrator twice; final gain cal getting observed in loop?
#Would be good to add different options for int time for calibrator vs other sources.
#
#This script is an attempt to update the fullSkyIpoint.pl script to incorporate
#options and more functionality.
#FullSkyIpoint is used to get radio pointing offsets to be used to update the radio 
#component of the pointing model after an antenna move.  The script ipoints on randomly
#chosen quasars to hopefully get a decent sky coverage of az/el combinations, while
#going back to ipoint on a user-chosen bright calibrator every 4th ipoint.
#
#Use the --help feature to see all options.  The setup currently "stuffs" or implements 
#offsets from the ipoints on the calibrator only; calibrator to be chosen with -c switch 
#at script start.  The offsets from ipoints on other sources are not stuffed, since they 
#may not be as bright as the calibrator.  Select minimum flux for all sources observerd, 
#or leave it to be default value of 1.5Jy.  
#
#Updated by Erin on 20110601 (June 1, 2011).

# correlator integration time for each scan in seconds (usually 30 or 60)
 $inttime=5;    
#
# This script finishes if cal1 is below $ElLowerLimit.

#$ncal1=7;   

# number of sources (other than $cal1) to be observed in each loop; number of
# scans on each source.
$nsrc=3;  $nscan=7;

#$nflux = 20;

# Limits (in degrees)
$sunDistanceLimit=30;  
$ElLowerLimit=20;
$ElUpperLimit=86;

#########################################################################
# initialization


use Getopt::Long;
$Getopt::Long::autoabbrev=1;

GetOptions('fluxmin=f'=>\$minflux,'subcompact','calibrator=s'=>\$cal1,'integration=i'=>\$integ,'gaincalint=i'=>\$ginteg,'kazushi','help');

@sourceList = ();

if ($opt_help) {
    &Usage; die "\n";
}

if($cal1 eq '')
{
    die "Plase specify a bright pointing calibrator using the -c command\n";
}

if($minflux eq '')
{
   $minflux=1.4;
   printf "No minimum source flux specified; using 1.4Jy.\n";
} else {
   $minflux=$minflux-0.1;
   printf "Minimum flux for sources is $minflux.\n";
   }

if($opt_subcompact)
{
   $ElLowerLimit=33; 
}

if($integ eq '')
{
   $integ=45;
}

if($ginteg eq '')
{
   $ginteg=45;
}

#For now, leave sma.pl commented out.  functions at end accommodate sma.pl
# load the sma library of functions used in this observing script.
#do 'sma.pl';  #For now, see end of script for filler functions.
$mypid = $$;
printPID();

# check participating antennas.
checkANT();

# just in case antennas are left in optical mode from opointing.
command("radio");
#command('setFeedOffset -f 230');

# set default integration time for the correlator.
command("integrate -t $inttime");

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

print "selected receiver is $freq.\n";

#check for a + in the calibrator name and put a \ before it
$calcheck = $cal1;
if($calcheck =~ /\+/)
{
    @plus = split('\+',$calcheck);
    $calcheck = $plus[0] . '\+' . $plus[1];
}
#get the list of sources over $minflux Jy.
getSources();

$numberInSourceList=$#sourceList;

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
    die "Please specify a brighter calibrator.\n";
}

for($g = 0; $g <= $#sourceList; $g++)  
{
    if($j2000 eq $sourceList[$g])
    {
	splice(@sourceList,$g,1);
    }
    if($source[$g] =~ /^[a-z]/)
    {
	splice @sourceList, $g, 1;
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
   	   command("observe -s $cal1");  command("antennaWait");
   	   if ($opt_kazushi) {
		command("ipoint -i $ginteg -r 1 -Q -s -k");
		} else {
			command("ipoint -i $ginteg -r 1 -Q -s");
		}
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
      command("observe -s $source");   command("antennaWait");
      if ($opt_kazushi) {
	command("ipoint -i $integ -r 1 -Q -k");
      } else  {
	command("ipoint -i $integ -r 1 -Q");  
    }
}
}  ### end of the while(1) loop

# last calibration on cal1
if (defined($cal1) and checkEl($cal1) > $ElLowerLimit) {
   command("observe -s $cal1"); command("antennaWait");
   if ($opt_kazushi) {
	command("ipoint -i $ginteg -r 1 -Q -s -k");
   }  else  {
	command("ipoint -i $ginteg -r 1 -Q -s");
	}
}

    
# bye-bye message
print "----- Congratulations! This script is done. -----\n";

sub Usage()
{
printf "Usage: fullSkyIpoint.pl -c (calibrator name)
options:
-f (minimum flux for the sources to be observed, default 1.5)
-i (same as ipoint, integration time for each point position. default=45seconds)
-g (same as -i, except for calibrator/repeat source. default=45seconds)
-s (subcompact, sets the lower elevation limit to 33, for use in subcompact configuration)
-c (calibrator, sets the calibrator to be used, must be over 1.5 Jy)
-k (same as ipoint, uses only scalar average of scans)\n";
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
		    if($fluxdata[8] > 1.5) #Is this for calibrator?
		    {
			$cal[$calnum] = $fsource;
#		    print "$fsource is bright enough.  It's common name is: $fluxdata[0]\n";
			$common[$calnum] = $fcommon;
			$calnum++;
		    }
		    if($fluxdata[8] > $minflux) #Changed 0.9 to $minflux input var.
		    {
			$sourceList[$num] = $fsource;
			$num++;
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
#EOF

