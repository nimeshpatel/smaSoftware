#!/usr/local/bin/perl
open(F,">/data/engineering/rpoint/ant2/chopscan.dat")||die "Could not open file.\n";
while(1)
{
#$azoff=`value -a 2 -v azoff`;
#$eloff=`value -a 2 -v eloff`;
$chopz= `value -a 2 -v chopper_z_counts`;
$sdcounts=`value -a 2 -v syncdet_volts`;
printf "%d %f\n",$chopz,$sdcounts;
printf F "%d %f\n",$chopz,$sdcounts;
#select undef,undef,undef,0.1;
}
close(F);
