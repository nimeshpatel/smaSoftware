#!/usr/bin/perl -w
# Experiment Title: iopointing on 2230+114 and hip112029
# while acquiring optical images on a nearby star
# to check repeatability of both radio and optical pointing.
# After about 10:40PM HST, 0359+509 will rise above 18deg and will be
# added to the cycle
# After about 11:50PM HST, 0423-013 will rise above 18deg and will be
# added to the cycle.
# All three objects will remain high enough to observe until the script
# is ended manually by the observers at about 3:15AM HST.
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597
#
# Note to observers: if the sky is not clear, please comment out all
# reference to optical pointing within this script.
#
# Assumptions: ipointing has been checked at least once before starting
# this script, and offsets updated.  Run 'showStars' on smadata to see
# the optical images.
#

####################
# set the following variables correctly before starting the script
#
$exposure=90;      # ccd exposure for Mv=3.4 star B8.5
$star="hip112029";     # nearby star for 2230+114 (Mv=3.4)
$startwo="hip18212";   # nearby star for 0359+509 (Mv=5.8)
$starthree="hip20341"; # nearby star for 0423-013 (Mv=5.9)
####################

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("observe -s 2230+114");

#starting infinite loop
command("radio");  
for(;;) {
  command("tsys");
  command("observe -s 2230+114");
  command("point -i 60 -r 1 -L -l -t -s");
  command("optical"); 
  command("observe -s $star"); 
  command("antennaWait -s $star");
  command("snapshot -s $exposure -c"); 
  command("radio");  

  $targel=checkEl("0359+509");
  if ($targel>18.) {
    command("observe -s 0359+509");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical"); 
    command("observe -s $startwo"); 
    command("antennaWait -s $startwo");
    command("snapshot -s 200 -c"); 
    command("radio");  
  }
  $targel=checkEl("0423-013");
  if ($targel>18.) {
    command("observe -s 0423-013");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical"); 
    command("observe -s $starthree"); 
    command("antennaWait -s $starthree");
    command("snapshot -s 200 -c"); 
    command("radio");  
  }
}
