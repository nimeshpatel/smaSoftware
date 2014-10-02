#!/usr//bin/perl -w
#
# delete the very first # sign on the topmost line to check this 
# script for syntax errors if you make any changes.
# Then put back the # sign.
#
# ----------------------------------------------------------------------------#
# ---------------------------    Subroutines    ------------------------------#
# These subroutines do not depend on specific observations. Thus, they should
# not be changed.

use Time::Local;

# init values for optional params.
use Getopt::Long;
$Getopt::Long::autoabbrev=1;



sub initialize() {
print "initializing.....\n";
$silent=0;
$newSourceFlag=0;
$tsysDelay=11.;
$observeDelay=10.;
$pointDelay=600.;
$prevUnixTime=0;
$unixTime=0;
$totalIntegrationTime=0;
$decsign=1;
$sourceNameArgindex=0;
$firstTime=1; # used for unixtime conversion if given UTC
$nloop=1;  
GetOptions('loop=i' => \$nloop, 'time=s' => \$givenUTC, 'simulate', 'restart');
$nloop--;
if($givenUTC) {
($mn,$d,$year,$gh,$gm)=split(' ',$givenUTC);
$mn=$mn-1;
$unixTime=timelocal(0,$gm,$gh,$d,$mn,$year);
$prevUnixTime=$unixTime;
}

if($opt_simulate) {$simulateMode=1;} else {$simulateMode=0;}
if($opt_restart) {$restart=1;} else {$restart=0;}

#Ctrl-C interrupt handler;
$SIG{INT}=\&finish;


$unameResponse = `uname -a`;
($thisOS,$thisMachine,$otherStuff)=split(' ',$unameResponse);

if($thisMachine ne "hal9000") {
	print "Not running on hal9000, entering simulation mode.\n";
	$simulateMode=1;
} 

$mypid = $$;
$myname = ${0};
if(not $simulateMode) {
command("project -r -i $mypid -f $myname");
printPID();
} else {
print "Script: $myname.\n";
}

}

# --- interrupt handler for CTRL-C ---
sub finish {
   print "Please remember to stow the antennas safely if you are leaving.\n";
   exit(1);
}

# --- print PID ---
# usage: printPID();
sub printPID {
 if(not $simulateMode) {
  print "The process ID of this $0 script is $mypid\n";
  }
}


# --- check antennas ---
# usage: checkANT();
# This subroutine checks active antennas and stores them
# as an array @sma.
sub checkANT {
initialize();
	my ($i,$exist);
   if(not $simulateMode) {print "Checking antenna status ... \n";}
   for ($i = 1; $i <=8; $i++) {
      if($simulateMode==0) { 
         $exist = `value -a $i -v antenna_status`;
         chomp($exist);
      } else {
         $exist=1;
      }
      if ($exist) {
	if(not $simulateMode) {print "Found antenna ",$i," in the array!\n";}
         @sma = (@sma,$i);
      }
   }
   if(not $simulateMode) { print "Antennas @sma are going to be used.\n";}
}

# --- check elevation of a source ---
# usage: checkEl($sourcename);
# This subroutine checks the elevation of a source and prints
# its value.
# e.g.,
#      $a=checkEl('your_source);
# will give you the eleveation in $a and also prints it on screen.
# Also,
#      $a=checkEl('your_source',1);
# will give $a the elevation but does not print the value.
# The second argument, 'silent flag', is optional.
sub checkEl {
	my ($sourcename)=$_[0];
	print "source name = $_[0] \n";
#   my ($silent)    =(defined($_[1]) and $_[1]);
	my ($sourceCoordinates,$sourceAz,$sourceEl,$sunDistance);

   if((not $simulateMode) or ($thisMachine eq "hal9000")) {
		$sourceCoordinates=`./lookup -s $sourcename`;
   	chomp($sourceCoordinates);
   	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
		if ($sourceAz =~ /Source/) {
			print "##########################################\n";
			print "######## WARNING WARNING WARNING #########\n";
			print "##### source $sourcename not found. ######\n";
			print "##########################################\n";
			die   " quiting from the script \n";
		}
     } else {
	$lookupTime="$mn $d $year $hour:$min";
	# check sourcename for ra dec input - if so, parse it
	if($sourcename =~ /-r/) {
	print "sourcename -r = $sourcename \n";
	print "got ra/dec arguments...parsing them...\n";
          if(!($sourcename =~ /-d/)) { die "both ra and dec are required\n";}
	@sourcenameArgs=split(' ',$sourcename);
	   $iarg=0;
	   foreach $arguments (@sourcenameArgs) {
	   if($arguments=~/s/) {$sourceNameArgindex=$iarg+1;}
	   if($arguments=~/r/) {$raArgindex=$iarg+1;}
	   if($arguments=~/d/) {$decArgindex=$iarg+1;}
	   if($arguments=~/e/) {$epochArgindex=$iarg+1;}
	   $iarg++;
	   }
	   $rastring=$sourcenameArgs[$raArgindex];
	   $sourceNameString=$sourcenameArgs[$sourceNameArgindex];
     	   print "sourceNameString = $sourceNameString \n";
	   $decstring=$sourcenameArgs[$decArgindex];
	   $givenEpoch=$sourcenameArgs[$epochArgindex];
	   ($rah,$ram,$ras)=split('\:',$rastring);
	   ($decd,$decm,$decs)=split('\:',$decstring);
	   $givenRA=$rah+$ram/60.+$ras/3600.;
	   if($decd<0.) {$decsign=-1;$decd=-$decd}
	   $givenDEC=$decd+$decm/60.+$decs/3600.;
	   if($decsign==-1){$givenDEC=-$givenDEC;}
	  $parsedSourcename="$sourceNameString -r $givenRA -d $givenDEC -e $givenEpoch ";
	$newSourceFlag=1;
	} else {
	$newSourceFlag=0;
	}
	
	if($newSourceFlag==1) {
	$sourceCoordinates=`./lookup -s $parsedSourcename -t "$lookupTime"`;
	} else {
print "lookup time: $lookupTime\n";
	$sourceCoordinates=`./lookup -s $sourcename -t "$lookupTime"`;
	}
	
   	chomp($sourceCoordinates);
   	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
		if ($sourceAz =~ /Source/) {
			print "##########################################\n";
			print "######## WARNING WARNING WARNING #########\n";
			print "##### source $sourcename not found. ######\n";
			print "##########################################\n";
			die   " quiting from the script \n";
   		}
   	if (not $silent) {
	if($newSourceFlag==1) {
    	 printf("%s is at %4.1f degrees elevation\n",$sourceNameString,$sourceEl);
	 } else {
      	 printf("%s is at %4.1f degrees elevation\n",$sourcename,$sourceEl);
	 }
   	}
   }   
   return $sourceEl;
}


# --- performs a shell task with delay and printing. ---
sub command {
	my ($givenCommand)=$_[0];
   if($simulateMode==0) {system("$givenCommand");} # execute the command
	
	# if simulation, then increment time as given by integrate command.
	if($simulateMode==1) {
		$prevUnixTime=$unixTime;
		if($givenCommand=~/integrate/) {
		($intcommand,$tors,$timeorscans)=split(' ',$givenCommand);
			if($tors =~/t/) {
			$integrationTime=$timeorscans;}
			if($tors =~/s/) {
			$numberOfScans=$timeorscans;
			$totalIntegrationTime=$integrationTime*$numberOfScans;
			}
			$unixTime=$unixTime+$totalIntegrationTime;
		}
		if($givenCommand=~/tsys/) {
			$unixTime=$unixTime+$tsysDelay;
		}
		if($givenCommand=~/observe/) {
			$unixTime=$unixTime+$observeDelay;
		}
		if($givenCommand=~/point/) {
		@pointArgs=split(' ',$givenCommand);
		for($iarg=0;$iarg<=$#pointArgs;$iarg++) {
		if($pointArgs[$iarg]=~/r/) {$pointRepeats=$pointArgs[$iarg+1];}
		}
			$unixTime=$unixTime+$pointDelay*$pointRepeats;
		}
   ($print_sec,$print_min,$print_hour,$print_d,$print_mon,
		$print_year,$wday,$yday,$isdst) = localtime($prevUnixTime);
	$print_year+=1900;
	$print_mon++;
 printf "\t%02d/%02d/%d %02d:%02d:%02d --> ",$print_mon,$print_d,
		$print_year,$print_hour,$print_min,$print_sec;
   ($print_sec,$print_min,$print_hour,$print_d,$print_mon,
		$print_year,$wday,$yday,$isdst) = localtime($unixTime);
	$print_year+=1900;
	$print_mon++;
  printf "%02d/%02d/%d %02d:%02d:%02d  ---- %s\n",$print_mon,$print_d,
		$print_year,$print_hour,$print_min,$print_sec,$givenCommand;
	  # if($givenCommand =~ /observe/) {
	  # ($observecommand,$presentsource)=split('\-s',$givenCommand);
	  # $presentElevation=checkEl($presentsource);
	  # printf " : el= %.2f deg.\n",$presentElevation;
	  # } else {print "\n";}
	} else {sleep 1;} 
}

# --- get LST in hours ---
# usage: LST(); or LST(1);
# The return value of this subroutine is LST in hours.
# The time is taken from the Reflective Memory. However, when in $simulatemode,
# then the return value is simulated LST in hours.
# It can print the current LST if used as LST(1);
sub LST {
	my ($LST);
		if($simulateMode) {$LST=simlst();}
  		else {$LST= `value -a $sma[0] -v lst_hours`;}
	  	chomp($LST);
	if (not $simulateMode and $_[0])  {printf("LST [hr]= %6.2f\n",$LST);}
	printf("LST [hr]= %6.2f\n",$LST);

	return $LST;
}

# --- simulate LST ------
sub simlst {
#$longitude = -4.76625185186; # hours (71d29'37.6s W) Haystack
$longitude = -10.365168199815; # hours (-155.477522997222 W, pad-1) MaunaKea

if($givenUTC eq "") {
$seconds=(gmtime)[0]+(gmtime)[1]*60.+(gmtime)[2]*3600.;
$ut=$seconds/3600.;
$d=(gmtime)[3];
if((gmtime)[5]>=98) {$year=1900+(gmtime)[5];}
else {$year=2000+(gmtime)[5];}

$mn=1+(gmtime)[4]; 
# adding 1 because the array index for month goes from 0 to 11.
}

# Using Eqn. 12.92-1, p 604 of the Green Book (Expl. Supp. to Almanac)
# for calculating JD from the given gregorian calendar
# date in terms of day number, month number and year number.
# lots of int()s have had to be inserted because  Perl 
# cannot do integer arithmetic. 

if($givenUTC) {
if($firstTime==1) { 
($mn,$d,$year,$gh,$gm)=split(' ',$givenUTC);
$mn=$mn-1;
$unixTime=timelocal(0,$gm,$gh,$d,$mn,$year);
$prevUnixTime=$unixTime;
#print "unixTime from given UTC: $unixTime\n";
$firstTime=0;
}
($sec,$min,$hour,$d,$mon,$year,$wday,$yday,$isdst) = localtime($unixTime);
$mn=$mon+1;
$year=1900+$year;
$seconds=$sec+$min*60.+$hour*3600.;
$ut=$seconds/3600.;
}


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
# ????-??-?? : Nimesh
# 2002-08-20 : adopted  by K. Sakamoto for the SMA nearby galaxy team
# 2003-11-06, ABP, pirated and rewritten for general template
# 2004-03-15, ABP, added "lookup" utility
# 2004-03-2?, Nimesh, simulation mode!
# 2004-03-23, KS, replaced getLST() with LST(). other housekeeping works.
# 2004-08-08, Nimesh, some more work on simulation  mode- added flow of time.


