#!/usr/bin/perl -w
  
#This script fetches quasars for the source list from the SMAOC 
#Calibrator List, based on criteria defined in variables below.  Using this source
# list, the script chooses random quasars to ipoint on.  Every 4th quasar is the
# calibrator chosen below.  The user will need to accept or decline ipoint offsets
#when the prompt shows up on the script terminal.  Be sure to choose an appropriate
#calibrator, and define an appropriate lower and upper elevation limit. 
#Current ipoint command is 'ipoint -i 30 -r 1 -Q -s' for main cal and 'ipoint -i 60 -r 1 -Q -s'
#for other source.
#
#

# correlator integration time for each scan in seconds (usually 30 or 60)
 $inttime=5;
# primary calibrator cal1 (use J2000 name);  number of scans on cal1
# This script finishes if cal1 is below $ElLowerLimit.
# Note, however, that $cal1 can be left undefined. In this case, the script
# does not periodically observe the primary calibrator. The script runs forever
# and need to be stopped by kill -9 PID.
# 
# !!! Note that using the script only recognizes J2000 XXXX+XXX style names. 
# !!! Using the "3cXXX" style names will cause BOTH names to be observed!!!  
# 3c111=0418+380,3c273=1229+020,3c84=0319+415,3c279=1256-057,3c454.3=2253+161
#bllac=2202+422,3c345=1642+398
#$cal1='0418+380';
$cal1='0423-013';
#$cal1='0530+135'; 
#$cal1='1229+020';
#$cal1='1256-057';
#$cal1='0854+201';
#$cal1='1924-292';
#$cal1='1229+020';
#$cal1='1058+015';
#$cal1='1642+398';
#$cal1='1751+096';
#$cal1='2253+161';
#$cal1='2232+117';
#$cal1='0319+415';
#$cal1='2225-049';
$ncal1=5;

# number of sources (other than $cal1) to be observed in each loop; number of
# scans on each source. 
$nsrc=4;      $nscan=7; 
#$nsrc=4;      $nscan=5;
#$nsrc=4;    $nscan=6;

#Parameters for collecting sources from the SMA Calibrator List.
$FREQ = 230;    #Which receivers are you using (230,345)?
$MINFlux = 1.5;  #How bright do you need the pointing sources(in Jy)?
$MINDate = 20090101;   #How old can the flux measurement be (i.e. yyyymmdd) 

print "\n\n";
print "Fetching calibrator list...\n";

my $html_text = &get_callist_html;     
$html_text || die "Can't fetch calibrator list from http://sma1.sma.hawaii.edu/callist/callist.html";

print "Collecting appropriate sources from calibrator list...\n";


@sourceList = getSources($html_text);

$numberInSourceList=@sourceList;

print "\n\n";
print "$numberInSourceList sources found: \n";
print @sourceList;
print "\n\n";

# Limits (in degrees)
$sunDistanceLimit=30;
$ElLowerLimit=17;
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
           command("observe -s $cal1");  command("antennaWait");
           command("ipoint -i 60 -r 1 -Q -s");
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
      command("ipoint -i 60 -r 1 -Q -s");  
   }  
      
}  ### end of the while(1) loop

# last calibration on cal1
if (defined($cal1) and checkEl($cal1) > 16) {
   command("observe -s $cal1");  command("antennaWait");
   command("ipoint -i 60 -r 1 -Q -s");
}  
   
# bye-bye message
print "----- Congratulations! This script is done. -----\n";
print "\n\n";

sub get_callist_html {
  use IPC::Open2;
  local (*R,*W); 
  open2(\*R, \*W, 'telnet 128.171.116.115 80') or return undef;
  print W "GET /callist/callist.html HTTP/1.0\n\n";
  my $html;
  while($html = <R>) {
    last if($html =~ /DOCTYPE/);
  } 
  local $/ = undef;
  $html .= <R>;
} 

sub getSources {

  #remove top of page html
  my @html = split('<tr align="center" valign="middle">', $html_text,2); 
  $html[1] =~ s/<[^>]*>//gs;    #gets rid of html tags
  @sources = split(" &nbsp;",$html[1]);    #splits into seperate sources

#Make sure a frequency is listed to collect correct flux info.  If 345 or 230 not specified, assumes 230.
  if ($FREQ eq 230)
  {
        $freq = "1mm";
  }
  if ($FREQ eq 345)
  {
        $freq = "850&micro;m";
  }
  if (($FREQ ne 345) && ($FREQ ne 230))
  {
        $freq = "1mm";
        print "No frequency given, assuming 230.";
        print "\n";
  }
#Temp variables for loops 
  my @flux_year = ();
  my @flux_month = ();
  my @flux_day = ();
  my @flux_date = ();
  my @fluxes = ();
  my @common = ();
  my @b1950 = ();
  my @j2000 = ();
  my @ra = ();
  my @dec = ();
  my @newSourceList = ();
  my $i=0;
  my $k=0;
  #Step through each source on calibrator list
  while ($i<=$#sources)
  {
        my @each_source=split(' ',$sources[$i]); #seperate source into parts
        my $j=0;
        foreach (@each_source)
        {       #Find date and flux  
                if ($each_source[$j] eq $freq)
                {
                        $flux_year[$i] = $each_source[$j+3];
                        $flux_month[$i] = $each_source[$j+2];
			if ($flux_month[$i] eq "Jan")  {
			        $flux_month[$i] = "01";
       				 }
			if ($flux_month[$i] eq "Feb")  {
			        $flux_month[$i] = "02";
       				 }
			if ($flux_month[$i] eq "Mar")  {
       				$flux_month[$i] = "03";
       				 }
			if ($flux_month[$i] eq "Apr")  {
			        $flux_month[$i] = "04";
       				 }
			if ($flux_month[$i] eq "May")  {
			        $flux_month[$i] = "05";
       				 }
			if ($flux_month[$i] eq "Jun")  {
			        $flux_month[$i] = "06";
       				 }
			if ($flux_month[$i] eq "Jul")  {
			        $flux_month[$i] = "07";
       				 }
			if ($flux_month[$i] eq "Aug")  {
			        $flux_month[$i] = "08";
       				 }
			if ($flux_month[$i] eq "Sep")  {
			        $flux_month[$i] = "09";
       				 }
			if ($flux_month[$i] eq "Oct")  {
			        $flux_month[$i] = "10";
       				 }
			if ($flux_month[$i] eq "Nov")  {
			        $flux_month[$i] = "11";
       				 }
			if ($flux_month[$i] eq "Dec")  {
			        $flux_month[$i] = "12";
       				 }
			$flux_day[$i] = $each_source[$j+1];
			 $flux_date[$i] = $flux_year[$i] . $flux_month[$i] . $flux_day[$i];
                        $fluxes[$i] = $each_source[$j+5];
                        if ($fluxes[$i] eq "MMA")
                        {
                                $fluxes[$i] = $each_source[$j+6];
                        }
                        #Condition for which sources to select
                        if (($fluxes[$i] >= $MINFlux) && ($flux_date[$i] >= $MINDate))
                        {       #Assign j2000 names into the sourcelist.  
                                ($common[$i], $j2000[$i], $ra[$i], $dec[$i]) = split(' ',$sources[$i],4);
                              #  $newSourceList[$k] = "'" . $j2000[$i] . "'";
				$newSourceList[$k] = $j2000[$i];
                                $k++;
                        }
                }
                $j++;
        }
        $i++;
  }
return @newSourceList;
}
