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
$limit = 22;
for(;;) {
  command("tsys");
  if (checkEl("3c111") > $limit) {
    command("observe -s 3c111");
    command("point -i 60 -r 2 -L -l -t -s -Q");
  }
#  if (checkEl("0530+135") > $limit) {
#    command("observe -s 0530+135");
#    command("point -i 60 -r 2 -L -l -t -s -Q");
#  }
  if (checkEl("3c279") > 20) {
    command("observe -s 3c279");
    command("point -i 45 -r 2 -L -l -t -s -Q");
  }
  if (checkEl("3c454.3") > $limit) {
    command("observe -s 3c454.3");
    command("point -i 60 -r 2 -L -l -t -s -Q");
  }
  if (checkEl("3c84") > $limit) {
    command("observe -s 0522-364");
    command("point -i 60 -r 2 -L -l -t -s -Q");
  }
  if (checkEl("3c273") > 20) {
    command("observe -s 3c273");
    command("point -i 45 -r 2 -L -l -t -s -Q");
  }
  if (checkEl("1749+096") >$limit) {
    command("observe -s 1749+096");
    command("point -i 60 -r 2 -L -l -t -s -Q");
  }
}
