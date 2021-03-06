#!/usr/bin/perl -w
#Planets and Jovian Moons 
$calib[0]="uranus";
$calib[1]="callisto";
#QSOs
$calqso[0]="2145+067";
$calqso[1]="3c279";
#####################################################
###initializing###
#load the SMA library of function used in this script
do 'sma.pl';
#check working antennas 
checkANT();
#pervent atennas are left in optical mode
command("radio");

####################
# start the script #
####################
$nloopend=3;
for ($i=0;$i<2;$i++)
{
 printPID();
 LST();
 $calel1=checkEl($calib[$i]);
 $calel2=checkEl($calqso[$i]);
 if ($calel1>20 and $calel1<80 && $calel2>20 and $calel2<80)
 {
  for ($nloop=0;$nloop<$nloopend;$nloop++)
  {
   command("observe -s $calib[$i]");
   command("tsys");
   command("integrate -s 10 -t 30 -w");
   command("observe -s $calqso[$i]");
   command("tsys");
   command("integrate -s 10 -t 30 -w");
  }
  print"observation of $calib[$i] and $calqso[$i] are done.\n"
 } 
else
 {
  print"$calib[$i] and $calqso[$i] are not available now.\n";
 }
}



