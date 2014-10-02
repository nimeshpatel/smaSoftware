#!/usr/bin/perl -w
#
# PI: SMA Pointing Team
# Author: Todd Hunter
# Contact person: Nimesh Patel
# Office phone: 617 496 7649 Evening phone: 617 577 1597 
# cellphone: 617 669 5665.

# load the sma library of functions used in this observing script.
do 'sma.pl';

printPID();

# check participating antennas.
checkANT();
command("setFeedOffset -f 230");
command("radio");
for(;;) {
  command("tsys");
  $targel0 = checkEl("3c273");
  $targel1 = checkEl("3c279");
  $targel2 = checkEl("1337-129");
  $targel3 = checkEl("3c345");
  $targel4 = checkEl("1504+104");
  $targel5 = checkEl("1751+096");
  if ($targel0 > 19) {
    command("observe -s 3c273");
    command("ipoint -r 1");
  }
  if ($targel1 > 19) {
    command("observe -s 3c279");
    command("ipoint -r 1");
  }
  if ($targel2 > 19) {
    command("observe -s 1337-129");
    command("ipoint -r 1");
  }
  if ($targel3 > 19) {
    command("observe -s 3c345");
    command("ipoint -r 1");
  }
  if ($targel4 > 19) {
    command("observe -s 1504+104");
    command("ipoint -r 1");
  }
  if ($targel5 > 19) {
    command("observe -s 1751+096");
    command("ipoint -r 1");
  }
}
