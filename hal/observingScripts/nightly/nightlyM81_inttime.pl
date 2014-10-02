#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
# Experiment Code: 2011B-S070
# Experiment Title: The Disk/Jet connection in LLAGN: The relationship between the mm and X-ray emission in M81*
# PI: Ian McHardy
# Contact Person: Ian McHardy, Mark Gurwell  
# Email  : imh@soton.ac.uk, mgurwell@cfa.harvard.edu  
# Office : 44-23-80592101, 617 495 7291  
# Home   : wouldn't you like to know...   
# Array  : all   
#
#
############## SPECIAL INSTRUCTIONS ################
#
# Please only run if all three sources are above 29 degrees
# elevation!
#
# just execute, should take just about 10 minutes (excluding
# initial slew time).  will use all calibration from the 
# main science track (flux/bandpass).
#
# run for whatever tuning/receiver/correlator set up is already
# in use!
#
# HOWEVER, if this is being run as a standalone, it will require
# flux calibration.  Use a Galilean moon and/or Titan, 10 minutes
# before and after if time allows, for this extra calibration
# please also make sure there are no huge chunk to chunk offsets
#
################## Priming ################################
#
# You should not have to 'prime' in the sense that this projec
# will use the tuning/calibration of the primary science track
# underway when this snippet gets run...
#
# observe -s M81star -r 09:55:33.173 -d +69:03:55.06 -e 2000 -v 0
# dopplerTrack -r 230.538 -u -s13
# restartCorrelator -R l -s32   
# setFeedOffset -f 230
#
################## Pointing ###############################
#
# Pointing: None requested
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
################## Source, Calibrator and Limits ##########
#
#
#The following code will search the log for the last integrate -t command 
#and save that value.
$path = "/global/logs/SMAshLog";
$foundvalue = 0;
$counter = 0;
$raw = `tail -10000 $path`;
@lines = split("\n", $raw);
while($foundvalue == 0 && $counter < 10000)
{
    @line = split(" ", $lines[($#lines - $counter)]);
    if($line[6] =~ /integrate/ && $line[7] =~ /\-t/)
    {
        $oldinttime = $line[8];
        $foundvalue = 1;
    }
    $counter++;
}
print "The old integration time is $oldinttime\n";
#end code segment

  $inttime="15"; 
  $targ0="M81star -r 09:55:33.173 -d +69:03:55.06 -e 2000 -v 0"; $ntarg0="6"; 
  $cal0="0958+655"; $ncal0="6";
  $cal1="1048+717"; $ncal1="6";
  $MINEL=29; $MAXEL=83;
  $MINEL_CHECK= 30; 
#
  $total_loops=2;
#
################## Script Initialization ##################
#
  do 'sma.pl';
  do 'sma_add.pl';
  checkANT();
  command("radio");
  command("integrate -t $inttime");
  $myPID=$$;
  command("project -r -p 'Ian McHardy' -d '2011B-S070'");
  print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#
  print "----- main science target observe loop -----\n";

  LST(); $elcal0 = checkEl($cal0);
  LST(); $elcal1 = checkEl($cal1);
  LST(); $eltarg0 = checkEl($targ0);

  if($elcal0 >= $MINEL and $elcal1 >= $MINEL and $eltarg0 >= $MINEL) {

      $loop = 0;

      while ($loop < $total_loops ) {

	  $loop = $loop + 1;

	  command("observe -s $cal0");
	  command("tsys");
	  command("integrate -s $ncal0 -w");

	  command("observe -s $targ0");
	  command("tsys");
	  command("integrate -s $ntarg0 -w");

	  command("observe -s $cal1");
	  command("tsys");
	  command("integrate -s $ncal1 -w");
      } 

      print "----- Congratulations!  This is the end of the script.  -----\n";      

  } else {

      print "***\n";
      print "***\n";
      print "*** ALL THREE SOURCES NEED TO BE ABOVE $MINEL FOR SCRIPT TO EXECUTE!\n";
      print "***\n";
      print "***\n";
  }

#This code will print whether or not the integration time was found
#and change it, if the value was found.
if($oldinttime)
{
    print "\n";
    print "The integration time is now being reset to the original value of : $oldinttime\n";
    `integrate -t $oldinttime`;
    print "\n";
}
else
{
    print "The old integration time could not be found. It will have to be set back manually.\n";
}
#end code segment



}

#
################## File End ###############################
