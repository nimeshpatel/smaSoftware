#!/usr/bin/perl -I/application/lib -w
# Experiment Title: Nightly pointing at 345 GHz on the brightest QSO in
#                   the sky (3C454, if available) and a nearby star.
#                   Otherwise, only do Polaris.
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

$targel=checkEl("3c454.3");
if ($targel>30.) {
    command("observe -s 3c454.3");
    command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
    command("optical");
    $targel = checkEl("hip112997");
    if ($targel < $elevLimit) {
	command("observe -s hip112997");
	command("antennaWait");
	command("snapshot -s 200");
    }
} else {
  $targel=checkEl("3c111");
  if ($targel>30.) {
    command("observe -s 3c111");
    command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
    command("optical");
#  hip19811 is near 3c111
    $targel = checkEl("hip19811");
    if ($targel < $elevLimit) {
	command("observe -s hip19811");
	command("antennaWait");
	command("snapshot -s 200");
    }
  } else {
    $targel=checkEl("3c273");
    if ($targel>30.) {
      command("observe -s 3c273");
      command("point -D -i 60 -r 1 -L -l -t -f /data/engineering/pointing/nightly");
      command("optical");
      $targel = checkEl("hip64238");
      if ($targel < $elevLimit) {
	  command("observe -s hip64238");
	  command("antennaWait");
	  command("snapshot -s 200");
      }
    }
  }
}
# The following command occasionally does not return the mixer bias
# voltage to the initial value on antenna 1.  I think I have fixed this
# problem in tune6. So let's try it again for awhile. - Todd July 19, 2006
command("tune -c iv0 &");
#
command("optical");
command("observe -s polaris");
command("antennaWait");
command("snapshot -s 200");
command("radio");  
command("dip");
command("writeDSM -m hal9000 -n 345 -v DSM_HAL_HAL_NIGHTLY_POINTING_S");
