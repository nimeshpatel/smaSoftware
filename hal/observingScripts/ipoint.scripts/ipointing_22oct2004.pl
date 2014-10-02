#!/usr/bin/perl -w
# Experiment Title: ipointing on 2230+114
# After about 10:40PM HST, 0359+509 will rise above 18deg and will be
# added to the cycle
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597
#

# Assumptions: ipointing has been checked at least once before starting
# this script, and offsets updated. 
#
# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("observe -s 0359+509");

#starting infinite loop
command("radio");  
for(;;) {
  command("tsys");
  command("observe -s 0359+509");
  command("point -i 60 -r 1 -L -l -t -s");
}
