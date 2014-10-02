#!/usr/bin/perl -w

#initial variable declarations and commands
$LST_start=0; $LST_end=24;
command("radecoff -r 0 -d 0");

################################
# Observing Script Subroutines #
################################

sub ObsLoop
{
    @obs_loop = @_;
    $nloop = 0;
    $init = 0;
    $obs_targ = 0;
    # ---------- Start of Observing loop -----------
    
    print "\n";
    print "#############\n";
    print "# Main Loop #\n";
    print "#############\n";
#
#  --follows given loop.
#
    $EXIT = 0;
    while($EXIT==0)
    {
        printPID();
        $nloop=$nloop+1;
        print "Loop No.= $nloop\n";
        #print "\n";
	@obs_loop_el=();
	$obs_loop_length=@obs_loop;
	
#      	print "---- Script will attempt to follow this requested order: ----\n";
#	for($i=0; $i<$obs_loop_length; $i++){ 
#	    $obs_loop_el[$i]=checkEl(${$obs_loop[$i]}, 1);
#            $name = "n$obs_loop[$i]";
#            print "${$name} scans on ${$obs_loop[$i]} (el: $obs_loop_el[$i])\n";
#        }
		
#Begin executing target loop:
        for($i=0;$i<$obs_loop_length;$i++)
        {
	    @array = split(//, $obs_loop[$i]);
	    	    
	    if(&checkLST==1){
		$EXIT=-1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    if($nloop >= 40){
		$EXIT=1;
		print "---- Maximum number of loops achieved.  Exiting.. ----\n";
		last;
	    }
	    if($init==0){
		print "\n";
		print "###########################\n";
		print "# Initial Elevation Check #\n";
		print "###########################\n";
		print "\n";
		
		LST(); 
		if(&Check_Target_El){
		    print "All target sources too low.  Exiting..\n";
		    command("standby");
		    exit;	
		}
		print "---- At least one target is available. ----\n";
		if(&Check_Gain_El){
		    print "None of the calibrators are up.  Exiting..\n";
		    command("standby");
		    exit;
		}
		print "---- At least one calibrator is available. ----\n";
		
		$init++;
	    }	    
            #TARGET:
	    if($array[0] eq "t")
	    {		
		$int_time = "n$obs_loop[$i]";
		print "\n";
		print "---- Observing Target Source (Loop = $nloop) ----\n";
		print "\n";
		LST(); $targel=checkEl(${$obs_loop[$i]});
	        if($targel < $MINEL_TARG){
		    if(&Check_Target_El){
			print "All target sources too low.\n";
		        $EXIT = 1;
			last;
		    }
		}elsif($targel > $MAXEL_TARG){
		    if(!&Source_Too_High_or_Low){
			while(!&Source_Too_High_or_Low){
			    print "---- None of the Targets are available.  Trying transcal. ----\n";
			    if(LST(1) < $LST_start or  LST() > $LST_end){
				last;
			    }
			    &Observe_Transcal; 
			}
		    }
		}elsif($nloop % 1 == 0){
		    $obs_targ = 1;
		    $current_source = ${$obs_loop[$i]};
		    command("observe -s ${$obs_loop[$i]}");
		    #command("tsys");
		    command("integrate -s ${$int_time} -w");
		}
	    }elsif($array[0] eq "c")
            {
		print "\n";
		print "---- Observing Gain Calibrator (Loop = $nloop) ----\n";
		#print "\n";
		
		@temp_array = split('\|', $obs_loop[$i]);
		$length = @temp_array;
		
		if($length <= 1){ $current_source = $obs_loop[$i];}
		if($length > 1){
		    print "---- Multiple calibrators possible ----\n";
		    print "---- Selecting between ${$temp_array[0]} "; 
		    for($j=1;$j<$length;$j++) { print "| ${$temp_array[$j]} ";}
		    print "\n";
		    $current_source = selectSource($obs_loop[$i]);
		}
		
		$int_time =  "n$current_source";
	        LST(); $targel=checkEl(${$current_source});
                if($targel < $MINEL_GAIN){
		    if(!&Cal_Too_High_or_Low){
			print "None of the calibrators are above $MINEL_GAIN degrees\n";
			$EXIT = 1;
		    }
		}elsif($targel > $MAXEL_GAIN){
		    if(!&Cal_Too_High_or_Low){
			print "None of the calibrators are available.  Trying transcal.\n";
			while(checkEl($cal0) > $MAXEL_GAIN){
			    LST(); &Observe_Transcal; 
			}
		    }
		}elsif($nloop % 1 == 0){
                    command("observe -s ${$current_source}");
                    command("tsys");
                    command("integrate -s ${$int_time} -w");
		}
            }elsif($array[0] eq "m")
	    {		
		
		print "\n";
		print "---- Target Source: Mosaic Mode (Loop = $nloop) ----\n";
		print "\n";
		LST();
		($T_el,$T_name,$T_roff,$T_doff,$T_access) = lookupMosaicEl($obs_loop[$i]);
		$targel=checkEl(${$T_access},1);
		$targel=int(100*$targel)/100;
		#print "(RA offset, DEC offset) [arcseconds]\n";
		print "$T_name (0\", 0\") elevation = $targel\n";
		print "$T_name ($T_roff\", $T_doff\") el = $T_el\n";
		if($T_el < $MINEL_TARG){
		    print "Target too low, checking other targets...\n";
		    if(&Check_Target_El){
			print "All target sources too low.\n";
		        $EXIT = 1;
			last;
		    }
		    print "At least one target is still above El limit.\n";
		}elsif($T_el > $MAXEL_TARG){
		    if(!&Source_Too_High_or_Low){
			print "---- None of the Targets are available.  Trying transcal. ----\n";
			while(!&Source_Too_High_or_Low){
			    print "---- None of the Targets are available.  Trying transcal. ----\n";  
			    if(LST(1) < $LST_start or  LST() > $LST_end){
				last;
			    }
			    &Observe_Transcal; 
			}
		    }
		}elsif($nloop % 1 == 0){
		    $obs_targ = 1;
		    $name= "n" . $T_access;
		    #$current_source = ${$obs_loop[$i]};
		    $T_roff = int(100000*$T_roff)/100000;
		    $T_doff = int(100000*$T_doff)/100000;
		    command("observe -s ${$T_access}");
		    command("radecoff -r $T_roff -d $T_doff");
		    #command("tsys");
		    command("integrate -s ${$name} -w");
		    command("radecoff -r 0 -d 0");
		}
	    }	
	}
    }
    if($EXIT!= -1 || $obs_targ >= 1){
	print "\n";
	print "##########################\n";
	print "# Final Gain Calibration #\n";
	print "##########################\n";
	print "\n";
	
	LST(); 
	$count= @obs_loop;
        for($j=0;$j<$count;$j++){
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "c"){
		$targel=checkEl(${$obs_loop[$j]});
	        if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
		    $current_source = ${$obs_loop[$j]};
	            $name = "n$obs_loop[$j]";
	            command("observe -s ${$obs_loop[$j]}");
	            command("tsys");
	            command("integrate -s ${$name} -w");
                    last;
		}
	    }
	}
    }
    print "##########################\n";
    print "# End of Observing Loop  #\n";
    print "##########################\n";
    #---------- End of Observing loop -----------
}

sub selectSource
{
    @temp_array = split('\|', $_[0]);
    $length = @temp_array;
    
    for($j=0; $j<$length; $j++){
	$targel=checkEl(${$temp_array[$j]});
	if(($targel > $MINEL_GAIN) and ($targel < $MAXEL_GAIN)) { return $temp_array[$j]; }
    }
    
    # if we get here, no sources are up: send 1st 
    # source and let ObsLoop decide what to do with it.
    return $temp_array[0];
}

sub PolLoop
{
    @pol_loop = @_;
    $EXIT = 0;
    $nloop=0;
    $pol_loop_length = @pol_loop;
    #print "length = $pol_loop_length\n";
    #print "array = @pol_loop\n";
    
    for($i=0; $i<$pol_loop_length; $i++){
	#rint "$i: ";
	#rint "$pol_loop[$i]  ";
	@temp = split('\[',$pol_loop[$i]);
	$obs_loop[$i]=$temp[0];
	$current_targ=${$obs_loop[$i]};
	#rint "$obs_loop[$i] $current_targ\n";
    }
    
    $prev_targ = "";
    $current_targ = "";
    
    #rint "pol_loop:  @pol_loop\nobs_loop:  @obs_loop\n";
    
    print "\n";
    print "#####################\n";
    print "# Polarization Loop #\n";
    print "#####################\n";
    #print "\n";
    $EXIT = 0;
    $initial = 0;
    
    print "# Requested loop: ";
    print "from LST $LST_start to $LST_end\n";
 
    for($i=0;$i<@pol_loop;$i++){
	@temp = split('\[',$pol_loop[$i]);
	$current_targ=${$temp[$0]};
	@pol_commands=split(',',substr($temp[1], 0, -1));
	print "#   Source: $current_targ\n";
	for($j=0;$j<@pol_commands;$j++){
	    print "#     do: ${$pol_commands[$j]}\n";
	}
    }
    print "# Repeat over LST range and target availability.\n";
    
    while($EXIT==0)
    {
        $nloop=$nloop+1;
        print "\nLoop No.= $nloop\n";
        print "\n";
	for($i=0;$i<@pol_loop;$i++){
	    #TARGET:
	    if(&checkLST==1){
		$EXIT=1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    if($nloop > 40){
		$EXIT=1;
		print "---- Maximum number of loops achieved.  Exiting.. ----\n";
		last;
	    }
	    
	    if($initial==0){
		print "\n";
		print "###################\n";
		print "# Elevation Check #\n";
	        print "###################\n";
		print "\n";
		LST();
		if(&Check_Target_El && &Check_Gain_El){
		    print "All target sources too low.  Exiting..\n";
		    command("standby");
		    exit;	
		}
		print "---- At least one target or calibrator is above minel. ----\n";
	#	if(&Check_Gain_El){
	#	    print "None of the calibrators are up.  Exiting..\n";
	#	    command("standby");
	#	    exit;
	#	}   
	#	print "---- At least one calibrator is available. ----\n\n\n";
		$initial++;
	    }

	    ##actual commands:
	    @temp = split('\[',$pol_loop[$i]);
	    $current_targ=${$temp[$0]};
	    @pol_commands=split(',',substr($temp[1], 0, -1));

	    $targel =  $targel=checkEl($current_targ);
	    if($targel < $MINEL_TARG || $targel > $MAXEL_TARG){
		print "Target $current_targ is out of elevation range $MINEL_TARG - $MAXEL_TARG.\n";


		@array = split(//, $temp[0]);
		if($array[0] eq "t"){
		    print "Source $current_targ is a target, skipping...\n";
		    $FOUND_REPLACEMENT = -1;
		}elsif($array[0] eq "c"){
		    print "Source $current_targ is a calibrator, searching for an alternate...\n";
		    $FOUND_REPLACEMENT = 0;
		    for($k=0;$k<@pol_loop;$k++){
			if($FOUND_REPLACEMENT == 0){
			    @temp2 = split('\[',$pol_loop[$k]);
			    @array2 = split(//, $temp2[0]);
			    if($array2[0] eq "c"){
				$test_targ=${$temp2[0]};
				@pol_command_test=split(',',substr($temp2[1], 0, -1));
				$chktargel = checkEl($test_targ);
				if($chktargel > $MINEL_GAIN && $chktargel < $MAXEL_GAIN){
				    print "Will use $test_targ instead (el: $chktargel).\n";
				    $FOUND_REPLACEMENT = 1;
				    $current_targ = $test_targ;
				    @pol_commands=@pol_command_test;
				}
			    }
			}
		    }
		    if($FOUND_REPLACEMENT == 0){
			print "No suitable replacement calibrator found... exiting.\n";
			$EXIT = 1;
			last;
		    }
		}
#.new
		
		#command("standby");
	    }
	    
	    if(($targel > $MINEL_GAIN && $targel < $MAXEL_GAIN) || ($FOUND_REPLACEMENT == 1)){
		if($prev_targ ne $current_targ){
		    command("observe -s $current_targ");
		    command("antennaWait");
		}
		#rint "@pol_commands\n";
		for($j=0;$j<@pol_commands;$j++){
		    command("${$pol_commands[$j]}");
		}
		if(&checkLST==1){
		    $EXIT=1;
		    print "Outside of LST range $LST_start - $LST_end\n";
		    last;
		}
		$prev_targ = $current_targ;
	    }
	}
    }
    print "Exiting Loop... \n";

}


sub checkLST
{
    $check=LST();
    if($LST_start > $LST_end){
	if(($check < $LST_start) and ($check > $LST_end)){
	    return 1;
	}
    }
    if($LST_start < $LST_end){
	if(($check < $LST_start) or  ($check > $LST_end)){
	    return 1;
	}
    }
    return 0;
}


sub Cal_Too_High_or_Low
{
    print "---- Source elevation limit. ----\n";
    print "---- Searching for an available calibrator.. ----\n";
    LST(); 
    $count=@obs_loop;
        for($j=0;$j<$count;$j++){
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "c"){
		@temp_array = split('\|', $obs_loop[$j]);
		$length = @temp_array;
		
		if($length <= 1) {$current_source = $obs_loop[$j];}
		if($length > 1){
		    $current_source = selectSource($obs_loop[$j]);
		}  
		$targel=checkEl(${$current_source});
	        if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
		    #$current_source = ${$obs_loop[$j]};
	            $name = "n$obs_loop[$j]";
	            command("observe -s ${$obs_loop[$j]}");
	            command("tsys");
	            command("integrate -s ${$name} -w");
	            return 1;
                }
            }
        } 
    return 0;
}

sub Source_Too_High_or_Low
{
    $TOO_HIGH=0;
    print "---- Source elevation limit. ----\n";
    print "---- Searching for an available target... ----\n";
    LST(); 
    $count=@obs_loop;
        for($j=0;$j<$count;$j++){
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "t"){
		$targel=checkEl(${$obs_loop[$j]});
	        if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
		   # $current_source = ${$obs_loop[$j]};
	           # $name = "n$obs_loop[$j]";
	           # command("observe -s ${$obs_loop[$j]}");
	           # command("tsys");
	           # command("integrate -s ${$name} -w");
	            $TOO_HIGH=1;
		    last;
                }
	    }elsif($array[0] eq "m"){
		@array2= lookupMosaicEl($obs_loop[$j]);
		if($array2[0] <  $MAXEL_GAIN and $array2[1] > $MINEL_GAIN){
		    $TOO_HIGH=1;
		    last;
		}
	    }
        }
    return $TOO_HIGH;
}

sub Check_Target_El
{
    $TARG_TOO_LOW=1;
    $count=@obs_loop;
    #print "there are $count targets to check.\n";
    for($j=0;$j<$count;$j++){
	@array = split(//, $obs_loop[$j]);
	#print "$array[0] - ${$obs_loop[$j]}\n";
	if($array[0] eq "t"){
	    $targel=checkEl(${$obs_loop[$j]});
	    #print "${$obs_loop[$j]} is at $targel degrees.\n";
	    if($targel > $MINEL_TARG){ 
		$TARG_TOO_LOW=0;
		last;
	    }   
        }elsif($array[0] eq "m"){
	    @array2= lookupMosaicEl($obs_loop[$j]);
	    if($array2[0] > $MINEL_TARG){
		$TARG_TOO_LOW=0;
		last;
	    }
	}
    }
    return $TARG_TOO_LOW;
}


sub Check_Gain_El
{
    $GAIN_TOO_LOW=1;
    $count=@obs_loop;
    for($j=0; $j<$count; $j++){
	@array = split(//, $obs_loop[$j]);
	if($array[0] eq "c"){
	    @temp_array = split('\|', $obs_loop[$j]);
	    $length = @temp_array;
	    	    
	    if($length <= 1){
		$current_source = $obs_loop[$j];
	    }
	    if($length > 1){
		$current_source = selectSource($obs_loop[$j]);
	    }  

	    $targel=checkEl(${$current_source});
	    if($targel > $MINEL_CHECK){ 
		$GAIN_TOO_LOW=0; 
		last; 
	    }
        }   
    }
    return $GAIN_TOO_LOW;
}

sub DoFlux 
{
 # ---------- Start of $flux1 loop -----------
    $total_scans = 0;
    $flux = $_[0];
    $nflux = $_[1];
    print "Checking elevation of Flux Cal ${$flux}...\n";
    LST(); $fluxel=checkEl(${$flux});
    if($fluxel < $MINEL_FLUX){
	print "too low, skipping.\n";
    }elsif($fluxel > $MAXEL_FLUX){
	print "too high, skipping.\n";
    }elsif($fluxel > $MINEL_FLUX and $fluxel < $MAXEL_FLUX){
	print "\n";
	print "#######################\n";
	print "# FLUX Calibration    #\n";
	print "#######################\n";
	print "\n";
	print "---- Observing $flux. ----\n"; 
	print "\n";
	
	$current_source = ${$flux};
	until(${$nflux} <= $total_scans){
	    LST(); $fluxel=checkEl(${$flux});
	    if($fluxel < $MINEL_FLUX or $fluxel > $MAXEL_FLUX){
		print "Source elevation limit reached.\n";
		print "Exiting ...\n";
		command("standby");
		exit;
	    }else{
		command("observe -s ${$flux} -t flux");
		if($total_scans == 0){
		    command("tsys");
		}
		command("integrate -s 10 -w");
		$total_scans = $total_scans+10;
		print "Finished $total_scans/${$nflux} on ${$flux}.\n";
	    }
	}
    }
    #---------- End of $flux1 loop -----------
}

sub DoPass
{
    # ---------- Start of $bpass1 loop -----------
    $total_scans = 0;
    $bpass = $_[0];
    $nbpass = $_[1];
    print "Checking elevation of Bandpass Cal ${$bpass}...\n";
    LST(); $bpel=checkEl(${$bpass});
    if($bpel < $MINEL_BPASS){
	print "too low, skipping.\n";
    }elsif($bpel > $MAXEL_BPASS){
	print "too high, skipping.\n";
    }elsif($bpel > $MINEL_BPASS and $bpel < $MAXEL_BPASS){
	print "\n";
	print "###########################\n";
	print "# BANDPASS Calibration    #\n";
        print "###########################\n";
	print "\n";
	print "----  Observing $bpass. ----\n"; 
	print "\n";
	
	$current_source = ${$bpass};
	until(${$nbpass} <= $total_scans){
	    LST(); $bpel=checkEl(${$bpass});
	    if($bpel < $MINEL_BPASS or $bpel > $MAXEL_BPASS){
		print "Source elevation limit reached.\n";
		print "Exiting ...\n";
		command("standby");
		exit;
	    }else{
		command("observe -s ${$bpass} -t bandpass");
		if($total_scans == 0){
		    command("tsys");
		}
		command("integrate -s 10 -w");
		$total_scans = $total_scans+10;
		print "Finished $total_scans/${$nbpass} on ${$bpass}.\n";
	    }
	}
    }
    # ---------- End of $bpass1 loop -----------
}

sub DoCal
{
 # ---------- Start of $bpass1 loop -----------


     $total_scans = 0;
     $cal = $_[0];
     $ncal = $_[1];
     LST(); $calel=checkEl($cal);
     if($calel > $MINEL_BPASS and $calel < $MAXEL_BPASS){
	 print "\n";
	 print "###########################\n";
	 print "# BANDPASS Calibration    #\n";
	 print "###########################\n";
	 print "\n";
	 print "----  Observing $cal. ----\n"; 
	 print "\n";
	 
          $current_source = $cal;
          until($ncal <= $total_scans){
               LST(); $calel=checkEl($cal);
               if($calel < $MINEL_BPASS or $calel > $MAXEL_BPASS){
                    print "Source elevation limit reached.\n";
                    print "Exiting ...\n";
                    command("standby");
                    exit;
               }else{
                    command("observe -s $cal ");
                    if($total_scans == 0){
                         command("tsys");
                    }
                    command("integrate -s 10 -w");
                    $total_scans = $total_scans+10;
                    print "Finished $total_scans/$nbpass on $bpass.\n";
               }
          }
      }
 # ---------- End of $bpass1 loop -----------
}


sub Observe_Transcal
{
    print "---- Searching for an available transit calibrator... ----\n";
     LST(); $transel=checkEl($transcal);
     if($transel > $MINEL_GAIN and $transel < $MAXEL_GAIN){
         $current_source = $transcal;
    print "---- Observing transit calibrator ----\n";
          command("observe -s $transcal");
          command("tsys");
          command("integrate -s 10 -w");
     }else{
         print "---- Transcal not available.  Exiting.. ----\n";
         command("standby");
         exit;
     }
}


#--------------functions for mosaicing-------

# targetRaDec($targ#)
# must be a full target string, with at least -r and -d.
# returns an array: (RA, DEC, epoch, name) in decimal format with 3 
# significant figures after decimal.
sub targetRaDec
{
    #print "     **$_[0]\n";
    @temp_array=();
    @temp_array2=();
    @coordinates=();
    $sign=0;
    @temp_array = split(' ',$_[0]);
    $coordinates[3]=$temp_array[0];
    $length = @temp_array;
    for($k=0; $k<$length; $k++){
	if($temp_array[$k] eq '-r'){
	    $k++;
	    @temp_array2 = split(":",$temp_array[$k]);
	    $coordinates[0]= int(1000*($temp_array2[0] + ($temp_array2[1]/60) + ($temp_array2[2]/3600)))/1000;
	}elsif($temp_array[$k] eq '-d'){
	    $k++;	
	    @temp_array2 = split(":",$temp_array[$k]);
	    if(length($temp_array2[0]) > 2){ 
		$sign=substr($temp_array2[0],0,1);
		$temp_array2[0]=substr($temp_array2[0],1);
	    }
	    $coordinates[1]=int(1000*($temp_array2[0] + ($temp_array2[1]/60) + ($temp_array2[2]/3600)))/1000;
	    if($sign eq "-"){$coordinates[1]=$sign . $coordinates[1];}
	}elsif($temp_array[$k] eq '-e'){
	    $k++;
	    $coordinates[2]=$temp_array[$k];
	}
    }    
    return @coordinates;
}

sub splitMosaicString
{
    #print "     ** $_[0]\n";
    @temp_array = ();
    @temp_array = split('\[', $_[0]);
    #print "     $temp_array[0], $temp_array[1]\n";
    $temp=substr($temp_array[0], 1);
    @temp_array = split(',', substr($temp_array[1], 0, -1));
    $temp_array[2]=$temp;
    return @temp_array;
}

sub lookupMosaicEl
{
    ($Roff,$Doff,$TARG)=splitMosaicString($_[0]);
    ($RA,$DEC,$EPOCH,$name)=targetRaDec(${$TARG});
    #print "**> $RA $DEC $Roff $Doff $TARG ${$TARG}\n";
    #print "**> lookup -s $name -r $RA -d $DEC -e $EPOCH\n";
    #@coords= `lookup -s $name -r $RA -d $DEC -e $EPOCH\n`;
    #print "**>     $coords[0] $coords[1] $coords[2]\n";
    #$RA+=($Roff/3600); 
    $RA = int(1000*($RA+($Roff/3600)))/1000;
    $DEC = int(1000*($DEC+($Doff/3600)))/1000;
    $RA_h = int($RA);
    $RA_m = int((((100*$RA)-(100*$RA_h))*60)/100);
    $RA_s = int(1000*((((((100*$RA)-(100*$RA_h))*60) - ($RA_m*100))*60)/100))/1000;
    $DEC_h = int($DEC);
    $DEC_m = int((((100*$DEC)-(100*$DEC_h))*60)/100);
    $DEC_s = int(1000*((((((100*$DEC)-(100*$DEC_h))*60) - ($DEC_m*100))*60)/100))/1000;
    #$DEC+=($Doff/3600);
    $temp= "$name:$Roff,$Doff -r $RA_h:$RA_m:$RA_s -d $DEC_h:$DEC_m:$DEC_s -e $EPOCH\n";
   # print "**> $temp\n";
	@coords = checkEl($temp, 1);
   # print "**> lookup -s $name -r $RA -d $DEC -e $EPOCH\n";
    #@coords= `lookup -s $name -r $RA -d $DEC -e $EPOCH\n`;
    $coords[0] = int(100*$coords[0])/100;
    $coords[1] = $name;
    $coords[2] = $Roff;
    $coords[3] = $Doff;
    $coords[4] = $TARG;
   # print "**>     $coords[0] $coords[1] $coords[2] $coords[3] $coords[4]\n";
    return @coords;
}
