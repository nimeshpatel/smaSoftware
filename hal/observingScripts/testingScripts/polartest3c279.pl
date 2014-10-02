#!/usr/bin/perl -w
#
# Testing Antenna 1 polar computer
#
##### PLEASE CHANGE NUMBER OF ANTENNAS IN NEXT LINE IF NEEDED
$nants="6";
#
####################  PRIMING  ####################
#
# tune -c "waveplate in"
# tune -c "hotload in"
# observe -s 3c279
# dopplerTrack -r 230.5 -u -s13
# setFeedOffset -f 230
# restartCorrelator -R l -s32
#
###################   POINTING  ###################
#
# Pointing: 
#  Point at start
############### Check for computer ==> Observation/Simulation
chop($hostname = `hostname`) ;
if($hostname ne "hal9000") {
$simul = "y";
print "Running Simulation\n";
}

############### CHOOSE PATTERN BASED ON NUMBER OF ANTENNAS ##############
if($nants eq "8") {
$ps="polarPattern -p 7 -w -c 1";
$pc="polarPattern -p 8 -w -c 1";
}

if($nants eq "7") {
$ps="polarPattern -p 1 -w -c 1";
$pc="polarPattern -p 2 -w -c 1";
}

if($nants eq "6") {
$ps="polarPattern -p 3 -w -c 1";
$pc="polarPattern -p 4 -w -c 1";
}

if($nants eq "5") {
$ps="polarPattern -p 5 -w -c 1";
$pc="polarPattern -p 6 -w -c 1";
}

###### SIMULATION ############################
if($simul eq "y") {
        $ps_t="integrate -s 16 -w";
        $pc_t="integrate -s 8 -w";
}

############### SOURCE, CALIBRATOR and LIMITS ##############

$cal3c273="3c273";
$cal3c279="3c279";

$tsys="tsys";

$MINEL_TARG = 19; $MAXEL_TARG = 87;
$MINEL_GAIN = 19; $MAXEL_GAIN = 87;
$MINEL_FLUX = 19; $MAXEL_FLUX = 87;
$MINEL_BPASS= 19; $MAXEL_BPASS= 87;
$MINEL_CHECK= 19;
##############################
###########################

do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");

if($simul eq "y") {command("integrate -t 15");}

$myPID=$$;
command("project -r -p 'SMA' -d 'Testing'");
print "----- initialization done, starting script -----\n";


print "----- Polarization Calibration loop -----\n";
$LST_start=9.0; $LST_end=18.0;
if($simul eq "y") {
        &PolLoop("cal3c279[tsys,ps,ps_t,ps,ps_t]","cal3c273[tsys,ps,ps_t]");
} else {
        &PolLoop("cal3c279[tsys,ps,ps]","cal3c273[tsys,ps]");
}

print "----- Congratulations!  This is the end of the script.  -----\n";

