#!/usr/bin/perl -w
{ BEGIN {$^W =0}

################## Priming ################################
#
# dopplerTrack -r 345.3397599 -u -s 16 -R h -r 345.3397599 -u -s 16
# restartCorrelator -R l -s64 -R h -s64
# setFeedOffset -f 345
#
################## Pointing ###############################
#
# Pointing: At start of track
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
################## Source, Calibrator and Limits ##########
#
$inttime="30";
$pants='2,3';
$targ0="3c111"; $ntarg0="10"; 
$targpol0="3c111_R -r 04:18:21.277 -d +38:01:35.80 -e 2000 -v 0"; $ntargpol0="10";

$cal0="0359+509"; $ncal0="7";
$calpol0="0359+509"; $ncalpol0="7";

$flux1="uranus"; $nflux1="10";
$fluxpol1="uranus_test";

$bpass0="0854+201"; $nbpass0="12";
$bpasspol0="0854_LR -r 08:54:48.875 -d +20:06:30.64 -e 2000"; $nbpasspol0="8";

$MINEL_TARG = 32; $MAXEL_TARG = 87;
$MINEL_GAIN = 32; $MAXEL_GAIN = 87;
$MINEL_FLUX = 32; $MAXEL_FLUX = 87;
$MINEL_BPASS= 32; $MAXEL_BPASS= 87;
$MINEL_CHECK= 33; 
#
################## Script Initialization ##################
#
do 'sma_test.pl';
do 'sma_add_test.pl';
print "error? $@\n";
checkANT();
command("radio");
command("integrate -t $inttime");
$myPID=$$;
command("project -r -p 'SMA' -d 'Dual Pol Test'");
print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#
print "----- initial flux and bandpass calibration -----\n";
$LST_start=23; $LST_end=1.0;
&DoPolFlux(flux1,nflux1);

$LST_start=1.0; $LST_end=14.0;
&DoPolPass(bpass0,nbpass0,bpasspol0,nbpasspol0);

$LST_start=4.0; $LST_end=6.8;
print "----- main science target observe loop -----\n";
&DualPolLoop(cal0,targ0,calpol0,targpol0);

print "----- final flux and bandpass calibration -----\n";

print "----- Congratulations!  This is the end of the script.  -----\n";
}
#
################## File End ###############################
