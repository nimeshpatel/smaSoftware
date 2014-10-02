#!/usr/bin/perl -w
# Experiment Title: iopointing on mwc349 and hip100501
# Goal: Full track of ipointing run on mwc349, 
#	h30alpha line, spectral line mode
# while acquiring optical images on a nearby star
# to check repeatability of both radio and optical pointing.
# PI: SMA Pointing Team
# Reference: Pointing Tiger Team meeting of July 27, 2004.
# 	Testing meeting of July 28, 2004.
# 
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597
#
# Note to observers: if the sky is not clear, please comment out all
# reference to optical pointing within this script.
# or even better- do not run this script at all; you just need
# to run the point command repeatedly until mwc349 sets.

# Assumptions: ipointing has been checked at least once before starting
# this script, and offsets updated.

####################
# set the following variables correctly before starting the script
$exposure=170; #ccd exposure for M0, mv=6 star in msec, change if required.
$star="hip19167";
####################

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("tsys");
command("setFeedOffset -f 230");
command("observe -s 0355+508");

#starting infinite loop
command("radio");  
for(;;) {
	command("tsys");
	command("observe -s 0355+508");
	command("point -i 60 -r 1 -L -l -t -s -p");
	command("optical"); 
	command("observe -s $star"); 
	command("antennaWait -s $star");
	command("snapshot -s $exposure"); 
	command("radio");  
}
