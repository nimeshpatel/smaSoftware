#!/usr/bin/perl -w
# Experiment Title: iopointing on 0423 and hip21139
# while acquiring optical images on a nearby star
# to check repeatability of both radio and optical pointing.
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
# this script, and offsets updated.  Run 'showStars' on d2o to see
# the optical images.
#

####################
# set the following variables correctly before starting the script
#
#$star="hip21139";     # nearby star for 0423-013 (Mv=5.9)
####################

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("observe -s 2230+114");

$star="hip112029";     # nearby star for 2230+114 (Mv=3.4)
$exposure=100; 

$startwo="hip18212";   # nearby star for 0359+509 (Mv=5.8)
$exposuretwo=200; 

#starting infinite loop
command("radio");  
for(;;) {
  command("tsys");
  $targel=checkEl("0359+509");
  if ($targel>28.) {
    command("observe -s 0359+509");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical"); 
    command("observe -s $startwo"); 
    command("antennaWait -s $startwo");
    command("snapshot -s $exposuretwo"); 
    command("radio");  
  } else {
    command("observe -s 2230+114");
    command("point -i 60 -r 1 -L -l -t -s");
    command("optical"); 
    command("observe -s $star"); 
    command("antennaWait -s $star");
    command("snapshot -s $exposure"); 
    command("radio");  
  }
}
