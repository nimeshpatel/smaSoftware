#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
#
############## SPECIAL INSTRUCTIONS ################
#
# !!! Antennas chosen in $pants for cross RX calibration MUST BE IN THE ARRAY !!!
#
################## Priming ################################
#
# observe -s dr21oh -r 20:39:01.2 -d 42:22:48.5 -e 2000 -v -3.5
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
# set scan time
$inttime="30";
#
# choose which antennas with waveplates crossed during the cross RX calibration
# !!! Antennas chosen MUST BE IN THE ARRAY !!!
$pants='1,3';

$bpass0="3C84"; $nbpass0="40";
$bpasspol0="3C84_LR"; $nbpasspol0="7";
#$bpasspol0="3C84_LR -r 03:19:48.1601 -d 41:30:42.103"; $nbpasspol0="7";

$bpass1="3C279"; $nbpass1="40";
$bpasspol1="3C279_LR"; $nbpasspol1="7";
#$bpasspol1="3C279_LR -r 12:56:11.1665 -d -05:47:21.524 -e 2000 -v 0"; $nbpasspol1="7";

$targ0="dr21oh -r 20:39:01.2 -d 42:22:48.5 -e 2000 -v -3.5"; $ntarg0="40";


$cal0="mwc349a"; $ncal0="7";
$calpol0="mwc349a_LR"; $ncalpol0="7";

$flux0="titan"; $nflux0="30";

$flux1="uranus"; $nflux1="30";

$MINEL_TARG = 32; $MAXEL_TARG = 87;
$MINEL_GAIN = 32; $MAXEL_GAIN = 87;
$MINEL_FLUX = 32; $MAXEL_FLUX = 81;
$MINEL_BPASS= 32; $MAXEL_BPASS= 87;
$MINEL_CHECK= 34; 
#
################## Script Initialization ##################
#

# do 'sma_add.pl';
# do 'sma_test.pl';

do 'sma.pl';
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
print "####################################################\n";
print "######### flux Calibration loop on totan #######\n";
print "####################################################\n";
$LST_start=8.0; $LST_end=12;
&DoPolFlux(flux0,nflux0);
#
#
print "###############################################\n";
print "### Polarization Calibration loop on 3c279 #####\n";
print "###############################################\n";
$LST_start=11.0; $LST_end=16.5;
&DoPolPass(bpass1,nbpass1,bpasspol1,nbpasspol1);
#
print "###############################################\n";
print "########### Main loop part DR21OH    ############\n";
print "###############################################\n";

$LST_start=16.4; $LST_end=1.4;
&DualPolLoop(calpol0,ncalpol0,cal0,ncal0,targ0,ntarg0);
#
# end the loop with cal0
command("observe -s $cal0 -n $calpol0");
command("tsys");
command("rotateWaveplate -s L");
command("rotateWaveplate -a $pants -s R");
command("integrate -s 7 -w");
#
#
print "####################################################\n";
print "######### flux Calibration loop on Uranus #######\n";
print "####################################################\n";
$LST_start=0.5; $LST_end=2.4;
&DoPolFlux(flux1,nflux1);

print "#########################################\n";
print "## Polarization Calibration loop on 3c84 ##\n";
print "#########################################\n";
$LST_start=1.0; $LST_end=5.5;
&DoPolPass(bpass0,nbpass0,bpasspol0,nbpasspol0);
#
print "----- Congratulations!  This is the end of the script.  -----\n";}
#
################## File End ###############################