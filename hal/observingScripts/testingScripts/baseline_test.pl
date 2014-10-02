#!/usr/bin/perl -I/application/lib -w

#This is the new baseline script. I took the flux script and modified it to work like the baseline script.

#-----------------------------------------------------#
#---- Below this line observers should not modify ----#
#---- This script no longer requires modification ----#
# Constants
$pi=atan2(1,1)*4.0;
$rad2deg=180.0/$pi;
$deg2rad=1./$rad2deg;
$LAT=19.82420526388;  # Lattitude of the SMA (Degree)
$LAT=$LAT*$deg2rad;

$scan=7;        # How many scans
$scancal=7;        # How many scans on the calibrator quasar
$inttime=30.0;   # correlator integration time in seconds
$nflux=20;       # Number of scans for Flux Calibrators
$ELULIMIT=86.3; # Upper Elevation Limit of the SMA, Be careful!!!!!
$ELLLIMIT=17.0;  # Lower Elevation Limit of the SMA
$SUNANGLE=26.0;  # Sun avoidance angle (degrees)
@sma = ();
@cal = ();
@el = ();
@ha = ();
@cal = ();
@common = ();

$nicname="SMA";

use Getopt::Long;
$Getopt::Long::autoabbrev=1;

GetOptions('restart','calibrator=s'=>\$calib,'help','subcompact','time');

if ($opt_help) 
{
    &Usage; die "\n";
}

if($calib eq '')
{
    die "Plase specify a calibrator using the -c command\n";
}

if($opt_subcompact)
{
    $ELLLIMIT=33.0;
}

if($opt_time)
{
    $inttime=15;
    $scancal=14;
    $scan=14;
    $nflux = 40;
}

if($opt_help && $opt_subcompact && $opt_restart) #this will get rid of the only used once message.
{
    print "Happy Birthday!\n";  #this shouldn't ever execute
}

##################
# Initialization #
################## 
$mypid=$$;            # check process id
$myname = ${0};
command("project -r -i $mypid -f $myname");
print "The process ID of this $myname script is $mypid\n";
checkANT();         # check antennas to be used
command("radio");   # note: command is also subroutine.
command("stopChopping");
command("integrate -t $inttime");
print "----- initialization done -----\n";
print "$nicname starts flux observations!!\n";
print "Good Luck, $nicname!!\n";
@sma = (1,2,3,4,5,6,7,8);

#find the currently active receiver
for($d = 0; $d <= $#sma; $d++)
{
#    @frequency = ("Response from antenna 1:", " Low frequency RX = 0 = 2:B1: 256-360 GHz Port 7 (address = 3)", "High frequency RX = 5 = 5: D: 400-520 GHz Port 6 (no micro present)");
    @frequency = `tune -a $sma[$d] -c active`;
#    $freq = substr $frequency[3], 20, 1;
    $freq = substr $frequency[1], 20, 1;
    if($freq eq '0')
    {
	$freq = '1mm';
    }
    elsif($freq eq '2')
    {	
	$freq = '850';
    }
    else
    {
	$freq = '';
    }
    if($freq ne '')
    {	
	last;
    }
}
#check for a + in the calibrator name and put a \ before it
$calcheck = $calib;
if($calcheck =~ /\+/)
{
    @plus = split('\+',$calcheck);
    $calcheck = $plus[0] . '\+' . $plus[1];
}


print "The current wavelength is: $freq.\n\n";
print "Fetching calibrator list...\n";

@source = getSources();

$nsource = scalar(@source);

#for($c = 0; $c <= $#source; $c++)
#{
#    print "$source[$c], $ra[$c], $dec[$c]\n"
#}

$check = 0;
for($c = 0; $c <= $#cal; $c++)
{
    if($cal[$c] =~ /$calcheck/)
    {
	$check = 1;
	$j2000 = $cal[$c];
    }
    elsif($common[$c] =~ /$calcheck/)
    {
	$check = 1;
	$j2000 = $cal[$c];
    }
}

if($check == 0)
{
    die "Please specify a bright calibrator.\n";
}

for($g = 0; $g <= $#source; $g++)
{
    if($j2000 eq $source[$g])
    {
	splice @source, $g, 1;
	splice @ra, $g, 1;
	splice @dec, $g, 1;
    }
    if($source[$g] =~ /mwc349a/)
    {
	splice @source, $g, 1;
	splice @ra, $g, 1;
	splice @dec, $g, 1;
    }
}

for ($i=0;$i<=$#source;$i++) 
{
    $el[$i]=0.0;
    $ha[$i]=10.0;
    $ra[$i]=$ra[$i]*$deg2rad;
    $dec[$i]=$dec[$i]*$deg2rad;
}

#########################################
# Check the Elevation of the Calibrator #
#########################################
getLST();
calcEL();
checkcalEL();


#####################################
# Start with the flux calibration   #
#####################################

$foundflux = 0;

unless($opt_restart)
{
    print "Searching for an available flux calibrator\n";
    
# Uranus
    $sourceCoordinates=`lookup -s uranus`;
    chomp($sourceCoordinates);
    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
    {
	print "Uranus is available, and will be observed right now.\n";
	command("observe -s uranus");  
	command("sleep 5");
	tsys();
	command("integrate -s $nflux -t $inttime -w");
	$foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
    }
    else 
    {
	print "Uranus is not available now.\n";
    }
    if($foundflux == 0)
    {
# Neptune
	$sourceCoordinates=`lookup -s neptune`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	{
	    print "Neptune is available, and will be observed right now.\n";
	    command("observe -s neptune");  
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else 
	{
	    print "Neptune is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Callisto
	$sourceCoordinates=`lookup -s callisto`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	{
	    print "Callisto is available, and will be observed right now.\n";
	    command("observe -s callisto");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else 
	{
	    print "Callisto is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Ganymede
	$sourceCoordinates=`lookup -s ganymede`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	{
	    print "Ganymede is available, and will be observed right now.\n";
	    command("observe -s ganymede");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
	}
	else 
	{
	    print "Ganymede is not available now.\n";
	}
    }
    if($foundflux == 0)
    {
# Titan
	$sourceCoordinates=`lookup -s titan`;
	chomp($sourceCoordinates);
	($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	{
	    print "Titan is now available and will be observed.\n";
	    command("observe -s titan");
	    command("sleep 5");
	    tsys();
	    command("integrate -s $nflux -t $inttime -w");
	    $foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	}
	else 
	{
	    print "Titan is not available now.\n";
	}
    }
}


########################
# Start Observing Loop #
########################
$nloop=0;
print "Starting main loop...................\n";
while(1)
{
    printPID();
    $nloop=$nloop+1;  
    print "Loop No.= $nloop\n";
    getLST();
    calcEL();
    checkcalEL();
    pickupQS();

    if ($calskip==1) 
    {      # If the calibrator is too high EL ..... or too close to the Sun
        for ($i=0;$i<$#target;$i++) 
	{
            $Coordtar=`lookup -s $target[$i]`;
            if ($Coordtar eq '') 
	    {
                print "#######################################\n";
                print "###### WARNING WARNING WARNING ########\n";
                print "##### source $target[$i] not found. ######\n";
                print "#######################################\n";
                print "Skip $target[$i] ..... \n";
            }
            else 
	    {
                chomp($Coordtar);
                ($aztar,$eltar,$suntar)=split(' ',$Coordtar);
                print "$target[$i] at EL=$eltar (Degree) and Az=$aztar (Degree) and SunDist=$suntar (Degree)\n";
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>$SUNANGLE) 
		{
                    print "$target[$i] is available, and will be observed right now.\n";
                   command("observe -s $target[$i]");
                   command("sleep 5");
                   command("tsys");
                   command("integrate -s $scan -t $inttime -w");
                }
                else 
		{
                    print "$target[$i] is NOT available now, skip ......\n";
                }
            }
        }
    }
    elsif ($calskip==0)  
    {  # If the calibrator is available ....
        command("observe -s $calib");
        command("sleep 5");
        command("tsys");
        command("integrate -s $scancal -t $inttime -w");
        for ($i=0; $i <= $#target; $i++) 
	{
            $Coordtar=`lookup -s $target[$i]`;
            if ($Coordtar eq '') 
	    {
                print "#######################################\n";
                print "###### WARNING WARNING WARNING ########\n";
                print "##### source $target[$i] not found. ######\n";
                print "#######################################\n";
                print "Skip $target[$i] ..... \n";
            }
            else 
	    {
                chomp($Coordtar);
                ($aztar,$eltar,$suntar)=split(' ',$Coordtar);
                print "$target[$i] at EL=$eltar (Degree) and Az=$aztar (Degree) and SunDist=$suntar (Degree)\n";
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>$SUNANGLE) 
		{
                    print "$target[$i] is available, and will be observed right now.\n";
                    command("observe -s $target[$i]");
                    command("sleep 5");
                    command("tsys");
                    command("integrate -s $scan -t $inttime -w");
                }
                else 
		{
                    print "$target[$i] is NOT available now, skip ......\n";
                }
            }
        }
    }
    else 
    {                  # If the calibrator is too low EL, finish observations!!
        print "------- Final Calibration ------- \n";
        # Calibrator
        $sourceCoordinates=`lookup -s $calib`;
        chomp($sourceCoordinates);
        ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
        if (($ELLLIMIT-1)<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	{
            command("observe -s $calib");
            command("sleep 5");
            command("tsys");
            command("integrate -s $scan -t $inttime -w");
        }
        else 
	{
            print "Calibrator $calib is NOT available now, skip ......\n";
        }

	if($foundflux == 0)
	{
	    print "Searching for an available flux calibrator\n";
	    
# Uranus
	    $sourceCoordinates=`lookup -s uranus`;
	    chomp($sourceCoordinates);
	    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	    {
		print "Uranus is available, and will be observed right now.\n";
		command("observe -s uranus");  
		command("sleep 5");
		tsys();
		command("integrate -s $nflux -t $inttime -w");
		$foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	    }
	    else 
	    {
		print "Uranus is not available now.\n";
	    }
	}
	if($foundflux == 0)
	{
# Neptune
	    $sourceCoordinates=`lookup -s neptune`;
	    chomp($sourceCoordinates);
	    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	    {
		print "Neptune is available, and will be observed right now.\n";
		command("observe -s neptune");  
		command("sleep 5");
		tsys();
		command("integrate -s $nflux -t $inttime -w");
		$foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	    }
	    else 
	    {
		print "Neptune is not available now.\n";
	    }
	}
	if($foundflux == 0)
	{
# Callisto
	    $sourceCoordinates=`lookup -s callisto`;
	    chomp($sourceCoordinates);
	    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	    {
		print "Callisto is available, and will be observed right now.\n";
		command("observe -s callisto");
		command("sleep 5");
		tsys();
		command("integrate -s $nflux -t $inttime -w");
		$foundflux = 1;
	    }
	    else 
	    {
		print "Callisto is not available now.\n";
	    }
	}
	if($foundflux == 0)
	{
# Ganymede
	    $sourceCoordinates=`lookup -s ganymede`;
	    chomp($sourceCoordinates);
	    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	    if ($ELLLIMIT<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	    {
		print "Ganymede is available, and will be observed right now.\n";
		command("observe -s ganymede");
		command("sleep 5");
		tsys();
		command("integrate -s $nflux -t $inttime -w");
		$foundflux = 1;
	    }
	    else 
	    {
		print "Ganymede is not available now.\n";
	    }
	}
	if($foundflux == 0)
	{
# Titan
	    $sourceCoordinates=`lookup -s titan`;
	    chomp($sourceCoordinates);
	    ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
	    if ($ELLLIMIT < $sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>$SUNANGLE) 
	    {
		print "Titan is now available and will be observed.\n";
		command("observe -s titan");
		command("sleep 5");
		tsys();
		command("integrate -s $nflux -t $inttime -w");
		$foundflux = 1;
#   command("integrate -s 0 -t $inttime -w");
	    }
	    else 
	    {
		print "Titan is not available now.\n";
	    }
	}
	
    }
}



###############
# Subroutines #
###############
sub Usage()
{
printf "Usage: nflux_main.pl -c (calibrator name, must be over 1.5 Jy)
options:
-r (restart, skips the flux calibrators.  Be sure to use the same calibrator)
-s (subcompact, sets the lower elevation limit to 33, for use in subcompact configuration)
-t (time, sets the integration time to 15 seconds and doubles the number of scans)\n";
}

# --- interrupt handler for CTRL-C ---
sub finish 
{
   exit(1);
}

# --- print PID ---
# usage: printPID();
sub printPID 
{
  print "The process ID of this $myname script is $mypid\n";
}

# --- check antennas ---
# usage: checkANT();
# This subroutine checks active antennas and stores them
# as an array @sma.
sub checkANT 
{
  print "Checking antenna status ... \n";
  for ($i = 1; $i <=8; $i++) 
  {
     $exist = `value -a $i -v antenna_status`;
     chomp($exist);
     if ($exist) 
     {
        print "Found antenna ",$i," in the array!\n";
        @sma = (@sma,$i);
     }
  }
  print "Antennas @sma are going to be used.\n";
}

# --- performs a shell task with delay and printing. ---
sub command 
{
   my ($a);
   print "@_\n";                        # print the command
   $a=system("@_");                     # execute the command
#   sleep 1;                             # sleep 1 sec
   return $a;
}

# --- get LST in hours ---
# usage: getLST();
# This subroutine updates the global variable $LST, and prints
# its value.
sub getLST 
{
    $LST= `value -a $sma[0] -v lst_hours`;
#    $LST = `./lst`;
    chomp($LST);
    print "LST [hr]= $LST\n";
}


# --- Calculate Elevation for each quasars ---
# usage: calcEL();
sub calcEL 
{
    print "Looking up available quasars ..........\n";
    for ($i=0;$i<=$#source;$i++) 
    {
        $ha[$i] = $LST*15.0*$deg2rad-$ra[$i] ;  #RA and Dec are both converted to radians eariler in the script
        $sinel[$i] = sin($LAT)*sin($dec[$i])+cos($LAT)*cos($dec[$i])*cos($ha[$i]);
        $cosel[$i] = (1.0 - $sinel[$i]**2.0)**0.5;
        $el[$i] = atan2($sinel[$i],$cosel[$i])*$rad2deg ;
        $ha[$i] = $ha[$i]*$rad2deg/15.0 ;
        if ($ha[$i]>12.0) {$ha[$i]=$ha[$i]-24.0;}
        if ($ha[$i]<-12.0) {$ha[$i]=$ha[$i]+24.0;}

        # print "SINE(EL) = $sinel[$i] \n" ;
        # print "COSINE(EL) = $cosel[$i] \n" ;
        # print "$source[$i] at EL=$el[$i] (Degree) and HA=$ha[$i] (hour)\n"
    }
}

#
# --- Check if your calibrator is usable .... ----
sub checkcalEL 
{
    $Coordcal=`lookup -s $calib`;
    chomp($Coordcal);
    ($azcal,$elcal,$suncal)=split(' ',$Coordcal);
    print "Calibrator $calib at EL=$elcal (Degree) and Az=$azcal (Degree) and SunDist=$suncal (Degree)\n";
    if ($elcal<$ELLLIMIT && $suncal>$SUNANGLE) 
    {
        print "Calibrator $calib is at too low EL=$elcal (deg) ...\n";
        print "                \n";
        print "You may be at the end of the flux track. \n";
        print "Thank you very much for your hard work. \n";
        print "Will do final sequence \n";
        print "                \n";
        print "If you have not done anything yet, \n";
        print "Please wait for a while, or use a different calibrator. \n";
        print "And.... You may kill this script with the process ID $mypid \n";
        $calskip=-1;
#        exit(1);
    }
    elsif ($ELLLIMIT<=$elcal && $elcal<$ELULIMIT && $suncal>$SUNANGLE) 
    {
        print "Calibrator $calib is in the observable EL range.\n";
        $calskip=0;
    }
    else 
    {
        print "Calibrator $calib is at too high EL=$elcal (deg) ... \n";
        print "Or Calibrator $calib is too close to the Sun at $suncal (deg) ... \n";
        $calskip=1;
    }
}


#
# --- Pick Up Quasars to Observe ---
sub pickupQS 
{
#    print "Source  Dec    HA     El\n";
    if(($nloop % 2) == 1)
    {
#	print "in the odd loop\n";
	@group1 = ();
	@group2 = ();
	@group3 = ();
	
	for($l = 0; $l <= $#source; $l++)
	{
#	    print "$source[$l], $dec[$l], $ha[$l], $el[$l]\n";
	    #In radians!
	    if($dec[$l] >= .610865 && $el[$l] > $ELLLIMIT && $el[$l] < $ELULIMIT)
	    {
		push @group1, $source[$l];
	    }
	    elsif($dec[$l] < .610865 && $dec[$l] >= -.087266 && $el[$l] > $ELLLIMIT && $el[$l] < $ELULIMIT)
	    {
		push @group2, $source[$l];
	    }
	    elsif($dec[$l] < -.087266 && $el[$l] > $ELLLIMIT && $el[$l] < $ELULIMIT)
	    {
		push @group3, $source[$l];
	    }
	}
    }
    else
    {
#	print "in the even group\n";
	@group1 = ();
	@group2 = ();
	@group3 = ();
	
	for($l = 0; $l <= $#source; $l++)
	{
#	    print "$source[$l], $dec[$l], $ha[$l], $el[$l]\n";
	    if($ha[$l] < -2.5 && $el[$l] > $ELLLIMIT && $el[$l] < $ELULIMIT)
	    {
		push @group1, $source[$l];
	    }
	    elsif($ha[$l] >= -2.5  && $ha[$l] <= 2.5 && $el[$l] > $ELLLIMIT && $el[$l] < $ELULIMIT)
	    {
		push @group2, $source[$l];
	    }
	    elsif($ha[$l] > 2.5 && $el[$l] > $ELLLIMIT && $el[$l] < $ELULIMIT)
	    {
		push @group3, $source[$l];
	    }
	}    
    }

#    print "Group 1\'s list:\n";
#    for($r = 0; $r <= $#group1; $r++)
#    {
#	print "$group1[$r]\n";
#    }
#    print "Group 2\'s list:\n";
#    for($r = 0; $r <= $#group2; $r++)
#    {
#	print "$group2[$r]\n";
#    }
#    print "Group 3\'s list:\n";
#    for($r = 0; $r <= $#group3; $r++)
#    {
#	print "$group3[$r]\n";
#    }

    $n1 = scalar(@group1);
    $n2 = scalar(@group2);
    $n3 = scalar(@group3);

#    print "group1 contains $n1 sources.\n";
#    print "group2 contains $n2 sources.\n";
#    print "group3 contains $n3 sources.\n";
    unless($n1 == 0)
    {
	$s1=int(rand($n1));
	$target[0]=$group1[$s1];
    }
    else
    {
	$s1=int(rand($nsource));
	$target[0]=$source[$s1];
    }
    unless($n2 == 0)
    {
	$s1=int(rand($n2));
	$target[1]=$group2[$s1];
    }
    else
    {
	$s1=int(rand($nsource));
	$target[1]=$source[$s1];
    }
    unless($n3 == 0)
    {
	$s1=int(rand($n3));
	$target[2]=$group3[$s1];
    }
    else
    {
	$s1=int(rand($nsource));
	$target[2]=$source[$s1];
    }

    for ($i=0; $i <= $#target; $i++) 
    {
        print "$nicname will observe $target[$i]\n";
    }
#    if(($nloop % 2) == 0)
#    {
#	exit;
#    }
}


#The function to get the sources from the file in /global
sub getSources 
{
    @fluxdata = ();  #each line in the file split on commas
    $fsource = 'hi';
    $num = 0;
    $calnum = 0;
    @newsourcelist = ();
    open(DATA, "</global/catalogs/callist/recent_flux_measurements.dat");
    while(<DATA>)
    {
	$line = $_;
	unless($line =~ /^\#/)  #skips any line that starts with #
	{
	    @fluxdata = split(",", $line);
	    $uselessnum = split(" ", $fluxdata[2]); #not actually useless
	    if($uselessnum == 1)  #this is zero if there's whitespace, so it carries the J2000 name to lines that are only white
	    {
		$fsource = $fluxdata[2];
		$fcommon = $fluxdata[0];
		$fra = $fluxdata[3];
		$fdec = $fluxdata[4];
	    }
#	print "$fsource\n";
#	print "$uselessnum\n";
	    if($fluxdata[7] =~ /SMA/)  #only sources observed at the sma
	    {
#	    print "the above source is an SMA source!\n";
		if($freq eq '')
		{
		    print "The current frequency couldn't be found, using 230.\n";
		    $freq = "1mm";
		}
		if($fluxdata[5] eq $freq) #only look at sources in the requested band
		{

		    if($fluxdata[8] > 1.5)
		    {
			$cal[$calnum] = $fsource;
#		    print "$fsource is bright enough.  It's common name is: $fluxdata[0]\n";
			$common[$calnum] = $fcommon;
			$calnum++;
		    }
		    if($fluxdata[8] > .9)
		    {
			$newsourcelist[$num] = $fsource;
			#calculate ra (3) and dec (4) in decimal form
			@temp = split(":", $fra);
			$ra[$num] = $temp[0] * 15 + $temp[1] * .25 + $temp[2] * .25 / 60;
			@temp = split(":", $fdec);
			if($temp[0] >= 0)
			{
			    $dec[$num] = $temp[0] + $temp[1] / 60 + $temp[2] / 3600;
			}
			else
			{
			    $dec[$num] = $temp[0] - $temp[1] / 60 - $temp[2] / 3600;
			}
			$num++;
		    }
		}
	    }
	}
    }
    close(DATA);
    return @newsourcelist;
}

sub tsys
{
    if($tsysant != 0)
    {
	print "tsys -a 1,4,5,6,7\n";
	if($simulateMode==0) {system("tsys -a 1,4,5,6,7");}
	sleep(1);
    }
    else
    {
	print "tsys\n";
	if($simulateMode==0) {system("tsys");}
	sleep(1);
    }
}
