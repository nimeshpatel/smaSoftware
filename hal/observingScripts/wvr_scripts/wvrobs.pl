#!/usr/bin/perl -w
#
# Things to make sure are that the combiner mirror is pointing at the WVR
# tune -a 4,5 -c "combiner -r wvr -m"
#
# The radiometers need to be logging data. This should be automatic although if
# you can start a new data file it would make matching up the data simpler.
# (I'll be online so just 2op me when your about to start the script.
# 
#
# Here is the goal of this script: 
# 1. Carry out two skydips to indicate conditions
# 2. Carry out a 60 minute observation of 3c273 with the correlator running at
#    2.5 seconds at 13MHz resolution   (restartCorrelator -s8)
# 3. Observe with the LO at ....

#########################################################################
#                                                                       #
#                  Issue these commands first!!!                        #
#                                                                       #
#########################################################################
#                                                                       #
#   standby                                                             #
#   tune -a 4,5 -c "combiner -r wvr -m"                                 #
#   restartCorrelator -d -s8                                            #
#   observe  -s 3c273                                                   #
#   dopplerTrack -r 231.5 -l -s12                                       #
#                                                                       #
#########################################################################

do 'sma.pl';
printPID();
checkANT();
$limit = 30;
@elevations = (85, 80, 70, 60, 50, 40, 30, 20, 30, 40, 50, 60, 70, 80, 85);

for($i=0; $i<2; $i++) {
	foreach $el (@elevations) {
		command("az -a 3,4,5,6 -d 10");
		command("el -a 3,4,5,6 -d $el");
		sleep 20;
	}
}

if (checkEl("3c273") > $limit) {
      command("observe -a 3,4,5,6 -s 3c273");
      command("tsys");
      command("newFile");
      command("integrate -t 2.5 -s 1440 -w");
}
