#!/usr/local/bin/perl -w
#
# NAME:
#	tpmon
#
# PURPOSE:
#	monitor total power (to check its stability)
# 
# REVISION HISTORY: 
#  2003 Jan 02  Created by Kazushi Sakamoto
#  2003 Aug.28  Added -s option to read syncdet output
#  2003 Oct.23  updated from where to read total power.
#               According to Nimesh, total power data can be read as the following
#               variables. The command is updated to handle all cases.
#
#  option       input 
#  none         continuum detector, lower freq (230 and 345) bands => cont1_det1
#  -u           continuum detector, upper freq (690)               => cont2_det1
#  -s or        syncdet output => syncdet_channels_v2 , which is an array of 2-elements
#  -s -u                          separated by a space. The 1st element is for
#                                 lower freq bands and the 2nd element is for upper
#                                 freq band.
#                                 
#--

$default_number = 60;
$default_interval = 1;

#-----------------------------------------------------------------------------#
sub Usage()
{
	my @cmd=split(/\//,$0);

	print "Usage: $cmd[@cmd-1] -a 4 -n $default_number -i $default_interval [-log -help]\n";
	print "  -a   [required] antenna number \n";
	print "  -n   number of data to be taken (default=$default_number)\n";
   print "  -i   data sampling interval in second (default=$default_interval)\n";
   print "       this is a sampling intergval, not an integration time\n"; 
	print "  -s   read syncdet output \n"; 
	print "  -u   read data for upper freqency band (above 400 GHz). default=low freq.\n"; 
	print "  -chan specify which channel to read [1-6]. overrides -s and/or -u\n";
        print "  -log   log total power into a file\n";
	print "  -help  print this help\n";
}



$pid=$$;  
print "PID=$pid \n";

if($#ARGV==-1){&Usage;die;}

use Getopt::Long;

$antenna=0; $number=0; $interval=0; 
GetOptions("a=i"    => \$antenna,
           "n:i"    => \$number,
           "i:i"    => \$interval,
           "log"    => \$log,
			  "s"      => \$syncdet,
			  "u"      => \$upperfreq,
           "chan:i" => \$channel,
           "help"   => \$help);


if($antenna == 0)   {&Usage;die;}
if($help)           {&Usage;die;}
if($number == 0)    {$number=$default_number;}
if($interval == 0)  {$interval=$default_interval;}

@cmd=split(/\//,$0);
$command="$cmd[@cmd-1] -a $antenna -n $number -i $interval";
if ($log)       {$command=$command." -log"}
if ($syncdet)   {$command=$command." -s"}
if ($upperfreq) {$command=$command." -u"}
print "$command \n";
if ($syncdet)   {print "reading syncdet output\n"}
if ($upperfreq) {print "reading output from high (> 400 GHz) freq band.\n"}


if ($log) {
	$date=`/bin/date +"%Y-%m-%d"`; chomp($date);
	$file="/global/data/engineering/tpmon/ant$antenna.$date.tpr";
	if (open(LOGFILE,">>$file"))	{print "log file $file has been opened\n"} 
	else 									{die "Can't open/create $file.\n"}
	use FileHandle; LOGFILE -> autoflush(1);  # no buffering
}


$freq=`getLOfreq`; chomp($freq);
$freq=$freq/1.0e9;


@out=(STDOUT);
if ($log) {@out=(@out,LOGFILE)}

if ($syncdet) {
	$rm_variable='syncdet_channels_v2';
} else {
	if ($upperfreq)	{$rm_variable='cont2_det1'} 
	else 					{$rm_variable='cont1_det1'}
	if (defined($channel)) {
		if 	($channel==1) {$rm_variable='total_power_volts'}
		elsif	($channel==2) {$rm_variable='total_power_volts2'}
		elsif	($channel==3) {$rm_variable='cont1_det1'}
		elsif	($channel==4) {$rm_variable='cont2_det1'}
		elsif	($channel==5) {$rm_variable='cont1_det2'}
		elsif	($channel==6) {$rm_variable='cont2_det2'}
		else					 {die "Error! unknown channel\n"}
	}
}
print "rm_variable=$rm_variable\n";
$cmd ="prio 50 value -a $antenna -v $rm_variable";

@ttpr=();
print " #   UT[hr] f(LO)[GHz] TTPR[V]\n";
for($i = 0; $i < $number; $i++) {

	$ut_hour = `prio 50 value -a $antenna -v utc_hours`; chomp($ut_hour);
	$volts =`$cmd`; chomp($volts);
   if ($syncdet) {
		@array=split(/ /,$volts);
		if ($upperfreq) {$volts=$array[1]} else {$volts=$array[0]}
   }
	foreach $_ (@out) { printf $_ "%3d %8.5f %7.3f %9.5f \n", $i, $ut_hour, $freq, $volts;}
	@ttpr=($volts,@ttpr);
	select undef, undef, undef, $interval;
}


@momnt=moment(@ttpr);
printf "mean= %9.5f,  stddev = %9.5f,  stddev/mean => %8.3f %% \n",
		$momnt[0],$momnt[1],$momnt[1]/$momnt[0]*100.0;
$min=$ttpr[0]; $max=$ttpr[0];
foreach $_ (@ttpr) {
	if ($_ > $max ) { $max = $_ }
	if ($_ < $min ) { $min = $_ }
}
printf "min = %9.5f (%6.2f %%),  max = %9.5f (%6.2f %%)\n",
		$min,($min/$momnt[0]-1)*100.0,
		$max,($max/$momnt[0]-1)*100.0;

if ($log) {close(LOGFILE)}



#-----------------------------------------------------------------------------#
sub doCommand {
   unless ($_[1]) { 
      print "$_[0]\n";
   } else {
      print "$_[0] ";
   }
   select undef, undef, undef, 0.2;	
}

#-----------------------------------------------------------------------------#
sub mean {
  my $sum=0;    
  my $num=@_;   

  foreach $_ (@_) { $sum += $_ }

  return $sum/$num;
}

#-----------------------------------------------------------------------------#
sub moment {   
  my $sum=0;  
  my $sum2=0;  
  my $num=@_;  
  my ($ave,$ave2);

	foreach $_ (@_) {
		$sum  += $_;
		$sum2 += $_**2;
	 }
 
	$ave =$sum/$num;
	$ave2=$sum2/$num;

  return ( $ave, sqrt( ($ave2 - $ave**2)*$num/($num-1) ) );
}

#-----------------------------------------------------------------------------#


