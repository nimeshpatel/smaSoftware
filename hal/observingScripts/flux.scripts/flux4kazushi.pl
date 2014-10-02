#!/usr/bin/perl -w
#
####################   INSTRUCTION  ###################
# This is a flux/calibrator-survey script for June 2009, CMP, in 230 band.
# It looks at potential calibrotors that my VEX project may use.
# 
# (1) The script can start anytime between 12-19h LST. 
# (2) Please let it do one or two loops. One loop takes about 30 min
#     to go through four vla*** sources. The scipt auto-terminates after 3 loops.
# (3) Please manually get some integrations (say 5-10min) on either 3C273,
# 3C454.3, Uranus, or Neptune for flux calibration.
#
# Contact Person: Kazushi Sakamoto  
# Email  : ksakamoto@asiaa.sinica.edu.tw 
#
####################  PRIMING  ####################  
#
# Nothing special is needed. Just a flux track.
# (can use the same setting as the adjacent science track.)
#
####################   POINTING  ###################
# Pointing: 
# If pointing isn't confirmed beforehand, a quick ipoint on nearby 
# 3C345 would be useful.
#   "ipoint -r 2 -i 30 -k" 
# would be suffice for 230.
#
############################################################
################ SOURCE, CALIBRATOR and LIMITS ##############
$inttime=30;
@vla=('vla1532 -r 15:32:46.345 -d +23:44:05.27 -e 2000. -v 0.0',
      'vla1606 -r 16:06:58.300 -d +27:17:05.58 -e 2000. -v 0.0',
      'vla1610 -r 16:10:42.027 -d +24:14:49.01 -e 2000. -v 0.0',
      'vla1522 -r 15:22:09.992 -d +31:44:14.38 -e 2000. -v 0.0');
$ntarg = 10;
#
$cal1='3c345';     $ncal1= 7; # primary calibrator;  number of scans
$cal2='1635+381';  $ncal2= 7; # calibrator;  number of scans
$flux1='uranus';   $nflux1=10; # flux calibrator; number of scans
$flux2='neptune';  $nflux2=10; # flux calibrator; number of scans
#
#########################################################################
# initialization
do 'sma.pl'; # load libraries
checkANT();  # check participating antennas
command('radio'); # set pointing mode
command("integrate -t $inttime"); # set default integration time at the correlator.

print "----- initialization done, starting script -----\n";




print "\n";
print "#################################\n";
print "# Flux/Calibrator Survey begins #\n";
print "#################################\n";
print "\n";

MAIN_LOOP:
while(LST(1) > 12 and LST() < 20.0){
   printPID();
   $nloop=$nloop+1;
   print "--- Loop No.= $nloop ---\n";

   # observe cal1
   LST(); ##
   command("observe -s $cal1");  command("tsys");
   command("integrate -s $ncal1 -t $inttime -w");
   # observe cal2
   LST(); ##
   command("observe -s $cal2");  command("tsys");
   command("integrate -s $ncal2 -t $inttime -w");
   
   # observe the possible calibrators
   foreach $qso (@vla) {
      LST(); ##     
      if (checkEl($qso) < 15.0) {last MAIN_LOOP};
      command("observe -s $qso");
      command("integrate -s $ntarg -t $inttime -w");
   }
  
  # 
  if ($nloop == 3) {last MAIN_LOOP};
}

print " ------ script ended ------\n";
print "  If time permits, please manually integrate for several min \n";
print "  on either 3C273, 3C454.3, Uranus, or Neptune \n";
print "  for flux calibration. Thank you! \n";
#########################################################################
