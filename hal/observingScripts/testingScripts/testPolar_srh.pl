#!/usr/bin/perl -w
#
# Experiment Code: 2011A-S031
# Experiment Title: Filaments, Star Formation and Magnetic Fields
# PI: Qizhou Zhang
# Contact Person: Qizhou Zhang/Baobab Liu (617-784-2258)
# Email  : qzhang@cfa.harvard.edu/baobabyoo@gmail.com
# Array  : Subcompact
############## SPECIAL INSTRUCTIONS ################
#
# Integration time should be 15s during track
# Script to be started at 11 LST.
# Do ipoint on 3c279
# Use single receiver, 4GHz bandwidth mode
#
# Please make sure that the targets are bracked by the
# gain calibrators.
#
# Probably add an ipoint after dr21oh transits
##
# The last loop on 3c454.3 can be cut short. 
# The gain calibrator mwc349a is also served as the flux calibrator.
# Other flux calibrator scans might be inserted, please double check
# the availability. Thanks. If no strong (but small) planet is available, 
# we will need significant time or either 3c279 or 3c454.3 for the
# passband
#
# Targets: NGC6334I, NGC6334V, G34.4.0, , G35.5, dr21oh
##### PLEASE CHANGE NUMBER OF ANTENNAS IN NEXT LINE IF NEEDED
$nants="7";
#
####################  PRIMING  ####################
#
# restartCorrelator -s128
# tune -c "waveplate in"
# tune -c "hotload in"
# observe -s dr21oh -r 20:38:59.11 -d 42:22:25.96 -e 2000 -v -3.5
# dopplerTrack -r 345.796 -u -s22
# setFeedOffset -f 345
#
#
# Pointing: usual post-sunset pointing update
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
#
############### Check for computer ==> Observation/Simulation
chop($hostname = `hostname`) ;
if($hostname ne "hal9000") {
$simul = "y";
print "Running Simulation\n";
}
############### CHOOSE PATTERN BASED ON NUMBER OF ANTENNAS
##############
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
# Source
$inttime="15"; 
# here use cal0
$targ0="ngc6334a -r 17:20:19.1 -d -35:54:45.0 -e 2000 -v -8";
$targ1="ngc6334i -r 17:20:53.44 -d -35:47:02.2 -e 2000 -v -8";
$targ2="ngc6334in -r 17:20:54.63 -d -35:45:08.5 -e 2000 -v -8";
$targ3="ngc6334v -r 17:19:57.40 -d -35:57:46.0 -e 2000 -v -8";

# here use cal1
$targ4="g34p4p0 -r 18:53:18.01 -d  01:25:25.6 -e 2000 -v 57";
$targ5="g34p4p1 -r 18:53:18.68 -d 01:24:47.2 -e 2000 -v 57";
$targ6="g35p2 -r 18:58:12.94 -d 01:40:37.1 -e 2000 -v 34";

# here use cal2
$targ7="dr21oh -r 20:39:01.2 -d 42:22:48.5 -e 2000 -v -3.5";
$targ8="dr17 -r 20:35:34.63 -d 42:20:08.8 -e 2000 -v 15";
$targ9="dr21_w -r 20:36:57.65 -d 42:11:30.2 -e 2000 -v 15";
$targ10="dr21oh_w -r 20:38:59.11 -d 42:22:25.96 -e 2000 -v -3.5";
$targ11="dr21oh_s -r 20:39:01.34 -d 42:22:04.9 -e 2000 -v -3.5";
$targ12="dr21oh_n -r 20:39:02.96 -d 42:25:51.0 -e 2000 -v -3.5";
$targ13="dr22 -r 20:40:05.39 -d 41:32:13.1 -e 2000 -v -3.5";


# Gain
$cal0="1733-130";
$cal1="1751+096";
$cal2="mwc349a";

# pol & bp
$cal3="3c84";
$cal4="3c454.3";

# flux
$cal5="Callisto";
$cal6="Uranus";


$tsys="tsys";

$MINEL_TARG = 18; $MAXEL_TARG = 87;
$MINEL_GAIN = 18; $MAXEL_GAIN = 87;
$MINEL_FLUX = 18; $MAXEL_FLUX = 87;
$MINEL_BPASS= 18; $MAXEL_BPASS= 87;
$MINEL_CHECK= 18;
#########################################################
#########################################################
do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");

if($simul eq "y") {command("integrate -t 20");}

$myPID=$$;
command("project -r -p 'Qizhou Zhang' -d ' 2011A-S031'");

print "----- initialization done, starting script -----\n";

print "#########################################\n";
print "##### Polarization Calibration loop #####\n";
print "#########################################\n";
$LST_start=3.0; $LST_end=15;
if($simul eq "y") {
        &PolLoop("cal3[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal3[tsys,ps,ps,ps]");
}


print "##########################################\n";
print "##### Main loop on NGC6334i, NGC6334v #######\n";
print "##########################################\n";
$LST_start=14.5; $LST_end=19.88;

if($simul eq "y") {
        &PolLoop("cal0[tsys,pc,pc_t]","targ3[tsys,ps,ps_t,ps,ps_t,ps,ps_t]","cal0[tsys,pc,pc_t]","targ1[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");

} else {
	&PolLoop("cal0[tsys,pc]","targ3[tsys,ps,ps,ps]","cal0[tsys,pc]","targ1[tsys,ps,ps,ps]");
}



print "##################################################\n";
print "##### Main loop G34.4.0, G35.5.2 #######################\n";
print "##################################################\n";
$LST_start=17.4; $LST_end=22.19;


if($simul eq "y") {
        &PolLoop("cal1[tsys,pc,pc_t]","targ6[tsys,ps,ps_t,ps,ps_t,ps,ps_t]","cal1[tsys,pc,pc_t]","targ4[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");

} else {
	&PolLoop("cal1[tsys,pc]","targ6[tsys,ps,ps,ps]","cal1[tsys,pc]","targ4[tsys,ps,ps,ps]");
}



print "#########################################\n";
print "##### Polarization Calibration loop #####\n";
print "#########################################\n";
$LST_start=22; $LST_end=23.32;
if($simul eq "y") {
        &PolLoop("cal4[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal4[tsys,ps,ps,ps]");
}


print "##################################################\n";
print "##### Main loop dr21oh ############################\n";
print "##################################################\n";
$LST_start=23.3; $LST_end=23.99;
if($simul eq "y") {
        &PolLoop("cal2[tsys,pc,pc_t]","targ7[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");

} else {
	&PolLoop("cal2[tsys,pc]","targ7[tsys,ps,ps,ps]");
}


$LST_start=0; $LST_end=1.87;
if($simul eq "y") {
        &PolLoop("cal2[tsys,pc,pc_t]","targ7[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");

} else {
	&PolLoop("cal2[tsys,pc]","targ7[tsys,ps,ps,ps]");
}


print "################################\n";
print "----- Flux loop on Callisto-----\n";
print "################################\n";
$LST_start=1.9; $LST_end=2.2;
if($simul eq "y") {
        &PolLoop("cal5[tsys,pc,pc_t,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
        &PolLoop("cal5[tsys,pc,pc,pc,pc]");
}


print "#########################################\n";
print "##### Polarization Calibration loop #####\n";
print "#########################################\n";
$LST_start=2.2; $LST_end=5.0;
if($simul eq "y") {
        &PolLoop("cal4[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal4[tsys,ps,ps,ps]");
}

print "----- Congratulations!  This is the end of the script.  -----\n";
