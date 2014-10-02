#!/usr/bin/perl -w
do 'sma.pl';
printPID();
checkANT();
$limit = 86.5;
for (;;) {
  command("tsys");
  if (checkEl("ganymede") < $limit) {
    command("observe -s ganymede");
    command("integrate -t 30 -s 11 -w");
  }
  if (checkEl("callisto") < $limit) {
    command("observe -s callisto");
    command("integrate -t 30 -s 11 -w");
  }
}
