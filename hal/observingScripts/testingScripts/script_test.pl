#!/usr/bin/perl -w

$ps="polarPattern -p 3 -w -c 1";
$pc="polarPattern -p 4 -w -c 1";

###### SIMULATION ############################
if($simul eq "y") {
       $ps_t="integrate -s 16 -w";
       $pc_t="integrate -s 8 -w";
}

############### SOURCE, CALIBRATOR and LIMITS ##############
#source
$targ0="IRC+10216";
# Gain
$cal0="0927+390";

#pol & bp
$bpass1="3c273";

#flux
$flux0="callisto";
$flux1="neptune";

#Additional pol & b
$bpass0="3c454.3";

$tsys="tsys";

$MINEL_TARG = 33; $MAXEL_TARG = 87;
$MINEL_GAIN = 33; $MAXEL_GAIN = 87;
$MINEL_FLUX = 33; $MAXEL_FLUX = 87;
$MINEL_BPASS= 33; $MAXEL_BPASS= 87;
$MINEL_CHECK= 33;
##############################
###########################

do 'sma.pl';
do 'sma_add.pl';
#print "error? $@\n";
checkANT();
command("radio");

if($simul eq "y") {command("integrate -t 20");}

$myPID=$$;
command("project -r -p 'SMA' -d 'Polar Test'");
print "----- initialization done, starting script -----\n";


#print "-----LST 13.8-14.1 please do pointing again !!-----"

print "----- BP & Polarization Calibration loop #1 -----\n";
$LST_start=7.0; $LST_end=12.0;
if($simul eq "y") {
       &PolLoop("cal0[tsys,pc,pc_t]","targ0[tsys,ps,ps_t,ps,ps_t,ps,ps_t]");
} else {
       &PolLoop("cal0[tsys,pc]","targ0[tsys,ps,ps,ps]");
}
