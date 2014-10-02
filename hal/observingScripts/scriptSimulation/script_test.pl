#!/usr/bin/perl -w
#
# Experiment Code: 2009B-S028
# Experiment Title: Magnetic Field in Massive Star-Forming Filaments
# PI: K. Qiu
# Contact Person: K. Qiu, Q. Zhang
# Email  : kqiu@cfa.harvard.edu
#          qzhang@cfa.harvard.edu
# Office : +49-228-525-491 (Qiu) 617-496-7655 (Zhang)
# Home   : +49-228-525-491 (Qiu)
# Array  : compact
############## SPECIAL INSTRUCTIONS ################
#
# Integration time should be 15s during track.
#
##### PLEASE CHANGE NUMBER OF ANTENNAS IN NEXT LINE IF NEEDED
$nants="7";
#
####################  PRIMING  ####################
#
# restartCorrelator -s128
# tune -c "waveplate in"
# tune -c "hotload in"
# observe -s I18360 -r 18:38:40.74 -d -05:35:04.2 -e 2000 -v 103.5
# dopplerTrack -r 345.796000 -u -s 3
# setFeedOffset -f 345
#
###################   POINTING  ###################
#
# ipoint on 3c273 at the beginning of the track
# ipoint on 1924-292 around .5 hr after sunrise 
# and repeat every 1hr
#
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

#source
$targ0="I18360 -r 18:38:40.74 -d -05:35:04.2 -e 2000 -v 103.5";
$targ1="G35.2N -r 18:58:12.94 -d +01:40:37.1 -e 2000 -v 103.5";

# Gain
$cal0="1751+096";
$cal1="1830+063";

#pol & bp
$cal2="3c273";
$cal3="3c454.3";

#flux
$cal4="Titan";
$cal5="Neptune";

$tsys="tsys";

$MINEL_TARG = 17; $MAXEL_TARG = 87;
$MINEL_GAIN = 17; $MAXEL_GAIN = 87;
$MINEL_FLUX = 17; $MAXEL_FLUX = 87;
$MINEL_BPASS= 17; $MAXEL_BPASS= 87;
$MINEL_CHECK= 23;
##############################
###########################

do 'sma.pl';
do 'sma_add.pl';
checkANT();
command("radio");
if($simul eq "y") {command("integrate -t 20");}

$myPID=$$;
command("project -r -p 'K. Qiu' -d '2009B-S028'");
print "----- initialization done, starting script -----\n";

print "----- BP & Polarization Calibration loop -----\n";
$LST_start=9.0; $LST_end=12.3;
if($simul eq "y") {
       &PolLoop("cal2[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
       &PolLoop("cal2[tsys,ps,ps,ps]");
}

print "----- Flux loop in between BP -----\n";
$LST_start=12.3; $LST_end=12.5;
if($simul eq "y") {
&PolLoop("cal4[tsys,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
       &PolLoop("cal4[tsys,pc,pc,pc]");
}

print "----- BP & Polarization Calibration loop -----\n";
$LST_start=12.5; $LST_end=14.4;
if($simul eq "y") {
       &PolLoop("cal2[tsys,ps,ps_t,ps,ps_t]");
} else {
       &PolLoop("cal2[tsys,ps,ps]");
}

print "----- Source Cal loop 1 -----\n";
$LST_start=14.4; $LST_end=22.4;
if($simul eq "y") {
&PolLoop("cal0[tsys,pc,pc_t,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]","cal0[tsys,pc,pc_t,pc,pc_t]","targ1[tsys,ps,ps_t,ps,ps_t,ps,ps_t]","cal1[tsys,pc,pc_t,pc,pc_t]");
} else {
&PolLoop("cal0[tsys,pc,pc]","targ0[tsys,ps,ps,ps]","cal0[tsys,pc,pc]","targ1[tsys,ps,ps,ps]","cal1[tsys,pc,pc,pc]");
}

print "----- Source Cal loop 2 -----\n";
$LST_start=22.4; $LST_end=22.9;
if($simul eq "y") {
&PolLoop("cal1[tsys,pc,pc_t,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]","cal1[tsys,pc,pc_t,pc,pc_t]","targ1[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
&PolLoop("cal1[tsys,pc,pc]","targ0[tsys,ps,ps,ps]","cal1[tsys,pc,pc]","targ1[tsys,ps,ps,ps]");
}

print "----- Additional flux loop -----\n";
$LST_start=22.9; $LST_end=23.1;
if($simul eq "y") {
&PolLoop("cal5[tsys,pc,pc_t,pc,pc_t,pc,pc_t]");
} else {
        &PolLoop("cal5[tsys,pc,pc,pc]");
}

print "----- Additional BP & Polarization Calibration loop -----\n";
$LST_start=23.1; $LST_end=25.6;
if($simul eq "y") {
       &PolLoop("cal3[tsys,ps,ps_t,ps,ps_t]");
} else {
       &PolLoop("cal3[tsys,ps,ps]");
}

print "----- Congratulations!  This is the end of the script.  -----\n";
