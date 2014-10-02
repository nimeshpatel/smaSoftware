#!/usr/bin/perl -I/application/lib -w
# Experiment Title: Nightly pointing at 230 GHz on a bright QSO and a nearby
#                   star, along with Polaris.
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597
#
# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("radio");  

# set elevation limit for optical stars
$elevLimit = 87.2; # 0.28 deg below the limit (i.e. 70 seconds of time) 

# $targel=checkEl("2230+114");  hip112029
$targel = checkEl("3c454.3");
if ($targel>25.0) {
    command("observe -s 3c454.3");
    command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
    command("optical");
    $targel = checkEl("hip112997");
    if ($targel < $elevLimit) {
	command("observe -s hip112997");
    }
} else {
#    $targel=checkEl("0359+509");
    $targel = checkEl("3c111");
    if ($targel>25.) {
#	command("observe -s 0359+509");
	command("observe -s 3c111");
	command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
	command("optical");
#  hip19811 is near 3c111
	$targel = checkEl("hip19811");
	if ($targel < $elevLimit) {
	    command("observe -s hip19811");
	}
#  hip19167 is near 0359+590
#	command("observe -s hip19167"); 
    } else {
	$targel = checkEl("3c273");
	if ($targel>25.) {
	    command("observe -s 3c273");
	    command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
	    command("optical");
	    $targel = checkEl("hip64238");
	    if ($targel < $elevLimit) {
		command("observe -s hip64238");
	    }
	} else {
	    $targel=checkEl("0851+202");
	    if ($targel > 25.0 && $targel<80.0) {
		command("observe -s 0851+202");
		command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
		command("optical");
		$targel = checkEl("hip42911");
		if ($targel < $elevLimit) {
		    command("observe -s hip42911");
		}
	    } else {
		command("observe -s 1924-292");
		command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
		command("optical");
		$targel = checkEl("hip95865");
		if ($targel < $elevLimit) {
		    command("observe -s hip95865");
		}
	    }
	}
    }
}
#
# The following command occasionally does not return the mixer bias
# voltage to the initial value on antenna 1.  I think I have fixed this
# problem in tune6. So let's try it again for awhile. - Todd July 19, 2006
command("tune -c iv0 &");
#
if ($targel < $elevLimit) {
  command("antennaWait");
  command("snapshot -s 200");
}
command("observe -s polaris");
command("antennaWait");
command("snapshot -s 100");
command("radio");
command("dip");
command("writeDSM -m hal9000 -n 230 -v DSM_HAL_HAL_NIGHTLY_POINTING_S");
#
# commented out by Todd
#command("tiltAzscan30.csh");
