#!/usr/bin/perl -w
#
# Experiment Code: Mosaic Test
# Experiment Title: Software test 
# PI: Nimesh Patel
# Contact Person: Nimesh Patel  
# Email  : npatel@cfa.harvard.edu  
# Office : 617-496-7649  
# Home   : 617-577-1597   
# Array  : compact   
####################  PRIMING  ####################  
#
# restartCorrelator -s 128
#
# observe -s irc_center -r 09:47:57.38 -d +13:16:43.70 -e 2000 -v -26.2
#
# dopplerTrack -r 349.0 -l -s12 
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
$inttime=15; 
$targ0="cwleo -r 09:47:57.38 -d +13:16:43.70 -e 2000 -v -26.2";
$ntarg0=32; 
$cal0="Titan"; 
$ncal0=16; 
$cal1="0851+202"; 
$ncal1=16; 
$flux0="callisto"; 
$nflux0=120; 
$bpass0="jupiter"; 
$nbpass0=120; 
$MINEL_TARG = 17; 
$MINEL_GAIN = 17; 
$MINEL_FLUX = 17; 
$MINEL_BPASS= 17; 
$MAXEL_TARG = 81; 
$MAXEL_GAIN = 81; 
$MAXEL_FLUX = 81; 
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
command("project -r -p 'Nimesh Patel' -d 'mosaicTest' -i $myPID");
print "----- initialization done, starting script -----\n";

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

$nloop=0;
while(LST(1) > $LST_start or LST() < $LST_end){
     printPID();
     $nloop=$nloop+1;
     print "Loop No.= $nloop\n";

     print "\n";
     print "---- Observing Gain Calibrator (Loop = $nloop) ----\n";
     print "\n";
     LST(); $targel=checkEl($cal1);
     if($targel < $MINEL_GAIN){
           if(!&Source_Too_High_or_Low){
                print "None of the calibrators are above $MINEL_GAIN degrees\n";
		last;
           }
     }elsif($targel > $MAXEL_GAIN){
           if(!&Source_Too_High_or_Low){
                print "None of the calibrators are below $MAXEL_GAIN degrees\n";
                if($transcal){
                     print "---- Going to transcal ----\n";
                     print "---- Entering a loop until calibrator becomes available ----\n";
                     while(checkEl($cal1) > $MAXEL_GAIN){
                          LST(); &Observe_Transcal; 
                     }
                     command("observe -s $cal1");
                     command("tsys");
                     command("integrate -s $ncal1 -w");
                }else{
                     print "---- No source to go to.  Exiting the main loop.. ----\n";
			last; 
                }
           }
     }elsif($nloop % 1 == 0){
           $current_source = $cal1;
           command("observe -s $cal1");
           command("tsys");
           command("integrate -s $ncal1 -w");
     }

     print "---- Check the elevations of the gain calibrators ----\n";
     if(&Check_Gain_El){
           print "---- None of the gain calibrators are above $MINEL_CHECK.  Finishing up... ----\n";
           last;
     }
     print "---- At least one calibrator is above $MINEL_CHECK degrees ----\n";

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

     LST(); $targel=checkEl($cal0);
           $current_source = $cal0;
           command("observe -s $cal0");
           command("tsys");
           command("integrate -s $ncal0 -w");

           $current_source = $targ0;
     LST(); $targel=checkEl($targ0);
           command("observe -s $targ0");
           command("tsys");
           command("integrate -s $ntarg0 -w");

           $current_source = $targ0;
     LST(); $targel=checkEl($targ0);
           command("observe -s $targ0");
           command("tsys");
	   command("radecoff -r 24 -d 24");
           command("integrate -s $ntarg0 -w");
	   command("radecoff -r 0 -d 0");

           $current_source = $cal0;
     LST(); $targel=checkEl($cal0);
           command("observe -s $cal0");
           command("tsys");
           command("integrate -s $ncal0 -w");

           $current_source = $targ0;
     LST(); $targel=checkEl($targ0);
           command("observe -s $targ0");
           command("tsys");
	   command("radecoff -r -24 -d 24");
           command("integrate -s $ntarg0 -w");
	   command("radecoff -r 0 -d 0");

           $current_source = $targ0;
     LST(); $targel=checkEl($targ0);
           command("observe -s $targ0");
           command("tsys");
	   command("radecoff -r 24 -d -24");
           command("integrate -s $ntarg0 -w");
	   command("radecoff -r 0 -d 0");

           $current_source = $cal0;
     LST(); $targel=checkEl($cal0);
           command("observe -s $cal0");
           command("tsys");
           command("integrate -s $ncal0 -w");

           $current_source = $targ0;
     LST(); $targel=checkEl($targ0);
           command("observe -s $targ0");
           command("tsys");
	   command("radecoff -r -24 -d -24");
           command("integrate -s $ntarg0 -w");
	   command("radecoff -r 0 -d 0");
     }

     if($nloop >= 15){
          print "---- Maximum number of loops achieved.  Exiting.. ----\n";
	  last;
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
LST(); $targel=checkEl($cal1);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal1;
     command("observe -s $cal1");
     command("tsys");
     command("integrate -s $ncal1 -w");
}
print "----- Congratulations!  This is the end of the script.  -----\n";
if($plotFlag==1) {print "plotting requested...\n";plotTracks();}

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
          command("tsys");
          command("integrate -s $ncal1 -w");
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
