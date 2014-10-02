#!/usr/local/bin/perl
# $time_ave=( ( split('/',(split(' ',(`ping -c 1 newdds`)[5]))[3]) )[1] ); 
# print "$time_ave\n";
$a1=1;
print "reboot newdds ....\n";
`touch /global/killnewdds`;
sleep(20);
print "waiting for newdds to come up ... ";
while ($size <= 4 ){@alllines=`ping -c 1 newdds`; $size=$#alllines;print "."; sleep(2);}
while ((split('/',(split(' ',(`ping -c 1 newdds`)[5]))[3]))[1] >= 0.25 ) {print ". "; sleep(0.5); $| ;}
print "newdds is up.\n";
print "\nCheck if encoderServer is running ..... and if not start it ...\n";
$answer=`ssh acc1 "ps -ax | grep encod"`;
if($answer !=~ /encod/) {`ssh acc1 "encoderServer >& /dev/null &"`;}
print "\n checking VVM lock .... \n \n";
open(VVM,"| vectorvoltmeter2 > /dev/null");
print(VVM "vv1\n");
print "\n";
print(VVM "key preset\n");
print "\n";
print(VVM "dan on\n");
print(VVM "key b\n");
print(VVM "ib 8 FREQ:BAND 12\n");
print(VVM "vv2\n");
print(VVM "key preset\n");
print(VVM "dan on\n");
print(VVM "key b\n");
print(VVM "ib 9 FREQ:BAND 12\n");
$answer=print(VVM "vlock2\n");
print(VVM "vv1\n");
$ratio=3.2;
while ($ratio <= 5.0 ) {
$old=(split(' ', ((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1])))[7];
#print "key up\n";
print(VVM "key up\n");
$new=(split(' ', ((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1])))[7];
$ratio=$new/$old;
print "$old, $new, $ratio\n";
}
# went over the end of range; now at highest gain.
print(VVM "key down\n");
# now at lowest gain
if (abs(((split(' ', ((`vvm_both_tt "/dev/null/" 1 1 1 1`)[1])))[7]) - 2500) >= 500) {print "power level or gain needs adjustment\n";}
