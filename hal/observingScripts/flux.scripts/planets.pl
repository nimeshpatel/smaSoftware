#!/usr/bin/perl -w
{ BEGIN {$^W =0}
#
################## Script Header Info #####################
#
# Experiment Code: Flux Cal
# Contact Person: Mark Gurwell  
# Email  : mgurwell@cfa.harvard.edu  
# Office : 617-495-7292 (Mark)
# Home   : 617-875-4631 (Mark)
# Array  : all   
#
####################################################
#
# NOW DOES SOLAR AVOIDANCE CHECKING!!!
#
############## SPECIAL INSTRUCTIONS ################
#
#
################## Priming ################################
#
# as necessary
#
################## Pointing ###############################
#
# Pointing: At start of track
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
################## Source, Calibrator and Limits ##########
#
  $inttime="15"; 

  $nfscn="8";

  $nflux="7";
  $flux[0]="titan";
  $flux[1]="mars";
  $flux[2]="ceres";
  $flux[3]="neptune";
  $flux[4]="callisto";
  $flux[5]="ganymede";
  $flux[6]="uranus";


  $MINEL = 33;
  $MAXEL = 78;
  $SUNLIM= 28;
#
  $total_loops=2;
#
################## Script Initialization ##################
#
  do 'sma.pl';
  do 'sma_add.pl';
  checkANT();
  command("radio");
  command("integrate -t $inttime");
  $myPID=$$;
  command("project -r -p 'PlanetBoy' -d 'Flux Cal'");
  print "----- initialization done, starting script -----\n";
#
################## Science Script #########################
#
  print "----- main science target observe loop -----\n";

  $loop = 0;
  
  while ($loop < $total_loops ) {
      
      $loop = $loop + 1;

# flux cals, looping through the available sources
      $indx = 0;
      while ($indx < $nflux ) {
	  LST(); $elev = checkEl($flux[$indx]); $sund = &checkSun($flux[$indx]);
	  if($elev >= $MINEL and $elev <= $MAXEL and $sund >= $SUNLIM) {
	      command("observe -s $flux[$indx]");
	      command("tsys");
	      print "Loop $loop.  Target is $flux[$indx].\n";
	      command("integrate -s $nfscn -w");
	  }
	  $indx = $indx + 1;
      }

  }

}

#Subroutine to check angular distance to Sun (new!)


sub checkSun {
        my ($sourcename)=$_[0];
#   my ($silent)    =(defined($_[1]) and $_[1]);
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
        $lookupTime="$mn $d $year $hour:$min";
        # check sourcename for ra dec input - if so, parse it
        if($sourcename =~ /-r/) {
#       print "got ra/dec arguments...parsing them...\n";
          if(!($sourcename =~ /-d/)) { die "both ra and dec are required\n";}
        @sourcenameArgs=split(' ',$sourcename);
           $iarg=0;
           foreach $arguments (@sourcenameArgs) {
           if($arguments=~/s/) {$sourceNameArgindex=$iarg;}
           if($arguments=~/r/) {$raArgindex=$iarg+1;}
           if($arguments=~/d/) {$decArgindex=$iarg+1;}
           if($arguments=~/e/) {$epochArgindex=$iarg+1;}
           $iarg++;
           }
           $rastring=$sourcenameArgs[$raArgindex];
           $sourceNameString=$sourcenameArgs[$sourceNameArgindex];
           $decstring=$sourcenameArgs[$decArgindex];
           $givenEpoch=$sourcenameArgs[$epochArgindex];
           ($rah,$ram,$ras)=split('\:',$rastring);
           ($decd,$decm,$decs)=split('\:',$decstring);
           $givenRA=$rah+$ram/60.+$ras/3600.;
           if($decd<0.) {$decsign=-1;$decd=-$decd} else {$decsign=1;}
           $givenDEC=$decd+$decm/60.+$decs/3600.;
           if($decsign==-1){$givenDEC=-$givenDEC;}
          $parsedSourcename="$sourceNameString -r $givenRA -d $givenDEC -e $givenEpoch ";
        $newSourceFlag=1;
        } else {
        $newSourceFlag=0;
        }

        $sourceCoordinates=`./lookup -s sun -t "$lookupTime"`;
        chomp($sourceCoordinates);
        ($sunAz,$sunEl,$sunDistance)=split(' ',$sourceCoordinates);

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
         printf("new source %s is at %4.1f degrees from Sun\n",$sourceNameString,$sunDistance);
         } else {
         printf("%s is at %4.1f degrees from Sun\n",$sourcename,$sunDistance);
         }
        }
   }   
   return $sunDistance;
}

#
################## File End ###############################
