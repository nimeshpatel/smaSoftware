#!/usr/bin/perl -w
#
#  Before running this script, please enable real-time tilt corrections
#  on half of the antennas = 1,2,3,4.  This script is meant to be run
#  during second shift in May/June 2006 (LST=19-24)

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("radio");
$limit = 20;
while (1) { 
  command("tsys");
  if (checkEl("2232+117") > $limit) {
    command("observe -s 2232+117");
    command("point -i 40 -r 2 -L -l -t -s -Q");
    command("integrate -s 10 -t 30 -w");
  }
  if (checkEl("3c446") > $limit) {
    command("observe -s 3c446");
    command("integrate -s 10 -w");
  }
  if (checkEl("bllac") > $limit) {
    command("observe -s bllac");
    command("integrate -s 10 -w");
  }
  if (checkEl("3c345") > $limit) {
      if (checkEl("3c345") < 85) {
	  command("observe -s 3c345");
	  command("integrate -s 10 -w");
      }
  }
  if (checkEl("1751+096") > $limit) {
    command("observe -s 1751+096");
    command("integrate -s 10 -w");
  }
  command("tsys");
  if (checkEl("pallas") > $limit) {
    command("observe -s pallas");
    command("integrate -s 10 -w");
  }
  if (checkEl("1924-292") > $limit) {
    command("observe -s 1924-292");
    command("integrate -s 10 -w");
  }
  if (checkEl("2148+069") > $limit) {
    command("observe -s 2148+069");
    command("integrate -s 10 -w");
  }
  if (checkEl("ceres") > $limit) {
    command("observe -s ceres");
    command("integrate -s 10 -w");
  }
}
