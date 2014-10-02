# ----------------------------------------------------------------------------#
# ---------------------------    Subroutines    ------------------------------#
# These subroutines do not depend on specific observations. Thus, they should
# not be changed.

# init values for optional params.
#use Getopt::Popt;
use Getopt::Long;
$Getopt::Long::autoabbrev=1;


$nloop=1;  
GetOptions('loop=i' => \$nloop, 'time=f' => \$givenLST, 'simulate', 'restart');
$nloop--;

if($opt_simulate) {$simulateMode=1;} else {$simulateMode=0;}
if($opt_restart) {$restart=1;} else {$restart=0;}

#Ctrl-C interrupt handler;
$SIG{INT}=\&finish;

$mypid = $$;
$myname = ${0};
command("project -r -i $mypid -f $myname");
print "The process ID of this $myname script is $mypid\n";

$unameResponse = `uname -a`;
($thisOS,$thisMachine,$otherStuff)=split(' ',$unameResponse);

if($thisMachine ne "hal9000") {
	print "Not running on hal9000, entering simulation mode.\n";
	$simulateMode=1;
} 

# --- interrupt handler for CTRL-C ---
sub finish {
	command("radecoff -r 0 -d 0");
	sleep(1);
   print "Please remember to stow the antennas safely if you are leaving.\n";
   print "Please check that ra and dec offsets are set to zeroes. \n";
   exit(1);
}

# --- print PID ---
# usage: printPID();
sub printPID {
  print "The process ID of this $0 script is $mypid\n";
}

# --- check antennas ---
# usage: checkANT();
# This subroutine checks active antennas and stores them
# as an array @sma.
sub checkANT {
	my ($i,$exist);
   print "Checking antenna status ... \n";
   for ($i = 1; $i <=8; $i++) {
      if($simulateMode==0) { 
         $exist = `value -a $i -v antenna_status`;
         chomp($exist);
      } else {
         $exist=1;
      }
      if ($exist) {
         print "Found antenna ",$i," in the array!\n";
         @sma = (@sma,$i);
      }
   }
   print "Antennas @sma are going to be used.\n";
# write out a copy of this script in the data area
  $thisfile=${0};
  $hour=(gmtime)[2];
  $minute=(gmtime)[1];
  $seconds=(gmtime)[0];
  $day=(gmtime)[3];
  $month=(gmtime)[4]+1;
  $year=(gmtime)[5];
  $year+=1900;
  $timeStamp=sprintf("%4d%02d%02d_%02d%02d%02d",
		$year,$month,$day,$hour,$minute,$seconds);
  $scriptFileName=$thisfile."_".$timeStamp;
  $directoryName=`readDSM -v DSM_AS_FILE_NAME_C80 -m m5 -l`;
  chomp($directoryName);
  $scriptFileNameWithPath=$directoryName."/".$scriptFileName;

  print "Copy command:
  $thisfile\n
  cp $thisfile $scriptFileNameWithPath\n";

  $cpresponse=`cp $thisfile $scriptFileNameWithPath`;
	if($cpresponse ne "") {
	print "Copying this script to data area...\n $cpresponse";
	}
}

# --- check elevation of a source ---
# usage: checkEl($sourcename);
# This subroutine checks the elevation of a source and prints
# its value.
# e.g.,
#      $a=checEl('your_source);
# will give you the eleveation in $a and also prints it on screen.
# Also,
#      $a=checkEl('your_source',1);
# will give $a the elevation but does not print the value.
# The second argument, 'silent flag', is optional.
sub checkEl{
	my ($sourcename)=$_[0];
   my ($silent)    =(defined($_[1]) and $_[1]);
	my ($sourceCoordinates,$sourceAz,$sourceEl,$sunDistance);

   if((not $simulateMode) or ($thisMachine eq "hal9000")) {
		$sourceCoordinates=`lookup -s $sourcename`;
   	chomp($sourceCoordinates);
   	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
		if ($sourceAz eq 'error_flag') {
			print "##########################################\n";
			print "######## WARNING WARNING WARNING #########\n";
			print "##### source $sourcename not found. ######\n";
			print "##########################################\n";
			for ($i=0; $i<7;$i++) {print "\a";sleep 1;}
			die   " quiting from the script \n";
		}
   } else {
   	$sourceEl=30.;
   }
   if (not $silent) {
      printf("%s is at %4.1f degrees elevation\n",$sourcename,$sourceEl);
   }
   return $sourceEl;
}

# ---- mainLoop ----
sub mainLoop () {
@inputArgs=@_;
$narg=$#inputArgs+1;
if(($narg%2)!=0) {die "Incorrect number of arguments.\n";}
        for ($i=1;$i<$narg;$i=$i+2) {
        $numbercheck=$inputArgs[$i];
                if(!($numbercheck !~ /\D/)) {
                $j=$i+1;die "Argument $j is Not a number\n";
                }
        }

        for ($i=0;$i<$narg;$i=$i+2) {
	LST(); $targel=checkEl($inputArgs[$i]);
        $current_source = $inputArgs[$i];

	if($targel>$MINEL_CHECK) {
        command("observe -s $inputArgs[$i]");
        command("tsys -a 1,3,4,5,6,7");
        command("integrate -s $inputArgs[$i+1] -w");
	} else {
	print "Source $inputArgs[$i] is too low: $targel deg.\n";
        print "Skipping it.\n";
        }

    }

}

sub mainLoopMosaic () {
@inputArgs=@_;
$narg=$#inputArgs+1;
if(($narg%3)!=0) {die "Incorrect number  of arguments.\n";}
        for ($i=1;$i<$narg;$i=$i+3) {
        $numbercheck=$inputArgs[$i];
                if(!($numbercheck !~ /\D/)) {
                $j=$i+1;die "Argument $j is Not a number\n";
                }
        }

        for ($i=0;$i<$narg;$i=$i+3) {
	LST(); $targel=checkEl($inputArgs[$i]);
        $current_source = $inputArgs[$i];

	if($targel>$MINEL_CHECK) {
        command("observe -s $inputArgs[$i]");
        ($raoff,$decoff)=split(',',$inputArgs[$i+2]);
        command("radecoff -r $raoff -d $decoff");
        command("tsys -a 1,3,4,5,6,7");
        command("integrate -s $inputArgs[$i+1] -w");
        command("radecoff -r 0 -d 0");
	} else {
	print "Source $inputArgs[$i] is too low: $targel deg.\n";
        print "Skipping it.\n";
        }

}
}





# --- performs a shell task with delay and printing. ---
sub command {
   print "@_\n";       				# print the command
   if($simulateMode==0) {system("@_");}	# execute the command
   sleep 1;                             	# sleep 1 sec
}

# --- get LST in hours ---
# usage: LST(); or LST(1);
# The return value of this subroutine is LST in hours.
# The time is taken from the Reflective Memory. However, when in $simulatemode,
# then the return value is simulated LST in hours.
# It can print the current LST if used as LST(1);
sub LST {
	my ($LST);

   if (defined($givenLST)) {
      $LST=$givenLST;
   } else {
		if($simulateMode) {$LST=simlst();}
  		else {$LST= `value -a $sma[0] -v lst_hours`;}
	  	chomp($LST);
   }
	if ($_[0]) {printf("LST [hr]= %6.2f\n",$LST);}

	return $LST;
}

# --- simulate LST ------
sub simlst {
#$longitude = -4.76625185186; # hours (71d29'37.6s W) Haystack
$longitude = -10.365168199815; # hours (-155.477522997222 W, pad-1) MaunaKea

$seconds=(gmtime)[0]+(gmtime)[1]*60.+(gmtime)[2]*3600.;
$ut=$seconds/3600.;
$d=(gmtime)[3];
$month=(Jan,Feb,Mar,Apr,May,Jun,July,Aug,Sep,Oct,Nov,Dec)[(gmtime)[4]];
if((gmtime)[5]>=98) {$year=1900+(gmtime)[5];}
else {$year=2000+(gmtime)[5];}


$mn=1+(gmtime)[4]; 
# adding 1 because the array index for month goes from 0 to 11.

# Using Eqn. 12.92-1, p 604 of the Green Book (Expl. Supp. to Almanac)
# for calculating JD from the given gregorian calendar
# date in terms of day number, month number and year number.
# lots of int()s have had to be inserted because  Perl 
# cannot do integer arithmetic. 

$term1=int((1461*int(($year+4800+int(($mn-14)/12))))/4);  
$term2=int((367*($mn-2-12*(int(($mn-14)/12))))/12);
$term3=int((3*int(($year+4900+int(($mn-14)/12))/100))/4);
$JD=$term1+$term2-$term3+$d-32075;

$TJD=$JD+$seconds/86400.;
$TJDint=int($TJD);

$T=($JD-2451545.0)/36525.; # definition.

$epsilon0 = 21.448-46.8150 * $T 
			- 0.00059 * $T * $T
				+ 0.001813 * $T * $T * $T;
#Eqn 3.222-1, p114 of the Green Book
$epsilon0=$epsilon0/3600.+ 26./60. + 23.; #in degrees

$epsilon0=$epsilon0* 0.0174534; #in radians

$dnum= $JD-2451545.0;

$radian=4.0*atan2(1,1)/180.;

$angle1=(125.0-0.05295*$dnum)*$radian;
$angle2=(200.9+1.97129*$dnum)*$radian;

$delta=0.0026 * cos($angle1)+0.0002*cos($angle2);

$epsilon=$epsilon0+$delta;

$Tdu=$dnum/36525.; # No. of Julian centuries
$theta=($ut*(1.002737909350795+5.9006e-11*$Tdu-5.9e-15*$Tdu*$Tdu))*3600.; # seconds
$Tdu2=$Tdu*$Tdu;
$Tdu3=$Tdu2*$Tdu;
$gmst0=24110.54841+8640184.812866*$Tdu+0.093104*$Tdu2-6.2e-6*$Tdu3;
$gmst0=$gmst0/3600.; # hours

$frac=$gmst0/24.;
$quot=int($frac);
$gmst0=$gmst0-$quot*24;

if($gmst0 lt 0.){$gmst0=$gmst0+24.;}
$gmst=($gmst0+$theta/3600.); # hours

$delta_psi=-0.0048*sin($angle1)-0.0004*sin($angle2);
#Eq 3.225-4, p120 (delta-psi is in degrees)

$eqnequinox=$delta_psi*cos($epsilon*$radian); # degrees
$eqnequinox=$eqnequinox*6.66666667e-2; # hours


$gast=$gmst+$eqnequinox;
$lst=$gast+$longitude;
$frac=$lst/24.;
$quot=int($frac);
$lst=$lst-$quot*24;

if($lst lt 0.){$lst=$lst+24.;}

return $lst;
}

# --------------------------------------------------------------------------- #
# History
# 2001-04---; adapted from iscript.pl: first set of subroutines for
#             sma scripted operations- Nimesh.
# 2002-08-20 : written by K. Sakamoto for the SMA nearby galaxy team
# 2003-11-06, ABP, pirated and rewritten for general template
# 2004-03-15, ABP, added "lookup" utility
# 2004-03-2?, Nimesh, simulation mode!
# 2004-03-23, KS, replaced getLST() with LST(). other housekeeping works.
# 2004-08---, Nimesh, some more work on simulation-- comments will be
#              added later
# 2004-08-26, Nimesh, copying itself into data area upon execution.
# 2007-02-06 Nimesh, added mainLoop and mainLoopMosaic subroutines.
