#!/usr/local/bin/perl
open(F,">/data/engineering/rpoint/ant2/chopxscan.dat")||die "Could not open file.\n";
for($c=-1000;$c<=5000;$c=$c+200) {
#$azoff=`value -a 2 -v azoff`;
#$eloff=`value -a 2 -v eloff`;
`chopperX -a 2 -c $c`;
$azoff=-115+($c-2300)*0.082;
`azoff -a 2 -s $azoff`;
$answer=<stdin>;
$chopz= `value -a 2 -v chopper_x_counts`;
$sdcounts=`value -a 2 -v syncdet_volts`;
printf "%d %f\n",$chopz,$sdcounts;
printf F "%d %f\n",$chopz,$sdcounts;
}
close(F);
