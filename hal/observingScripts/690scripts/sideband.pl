#!/usr/bin/perl -w
############################################
# A Perl Script for Baseline Track.        #
#                                          #
# Author: S. Takakuwa (C.f.A.) 2003Aug18   #
#                                          #
# 2004Jan22; Only Strong Source            #
# 2004Jun30; 2 quasars for one calib       #
# 2004Sep22; Add SunDist Checking ...      #
############################################
# Constants
$pi=atan2(1,1)*4.0;
$rad2deg=180.0/$pi;
$deg2rad=1./$rad2deg;
$LAT=19.82420526388;  # Lattitude of the SMA (Degree)
$LAT=$LAT*$deg2rad;

$ELULIMIT=80.0;        # Upper Elevation Limit of the SMA
$ELLLIMIT=20.0;        # Lower Elevation Limit of the SMA


# Observational Parameters
$inttime=30.0; # correlator integration time in seconds
$scan=20;       # How many scans

$nicname="SMA";


###########################
# Specify Calibrator Here #
###########################
# $calib="3c454.3";
# $calib="2148+069";
$calib="0423-013";
# $calib="1749+096";


#########################
# List of Quasars       #
# Need to be updated !! #
#########################
$source[0]="3c454.3";
$ra[0]=343.49061625;
$dec[0]=16.1482;

$source[1]="3c84";
$ra[1]=49.9506670833333;
$dec[1]=41.511696111;

$source[2]="0423-013";
$ra[2]=65.81583625;
$dec[2]=-1.34251805555556;

$source[3]="0521-365";
$ra[3]=80.7416025;
$dec[3]=-36.458569722;

$source[4]="0234+285";
$ra[4]=39.66220875;
$dec[4]=28.5;

$num=@source; # number of listed quasars


# Degree to Radian ...
for ($i=0;$i<$num;$i++) {
    $ra[$i] = $ra[$i]*$deg2rad ;
    $dec[$i] = $dec[$i]*$deg2rad ;
    $el[$i] = 0.0;
    $ha[$i] = 10.0;
}



#########################
# Start Observations !! #
#########################
$mypid=$$;          # check process id
$myname = ${0};
command("project -r -i $mypid -f $myname");
print "The process ID of this $myname script is $mypid\n";
checkANT();         # check antennas to be used
command("radio");   # note: command is also subroutine.
command("integrate -t $inttime");
print "----- initialization done -----\n";
print "$nicname will start baseline observations!!\n";


########################
# Start Observing Loop #
########################
$nloop=0;
while(1){
    printPID();
    $nloop=$nloop+1;  print "Loop No.= $nloop\n";
    getLST();
    calcEL();
    checkcalEL();
    pickupQS();

    if ($calskip==1) {      # If the calibrator is too high EL
                            # ..... or too close to the Sun
        for ($i=0;$i<$num_of_tar-1;$i++) {
            $Coordtar=`lookup -s $target[$i]`;
            if ($Coordtar eq '') {
                print "#######################################\n";
                print "###### WARNING WARNING WARNING ########\n";
                print "##### source $target[$i] not found. ######\n";
                print "#######################################\n";
                print "Skip $target[$i] ..... \n";
            }
            else {
                chomp($Coordtar);
                ($aztar,$eltar,$suntar)=split(' ',$Coordtar);
                print "$target[$i] at EL=$eltar (Degree) and Az=$aztar (Degree) and SunDist=$suntar (Degree)\n";
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>50.0) {
                    print "$target[$i] is available, and will be observed right now.\n";
                    command("observe -s $target[$i]");
                    command("sleep 5");
                    command("tsys");
                    command("integrate -s $scan -t $inttime -w");
                }
                else {
                    print "$target[$i] is NOT available now, skip ......\n";
                }
            }
        }
    }
    elsif ($calskip==0)  {  # If the calibrator is available ....
        command("observe -s $calib");
        command("sleep 5");
        command("tsys");
        command("integrate -s $scan -t $inttime -w");

        for ($i=0;$i<$num_of_tar-1;$i++) {
            $Coordtar=`lookup -s $target[$i]`;
            if ($Coordtar eq '') {
                print "#######################################\n";
                print "###### WARNING WARNING WARNING ########\n";
                print "##### source $target[$i] not found. ######\n";
                print "#######################################\n";
                print "Skip $target[$i] ..... \n";
            }
            else {
                chomp($Coordtar);
                ($aztar,$eltar,$suntar)=split(' ',$Coordtar);
                print "$target[$i] at EL=$eltar (Degree) and Az=$aztar (Degree) and SunDist=$suntar (Degree)\n";
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>50.0) {
                    print "$target[$i] is available, and will be observed right now.\n";
                    command("observe -s $target[$i]");
                    command("sleep 5");
                    command("tsys");
                    command("integrate -s $scan -t $inttime -w");
                }
                else {
                    print "$target[$i] is NOT available now, skip ......\n";
                }
            }
        }
    }
    else {                  # If the calibrator is too low EL, finish observations!!
        print "------- Final Calibration ------- \n";
        # Calibrator
        $sourceCoordinates=`lookup -s $calib`;
        chomp($sourceCoordinates);
        ($sourceAzDegrees,$sourceElDegrees,$sunDistance)=split(' ',$sourceCoordinates);
        if (18.0<$sourceElDegrees && $sourceElDegrees<$ELULIMIT && $sunDistance>50.0) {
            command("observe -s $calib");
            command("sleep 5");
            command("tsys");
            command("integrate -s $scan -t $inttime -w");
        }
        else {
            print "Calibrator $calib is NOT available now, skip ......\n";
        }


        print "Baseline Observation is done!! \n";
        exit(1);
    }
}



###############
# Subroutines #
###############
# --- interrupt handler for CTRL-C ---
sub finish {
   exit(1);
}

# --- print PID ---
# usage: printPID();
sub printPID {
  print "The process ID of this $myname script is $mypid\n";
}

# --- check antennas ---
# usage: checkANT();
# This subroutine checks active antennas and stores them
# as an array @sma.
sub checkANT {
  print "Checking antenna status ... \n";
  for ($i = 1; $i <=8; $i++) {
     $exist = `value -a $i -v antenna_status`;
     chomp($exist);
     if ($exist) {
        print "Found antenna ",$i," in the array!\n";
        @sma = (@sma,$i);
     }
  }
  print "Antennas @sma are going to be used.\n";
}

# --- performs a shell task with delay and printing. ---
sub command {
   my ($a);
   print "@_\n";                        # print the command
   $a=system("@_");                     # execute the command
   sleep 1;                             # sleep 1 sec
   return $a;
}

# --- get LST in hours ---
# usage: getLST();
# This subroutine updates the global variable $LST, and prints
# its value.
sub getLST {
  $LST= `value -a $sma[0] -v lst_hours`;
  chomp($LST);
  print "LST [hr]= $LST\n";
}


# --- Calculate Elevation for each quasars ---
# usage: calcEL();
sub calcEL {
    for ($i=0;$i<$num;$i++) {
        $ha[$i] = $LST*15.0*$deg2rad-$ra[$i] ;
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
sub checkcalEL {
    $calname_correct=0;
    for ($i=0;$i<$num;$i++) {
        if($source[$i] eq $calib){
            # $elcal=$el[$i];
            $calname_correct=1;
        }
    }
    if ($calname_correct==0) {
        print "Calibrator Name $calib is wrong!! \n";
        print "Maybe you used B1950 name ...\n";
        print "Use J2000 name instead of B1950 name.\n";
        print "Abort this script ....\n";
        exit(1);
    }
    $Coordcal=`lookup -s $calib`;
    if ($Coordcal eq '') {
        print "#######################################\n";
        print "###### ERROR ERROR ERROR ########\n";
        print "##### calibrator $calib not found. ######\n";
        print "#######################################\n";
        print "Abort this script ..... \n";
        exit(1);
    }
    chomp($Coordcal);
    ($azcal,$elcal,$suncal)=split(' ',$Coordcal);
    print "Calibrator $calib at EL=$elcal (Degree) and Az=$azcal (Degree) and SunDist=$suncal (Degree)\n";
    if ($elcal<$ELLLIMIT && $suncal>50.0) {
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
    elsif ($ELLLIMIT<=$elcal && $elcal<$ELULIMIT && $suncal>50.0) {
        print "Calibrator $calib is in the observable EL range.\n";
        $calskip=0;
    }
    else {
        print "Calibrator $calib is at too high EL=$elcal (deg) ... \n";
        print "Or Calibrator $calib is too close to the Sun at $suncal (deg) ... \n";
        $calskip=1;
    }
}


#
# --- Pick Up Quasars to Observe ---
sub pickupQS {

# Extract Candidate Quasars above 22 deg and below ELLIMIT in EL
# except for calibrator
    $num_of_can=0;
    for ($i=0;$i<$num;$i++) {
        if ($el[$i]>$ELLLIMIT && $el[$i]<$ELULIMIT && $source[$i] ne $calib) {   
            $candidates[$num_of_can]=$source[$i];
            $canha[$num_of_can]=$ha[$i];
            $canel[$num_of_can]=$el[$i];
            $num_of_can++;
        }
    }
    print "Number of Available Quasar except for calibrator is $num_of_can ...\n";


# Pick Up three Quasars from the Candidates
    if ($num_of_can<4) {
        $num_of_tar=$num_of_can;
        @target=@candidates;
        @tarha=@canha;
        @tarel=@canel;
    }
    else {
        $num_of_tar=3;
        $s1=int(rand($num_of_can));
        $s2=int(rand($num_of_can));
        while ($s2==$s1){$s2=int(rand($num_of_can));}
        $s3=int(rand($num_of_can));
        while ($s3==$s2 || $s3==$s1){$s3=int(rand($num_of_can));}

        $target[0]=$candidates[$s1];
        $tarha[0]=$canha[$s1];
        $tarel[0]=$canel[$s1];
        $target[1]=$candidates[$s2];
        $tarha[1]=$canha[$s2];
        $tarel[1]=$canel[$s2];
        $target[2]=$candidates[$s3];
        $tarha[2]=$canha[$s3];
        $tarel[2]=$canel[$s3];
    }
    print "Calibrator = $calib \n";
    for ($i=0;$i<$num_of_tar;$i++) {
        print "$nicname will observe $target[$i] at HA=$tarha[$i] and EL=$tarel[$i] \n";
    }
#    print "and beautiful women ..... \n";
}
