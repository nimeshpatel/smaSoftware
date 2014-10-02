#!/usr/bin/perl
if( $#ARGV < 2) {die "Usage: holoSetup a1 a2 a3(ref) level\n"; }
$a1=shift;
$a2=shift;
$a3=shift;
$level0=shift;
$pick=9;
`setIFLevels -D -1`;
#`setIFLevels -a 5 -c -R h -1 -l 6.0`;
@atten = split " ",`shmValue -mm5 C1DC_STATUS_X:IF_CNTR_VOLT_V3_V9_F`;
print "$atten[$pick+$a1] $atten[$pick+$a2] $atten[$pick+$a3] \n";
@level =  split " ",`shmValue -mm5 C1DC_STATUS_X:IF_POWER_V3_V9_F`;
print "$level[$pick+$a1] $level[$pick+$a2] $level[$pick+$a3] \n";
while ($level0 <=  $level[$pick+$a1]) {
$dac = $atten[$pick+$a1]+0.1*10*log($level[$pick+$a1]/$level0)/log(10);
`setC1DCVoltage -a $a1 -v $dac`;
sleep(5);
@atten = split " ",`shmValue -mm5 C1DC_STATUS_X:IF_CNTR_VOLT_V3_V9_F`;
@level =  split " ",`shmValue -mm5 C1DC_STATUS_X:IF_POWER_V3_V9_F`;
print "$atten[$pick+$a1] $atten[$pick+$a2] $atten[$pick+$a3] \n";
print "$level[$pick+$a1] $level[$pick+$a2] $level[$pick+$a3] \n";
}
print "Done $a1\n";
while ($level0 <=  $level[$pick+$a2]) {
$dac = $atten[$pick+$a2]+0.1*10*log($level[$pick+$a2]/$level0)/log(10);
`setC1DCVoltage -a $a2 -v $dac`;
sleep(5);
@atten = split " ",`shmValue -mm5 C1DC_STATUS_X:IF_CNTR_VOLT_V3_V9_F`;
@level =  split " ",`shmValue -mm5 C1DC_STATUS_X:IF_POWER_V3_V9_F`;
print "$atten[$pick+$a1] $atten[$pick+$a2] $atten[$pick+$a3] \n";
print "$level[$pick+$a1] $level[$pick+$a2] $level[$pick+$a3] \n";
}
print "Done $a2\n";
while ($level0 <=  $level[$pick+$a3]) {
$dac = $atten[$pick+$a3]+0.1*10*log($level[$pick+$a3]/$level0)/log(10);
`setC1DCVoltage -a $a3 -v $dac`;
sleep(5);
@atten = split " ",`shmValue -mm5 C1DC_STATUS_X:IF_CNTR_VOLT_V3_V9_F`;
@level =  split " ",`shmValue -mm5 C1DC_STATUS_X:IF_POWER_V3_V9_F`;
print "$atten[$pick+$a1] $atten[$pick+$a2] $atten[$pick+$a3] \n";
print "$level[$pick+$a1] $level[$pick+$a2] $level[$pick+$a3] \n";
}
print "Done $a2\n";
