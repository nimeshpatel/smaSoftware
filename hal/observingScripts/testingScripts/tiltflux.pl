#!/usr/bin/perl -w
#
#  Before running this script, please enable real-time tilt corrections
#  on half of the antennas = 1,2,3,4.  This script is meant to be run
#  during first shift in May/June 2006 (LST=10-18)

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
  if (checkEl("3c279") > $limit) {
    command("observe -s 3c279");
    command("point -i 40 -r 2 -L -l -t -s -Q");
    command("integrate -s 10 -t 30 -w");
  }
  if (checkEl("1159+292") > $limit) {
    command("observe -s 1159+292");
    command("integrate -s 10 -w");
  }
  if (checkEl("1517-243") > $limit) {
    command("observe -s 1517-243");
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
  if (checkEl("1849+670") > $limit) {
    command("observe -s 1849+670");
    command("integrate -s 10 -w");
  }
  if (checkEl("1924-292") > $limit) {
    command("observe -s 1924-292");
    command("integrate -s 10 -w");
  }
  if (checkEl("ganymede") > $limit) {
      command("observe -s ganymede");
      command("integrate -s 10 -w");
  }
  if (checkEl("callisto") > $limit) {
    command("observe -s callisto");
    command("integrate -s 10 -w");
  }
}
