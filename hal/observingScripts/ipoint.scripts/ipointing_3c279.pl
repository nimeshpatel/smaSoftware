#!/usr/bin/perl -w
#
# PI: SMA Pointing Team
# Author: Todd Hunter, Nimesh Patel, Taco
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
  $targel=checkEl("3c279");
  if ($targel>18.) {
    command("observe -s 3c279");
    command("point -i 60 -r 1 -L -l -t -s");
  }
}
