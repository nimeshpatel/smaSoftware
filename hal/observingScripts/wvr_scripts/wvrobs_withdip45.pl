#!/usr/bin/perl -w
#
# Experiment Code: wvrobs
# Experiment Title: Water Vapour Radiometer Tests
# PI: Ross Williamson
# Contact Person: Ross, Mike or Alison 
# Office phone:   
# Home   phone: (Ross) +44 1223 574984   
# Array config: extended   
####################  PRIMING  ####################  
#
#  NOTE -- there are some extra commands!!
#
###################################################
#
# restartCorrelator -d -s8
#
# observe -s 3c273
#
# dopplerTrack -r 230.538 -u -s12 
#
# tune -a 4,5 -c "combiner -r wvr -m"
#
# setFeedOffset -f 230
#
# az -a 4,5 -d 10
#
# newFile
#
###################   POINTING  ###################  
#
# Pointing: None requested
# Syntax Example: point -i 90 -r 3 -L -l -t -Q
#
############### SOURCE, CALIBRATOR and LIMITS ############## 
#
$inttime=3; 
$cal0="3c279"; 
$ncal0=720; 
$cal1="3c273"; 
$ncal1=720; 
$cal2="2232+117"; 
$ncal2=720; 
$cal3="1921-293"; 
$ncal3=720; 
$cal4="3c454.3"; 
$ncal4=720;
$cal5="3c111";
$ncal5=720; 
$MINEL_GAIN = 35; 
$MAXEL_GAIN = 76; 
$MINEL_CHECK= 33; 
$LST_start=0; 
$LST_end=24; 
######################################################### 

do 'sma.pl';
checkANT();
command("radio");
command("integrate -t $inttime");
$myPID=$$;
command("project -r -p 'Ross Williamson' -d 'wvrobs' -i $myPID");
print "----- initialization done, starting script -----\n";
if(!$restart){
}

print "\n";
print "#####################################################################\n";
print "# corrPlotter scan mode does not update, use MIR mode (phs vs time) #\n";
print "#                 all scans are being recorded                      #\n";
print "#####################################################################\n";
print "\n";

     command("integrate -n -p");

$limit = 30;
@elevations = (85, 80, 70, 60, 50, 40, 30, 20, 30, 40, 50, 60, 70, 80, 85);

for($i=0; $i<2; $i++) {
        foreach $el (@elevations) {
                command("az -d 10");
                command("el -d $el");
                command("integrate -s 8 -w"); 
#               sleep 20;
        }
}


print "\n";
print "###########################\n";
print "# Initial Elevation Check #\n";
print "###########################\n";
print "\n";


if(&Check_Gain_El){
     print "None of the calibrators are up.  Exiting..\n";
     command("standby");
     exit;
}
     print "---- At least one calibrator is available. ----\n";
#print "\n";
#print "###############################################\n";
#print "# Main Loop -- corrPlotter MIR mode should be updating #\n";
#print "###############################################\n";
#print "\n";
# Here is what is happening in the following loop 
# For the target source: 
#    --Check the elevation 
#        1. if below MINEL then exit the script 
#        2. if higher than MAXEL then 
#           a. if transcal is defined then loop on 
#              transcal until the calibrator becomes available
#           b. if transcal is not defined, then skip to the next calibrator. 
#        3. if not 1 or 2 then observe the source 
# 
# For the calibrator: 
#    --Check the elevation 
#        1. if below MINEL then check all of the other calibrators 
#           a. if a different calibrator is available, then observe that. 
#           b. if not exit the script.
#        2. if higher than MAXEL then check all of the other calibrators 
#           a. if a different calibrator is available, then observe that. 
#           b. if not and if transcal is defined, then loop on
#              transcal until the calibrator becomes available
#           c. if transcal is not defined, we are out of sources to go to.  Exit the script.
#        3. if not 1 or 2 then observe the calibrator. 
# 
while(LST(1) > $LST_start or LST() < $LST_end){
     printPID();
     $nloop=$nloop+1;
     print "Loop No.= $nloop\n";

     print "\n";
     print "---- Observing Gain Calibrator (Loop = $nloop) ----\n";
     print "\n";
     LST(); $targel=checkEl($cal0);
     if($targel < $MINEL_GAIN){
           if(!&Source_Too_High_or_Low){
                print "None of the calibrators are above $MINEL_GAIN degrees\n";
                command("standby");
                exit;
           }
     }elsif($targel > $MAXEL_GAIN){
           if(!&Source_Too_High_or_Low){
                print "None of the calibrators are below $MAXEL_GAIN degrees\n";
                if($transcal){
                     print "---- Going to transcal ----\n";
                     print "---- Entering a loop until calibrator becomes available ----\n";
                     while(checkEl($cal0) > $MAXEL_GAIN){
                          LST(); &Observe_Transcal; 
                     }
                     command("observe -s $cal0");
                     command("tsys");
                     command("integrate -s $ncal0 -w");
                }else{
                     print "---- No source to go to.  Exiting.. ----\n";
                     command("standby");
                     exit;
                }
           }
     }elsif($nloop % 1 == 0){
           $current_source = $cal0;
           command("observe -s $cal0");
#           command("integrate -n -i");
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal0 -w");
#           command("integrate -n -i");           
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal0 -w");

     }

#     print "---- Check the elevations of the gain calibrators ----\n";
     if(&Check_Gain_El){
           print "---- None of the gain calibrators are above $MINEL_CHECK.  Finishing up... ----\n";
           last;
     }
     print "---- At least one calibrator is above $MINEL_CHECK degrees ----\n";

     if($nloop >= 1){
#          print "---- Maximum number of loops achieved.  Exiting.. ----\n";
print "----- This is the end of the script.  -----\n";
          exit;
     }

}

#print "\n";
#print "##########################\n";
#print "# Final Gain Calibration #\n";
#print "##########################\n";
#print "\n";
#
#     LST(); $targel=checkEl($cal0);
#     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
#    $current_source = $cal0;
#     command("observe -s $cal0");
#     command("tsys");
#     command("integrate -s $ncal0 -w");
#}
#LST(); $targel=checkEl($cal1);
#if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
#    $current_source = $cal1;
#     command("observe -s $cal1");
#     command("tsys");
#     command("integrate -s $ncal1 -w");
#}
#LST(); $targel=checkEl($cal2);
#if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
#    $current_source = $cal2;
#     command("observe -s $cal2");
#     command("tsys");
#     command("integrate -s $ncal2 -w");
#}
#LST(); $targel=checkEl($cal3);
#if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
#    $current_source = $cal3;
#     command("observe -s $cal3");
#     command("tsys");
#     command("integrate -s $ncal3 -w");
#}
#LST(); $targel=checkEl($cal4);
#if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
#    $current_source = $cal4;
#     command("observe -s $cal4");
#     command("tsys");
#     command("integrate -s $ncal4 -w");
#}
#LST(); $targel=checkEl($cal5);
#if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
#    $current_source = $cal5;
#     command("observe -s $cal5");
#     command("tsys");
#     command("integrate -s $ncal5 -w");
#}
#print "\n";
#print "#################################\n";
#print "# Flux and Bandpass Calibration #\n";
#print "#################################\n";
#print "\n";
print "----- Congratulations!  This is the end of the script.  -----\n";

###############
# Subroutines #
###############

sub Source_Too_High_or_Low
{
     print "---- Source elevation limit. ----\n";
     print "---- Searching for an available calibrator.. ----\n";
    LST(); $targel=checkEl($cal0);
     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
         $current_source = $cal0;
          command("observe -s $cal0");
          command("tsys");
          command("integrate -s $ncal0 -w");
          return 1;
     }
    LST(); $targel=checkEl($cal1);
     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
         $current_source = $cal1;
          command("observe -s $cal1");
#           command("integrate -n -i");
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal1 -w");
#           command("integrate -n -i");           
           command("tsys");
#           command("integrate -i");
          command("integrate -s $ncal1 -w");
          return 1;
     }
    LST(); $targel=checkEl($cal2);
     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
         $current_source = $cal2;
          command("observe -s $cal2");
#           command("integrate -n -i");
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal2 -w");
#           command("integrate -n -i");           
           command("tsys");
#           command("integrate -i");
          command("integrate -s $ncal2 -w");
          return 1;
     }
    LST(); $targel=checkEl($cal3);
     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
         $current_source = $cal3;
          command("observe -s $cal3");
#           command("integrate -n -i");
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal3 -w");
#           command("integrate -n -i");           
           command("tsys");
#           command("integrate -i");
          command("integrate -s $ncal3 -w");
          return 1;
     }
    LST(); $targel=checkEl($cal4);
     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
         $current_source = $cal4;
          command("observe -s $cal4");
#           command("integrate -n -i");
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal4 -w");
#           command("integrate -n -i");           
           command("tsys");
#           command("integrate -i");
          command("integrate -s $ncal4 -w");
          return 1;
     }
    LST(); $targel=checkEl($cal5);
     if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
         $current_source = $cal5;
          command("observe -s $cal5");
#           command("integrate -n -i");
           command("tsys");
#           command("integrate -i");
           command("integrate -s $ncal0 -w");
#           command("integrate -n -i");           
           command("tsys");
#           command("integrate -i");
          command("integrate -s $ncal5 -w");
          return 1;
     }
}

sub Check_Gain_El
{
     $GAIN_TOO_LOW=1;
     LST(); $targel=checkEl($cal0);
     if($targel > $MINEL_CHECK){
           $GAIN_TOO_LOW = 0;
     }

     LST(); $targel=checkEl($cal1);
     if($targel > $MINEL_CHECK){
           $GAIN_TOO_LOW = 0;
     }

     LST(); $targel=checkEl($cal2);
     if($targel > $MINEL_CHECK){
           $GAIN_TOO_LOW = 0;
     }

     LST(); $targel=checkEl($cal3);
     if($targel > $MINEL_CHECK){
           $GAIN_TOO_LOW = 0;
     }

     LST(); $targel=checkEl($cal4);
     if($targel > $MINEL_CHECK){
           $GAIN_TOO_LOW = 0;
     }

     LST(); $targel=checkEl($cal5);
     if($targel > $MINEL_CHECK){
           $GAIN_TOO_LOW = 0;
     }

     return $GAIN_TOO_LOW;
}

sub Observe_Transcal
{
     LST(); $transel=checkEl($transcal);
     if($transel > $MINEL_GAIN and $transel < $MAXEL_GAIN){
         $current_source = $transcal;
          command("observe -s $transcal");
          command("tsys");
          command("integrate -s 10 -w");
     }else{
         print "---- Transcal not available.  Exiting.. ----\n";
         command("standby");
         exit;
     }
}
 
