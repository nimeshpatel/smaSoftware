#!/usr/bin/perl
# usage:grab_dip.pl <dipfit_filename>
if($#ARGV<0) {die "Usage: grab_dip.pl <dipfit_filename>\n"; }
$dipfitfile=shift;
open(FILE, $dipfitfile)||die "Could not open this file.\n";
@alllines=<FILE>;
close(FILE);
$ant_num=(split(' ',$alllines[1]))[1];
$tau_zenith=(split(' ',$alllines[22]))[2];
$Thot=(split(' ',$alllines[11]))[3];
$Tatm=(split(' ',$alllines[21]))[3];
$eta_l=(split(' ',$alllines[19]))[3];
$freq=(split(' ',$alllines[3]))[2];
$chop_throw_gain=1;
$Tatm=$Thot-$Tatm;
$com="writedipresults $ant_num $tau_zenith $Tatm $eta_l $freq $chop_throw_gain\n";
print "$com\n";
`$com`;
