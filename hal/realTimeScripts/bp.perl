#!/usr/bin/perl
# This script takes a beamPattern data filename as a command-line
# input parameter.  It filters it for useful data and outputs the results
# into a plain text file.  The first line contains the following 6 numbers:
#     startAzim startElev endAzim endElev startUT endUT
# The rest of the file contains the scan data.
#   - Todd Hunter 28 June 2006
#
$filename = shift(@ARGV);
open(IN,$filename) || die "cannot open inputfile = $filename";
$outname = $filename . ".bp";
$outfile = ">" . $outname;
open(OUT,$outfile) || die "cannot open outputfile = $outfile";
$lastazoff = 999999;
$ctr = 0;
while ( defined ($_ = <IN>)) {
  if ( /\#/ ) {
#    skip the comment lines
  } else {
    ($ut,$az,$el,$azoff,$eloff,$power) = split();
    if ( $azoff > $lastazoff) {
      if ($ctr == 0) {
	$startut = $ut;
	$startaz = $az;
	$startel = $el;
      } else {
	$endut = $ut;
	$endaz = $az;
	$endel = $el;
      }
      $ctr++;
    }
    $lastazoff = $azoff;
  }
}
close(IN);
open(IN,$filename) || die "cannot open inputfile = $filename";
printf OUT "%.2f %.2f %.2f %.2f %.3f %.3f\n", $startaz, $startel, $endaz, $endel, $startut, $endut;
$lastazoff = 999999;
while ( defined ($_ = <IN>)) {
  if ( /\#/ ) {
#    skip the comment lines
  } else {
    ($ut,$az,$el,$azoff,$eloff,$power) = split();
    if ( $azoff > $lastazoff) {
      print OUT $_;
    }
    $lastazoff = $azoff;
  }
}
close(IN);
close(OUT);
print "Results left in file = $outname\n";
#
