#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
# Experiment Code: 2014A-S013
# Experiment Title: Has the central black hole in Mrk 590 run out of gas?
# PI: Jun Yi Koay
# Contact Person: Jun Yi Koay  
# Email  : koayjy@dark-cosmology.dk  
# Office : +4535320519  
# Home   : +4542681063   
# Array  : compact   
#
#
############## SPECIAL INSTRUCTIONS ################
#
# We leave the choice of the flux calibrator and corresponding 
# 
# number of scans to the scheduler, depending on which 
# planet/moon is on the sky during the time of the 
# observations. 
#
################## Priming ################################
#
# observe -s Mrk590 -r 02:14:33.562 -d -00:46:00.09 -e 2000 -v 0
# dopplerTrack -r 224.68 -u -s18
# restartCorrelator -s32   
# setFeedOffset -f 230
#
################## Pointing ###############################
#
# Pointing: At start and in middle of track
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
################## Source, Calibrator and Limits ##########
#
$inttime="30"; 
$targ0="Mrk590 -r 02:14:33.562 -d -00:46:00.09 -e 2000 -v 0"; $ntarg0="30"; 
$cal0="0224+069"; $ncal0="4";
$flux0="Neptune"; $nflux0="20";
$flux1="Uranus"; $nflux1="20";
$bpass0="3C454.3"; $nbpass0="120";
$bpass1="3C84"; $nbpass1="120";
$MINEL_TARG = 17; $MAXEL_TARG = 83;
$MINEL_GAIN = 17; $MAXEL_GAIN = 83;
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
command("project -r -p 'Jun Yi Koay' -d '2014A-S013'");
print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#
print "----- initial flux and bandpass calibration -----\n";
if(!$restart){
  &DoPass(bpass0,nbpass0);
  &DoPass(bpass1,nbpass1);
  &DoFlux(flux0,nflux0);
  &DoFlux(flux1,nflux1);
}

print "----- main science target observe loop -----\n";
  &ObsLoop(cal0,targ0);

print "----- final flux and bandpass calibration -----\n";
  &DoFlux(flux0,nflux0);
  &DoFlux(flux1,nflux1);
  &DoPass(bpass0,nbpass0);
  &DoPass(bpass1,nbpass1);

print "----- Congratulations!  This is the end of the script.  -----\n";}
#
################## File End ###############################
