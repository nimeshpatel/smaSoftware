#!/usr/local/bin/perl
print "Scan LO\n";
#$fstart=232.476000;
$fstart=232.477475;
$fend = 232.478000;
$freq=$fstart;
for($i=0;$i<=4000;$i++) { 
system("resume");
system("setFreq -a 6 -f $freq -s l -t 16");
sleep(1);
printf ("freq %f; check lock", $freq);
$dummy=<STDIN>;
$freq+=0.000005;
}
