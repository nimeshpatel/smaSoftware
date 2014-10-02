#!/usr/bin/perl -w
#
# Here is the goal of this script: (to be run 18Jan2006)
# 0. The cycle time is 5 minutes (2.5 minutes per source)
# 1. At the beginning, when Vesta is too low, observe RCas & 3C454.3
# 2. Later, when Vesta is near the same elevation of TX Cam, then
#    observe Vesta & TXCam.
# 3. Later still, when Titan is near the same elevation as TX Cam,
#    observe Titan & TXCam.
#
do 'sma.pl';
printPID();
checkANT();
$limit = 30;
for (;;) {
  command("tsys");
  if (checkEl("vesta") > 36) {
      if (checkEl("vesta") < 67) {
	  command("observe -s vesta");
	  command("integrate -t 15 -s 10 -w");
      } else {
	  command("observe -s titan");
	  command("integrate -t 15 -s 10 -w");
      }
      if (checkEl("txcam") > $limit) {
	  command("observe -s txcam");
	  command("integrate -t 15 -s 10 -w");
      }
  } else {
      if (checkEl("rcas") > $limit) {
	  command("observe -s rcas");
	  command("integrate -t 15 -s 10 -w");
      }
      if (checkEl("3c454.3") > $limit) {
	  command("observe -s 3c454.3");
	  command("integrate -t 15 -s 10 -w");
      }
  }
}
