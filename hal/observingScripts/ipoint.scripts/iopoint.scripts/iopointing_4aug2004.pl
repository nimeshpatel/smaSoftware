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
$sideband="l";
$block=4;
$chunk=4;
$width=70;
$linecenter=65;
$exposure=400; #ccd exposure for M0, mv=6 star in msec, change if required.
####################

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("tsys");
command("setFeedOffset -f 230");
command("observe -s mwc349");

#starting infinite loop
for(;;) {
	command("radio");  
	command("tsys");
	command("observe -s mwc349");
	command("point -p -l -t -s -S $sideband -b $block -c $chunk -w $width -C $linecenter");
	command("optical"); 
	command("observe -s hip100501"); 
	command("antennaWait -s hip100501");
	command("snapshotTemp -s $exposure"); 
	
	$targel=checkEl("mwc349");
	if($targel<15.) {last;}
}
