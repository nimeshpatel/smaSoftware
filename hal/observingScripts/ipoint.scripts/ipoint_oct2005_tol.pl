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
command("setFeedOffset -f 345");
command("radio");
command("tol -a 3,6 -c off");
for(;;) {
  command("tsys");
  $targel1 = checkEl("1924-292");
  $targel2 = checkEl("3c454.3");
  $targel3 = checkEl("3c111");
  if ($targel1 > 22) {
    command("observe -s 1924-292");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel2 > 20) {
    command("observe -s 3c454.3");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel3 > 20) {
    command("observe -s 3c111");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  $targel1 = checkEl("1924-292");
  $targel2 = checkEl("3c454.3");
  $targel3 = checkEl("3c111");
  if ($targel1 > 22) {
    command("observe -s 1924-292");
    command("antennaWait -a 3,6 -e 2");
    command("tol -a 3,6 -c on");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel2 > 20) {
    command("observe -s 3c454.3");
    command("antennaWait -a 3,6 -e 2");
    command("tol -a 3,6 -c on");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel3 > 20) {
    command("observe -s 3c111");
    command("antennaWait -a 3,6 -e 2");
    command("tol -a 3,6 -c on");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  command("tol -a 3,6 -c off");
}
