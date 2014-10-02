#!/usr/bin/perl

# written: 02sep06 James Battat (jbattat@cfa.harvard.edu)
# converts getunixtime (in seconds) into the following format:
# [yr][mon][date]hst[hr][min]
# e.g. 02sep06hst1145
# because it's running on hal, which uses UTC, we must subtract 10 from 
# the hour given

# ideally, you could just set the $ENV{TZ} variable to be HST, but i dont know how and i dont want to mess with hal.

use Time::Local;

my @monthnames = ( qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec) );
my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday);

$TimeInSeconds = `getunixtime`;

#for testing only
$TimeInSeconds -= 40000;

($sec, $min, $hour, $mday, $mon, $year, $wday, $yday) = localtime($TimeInSeconds);

$year += 1900;
$year_print_str = sprintf("%02d", $year % 100);
$date = $year_print_str.$monthnames[$mon].$mday;

$verbose = 1;

if ($verbose) {
   print "UTC\n";
   print "[$date]\n";
   print "$hour:$min\n";
}

$hour -= 10;

if ($hour<0) {
   $hour += 24;
   $mday -= 1;
}

$date = $year_print_str.$monthnames[$mon].$mday;

if ($verbose) {
   print "\nHST\n";
   print "[$date]\n";
   print "$hour:$min\n";
}

$hour_str = sprintf("%02d", $hour);
$min_str  = sprintf("%02d", $min);

$file_dateString = $date."hst".$hour_str.$min_str;

print "$file_dateString\n";