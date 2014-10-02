#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
# Experiment Code: 2009B-S096
# Experiment Title: eSMA:  Spectral lines from material which is still accelerating in the IRC+10216 envelope
# PI: Ken Young
# Contact Person: Ken Young  
# Email  : kyoung@cfa.harvard.edu  
# Office : (617) 495-7330  
# Home   : (617) 388-5572   
# Array  : vextended   
#
#
############## SPECIAL INSTRUCTIONS ################
#
# none
#
################## Priming ################################
#
# observe -s cwleo -r 09:47:57.38 -d +13:16:43.7 -e 2000 -v -26.2
# dopplerTrack -r 341.322 -u -s12
# restartCorrelator -R l -s128 
# setFeedOffset -f 345
#
################## Pointing ###############################
#
# Pointing: None requested
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
################## Source, Calibrator and Limits ##########
#
$inttime="30"; 
$targ0="cwleo -r 09:47:57.38 -d +13:16:43.7 -e 2000 -v -26.2"; $ntarg0="40"; 
$cal0="0854+201"; $ncal0="10";
$flux0="titan"; $nflux0="60";
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
command("project -r -p 'Ken Young' -d '2009B-S096'");
print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#
print "----- initial flux and bandpass calibration -----\n";
if(!$restart){
  &DoFlux(flux0,nflux0);
  &DoPass(bpass0,nbpass0);
}

print "----- main science target observe loop -----\n";
  &ObsLoop(cal0,targ0);

print "----- final flux and bandpass calibration -----\n";
  &DoFlux(flux0,nflux0);
  &DoPass(bpass0,nbpass0);

print "----- Congratulations!  This is the end of the script.  -----\n";}
#
################## File End ###############################
