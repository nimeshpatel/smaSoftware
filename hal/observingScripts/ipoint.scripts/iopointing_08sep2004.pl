#!/usr/bin/perl -w
# Experiment Title: iopointing on 2230+114 and hip112029
# while acquiring optical images on a nearby star
# to check repeatability of both radio and optical pointing.
# PI: SMA Pointing Team
# 
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597
#
# Note to observers: if the sky is not clear, please comment out all
# reference to optical pointing within this script.
# or even better- do not run this script at all; you just need
# to run the point command repeatedly until the quasar sets.

# Assumptions: ipointing has been checked at least once before starting
# this script, and offsets updated.  Run 'showStars' on smadata to see
# the optical images.
#

####################
# set the following variables correctly before starting the script
#$exposure=170;    # ccd exposure for Mv=4.2 star A0IV
#$star="hip19167"; # nearby star for 0355+508
#
$exposure=90;      # ccd exposure for Mv=3.4 star B8.5
$star="hip112029"; # nearby star for 2230+114
####################

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("tsys");
command("setFeedOffset -f 230");
command("observe -s 2230+114");

#starting infinite loop
command("radio");  
for(;;) {
	command("tsys");
	command("observe -s 2230+114");
# removed the -p option in following because ipointing
# now converges very well after bug-fix
	command("point -i 60 -r 1 -L -l -t -s");
	command("optical"); 
	command("observe -s $star"); 
	command("antennaWait -s $star");
#        command("sleep 3");
	command("snapshot -s $exposure"); 
# Please replace the above snapshot command with the following
# one if the images need to be cleaned before centroiding (if
# online centroiding fails due to hot pixels.
#	command("snapshot -s $exposure" -c); 
	command("radio");  
}
