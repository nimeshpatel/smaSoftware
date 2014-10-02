#!/usr/local/bin/perl
#$old=(split((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1]))[5];
open(VVM,"| vectorvoltmeter2 > holoSetupTmp");
print(VVM "ib 8 STAT:OPER:COND?\n");
print(VVM "ibr 8\n");
print(VVM "ib 9 STAT:OPER:COND?\n");
print(VVM "ibr 9\n");
close(VVM);
$lock1=4 & (split(' ',`tail -2 holoSetupTmp | head -1`))[6];
$lock2=4 & (split(' ',`tail -6 holoSetupTmp | head -1`))[6];
print "lock1 = $lock1; lock 2 = $lock2\n";
open(VVM,"| vectorvoltmeter2 > /dev/null");
print(VVM "vv1\n");
print "VVM1\n";
$ratio=3.2;
while ($ratio <= 5.0 ) {
$old=(split(' ', ((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1])))[7];
print "key up\n";
print(VVM "key up\n");
$new=(split(' ', ((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1])))[7];
$ratio=$new/$old;
print "$old, $new, $ratio\n";
}
# went over the end of range; now at highest gain.
print "went over the end of range; now at highest gain. \n";
print(VVM "key down\n");
# now at lowest gain
print "go one step down ...\n ... done; now at lowest gain \n";
if (abs(((split(' ', ((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1])))[7]) - 2500) >= 500) {
print "power level or gain needs adjustment\n";
}
