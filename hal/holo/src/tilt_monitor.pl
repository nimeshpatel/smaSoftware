#!/usr/local/bin/perl
#usage: tilt_monitor 

($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
$date_name=sprintf "%02d%02d%02d_%02d%02d",$year-100,$mon+1,$mday,$hour,$min;
$filename=">>/common/data/tilt/$date_name.tilt";
print "filename: $filename\n";
open (OUT, $filename)||die("cannot open the output file\n");
print "Start tilt scan.\n";

print "#time temparature cabin_temp2 tiltx2 tilty2 cabin_temp3 tiltx3 cabin_temp4 tilty3 tiltx4 tilty4\n";
print OUT "#time temparature cabin_temp2 tiltx2 tilty2 cabin_temp3 tiltx3 cabin_temp4 tilty3 tiltx4 tilty4\n";
while(1){
($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
	$tx2=`value -a 2 -v tiltx_arcsec`;
	$ty2=`value -a 2 -v tilty_arcsec`;
	$tx3=`value -a 3 -v tiltx_arcsec`;
	$ty3=`value -a 3 -v tilty_arcsec`;
	$tx4=`value -a 4 -v tiltx_arcsec`;
	$ty4=`value -a 4 -v tilty_arcsec`;
	$te=`value -a 2 -v weather_temp`;
	$tamb2=`value -a 2 -v ambientload_temperature`;
	$tamb3=`value -a 3 -v ambientload_temperature`;
	$tamb4=`value -a 4 -v ambientload_temperature`;
	printf "%02d:%02d:%02d %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",$hour,$min,$sec,$te,$tamb2,$tx2,$ty2,$tamb3,$tx3,$ty3,$tamb4,$tx4,$ty4;
	printf OUT "%02d:%02d:%02d %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",$hour,$min,$sec,$te,$tamb2,$tx2,$ty2,$tamb3,$tx3,$ty3,$tamb4,$tx4,$ty4;
	sleep 10;
}

