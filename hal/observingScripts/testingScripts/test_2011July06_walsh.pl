#!/usr/bin/perl -w
#
# Experiment Code: 2011A-S031
# Experiment Title: Filaments, Star Formation and Magnetic Fields
# PI: Qizhou Zhang
# Contact Person: Qizhou Zhang/Ya-Wen Tang/Keping Qiu/Hauyu Lu
# Email  : qzhang@cfa.harvard.edu/tang@obs.u-bordeaux1.fr/kqiu@mpifr-bonn.mpg.de/baobabyoo@gmail.com
# Array  : subcompact
############## SPECIAL INSTRUCTIONS ################
#
# Integration time should be 15s during track
# Use single receiver, 4GHz bandwidth mode
# Please insert a gain calib manually in the end of each main loop!!
##### PLEASE CHANGE NUMBER OF ANTENNAS IN NEXT LINE IF NEEDED
$nants="7";
#
################## Priming  ###################################
#
# observe -s w51e2 -r 19:23:43.95 -d 14:30:34.0 -e 2000 -v 58.0
# dopplerTrack -r 345.3397599 -u -s 16 -R h -r 345.3397599 -u -s 16
# restartCorrelator -R l -s64 -R h -s64
# setFeedOffset -f 345
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
$targ0="ngc6334i -r 17:20:53.44 -d -35:47:02.2 -e 2000 -v -8.0"; 
$targ1="w51e2 -r 19:23:43.95 -d 14:30:34.0 -e 2000 -v 58.0"; 
$targ2="dr21oh -r 20:39:01.2 -d 42:22:48.5 -e 2000 -v -3.5"; 

# Gain
$cal0="1733-130";
$cal1="1751+096";
$cal2="mwc349a";

# pol & bp
$cal3="3c279";
$cal4="3c454.3";

# flux
$cal5="Callisto";
$cal6="Callisto";

$tsys="tsys";

$MINEL_TARG = 32; $MAXEL_TARG = 87;
$MINEL_GAIN = 32; $MAXEL_GAIN = 87;
$MINEL_FLUX = 32; $MAXEL_FLUX = 87;
$MINEL_BPASS= 32; $MAXEL_BPASS= 87;
$MINEL_CHECK= 33;
#########################################################
#########################################################
do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");

if($simul eq "y") {command("integrate -t 20");}

$myPID=$$;
command("project -r -p 'Qizhou Zhang' -d ' 2011A-S031'");
print "####################################################\n";
print "######### initialization done, starting script #####\n";
print "######### Polarization Calibration loop ############\n";
print "####################################################\n";
$LST_start=11; $LST_end=15.4;
if($simul eq "y") {
        &PolLoop("cal3[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal3[tsys,ps,ps,ps]");
}


print "###############################################\n";
print "########### Flux loop on Titan ################\n";
print "###############################################\n";

$LST_start=15.4; $LST_end=15.8;
if($simul eq "y") {
        &PolLoop("cal5[tsys,pc,pc_t,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
        &PolLoop("cal5[tsys,pc,pc,pc,pc]");
}


print "####################################################\n";
print "######### initialization done, starting script #####\n";
print "######### Polarization Calibration loop ############\n";
print "####################################################\n";
$LST_start=15.8; $LST_end=16.4;
if($simul eq "y") {
        &PolLoop("cal3[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal3[tsys,ps,ps,ps]");
}


print "#############################################################\n";
print "########### Main loop part 1: NGC6334I ######################\n";
print "#############################################################\n";

$LST_start=16.4; $LST_end=18.4;
if($simul eq "y") {
        &PolLoop("cal0[tsys,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal0[tsys,pc]","targ0[tsys,ps,ps,ps]");
}

print "###############################################\n";
print "######## Main loop part 2: W51e2 ##############\n";
print "###############################################\n";

$LST_start=18.4; $LST_end=20.0;
if($simul eq "y") {        
        &PolLoop("cal1[tsys,pc,pc_t]","targ1[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal1[tsys,pc]","targ1[tsys,ps,ps,ps]");
}

print "#############################################################\n";
print "############ Main loop part 3: DR21OH  ######################\n";
print "#############################################################\n";
$LST_start=20.0; $LST_end=21.5;
if($simul eq "y") {
        &PolLoop("cal2[tsys,pc,pc_t]","targ2[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal2[tsys,pc]","targ2[tsys,ps,ps,ps]");
}


print "###############################################\n";
print "######### Polarization Calibration loop #######\n";
print "###############################################\n";
$LST_start=21.5; $LST_end=23.99;
if($simul eq "y") {
        &PolLoop("cal4[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal4[tsys,ps,ps,ps]");
}

print "###############################################\n";
print "########### Flux loop on Callisto #############\n";
print "###############################################\n";

$LST_start=0.0; $LST_end=0.55;
if($simul eq "y") {
        &PolLoop("cal6[tsys,pc,pc_t,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
        &PolLoop("cal6[tsys,pc,pc,pc,pc]");
}


print "###############################################\n";
print "######### Polarization Calibration loop #######\n";
print "###############################################\n";
$LST_start=0.55; $LST_end=10.0;
if($simul eq "y") {
        &PolLoop("cal4[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal4[tsys,ps,ps,ps]");
}


print "----- Congratulations!  This is the end of the script.  -----\n";
