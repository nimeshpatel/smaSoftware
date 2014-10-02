#!/usr/bin/perl -w
do 'sma.pl';
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
$LAT=19.82420526388;  # Latitude of the SMA (Degree)
$LAT=$LAT*$deg2rad;

$ELULIMIT=86.47; # Upper Elevation Limit of the SMA
$ELLLIMIT=20.0;  # Lower Elevation Limit of the SMA
$SUNANGLE=35.0;  #Do not choose a primary calibrator within this Sun angle!

# Observational Parameters
$inttime=30.0; # correlator integration time in seconds
$scan=11;       # How many scans

$nicname="SMA";


###########################
# Specify Calibrator Here #
###########################
     $calib="3c454.3";
# $calib="0359+509";
#   $calib="3c279";
#  $calib="1751+096";
# $calib="1924-292";
# $calib="0423-013";
#$calib="0854+201";
#  $calib="3c84";

#########################
# List of Quasars       #
# Need to be updated !! #
#########################
$source[0]="1635+381";
$ra[0]=248.81455375;   # (Degree)
$dec[0]=38.1346;

$source[1]="3c345";
$ra[1]=250.74504125;
$dec[1]=39.8103;

$source[2]="nrao530";
$ra[2]=263.26127375;
$dec[2]=-13.0804;

$source[3]="1743-038";
$ra[3]=265.99523375;
$dec[3]=-3.83462;

$source[4]="1751+096";
$ra[4]=267.88674375;
$dec[4]=9.65020222;

# $source[5]="1849+670";
# $ra[5]=282.316967916667;
# $dec[5]=67.0949;

# $source[6]="1911-201";
# $ra[6]=287.79022;
# $dec[6]=-20.1153080555556;

$source[5]="1924-292";
$ra[5]=291.212732916667;
$dec[5]=-29.2417002777778;

# $source[8]="1925+211";
# $ra[8]=291.498355416667;
# $dec[8]=21.1073;

# $source[9]="1927+739";
# $ra[9]=291.952063333333;
# $dec[9]=73.9671;

$source[6]="2015+371";
$ra[6]=303.869722083333;
$dec[6]=37.1832;

$source[7]="2148+069";
$ra[7]=327.022744166667;
$dec[7]=6.96072;

$source[8]="bllac";
$ra[8]=330.680380416667;
$dec[8]=42.2778;

$source[9]="3c454.3";
$ra[9]=343.49061625;
$dec[9]=16.1482;

$source[10]="2258-279";
$ra[10]=344.524845;
$dec[10]=-27.9726;

$source[11]="0136+478";
$ra[11]=24.244145;
$dec[11]=47.8581;

# $source[16]="0228+673";
# $ra[16]=37.2085479166667;
# $dec[16]=67.3508413888889;

$source[12]="3c84";
$ra[12]=49.9506670833333;
$dec[12]=41.511696111;

# $source[18]="0325+469";
# $ra[18]=51.33460125;
# $dec[18]=46.9185202777778;

$source[13]="0359+509";
$ra[13]=59.8739466666667;
$dec[13]=50.9639336111111;

$source[14]="3c111";
$ra[14]=64.588833333333;
$dec[14]=38.026625;

$source[15]="0423-013";
$ra[15]=65.81583625;
$dec[15]=-1.34251805555556;

$source[16]="3c120";
$ra[16]=68.29623125;
$dec[16]=5.354338611111;

# $source[23]="0457-234";
# $ra[23]=74.2632466666;
# $dec[23]=-23.41445;

$source[17]="0522-364";
$ra[17]=80.7416025;
$dec[17]=-36.458569722;

$source[18]="0530+135";
$ra[18]=82.735069583;
$dec[18]=13.531985833;

$source[19]="0510+180";
$ra[19]=82.0;
$dec[19]=18.0;

# $source[26]="0555+398";
# $ra[26]=88.878356666;
# $dec[26]=39.8136566666;

# $source[27]="0609-157";
# $ra[27]=92.420622916;
# $dec[27]=-15.71129777;

# $source[28]="0646+448";
# $ra[28]=101.63344125;
# $dec[28]=44.85460833;

$source[20]="0721+713";
$ra[20]=110.47270166;
$dec[20]=71.34343416;

$source[21]="0730-116";
$ra[21]=112.579635;
$dec[21]=-11.6868333;

$source[22]="0739+016";
$ra[22]=114.8251408333;
$dec[22]=1.61794944444;

# $source[32]="0757+099";
# $ra[32]=119.27767875;
# $dec[32]=9.9430144444444;

# $source[33]="0836-202";
# $ra[33]=129.163396666;
# $dec[33]=-20.28319527777;

# $source[34]="3c207";
# $ra[34]=130.1982875;
# $dec[34]=13.206544444444;

$source[23]="0854+201";
$ra[23]=133.703645416;
$dec[23]=20.108511;

$source[24]="0927+390";
$ra[24]=141.7625579166;
$dec[24]=39.0391252777;

# $source[37]="1048+717";
# $ra[37]=162.115082916;
# $dec[37]=71.72664944444;

$source[25]="1058+015";
$ra[25]=164.623355;
$dec[25]=1.566339722222;

$source[26]="1159+292";
$ra[26]=179.88264125;
$dec[26]=29.245507222;

$source[27]="3c273";
$ra[27]=187.2779154166;
$dec[27]=2.05238833333333;

# $source[26]="3c274";
# $ra[26]=187.705930416667;
# $dec[26]=12.3911230555556;

$source[28]="3c279";
$ra[28]=194.046527083333;
$dec[28]=-5.7893122222;

$source[29]="1310+323";
$ra[29]=197.6194325;
$dec[29]=32.345495;

$source[30]="1337-129";
$ra[30]=204.41576125;
$dec[30]=-12.95685916666;

$source[31]="2232+117";
$ra[31]=338.15170375;
$dec[31]=11.7308066666667;

# $source[45]="1419+543";
# $ra[45]=214.944155833;
# $dec[45]=54.3874408333;

# $source[46]="1517-243";
# $ra[46]=229.42422125;
# $dec[46]=-24.372076388888;

# $source[47]="2232+117";
# $ra[47]=338.15170375;
# $dec[47]=11.7308066666667;

# $source[48]="0238+166";
# $ra[48]=39.66220875;
# $dec[48]=16.616465;

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
        for ($i=0;$i<$num_of_tar;$i++) {
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
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>$SUNANGLE) {
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

        for ($i=0;$i<$num_of_tar;$i++) {
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
                if ($ELLLIMIT<$eltar && $eltar<$ELULIMIT && $suntar>$SUNANGLE) {
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
    if ($elcal<$ELLLIMIT && $suncal>$SUNANGLE) {
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
    elsif ($ELLLIMIT<=$elcal && $elcal<$ELULIMIT && $suncal>$SUNANGLE) {
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
