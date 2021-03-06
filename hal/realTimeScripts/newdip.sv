#!/usr/bin/perl
#usage: dip Ant# 
# original version M.Saito, unknown date
# flush included after system; it doesn't work otherwise! TK 07Feb2001
# checking to see if antenna reached elevation : TK 07Feb2001
# command argument in standard format: MS 17Feb2001
# Modified all occurences of el command to remove the -w switch
# which is no longer valid- NAP 24 Mar 01
# 
# You can now run dips on all antennas at once - TRH 25 Mar 2001
# Modified to include antenna-7 - NAP 14 Jul 2001
# Modified to show Process ID Masao 18 aug 2001
# Print "#" in front of header lines in data file 09 May 2003, TKS.
# openM3 to start! 10 May 2003, TKS.
# Use CONT1_DET1 instead of TOTAL_POWER_VOLTS  15-Oct-2003 TRH
# Also record high freq data from CONT2_DET1   15-Oct-2003 TRH
# Add the actual fitting arithmetic to this script  09-Apr-2004 TRH (not yet tested)
# Fixed a bug in filename, while opening the output file. 30-Jun-2004 NAP
# Added a write to the SMAshLog so that we can see it on the 'l' page. 13-Jul-2004 TRH
# get cold and coldhigh from reflective memory and write them out. NAP, TK 16 Nov 04
# corrected airmass computation TK 14 Feb 05
# Only print coldload values if they are less than 8 hours old.  TRH 22-Jul-2005
# Major revisions by RWW 8/2/07 to average all data, including that from
#   the heated and unheated loads.  The header is re-ordered.

$hourlimit = 8;
$obsFrequency = 0; 
$obsFrequencyHigh = 684.0;

@el_list = (60.81, 41.81, 30, 23.58, 19.47, 16.60);
# Make tune 6 quiet if $q == "-q"
$q = "-q";
$nSamples = 10;
$directory = "/usr/home/rwilson/tmp";
#$directory = "/data/engineering/dip_scan";
$firstElev = 80.0;

sub Usage() {
  print "This script will command the antennas to do a skydip.  Usage:\n\n" .
  "dip [-a 4] [-f 237.4] [-g 658.0] -c<optional> [-e <firstElev>]" .
  "-s<optional>\n" .
  "  -- or --\n" .
  "dip [--antenna 4] [--frequency 237.4] --coldload<optional> --simulate\n\n" .
  "  if -a is not given, all antennas in the project command will be dipped\n" .
  "  simultaneously\n\n" .
  "  The -f flag is optional. If given, the argument should be the LO" .
  "frequency\n\n" .
  "  If it is not given (this is preferred), the value will read from DSM.\n\n".
  "  The -g flag is optional. If given, the argument should be the" .
  "high-frequency\n" .
  "  receiver's LO frequency.  Otherwise, it will be read from DSM.\n\n" .
  "  More than one antenna can be specified,  for example, -a 3..5,8\n\n" .
  "  The coldload dataline will be written if the timestamp is recent enough\n".
  "   ($hourlimit hours).  Use the -c argument to force it to be written.\n";
}
 
$args = "";
foreach $i ( @ARGV ) {
    $args = $args . $i . " ";
}

use  POSIX;
use Getopt::Long;
Getopt::Long::Configure(qw(auto_abbrev));
use FileHandle;
$pid=POSIX::getpid();
$Getopt::Long::autoabbrev=1;
$antennaPick = 0;
#
# read obsFrequency from DSM
# old way:
#$obsFrequency = `readDSM -l -m m5 -v DSM_AS_IFLO_REST_FR_V2_D -i 0`;
#$sideband = `readDSM -l -m m5 -v DSM_AS_IFLO_SIDEBAND_V2_S -i 0`;
#  by Taco's standard, having the line in USB corresponds to sideband==-1
#$obsFrequency += $sideband*5.0;
#
# new way:
$obsFrequency = `readDSM -l -m newdds -v dds_to_hal_x -s FREQ_V3_D -i 1`;
$obsFrequency *= 0.000000001;  # convert to GHz
#
# read obsFrequencyHigh from DSM
# old way:
# $sidebandHigh = `readDSM -l -m m5 -v DSM_AS_IFLO_SIDEBAND_V2_S -i 1`;
# $obsFrequencyHigh = `readDSM -l -m m5 -v DSM_AS_IFLO_REST_FR_V2_D -i 1`;
# $obsFrequencyHigh += $sidebandHigh*5.0;
#
# new way (more accurate):
$obsFrequencyHigh = `readDSM -l -m newdds -v dds_to_hal_x -s FREQ_V3_D -i 2`;
$obsFrequencyHigh *= 0.000000001;  # convert to GHz
#  by Taco's standard, having the line in USB corresponds to sideband==-1
if ($obsFrequencyHigh != $obsFrequencyHigh || $obsFrequencyHigh < 300
    || $obsFrequencyHigh > 900) {
    $obsFrequencyHigh = 684.0;
}
#
$opt_simulate = 8;
GetOptions('antenna=s'=>\$antennaPick,'frequency=f'=>\$obsFrequency,
	'gFrequencyHigh=f'=>\$obsFrequencyHigh,'coldload','help',
	'elevation=f'=>\$firstElev,
#	'syncdet',
	'simulate');
@antennalist = ();

if($opt_simulate == 8) {
  print "WARNING: You forgot to specify simulation\n";
}

# always print the cold load value, if the timestamp is recent enough
$opt_coldload = 1;

if ($firstElev > 85 || $firstElev < 45) {
    Usage();
    print "First elev must be between 45 and 85\n";
    exit(0);
} 

if ($obsFrequency<160 or $obsFrequency>900) {
  print "Please specify the LO frequency in GHz using -f \n";
  print "Frequency must be 160 - 900 GHz.\n";
  Usage();
  exit(0);
}

if($opt_help) {
  Usage();
  exit(0);
}


#die "debug\n";
#print "antennaPick = $antennaPick\n";
if ($antennaPick != 0) {
  $checklist = `/application/bin/getAntList -a $antennaPick -l`;
  @antennalisttemp = split(' ',$checklist);
  $numants = @antennalisttemp;
  for ($antindex=0; $antindex<$numants; $antindex++) {
    if (($antennalisttemp[$antindex]<1) || ($antennalisttemp[$antindex]>8)) {
      Usage();
      die "Check the command-line arguments.\n";
    }
    push @antennalist, $antennalisttemp[$antindex];
#    printf "Will use antenna %d\n",$antennalisttemp[$antindex];
  }
} else {
# no antenna was specified, get the list from the project command
    print "Will use all antennas in the project:\n";
    $antcount = 0;
    for ($i=1; $i<=8; $i++) {
	$antenna_status=`value -a $i -v antenna_status`;
	if ($antenna_status==1) { 
	    push(@antennalist,$i); 
	    print "Will use antenna $i\n";
	    $antcount++;
	}       
    }
    print "@antennalist\n";
    $command = "commaAntennaList -a \"@antennalist\"";
    print $command."\n";
    $antennaPick = `commaAntennaList "@antennalist"`;
    chomp($antennaPick);
}

print "Will use antennas: $antennaPick\n"; 

if(!$opt_simulate) {
  open(LOGFILE,">>/rootfs/logs/SMAshLog");
  $line = `date -u +"%a %b %d %X %Y"`;
  chop($line);
  $user = $ENV{"USER"};
  printf LOGFILE "%s ($user): $0 %s\n", $line, $args;
  close(LOGFILE);
}

@fileHandles = ();

# Send telescopes to the first elevation
sys_command("openM3 -a $antennaPick");
sleep 3;
sys_command("el -d $firstElev -a $antennaPick");
sleep 3;
sys_command("resume -a $antennaPick");
sleep 3;

# create names and open data files for each antenna.  The file handles go in
# @fileHandles
foreach $antennaNumber (@antennalist) {
# open up data files and send each telescope to starting elevation
  print "Start dip scan Ant $antennaNumber\n";
  ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst)=localtime(time);
  $date_name = sprintf "%02d%02d%02d_%02d%02d",$year-100,$mon+1,$mday,
	$hour,$min;
  $filename[$antennaNumber] = "$directory/$date_name.dip$antennaNumber";
  print "opening file: $filename[$antennaNumber]\n";
  $fh = new FileHandle $filename[$antennaNumber], O_WRONLY|O_CREAT
	  or die("cannot open the output file $filename[$antennaNumber]\n");
  push @fileHandles, $fh;
}

# Record the header values which are not averaged
@aLTemp = split " ",`shmValue -a$antennaPick RM_UNHEATEDLOAD_TEMPERATURE_F`
	or die "Can't get unheated load temps with shmValue\n";
@aTemp = split " ", `shmValue -a$antennaPick RM_WEATHER_TEMP_F`
	or die "Can't get weather temps with shmValue\n";
@hLTemp = split " ", `shmValue -a$antennaPick RM_HEATEDLOAD_TEMPERATURE_F`
	or die "Can't get heated load temps with shmValue\n";
foreach $i (0 .. $#antennalist) {

  print { $fileHandles[$i] } "# antennaNumber $antennalist[$i]\n";
  print { $fileHandles[$i] } "# date $date_name\n";
  print { $fileHandles[$i] } "# bias 0.0\n";
  print { $fileHandles[$i] } "# freq $obsFrequency $obsFrequencyHigh\n";
  print { $fileHandles[$i] } "# tamb ", 273.15 + $aTemp[$i], "\n";
  print { $fileHandles[$i] } "# tcold 77.0\n";
  print { $fileHandles[$i] } "# thot ", 273.15 + $aLTemp[$i], "\n";
  print { $fileHandles[$i] } "# theated ", 273.15 + $hLTemp[$i], "\n";
}

# put in the hot (unheated) load
sys_command("tune $q -a $antennaPick -c hotload in");
sleep(8);
AveragePowerAndPrint("# hot");

# put in the heated load
sys_command("tune $q -a $antennaPick -c heatedload in");
sys_command("tune $q -a $antennaPick -c hotload out");
sleep(3);
AveragePowerAndPrint("# heated");
sys_command("tune $q -a $antennaPick -c heatedload in");
sleep(3);

# check the antenna lower limits in case we are in sub-compact
@lowerLimits = split " ", `shmValue -a$antennaPick RM_SCB_LOW_LIMIT_F`;
#print "The lower elevation limits are: @lowerLimits\n";
$lastElev = 0;
foreach $ll (@lowerLimits) {
  if($ll > $lastElev) {
    $lastElev = $ll;
  }
}
$lastElev += 0.1;

print "goto $firstElev\n";
gotoel($firstElev);
foreach $el (@el_list) {
  if($el > $firstElev) {
    print "Skipping $el > $firstElev\n";
    next;
  }
  if($el < $lastElev) {
    gotoel($lastElev);
    last;
  }
  gotoel($el);
}
exit(0);

# record the hot voltage and remove the hot load
$index = 0;
foreach $antennaNumber (@antennalist) {
# record hotload voltage, etc. in the file header
  if($opt_syncdet) {
    $powers = `value -a $antennaNumber -v syncdet2_channels_v2`;
    @list = split(' ',$powers);
    $hot = $list[0];
    $hothigh = $list[1];
  } else {
    $hot = `value -a $antennaNumber -v hotload_lowfreq_volts`;
    $hothigh = `value -a $antennaNumber -v hotload_highfreq_volts`;
  }
  chomp($hot);
#  sys_command($command);
#  flush;
  $hot_tmp = 273.15+`value -a $antennaNumber -v ambientload_temperature`;
  $ambient_tmp = 273.15+`value -a $antennaNumber -v weather_temp`;
  if ($opt_coldload) {
    print { $fileHandles[$index] } "# tcold 77.0\n";
    $cold = `value -a $antennaNumber -v coldload_lowfreq_volts`;
    chomp($cold);
    $coldtime = `value -a $antennaNumber -v coldload_lowfreq_volts_timestamp`;
    chomp($coldtime);
    $coldhigh = `value -a $antennaNumber -v coldload_highfreq_volts`;
    chomp($coldhigh);
    $coldhightime = `value -a $antennaNumber -v coldload_highfreq_volts_timestamp`;
    chomp($coldhightime);
    $unixtime = `value -a $antennaNumber -v unix_time`;
    chomp($unixtime);
    print " low-freq rx: cold load = $cold at timestamp = $coldtime, unixtime = $unixtime";
    print "high-freq rx: cold load = $coldhigh at timestamp = $coldhightime, unixtime = $unixtime";
  }
  print { $fileHandles[$index] } "# thot $hot_tmp\n";
  print { $fileHandles[$index] } "# hot $hot $hothigh\n";
  print { $fileHandles[$index] } "# bias 0.0\n";
  print { $fileHandles[$index] } "# tamb $ambient_tmp\n";
  print { $fileHandles[$index] } "# freq $obsFrequency $obsFrequencyHigh\n";
  if ($opt_coldload) {
      $age = $unixtime-$coldtime;
      if ($unixtime > $coldtime) {
	  if (($unixtime-$coldtime) < ($hourlimit*3600)) {
	      print { $fileHandles[$index] } "# cold $cold $coldhigh\n";
	      print { $fileHandles[$index] } "# timestamp $coldtime $coldhightime\n";
	  } else {
	      print "Not printing cold load values to the file because the timestamp is too old (i.e. more than $hourlimit hours).\n";
	  }
      } else {
	  print "Not printing cold load values to the file because the timestamp appears to be in the future.\n";
      }
  }
  print { $fileHandles[$index] } "# antennaNumber $antennaNumber\n";
  print { $fileHandles[$index] } "# date $date_name\n";
  $index++;
}

#RWW Add measurement of the heated load
# put in the hot load
$command="tune $q -a $antennaPick -c heatedload in";
sys_command($command);
$command="tune $q -a $antennaPick -c hotload out";
sys_command($command);
sleep(3);

# record the heated load voltage
$index = 0;
foreach $antennaNumber (@antennalist) {
# record heatedload voltage, etc. in the file header
  $powers = `value -a $antennaNumber -v cont_det_muwatt_v2`;
  @list = split(' ',$powers);
  $heated = $list[0];
  $heatedhigh = $list[1];
  chomp($heated);
  $heated_tmp = 273.15+`value -a $antennaNumber -v heatedload_temperature`;
  print { $fileHandles[$index] } "# theated $heated_tmp\n";
  print { $fileHandles[$index] } "# heated $heated $heatedhigh\n";

  $index++;
}
$command="tune $q -a $antennaPick -c heatedload out";
sys_command($command);
#RWW End of Heated load measurement

sleep(5);

#die "debug\n";
foreach $a (@antennalist) {
  $command="resume -a $a";
  sys_command($command);
  flush;
  flush;
#  $xnpts[$a] = 0;      $sumx[$a] = 0;      $sumy[$a] = 0;
#  $sumy2[$a]= 0;      $sumx2[$a] = 0;      $sumxy[$a] = 0;
}

#$alist = "-a" . join ',', @antennalist;
#print "alist = $alist\n";
@lowerLimits = split " ", `shmValue -a$antennaPick RM_SCB_LOW_LIMIT_F`;
print "The lower elevation limits are: @lowerLimits\n";
$lastElev = 0;
foreach $ll (@lowerLimits) {
  if($ll > $lastElev) {
    $lastElev = $ll;
  }
}
$lastElev += 0.1;
if($lastElev < 16.60) {
  gotoel($firstElev);
  gotoel(60.81);
  gotoel(41.81);
  gotoel(30.0);
  gotoel(23.58);
  gotoel(19.47);
  gotoel(16.60);

# gotoel(14.48);
# gotoel(12.84);
# gotoel(11.54);
# gotoel(10.48);
} else {
  $R = 180. / 3.14159265358979;
  $lowAirMass = 1/sin($firstElev / $R);
  $highAirMass = 1/sin($lastElev / $R);
  $delAm = ($highAirMass - $lowAirMass)/4.0;
  for($am = $lowAirMass; $am < $highAirMass + 0.01; $am += $delAm) {
    gotoel($R * asin(1.0 / $am));
  }
}

print "closing file handles\n";
for ($i=0; $i<=$#antennalist; $i++) {
  close($fileHandles[$i]);
}
foreach $antennaNumber (@antennalist) {
  print "Data left in file = $filename[$antennaNumber]\n";
  `chmod 666 $filename[$antennaNumber]`;
}
print "done!\n";
print "To see the results, go to the monitor 'w' page.\n";

sub gotoel {
  my @sunAvoid;
  my $el_command=shift(@_);

#  print "process ID = $pid ";
  print " El Command $el_command\n";
# send all antennas to the specified elevation
  sys_command("el -a $antennaPick -d $el_command");
  sleep(2);
  @sunAvoid = split " ", `shmValue -a$antennaPick RM_SERVO_POS_IN_SUN_AVOID_S`;
  foreach $sa (@sunAvoid) {
    if($sa != 0) {
      print "This position too close to the Sun, skipping el $el_command\n";
      return;
    }
  }
  sys_command("antennaWait -a $antennaPick -e 10");
  AveragePowerAndPrint();
#  read_power($el_command);
}

sub read_power {#read total power
    my $elcmd = shift(@_);
    $pi = 3.14159265358979;
    for($i=0;$i<10;$i++){
      $index = 0;
      foreach $a (@antennalist) {
	if ($opt_syncdet) {
	      $powers = `value -a $a -v syncdet2_channels_v2`;
	      @list = split(' ',$powers);
	      $tot = $list[0];
	      $tot2 = $list[1];
	} else {
	      $totpowers = `value -a $a -v cont_det_muwatt_v2`;
	      @totpowerslist=split(' ',$totpowers);
	      $tot = $totpowerslist[0];
	      $tot2 = $totpowerslist[1];
	}
	$el = `value -a $a -v actual_el_deg`;
        $airmass = 1.0/cos((90.0-$el)*$pi/180.0);
#       $xnpts[$a] += 1;
#	$sumx[$a] += $airmass;
#	$sumx2[$a] += $airmass*$airmass;
#	$sumy[$a] += $tot;
#	$sumy2[$a] += $tot*$tot;
#	$sumxy[$a] += $tot*$airmass;
#	$sumyhf[$a] += $tot2;
#	$sumy2hf[$a] += $tot2*$tot2;
#	$sumxyhf[$a] += $tot2*$airmass;
	printf "%6.3f %8.5f %8.5f %8.5f\n",$el, $tot, $tot2, $airmass;
	printf { $fileHandles[$index] } "%6.3f %8.5f %8.5f %8.5f\n",$el, $tot, $tot2, $airmass;
	$index++;
      }
      sleep 1;
#      if ($elcmd == 16.60) {
#	  $slope = ( $xnpts[$a]*$sumxy[$a] - $sumx[$a]*$sumy[$a] ) / ( $xnpts[$a]*$sumx2[$a] - $sumx[$a]*$sumx[$a] );
#	  $intercept = ( $sumy[$a]*$sumx2[$a] - $sumx[$a]*$sumxy[$a] ) / ( $xnpts[$a]*$sumx2[$a] - $sumx[$a]*$sumx[$a] );
#	  $slope2 = ( $xnpts[$a]*$sumxyhf[$a] - $sumx[$a]*$sumyhf[$a] ) / ( $xnpts[$a]*$sumx2[$a] - $sumx[$a]*$sumx[$a] );
#	  $intercept2 = ( $sumyhf[$a]*$sumx2[$a] - $sumx[$a]*$sumxyhf[$a] ) / ( $xnpts[$a]*$sumx2[$a] - $sumx[$a]*$sumx[$a] );
#	  printf "antenna %d: tau (low-freq) = %.3f    (high-freq) = %.3f\n", $a, 
#	    $slope, $slope2;
#      }
    }
}

sub AveragePowerAndPrint {
  my $lineStart = shift(@_);
  my $i, $j, @in, @sum = (), @ssq = ();
  my @elevations;

  open(IN, "shmValue -a $antennaPick -n$nSamples RM_CONT_DET_MUWATT_V2_F |")
	or die "Can't run shmValue";
  while($l = <IN>) {
#    print $l;
    @in = split(" ", $l);
    for $i (0 .. $#in) {
      $sum[$i] += $in[$i];
      $ssq[$i] += $in[$i] * $in[$i];
    }
  }
  close IN;
  if(length($lineStart) == 0) {
    $l = `shmValue -a $antennaPick RM_ACTUAL_EL_DEG_F`;
    @elevations = split(" ", $l);
  }
  for $i (0..$#fileHandles) {
#   $j indexes the values from shmValue which contain results from 2 receivers
    $j = $i * 2;

    if(length($lineStart) == 0) {
      printf { $fileHandles[$i] } "%6.3f ", $elevations[$i];
    } else {
      printf { $fileHandles[$i] } "%s ", $lineStart;
    }
    printf { $fileHandles[$i] } "%8.5f %8.5f %8.5f %8.5f\n",
	$sum[$j]/$nSamples,
	sqrt($ssq[$j]/$nSamples -($sum[$j]/$nSamples)**2),
	$sum[$j+1]/$nSamples,
	sqrt($ssq[$j+1]/$nSamples -($sum[$j+1]/$nSamples)**2),
  }
}

# Prints and performs a shell task or, if simulating, merely prints it.
sub sys_command {
   print "@_\n";
   if(!$opt_simulate) {
     system("@_");
   }
}
