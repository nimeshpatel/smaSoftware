#!/usr/bin/perl -w
# Experiment Title: iopointing on a quasar and Uranus 
# while acquiring optical images on a nearby stars
# to check repeatability of both radio and optical pointing.
# (and to detect jumps if any).  
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597 
# cellphone: 617 669 5665.
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
command("observe -s 3c279");

$star="hip64238";   # nearby star for 3c279 (Mv=4.4)
$exposure=100; 

#starting infinite loop

for(;;) {
  command("radio");
  command("tsys");
  $targel=checkEl("3c279");
  if ($targel>18.) {
    command("observe -s 3c279");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical");
    command("observe -s $star"); 
    command("antennaWait -s $star");
    command("snapshot -s $exposure"); 
# Note: if the snapshot command on hal appears to be broken, 
# you can use the low-level command in the server instead:
#    command("c90cmd -c 'snapshot -s $exposure'"); 
  }
}
