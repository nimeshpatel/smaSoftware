#!/usr/bin/perl -w
#
# Experiment Code: A014
# Experiment Title: Searching for very dense gas towards the center of Cen A
# PI: Sebastien Muller
# Contact Person: S. Muller  
# Email  : muller@asiaa.sinica.edu.tw  
# Office :  886-2-33652200 xxx 830  
# Home   : -   
# Array  : compact   
####################  PRIMING  ####################  
#
# restartCorrelator -s128 -s3:512 -s23:512
#
# observe -s CENA -r 13:25:27.60 -d -43:01:09.00 -e 2000 -v 550
#
# dopplerTrack -r 267.558 -l -s3 
#
# setFeedOffset -f 345
#
###################   POINTING  ###################  
#
# Pointing: None requested
# Syntax Example: point -i 90 -r 3 -L -l -t -Q
#
############### SOURCE, CALIBRATOR and LIMITS ############## 
#
$inttime=30; 
$targ0="CENA -r 13:25:27.60 -d -43:01:09.00 -e 2000 -v 550";
$ntarg0=40; 
$cal0="1424-418"; 
$ncal0=5; 
$flux0="titan"; 
$nflux0=90; 
$bpass0="3c273"; 
$nbpass0=120; 
$MINEL_TARG = 17; 
$MINEL_GAIN = 17; 
$MINEL_FLUX = 17; 
$MINEL_BPASS= 17; 
$MAXEL_TARG = 81; 
$MAXEL_GAIN = 81; 
$MAXEL_FLUX = 85; 
$MAXEL_BPASS= 81; 
$MINEL_CHECK= 23; 
$LST_start=0; 
$LST_end=24; 
######################################################### 

do 'sma.pl';
checkANT();
command("radio");
command("integrate -t $inttime");
$myPID=$$;
command("project -r -p 'Sebastien Muller' -d 'A014' -i $myPID");
print "----- initialization done, starting script -----\n";
print "\n";
print "#################################\n";
print "# FLUX and BANDPASS Calibration #\n";
print "#################################\n";
print "\n";

if(!$restart){
     $total_scans = 0;
     LST(); $fluxel=checkEl($flux0);
     if($fluxel > $MINEL_FLUX and $fluxel < $MAXEL_FLUX){
          print "\n";
          print "---- Observing $flux0. ----\n"; 
          print "\n";

          $current_source = $flux0;
          until($nflux0 <= $total_scans){
               LST(); $fluxel=checkEl($flux0);
               if($fluxel < $MINEL_FLUX or $fluxel > $MAXEL_FLUX){
                    print "Source elevation limit reached.\n";
                    print "Exiting ...\n";
                    command("standby");
                    exit;
               }else{
                    command("observe -s $flux0 -t flux");
                    if($total_scans == 0){
                         command("tsys");
                    }
                    command("integrate -s 10 -w");
                    $total_scans = $total_scans+10;
                    print "Finished $total_scans/$nflux0 on $flux0.\n";
               }
          }
      }
     $total_scans = 0;
     LST(); $bpel=checkEl($bpass0);
     if($bpel > $MINEL_BPASS and $bpel < $MAXEL_BPASS){
          print "\n";
          print "----  Observing $bpass0. ----\n"; 
          print "\n";

          $current_source = $bpass0;
          until($nbpass0 <= $total_scans){
               LST(); $bpel=checkEl($bpass0);
               if($bpel < $MINEL_BPASS or $bpel > $MAXEL_BPASS){
                    print "Source elevation limit reached.\n";
                    print "Exiting ...\n";
                    command("standby");
                    exit;
               }else{
                    command("observe -s $bpass0 -t bandpass");
                    if($total_scans == 0){
                         command("tsys");
                    }
                    command("integrate -s 10 -w");
                    $total_scans = $total_scans+10;
                    print "Finished $total_scans/$nbpass0 on $bpass0.\n";
               }
          }
      }
}
print "\n";
print "###########################\n";
print "# Initial Elevation Check #\n";
print "###########################\n";
print "\n";

LST(); $targel=checkEl($targ0);
if($targel < $MINEL_TARG){
     print "Target source too low.  Exiting..\n";
     command("standby");
     exit;
}
     print "---- Target source is available. ----\n";
if(&Check_Gain_El){
     print "None of the calibrators are up.  Exiting..\n";
     command("standby");
     exit;
}
     print "---- At least one calibrator is available. ----\n";
print "\n";
print "#############\n";
print "# Main Loop #\n";
print "#############\n";
print "\n";
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
     print "---- Observing Target Source (Loop = $nloop) ----\n";
     print "\n";
     LST(); $targel=checkEl($targ0);
     if($targel < $MINEL_TARG){
           last;
     }elsif($targel > $MAXEL_TARG){
          if($transcal){
               print "---- Target Source too high. Going to Transcal ----\n";
               print "---- Entering a loop until source becomes available ----\n";
               while(checkEl($targ0) > $MAXEL_TARG){
                    LST(); &Observe_Transcal; 
               }
               next;
          }else{
              $current_source = $targ0;
               print "---- Transcal not specified. Continuing script until source becomes available. ----\n";
          }
     }elsif($nloop % 1 == 0){
           $current_source = $targ0;
           command("observe -s $targ0");
           command("tsys");
           command("integrate -s $ntarg0 -w");
     }

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
           command("tsys");
           command("integrate -s $ncal0 -w");
     }

     print "---- Check the elevations of the gain calibrators ----\n";
     if(&Check_Gain_El){
           print "---- None of the gain calibrators are above $MINEL_CHECK.  Finishing up... ----\n";
           last;
     }
     print "---- At least one calibrator is above $MINEL_CHECK degrees ----\n";

     if($nloop >= 64){
          print "---- Maximum number of loops achieved.  Exiting.. ----\n";
          exit;
     }

}

print "\n";
print "##########################\n";
print "# Final Gain Calibration #\n";
print "##########################\n";
print "\n";

LST(); $targel=checkEl($cal0);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal0;
     command("observe -s $cal0");
     command("tsys");
     command("integrate -s $ncal0 -w");
}
print "\n";
print "#################################\n";
print "# Flux and Bandpass Calibration #\n";
print "#################################\n";
print "\n";
$total_scans = 0;
LST(); $fluxel=checkEl($flux0);
if($fluxel > $MINEL_FLUX and $fluxel < $MAXEL_FLUX){
     print "\n";

     print "---- Observing $flux0. ----\n"; 
     print "\n";

     $current_source = $flux0;
     until($nflux0 <= $total_scans){
          LST(); $fluxel=checkEl($flux0);
          if($fluxel < $MINEL_FLUX or $fluxel > $MAXEL_FLUX){
               print "Source elevation limit reached.\n";
               print "Exiting ...\n";
               command("standby");
               exit;
          }else{
               command("observe -s $flux0");
                    if($total_scans == 0){
                         command("tsys");
                    }
               command("integrate -s 10 -w");
               $total_scans = $total_scans+10;
               print "Finished $total_scans/$nflux0 on $flux0.\n";
          }
     }
}
$total_scans = 0;
LST(); $bpel=checkEl($bpass0);
if($bpel > $MINEL_BPASS and $bpel < $MAXEL_BPASS){
     print "\n";
     print "----  Observing $bpass0. ----\n"; 
     print "\n";

     $current_source = $bpass0;
     until($nbpass0 <= $total_scans){
          LST(); $bpel=checkEl($bpass0);
          if($bpel < $MINEL_BPASS or $bpel > $MAXEL_BPASS){
               print "Source elevation limit reached.\n";
               print "Exiting ...\n";
               command("standby");
               exit;
          }else{
               command("observe -s $bpass0");
                    if($total_scans == 0){
                         command("tsys");
                    }
               command("integrate -s 10 -w");
               $total_scans = $total_scans+10;
               print "Finished $total_scans/$nbpass0 on $bpass0.\n";
          }
     }
}
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
}

sub Check_Gain_El
{
     $GAIN_TOO_LOW=1;
     LST(); $targel=checkEl($cal0);
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
 
