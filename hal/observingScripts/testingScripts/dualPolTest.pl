#!/usr/bin/perl -w
#
# Experiment Code: Dual RX pol test
# Experiment Title: Dual RX pol test
# PI: Ken Young
# Array  : compact
############## SPECIAL INSTRUCTIONS ################
#
# Integration time should be 15s during track
# Make sure using walsh cycle modulation
#
##### PLEASE CHANGE NUMBER OF ANTENNAS IN NEXT LINE IF NEEDED
$nants="7";
#
################## Priming  ###################################
#
# observe -s iras4a -r 03:29:10.51 -d +31:13:31.0 -e 2000 -v 6.7
# dopplerTrack -r 345.3397599 -u -s 16 -R h -r 345.3397599 -u -s 16
# restartCorrelator -R l -s64 -R h -s64
# tune -c "waveplate in"
# tune -c "hotload in"
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
$targ0="iras4a -r 03:29:10.51 -d +31:13:31.0 -e 2000 -v 6.7"; 

# Gain, pol & bp
$cal0="3c84";

# additional pol & bp
$cal1="3c454.3";
$cal2="0854+201";

# flux
$cal4="titan";
$cal3="Callisto";

$tsys="tsys";

$MINEL_TARG = 17; $MAXEL_TARG = 87;
$MINEL_GAIN = 17; $MAXEL_GAIN = 87;
$MINEL_FLUX = 17; $MAXEL_FLUX = 87;
$MINEL_BPASS= 17; $MAXEL_BPASS= 87;
$MINEL_CHECK= 23;

#########################################################
#########################################################
do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");

if($simul eq "y") {command("integrate -t 20");}

$myPID=$$;
command("project -r -p 'Ken Young' -d 'test'");
print "####################################################\n";
print "######### initialization done, starting script #####\n";
print "####### Polarization Calibration loop  on 3c454 ####\n";
print "####################################################\n";
$LST_start=15; $LST_end=23.5;
if($simul eq "y") {
        &PolLoop("cal1[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal1[tsys,ps,ps,ps]");
}


print "###############################################\n";
print "########### Flux loop on callisto ################\n";
print "###############################################\n";

$LST_start=23.00; $LST_end=24.0;
if($simul eq "y") {
        &PolLoop("cal3[tsys,pc,pc_t,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
        &PolLoop("cal3[tsys,pc,pc,pc,pc]");
}

print "#############################################################\n";
print "########### Main loop on NGC1333 IRS4a ######################\n";
print "#############################################################\n";

$LST_start=23.8; $LST_end=2.5;
if($simul eq "y") {
        &PolLoop("cal0[tsys,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal0[tsys,ps]","targ0[tsys,ps,ps,ps]");
}


print "###############################################\n";
print "######### Polarization Calibration loop #######\n";
print "###############################################\n";
$LST_start=2.3; $LST_end=4;
if($simul eq "y") {
        &PolLoop("cal0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal0[tsys,ps,ps,ps]");
}

print "#############################################################\n";
print "########### Main loop on NGC1333 IRS4a ######################\n";
print "#############################################################\n";
$LST_start=3.8; $LST_end=8.1;
if($simul eq "y") {
        &PolLoop("cal0[tsys,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
        &PolLoop("cal0[tsys,ps]","targ0[tsys,ps,ps,ps]");
}

print "###############################################\n";
print "########### Flux loop on titan    #############\n";
print "###############################################\n";

$LST_start=8; $LST_end=9.0;
if($simul eq "y") {
        &PolLoop("cal4[tsys,pc,pc_t,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
        &PolLoop("cal4[tsys,pc,pc,pc,pc]");
}



print "----- Congratulations!  This is the end of the script.  -----\n";
