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
  $targel0 = checkEl("3c345");
  $targel1 = checkEl("1924-292");
  $targel2 = checkEl("3c454.3");
  $targel3 = checkEl("3c84");
  if ($targel0 > 22) {
    command("observe -s 3c345");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel1 > 20) {
    command("observe -s 1924-292");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel2 > 20) {
    command("observe -s 3c454.3");
    command("point -i 60 -r 2 -L -l -t -s");
  }
  if ($targel3 > 22) {
    command("observe -s 3c84");
    command("point -i 60 -r 2 -L -l -t -s");
  }
}
