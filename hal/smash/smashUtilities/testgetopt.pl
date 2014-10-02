#!/usr/bin/perl

use Getopt::Std;

getopts('a:h');
@antList = `getAntList -a $opt_a`;
printf("opt_a = %s  opt_h = %s\n", $opt_a, $opt_h);
print $antList[0], "\n";
