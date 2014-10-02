#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
# Experiment Code: 2011A-A029
# Experiment Title: Director\'s track for 2011A-A010 and A014
# PI: Tien Hao Hsieh
# Contact Person: Oscar Morata  
# Email  : omorata@asiaa.sinica.edu.tw  
# Office : +886-920-133678  
# Home   : +886-223-665427   
# Array  : compact   
#
#
############## SPECIAL INSTRUCTIONS ################
#
# This is the script for the part of 2011A-A014. Notice that the 
# frequency and correlator setup are different
#
# The script should be run as soon as possible after the
# receiver set-up is ready and it should go directly to the
# main loop. The loop should run until the target sources set 
# (about LST=0.35 hr). Flux and bandpass calibration should be
# done only at the end of the loop. 
#
# After this part of the script finished, there should be a new setup
# for the frequencies and correlator to observe per064.
#
################## Priming ################################
#
# observe -s L673_smm5 -r 19:20:52.0 -d +11:13:49.5 -e 2000 -v 7.0
# dopplerTrack -r 225.75 -u -s3
# restartCorrelator -R l -s64 -s01-s02:32 -s03:256 -s04-05:128 -s15:256 -s17:256 -s42:256 
# setFeedOffset -f 230
#
################## Pointing ###############################
#
# Pointing: In middle of track
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
################## Source, Calibrator and Limits ##########
#
$inttime="30"; 
#
# target and calibrators for the first half of the track
#
$targ0="L673_smm5 -r 19:20:52.0 -d +11:13:49.5 -e 2000 -v 7.0"; $ntarg0="12"; 
$targ1="L673_smm3 -r 19:20:48.2 -d +11:14:10.5 -e 2000 -v 7.0"; $ntarg1="12"; 
$cal0="1925+211"; $ncal0="8";
$flux0="Uranus"; $nflux0="20";
$bpass0="3c84"; $nbpass0="180";
# 
#  target and calibrators for the second half of the track
#
$targ2="per064 -r 03:28:32.57 -d +31:11:05.3 -e 2000 -v 6.7"; $ntarg2="30";
$cal1="3c84"; $ncal1="10";
$flux1="Uranus"; $nflux1="20";
$bpass1="3c279"; $nbpass1="240";
$transcal="bllac";
#
$MINEL_TARG = 17; $MAXEL_TARG = 85;
$MINEL_GAIN = 17; $MAXEL_GAIN = 85;
$MINEL_FLUX = 17; $MAXEL_FLUX = 81;
$MINEL_BPASS= 17; $MAXEL_BPASS= 87;
$MINEL_CHECK= 19; 
#
################## Script Initialization ##################
#
do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");
command("integrate -t $inttime");
$myPID=$$;
command("project -r -p 'Tien Hao Hsieh' -d '2011A-A029'");
print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#

$LST_start=18.0; $LST_end=2.1;

  print "----- main science target observe loop -----\n";
    &ObsLoop(cal0,targ0,targ1);
  if(!$restart){
    print "----- final flux and bandpass calibration -----\n";
      &DoFlux(flux0,nflux0);
      &DoPass(bpass0,nbpass0);
  }
  print "\n \n";
  print "#####################################\n";
  print "#####################################\n";
  print "####                             ####\n";
  print "####  STOP SCRIPT AND RETUNE!!!  ####\n";
  print "####                             ####\n";
  print "#####################################\n";
  print "#####################################\n";
  print "\n \n";

$LST_start=2.1; $LST_end=12.1;

  print "----- ititial flux calibration -----\n";
    &DoFlux(flux1,nflux1);
  print "----- main science target observe loop -----\n";
    &ObsLoop(cal1,targ2);
  print "----- final flux and bandpass calibration -----\n";
    &DoPass(bpass1,nbpass1);

print "----- Congratulations!  This is the end of the script.  -----\n";}
#
################## File End ###############################

