#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
# Experiment Code: 2008A-S004
# Experiment Title: Gas Dynamics in the High Mass Star Forming Region K3-50A
# PI: Pamela Klaassen
# Contact Person: Pamela Klaassen
# Email  : klaassp@physics.mcmaster.ca
# Office : (905) 525 9140 x 24574
# Home   : (905) 296 5946
# Array  : compact
#
################## Priming ################################
#
# restartCorrelator -s128 -s3:512 -s5:512 -s12:256 -s19:256
# observe -s K3-50A -r 20:01:45.60 -d +33:32:42.0 -e 2000 -v -24
# dopplerTrack -r 217.104 -l -s3
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
$targ0="K3-50A -r 20:01:45.60 -d +33:32:42.0 -e 2000 -v -24"; $ntarg0="40";
$cal0="2023+336"; $ncal0="10";
$cal1="2013+370"; $ncal1="10";
$flux0="Uranus"; $nflux0="90";
$flux1="Neptune"; $nflux1="90";
$bpass0="3c273"; $nbpass0="120";
$MINEL_TARG = 17; $MAXEL_TARG = 81;
$MINEL_GAIN = 17; $MAXEL_GAIN = 81;
$MINEL_FLUX = 17; $MAXEL_FLUX = 81;
$MINEL_BPASS= 17; $MAXEL_BPASS= 81;
$MINEL_CHECK= 23;
#
################## Script Initialization ##################
#
do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");
command("integrate -t $inttime");
$myPID=$$;
command("project -r -p 'Pamela Klaassen' -d '2008A-S004'");
print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#
print "----- initial flux and bandpass calibration -----\n";
if(!$restart){
  &DoFlux(flux0,nflux0);
  &DoFlux(flux1,nflux1);
  &DoPass(bpass0,nbpass0);
}

print "----- main science target observe loop -----\n";
  &ObsLoop(cal0,targ0,cal1,targ0);

print "----- final flux and bandpass calibration -----\n";
  &DoFlux(flux0,nflux0);
  &DoFlux(flux1,nflux1);
  &DoPass(bpass0,nbpass0);

print "----- Congratulations!  This is the end of the script.  -----\n";}
#
################## File End ###############################

