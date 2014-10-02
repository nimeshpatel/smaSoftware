#!/usr/bin/perl -w
# Experiment Title: iopointing on 2230+114 and Uranus 
# while acquiring optical images on a nearby stars
# to check repeatability of both radio and optical pointing.
# (and to detect jumps if any).  When 2230 gets too low, switch
# over to 0359+509.
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597 
# cellphone: 617 669 5665.
#
# Note to observers: if the sky is not clear, then run instead
#      the script ipointing_01dec2004.pl
#
# Assumptions: ipointing has been checked at least once before starting
# this script, and offsets updated.  Run 'showStars' on d2o to see
# the optical images.
# (At least initially- note that showStars often hangs- need to hit Return
# to continue- if this becomes annoying, just kill showStars- it is not
# absolutely necessary to run it later on through the track as long as
# the guidescopes are working. 
#


# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("observe -s 2230+114");

$star1="hip112029";     # nearby star for 2230+114 (Mv=3.4)
$exposure=90; 

$star1a="hip111123";   # nearby star for 0359+509 (Mv=5.8)
$exposure1a=200; 


$star2="hip110000";   # nearby star (Mv=5.3) for Uranus
$exposure2=200; 

$targ1 = "2230+114";
$targ2 = "uranus";
$targ1a = "0359+509";

#starting infinite loop

for(;;) {
 
  command("radio");
  command("tsys");
  $targel=checkEl("$targ1");
  if ($targel>22.) {
    command("observe -s $targ1");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical");
    command("observe -s $star1"); 
    command("antennaWait -s $star1");
    command("snapshot -s $exposure"); 
  } else {
    command("observe -s $targ1a");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical");
    command("observe -s $star1a"); 
    command("antennaWait -s $star1a");
    command("snapshot -s $exposure1a"); 
  }

  command("radio");  
  command("tsys");
  $targel=checkEl("$targ2");
  if ($targel>20.) {
    command("observe -s $targ2");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical");
    command("antennaWait -s $targ2");
    command("snapshot -s $exposure2"); 
    command("observe -s $star2"); 
    command("antennaWait -s $star2");
    command("snapshot -s $exposure2"); 
  }
}
