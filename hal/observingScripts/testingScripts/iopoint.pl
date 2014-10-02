#!/usr/bin/perl -w
# Experiment Title: iopointing  at 230 GHz on a bright QSO and a nearby
#                   star.
# this script is a modified version of nightly230.pl
#
# PI: SMA Pointing Team
# Author: Nimesh Patel (original script by Todd Hunter).
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597
#
# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("radio");  
LST();
$targel=checkEl("3c454.3");
if ($targel>17. && $targel<87.) { $sourceUp=1;}
else {$sourceUp=0.;}
while($sourceUp==1) {
LST();
$targel=checkEl("3c454.3");
if ($targel>17. && $targel<87.) {
$sourceUp=1;} else {$sourceUp=0;}
command("observe -s 3c454.3");
command("point -i 60 -r 1 -L -l -t -s");
command("optical");
command("observe -s hip112997");
command("antennaWait");
#command("c90cmd -c 'snapshot -e 200'");
command("snapshot -s 200");
command("radio");
}
