#!/usr/bin/perl -w
#  
#       ############################################## 
#       #  Please fill in the following information  #
#       #  identifying your project:                 #
#       ##############################################
# Experiment Code: 2004-035
# Experiment Title: Internal Structure and Motions of High-Mass Starless
#                   Cores
# PI: T. K. Sridharan
# 
# Contact person: T. K. Sridharan
# Office phone: 617.495.7090            Evening phone: 617 666 1246
#
# Have you submitted a cover letter with special instructions
# for your project?                      no
# 
############################################################
# 
#       ##############################################
#       #  The following information is required to  #
#       #  ensure the correct correlator setup and   #
#       #  receiver tuning for your project.         #
#       ##############################################
#
#
# What is the exact rest frequency of your line? 225.697775 
#
# Which sideband do you wish to put your line in?    USB
#
# Which chunk do you wish to center your 
#   line in?  s01
#
# Do you require higher spectral resolution in the 
#   chunk in which your line appears?                yes
#
# correlator setup:
# hal9000> restartCorrelator -d -s 1:512 -s 5:512 -s 10:512 -s 13:512 -s 17:512 -s 22:512 -s 128
# 
# Doppler tracking:
#  hal9000> standby
#  hal9000> observe -s hmsc1 -r 18:41:17.3 -d -05:10:04 -e 2000 -v 46.0
#  hal9000> dopplerTrack -r 226.68 -u -s 13
#

#############################################################
#
#       ###############################################
#       #  In the following section, you need to set  #
#       #  the variables which will be used by the    #
#       #  observing script.  Please enter the        #
#       #  J2000 coordinates of your target source    #
#       #  and the LSR velocity (if any) in case the  #
#       #  source is not in the SMA catalog.          #
#       #  Enter the source names within quotes, and  #
#       #  the number of scans without quotes.        #
#       #  This script will observe your primary      #
#       #  calibrator, target source, secondary       #
#       #  calibrator, target source, primary cal,    #
#       #  etc, in that order.  If you wish to use a  #
#       #  different loop, please send a cover letter #
#       #  specifying how you'd like to set it up.    #
#       #  The script will also observe a bandpass    #
#       #  calibrator and a flux calibrator.  Please  #
#       #  indicate the names of your calibrators     #
#       #  and the number of scans you would like on  #
#       #  each.  This script also has the ability to #
#       #  add real-time pointing updates, if desired.#
#       #  LST times in the script are approximate    #
#       #  and may be changed by the scheduler.       #
#       ###############################################
#
#  If you wish to incorporate real-time pointing updates, change $ptg to 1 
#  and indicate how many cycles of the script between pointing checks:
# 
$ptg=1;  # 0 is for NO pointing. 1 (or any non-zero number) will do pointing.
$ncycle=3;
#
#  Note: pointing is only prudent if there is a source brighter
#  than 5 Jy near your target.  If one of your calibrators is
#  bright enough, you may use the same source for pointing.
#
#
# enter correlator integration time for each scan in seconds (usually 30 or 60)
$inttime=30;
#
# sources and number of integrations on them
$targ="hmsc1 -r 18:41:17.3 -d -05:10:04 -e 2000 -v 46.0";    $ntarg = 40;

#
# Please provide precise coords for the target for the scheduler's information:
# Target RA:             Dec:               Vlsr:
# hsmc1 18 41 17.3  -05 10 04             46 
#
#
$cal1="1741-038";  $ncal1= 6; # primary calibrator;  number of scans
$cal3="1741-038";  $nrep3 = 3; # pointing source; number of reps 
$bpass1="jupiter"; $nbpass1= 60;# bandpass calibrator;  number of scans
$bpass2="uranus"; $nbpass2= 120;# bandpass calibrator;  number of scans
#
# The following source may not be necessary for all projects:
# If your target or primary calibrator will transit higher than 85 degrees
# in elevation, please choose a source within the range reachable at that time
# to observe during transit:
#
#$transcal="source name"; #source to be observed if target goes above el limit
#
# If you are using real-time pointing checks, and wish to do pointing before 
# observing a resolved source for bandpass calibration, indicate below
# i.e. if bpass is Jupiter, you may wish to use Callisto.  If you're using
# Callisto as a flux calibrator, additional pointing is probably unnecessary.
#
#$bptg="source name"; $nbptg=2; # pointing source for bpass, if different
#
#########################################################################
#                                                                       #
#   If you edit below this line, please send a cover letter indicating  #
#   any changes you have made.                                          #
#                                                                       #
#########################################################################

# initialization

# load the sma library of functions used in this observing script.
do 'sma.pl';

# check participating antennas.
checkANT();

# just in case antennas are left in optical mode from opointing.
command("radio");

command("setFeedOffsets -f 230");

# set default integration time for the correlator.
command("integrate -t $inttime");

print "----- initialization done, starting script -----\n";

# 1st bandpass calibration
if (LST(1) > 14 and LST() < 15.5) {
    command("observe -s $bpass1"); 
    command("tsys");
    command("integrate -s $nbpass1 -w -b");
}

# initial pointing on pointing source
if (LST() > 15) {
   command("observe -s $cal3"); 
   command("tsys");
   command("point -i 60 -r $nrep3 -L -l -t -s");
}

# loop through cal1 and cal2, and targs
while(LST(1) > 14 and LST() < 19){ 
    $targel=checkEl($targ);
    $calel =checkEl($cal1);
    if ($targel > 20 and $calel > 19) {
            printPID();
            $nloop=$nloop+1;  print "Loop No.= $nloop\n";

            command("observe -s $cal1");  command("tsys");
            command("integrate -s $ncal1 -w");
            command("observe -s $targ"); command("tsys");
            command("integrate -s $ntarg -w");

    } else {
     print "Target or primary calibrator is below 20 degrees, finishing loops.\n";
        last;
    }
}
# do a pointing check
if (LST() > 18.5) {
   command("observe -s $cal3"); 
   command("tsys");
   command("point -i 60 -r 2 -L -l -t -s");
}

# continue observing: loop through cal1 and cal2, and targs
while(LST(1) > 18.5 and LST() < 23.9){ 
    $targel=checkEl($targ);
    $calel =checkEl($cal1);
    if ($targel > 20 and $calel > 19) {
            printPID();
            $nloop=$nloop+1;  print "Loop No.= $nloop\n";

            command("observe -s $cal1");  command("tsys");
            command("integrate -s $ncal1 -w");
            command("observe -s $targ"); command("tsys");
            command("integrate -s $ntarg -w");

    } else {
     print "Target or primary calibrator is below 20 degrees, finishing loops.\n";
        last;
    }
}

# final calibration
            command("observe -s $cal1");  command("tsys");
            command("integrate -s $ncal1 -w");

#  final flux-passband calibration with pointing on Uranus.
#  repeat point/integration cycle 2 times and the integrate for
#  passband.
      command("observe -s $bpass2");  
      command("tsys");
      command("point -i 60 -r 2 -L -l -t -s");
      command("integrate -s $nbpass2 -w -b");
# bye-bye message
print "----- Congratulations! This script is done. -----\n";
print "Please take tilt data, Thanks!\n";
print "command: tilt -e 84 -d cw -r 1 and after this is done... \n";
print "command: tilt -e 84 -d ccw -r 1\n";

