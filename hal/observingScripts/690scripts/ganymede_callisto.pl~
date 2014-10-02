#!/usr/bin/perl -w
do 'sma.pl';
printPID();
checkANT();
$limit = 86;
for (;;) {
  command("tsys");
  if (checkEl("titan") < $limit) {
    command("observe -s titan");
    command("integrate -t 30 -s 11 -w");
  }
  if (checkEl("vesta") < $limit) {
    command("observe -s vesta");
    command("integrate -t 30 -s 11 -w");
  }
}
