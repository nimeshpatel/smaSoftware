#!/usr/local/bin/perl
open(VVM, "| vectorvoltmeter > tmpfile");
#while (1) {
print(VVM "ib 8 FREQ:BAND 12\n");
print(VVM "ib 8 FREQ:BAND?\n");
print(VVM "ibr 8\n");
close(VVM);
sleep(2);
$ans=(split(' ',`tail -2 tmpfile | head -1`))[6];
print "answer: $ans\n";
open(VVM, "| vectorvoltmeter > tmpfile");
print(VVM "ib 8 STAT:OPER:COND?\n");
print(VVM "ibr 8\n");
close(VVM);
sleep(2);
$ans=(split(' ',`tail -2 tmpfile | head -1`))[6];
$lock = $ans & 4;
print "lock = $lock/4\n";
print "answer: $ans\n";
#open(FILE, "tmpfile")||die "Could not open this file.\n";
#@alllines=<FILE>;
#close(FILE);
#$ant_num=(split(' ',$alllines[1]))[1];
#
#sleep(1);
#}
