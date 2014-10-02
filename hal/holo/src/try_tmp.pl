#!/usr/local/bin/perl
$label=shift;
goto $label;
readatten: $ans=(split(' ',`ssh crate4 c2dc_control -r 1 upper rdatten`))[6];
print "answer: $ans\n";
terminate: print "exiting ...\n"; exit;
