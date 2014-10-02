#!/usr/local/bin/perl
#usage: tilt_monitor 

$ant=2;

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
$date_name=sprintf "%02d%02d%02d_%02d%02d",$year-100,$mon+1,$mday,$hour,$min;
$filename=">>/common/data/tilt/$date_name.tilt$ant";
print "filename: $filename\n";
open (OUT, $filename)||die("cannot open the output file\n");
print "Start tilt scan.\n";

	printf OUT "#az el tiltx tilty\n";
while(1){
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
	$tx=`value -a $ant -v tiltx_arcsec`;
	$ty=`value -a $ant -v tilty_arcsec`;
	$taz=`value -a $ant -v actual_az_deg`;
	$tel=`value -a $ant -v actual_el_deg`;
	printf "%8.4f %8.4f %5.2f %5.2f\n",$taz,$tel,$tx,$ty;
	printf OUT "%8.4f %8.4f %5.2f %5.2f\n",$taz,$tel,$tx,$ty;
	sleep 1;
}

