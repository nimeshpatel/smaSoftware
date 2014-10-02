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
# observe -s g192 -r 05:58:13.55 -d 16:31:58.3 -e 2000 -v 5.7
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
$bpasspol0="3C84_LR -r 03:19:48.1601 -d 41:30:42.103"; $nbpasspol0="7";

$bpass1="3C279"; $nbpass1="40";
$bpasspol1="3C279_LR -r 12:56:11.1665 -d -05:47:21.524 -e 2000 -v 0"; $nbpasspol1="7";


$targ0="g192 -r 05:58:13.55 -d 16:31:58.3 -e 2000 -v 5.7"; $ntarg0="30";
# $targpol0="g192 -r 05:58:13.55 -d 16:31:58.3 -e 2000 -v 5.7"; $ntargpol0="10";

$targ1="g240 -r 07:44:51.97 -d -24:07:42.5 -e 2000 -v 67.5"; $ntarg1="30";
# $targpol1="g240 -r 07:44:51.97 -d -24:07:42.5 -e 2000 -v 67.5"; $ntargpol1="10";

$cal0="0530+135"; $ncal0="7";
$calpol0="0530_LR -r 05:30:56.4167 -d 13:31:55.149 -e 2000 -v 0"; $ncalpol0="7";

$cal1="0730-116"; $ncal1="7";
$calpol1="0730_LR -r 07:30:19.1124 -d -11:41:12.600 -e 2000 -v 0"; $ncalpol1="7";

$flux0="Uranus"; $nflux0="20";

$flux1="Callisto"; $nflux1="20";

$MINEL_TARG = 32; $MAXEL_TARG = 87;
$MINEL_GAIN = 32; $MAXEL_GAIN = 87;
$MINEL_FLUX = 32; $MAXEL_FLUX = 81;
$MINEL_BPASS= 32; $MAXEL_BPASS= 87;
$MINEL_CHECK= 35; 
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
print "######### flux Calibration loop on Uranus #######\n";
print "####################################################\n";
$LST_start=20.0; $LST_end=24;
&DoPolFlux(flux0,nflux0);

#
print "####################################################\n";
print "######### flux Calibration loop on Callisto  #######\n";
print "####################################################\n";

$LST_start=23.5; $LST_end=0.6;
&DoPolFlux(flux1,nflux1);

#
print "####################################################\n";
print "####### Polarization Calibration loop on 3c84 ######\n";
print "####################################################\n";

$LST_start=0.5; $LST_end=2.0;
&DoPolPass(bpass0,nbpass1,bpasspol0,nbpasspol0);

#
print "###############################################\n";
print "########### Main loop part 1: G192 ############\n";
print "###############################################\n";

$LST_start=1.9; $LST_end=6.6;
&DualPolLoop(calpol0,cal0,targ0);

# end the loop with cal0
command("observe -s $cal0");
command("tsys");
command("rotateWaveplate -s L");
command("integrate -s 7 -w");

#
print "###############################################\n";
print "########### Main loop part 2: G240 ############\n";
print "###############################################\n";

$LST_start=6.5; $LST_end=10.3;
&DualPolLoop(calpol1,cal1,targ1);
#
# end the loop with cal1
command("observe -s $cal1");
command("tsys");
command("rotateWaveplate -s L");
command("integrate -s 7 -w");
#
print "###############################################\n";
print "### Polarization Calibration loop on 3c279 #####\n";
print "###############################################\n";
$LST_start=10.0; $LST_end=15.0;
&DoPolPass(bpass1,nbpass1,bpasspol1,nbpasspol1);

#
print "----- Congratulations!  This is the end of the script.  -----\n";}
#
################## File End ###############################