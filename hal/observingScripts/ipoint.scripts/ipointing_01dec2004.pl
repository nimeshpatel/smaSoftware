#!/usr/bin/perl -w
# Experiment Title: ipointing on 2230+114 and Uranus 
# (to detect jumps if any).  When 2230 gets too low, it switches
# over to 0359+509.
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597 
# cellphone: 617 669 5665.
#
# Note to observers: if the sky is clear, then run instead
#      the script iopointing_01dec2004.pl
#
# Assumptions: ipointing has been checked at least once before starting
# this script, and offsets updated.  


# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("observe -s 2230+114");
command("radio");
$targ1 = "2230+114";
$targ1a = "0359+509";
$targ2 = "uranus";

#starting infinite loop
for (;;) {
  command("tsys");
  $targel=checkEl("$targ1");
  if ($targel>22.) {
    command("observe -s $targ1");
    command("point -i 60 -r 1 -L -l -t -s");
  } else {
    command("observe -s $targ1a");
    command("point -i 60 -r 1 -L -l -t -s");
  }
  command("tsys");
  $targel=checkEl("$targ2");
  if ($targel>20.) {
    command("observe -s $targ2");
    command("point -i 60 -r 1 -L -l -t -s");
  }
}
