#!/usr/bin/perl -w
#
# Experiment Code: 2010A-A008
# Experiment Title: Do magnetic fields regulate low mass star formation in NGC 1333?
# PI: Ramprasad Rao
# Contact Person: Ramprasad Rao
# Email  : rrao@sma.hawaii.edu
# Office : 1 808 961 2935
# Cell   : 1 808 933 2973
# Home   : 1 804 286 4282 <---- Note different area code
# Array  : compact
############## SPECIAL INSTRUCTIONS ################
#
# Integration time should be 15s during track
# Script to be started at 1:30 LST.
##### PLEASE CHANGE NUMBER OF ANTENNAS IN NEXT LINE IF NEEDED
$nants="7";
#
####################  PRIMING  ####################
#
# restartCorrelator -s128
# tune -c "waveplate in"
# tune -c "hotload in"
# observe -s iras4a -r 03:28:55.58  -d +31:14:37.1 -e 2000 -v 7
# dopplerTrack -r 345.798 -u -s4
# setFeedOffset -f 345
#
###################   POINTING  ###################
#
# Pointing: Usual post-sunset pointing update
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
#
############### Check for computer ==> Observation/Simulation
chop($hostname = `hostname`) ;
if($hostname ne "hal9000") {
$simul = "y";
print "Running Simulation\n";
}

############### CHOOSE PATTERN BASED ON NUMBER OF ANTENNAS ##############
if($nants eq "7") {
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
#source
$targ0="1337-130";
# Gain
$cal0="3c279";

#pol & bp
$bpass1="3c273";

#flux
$flux0="callisto";
$flux1="neptune";

#Additional pol & b
$bpass0="3c279";

$tsys="tsys";

$MINEL_TARG = 17; $MAXEL_TARG = 87;
$MINEL_GAIN = 17; $MAXEL_GAIN = 87;
$MINEL_FLUX = 17; $MAXEL_FLUX = 87;
$MINEL_BPASS= 17; $MAXEL_BPASS= 87;
$MINEL_CHECK= 21;
##############################
###########################

do 'sma.pl';
do 'sma_add.pl';

checkANT();
command("radio");

if($simul eq "y") {command("integrate -t 20");}

$myPID=$$;
command("project -r -p 'Ramprasad Rao' -d '2010A-A008'");
print "----- initialization done, starting script -----\n";


#print "-----LST 13.8-14.1 please do pointing again !!-----"


print "----- Source Cal loop #1 -----\n";
if($simul eq "y") 
{
    &PolLoop("cal0[tsys,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} 
else 
{
    &PolLoop("cal0[tsys,pc]","targ0[tsys,ps,ps,ps]");
}


print "----- Congratulations!  This is the end of the script.  -----\n";
