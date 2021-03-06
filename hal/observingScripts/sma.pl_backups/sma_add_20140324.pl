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
    @cals = ();
    foreach $object (@obs_loop)
    {
	if( $object =~ /^c/)
	{
	    if(@cals == ())
	    {
		push @cals, ${$object};
	    }
	    else
	    {
		$yes = 0;
		foreach $thing (@cals)
		{
		    if($thing eq ${$object})
		    {
			$yes = 1;
		    }
		}
		if($yes == 0)
		{
		    push @cals, ${$object};
		}
	    }
	}
    }
    #print "cals is: \n @cals \n";
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
	if($nloop == 2 && $thisMachine =~ /hal/)
	{
	    scriptcopy();
	}
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
	    if(&checkLST==1)
	    {
		$EXIT=-1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    if($nloop >= 200)
	    {
		$EXIT=1;
		print "---- Maximum number of loops achieved.  Exiting.. ----\n";
		last;
	    }
	    if($init==0)
	    {
		print "\n";
		print "###########################\n";
		print "# Initial Elevation Check #\n";
		print "###########################\n";
		print "\n";
		
		LST(); 
		if(&Check_Target_El)
		{
		    print "All target sources too low.  Exiting..\n";
		    command("standby");
		    exit;	
		}
		print "---- At least one target is available. ----\n";
		if(&Check_Gain_El)
		{
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
		LST(); 
		$targel=checkEl(${$obs_loop[$i]});
		$targelcheck = $MINEL_TARG;
		if($sourceAz < 360 && $sourceAz > 180)
		{
		    $targelcheck = $MINEL_CHECK;
		}
		$currentel = $targel;
	        if($targel < $targelcheck)
		{
		    if(&Check_Target_El)
		    {
			print "All target sources too low.\n";
		        $EXIT = 1;
			last;
		    }
		}
		elsif($targel > $MAXEL_TARG)
		{
		    while(!&Source_Too_High_or_Low)
		    {
			print "---- None of the Targets are available.  Trying transcal. ----\n";
			if($LST_start < $LST_end)
			{
			    if((LST() < $LST_start) || (LST() > $LST_end))
			    {
				last;
			    }
			}
			else
			{
			    if((LST() < $LST_start) && (LST() > $LST_end))
			    {
				last;
			    }
			}
			&Observe_Transcal; 
		    }
		}
		#elsif($nloop % 1 == 0)  replaced because this check is retarded.
		else
		{
		    $icount = $i;
		    $total_time = 0;
		    #print "The first source variable is: $obs_loop[$icount]\n";
		    while(($obs_loop[$icount] !~ /^c/) && ($icount != $obs_loop_length))
		    {
			$temp = "n$obs_loop[$icount]";
			if(${$temp} == "")
			{
			    if($temp =~ /\[/)
			    {
				@temparray = split (/targ/, $temp);
				#print "temparray has the following: @temparray\n";
				@scans = split /\[/, $temparray[1];
				#print "scans has the following: @scans\n";
			        $temp = "ntarg" . $scans[0];
			    }
			}
			#print "The number of scans for n$obs_loop[$icount] should be in this variable: ${$temp}.\n";
			$total_time += "${$temp}";
			$total_time += 1;
			$icount++;
		    }
		    $total_time *= $inttime;
		    if($simulateMode)
		    {
			$future_time = $unixTime + $total_time;
			@lookup_time = localtime($future_time);
		    }
		    else
		    {
			$future_time = time + $total_time;
			@lookup_time = gmtime($future_time);
		    }
		    #print "The next cal will be observed in $total_time seconds.\n";
		    $month = (qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec))[$lookup_time[4]];
		    $year = $lookup_time[5] + 1900;
		    if($obs_loop[$icount] =~ /^c/)
		    {
			$temp_source = "${$obs_loop[$icount]}";
		    }
		    else
		    {
			$temp_source = "${$obs_loop[0]}";
		    }
		    $future_time = " -t \"$lookup_time[3] $month $year $lookup_time[2]:$lookup_time[1]:$lookup_time[0]\"";
		    #print "$future_string\n";
		    $check = &callookup;
		    if($check == 0)
		    {
			&findothercal;
		    }
		    $future_string = "";
		    #print "before the if check is $check\n";
		    if($check == 1)#check to see if the next cal will be up by the end of the source(s)
		    {
			unless(${$int_time})
			{
			    print "The current source \(${$obs_loop[$i]}\) does not have any scans scheduled. The number of scans is probably not set in the script.\n";
			}
			$obs_targ = 1;
			$current_source = ${$obs_loop[$i]};
			command("observe -s ${$obs_loop[$i]}");
			command("tsys");
			command("integrate -s ${$int_time} -w");
			$lastsource = ${$obs_loop[$i]};
		    }
		    else
		    {
			$EXIT = 1;
			#print "The calibrator will be too low after this source loop, skipping to the final gain cal.  The last target is currently at $currentel degrees.\n";
			print "THE CALIBRATOR WILL BE TOO LOW AFTER THIS SOURCE LOOP, SKIPPING TO THE FINAL GAIN CAL. \nTHE LAST TARGET IS CURRENTLY AT $currentel DEGREES.\n";
			last;
		    }
		}
	    }
	    elsif($array[0] eq "c")
            {
		print "\n";
		print "---- Observing Gain Calibrator (Loop = $nloop) ----\n";
		#print "\n";
		
		@temp_array = split('\|', $obs_loop[$i]);
		#print "temp array contains the following:\n @temp_array\n";
		$length = @temp_array;
		
		if($length <= 1){ $current_source = $obs_loop[$i];}
		if($length > 1)
		{
		    print "---- Multiple calibrators possible ----\n";
		    print "---- Selecting between ${$temp_array[0]} "; 
		    for($j=1;$j<$length;$j++) { print "| ${$temp_array[$j]} ";}
		    print "\n";
		    $current_source = selectSource($obs_loop[$i]);
		}
		
		$int_time =  "n$current_source";
	        LST(); $targel=checkEl(${$current_source});
                if($targel < $MINEL_GAIN)
		{
		    if(!&Cal_Too_High_or_Low)
		    {
			print "None of the calibrators are above $MINEL_GAIN degrees\n";
			$EXIT = 1;
		    }
		}
		elsif($targel > $MAXEL_GAIN)
		{
		    if(!&Cal_Too_High_or_Low)
		    {
			print "None of the calibrators are available.  Trying transcal.\n";
			while(checkEl($cal0) > $MAXEL_GAIN)
			{
			    LST(); 
			    &Observe_Transcal; 
			}
		    }
		}
		#elsif($nloop % 1 == 0) another one, really?
		elsif(${$current_source} eq $lastsource)
		{
		    print "The current calibrator was just observed, skipping to the next source.\n";
		}
		else
		{
		    unless(${$int_time})
		    {
			print "The current source \(${$current_source}\) does not have any scans scheduled. The number of scans is probably not set in the script.\n";
		    } 
                    command("observe -s ${$current_source}");
                    command("tsys");
                    command("integrate -s ${$int_time} -w");
		    $lastsource = ${$current_source};
		}
            }
	    elsif($array[0] eq "m")
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
		$targelcheck = $MINEL_TARG;
		if($sourceAz < 360 && $sourceAz > 180)
		{
		    $targelcheck = $MINEL_CHECK;
		}

		if($T_el < $targelcheck)
		{
		    print "Target too low, checking other targets...\n";
		    if(&Check_Target_El)
		    {
			print "All target sources too low.\n";
		        $EXIT = 1;
			last;
		    }
		    print "At least one target is still above El limit.\n";
		}
		elsif($T_el > $MAXEL_TARG)
		{
		    if(!&Source_Too_High_or_Low)
		    {
			print "---- None of the Targets are available.  Trying transcal. ----\n";
			while(!&Source_Too_High_or_Low)
			{
			    print "---- None of the Targets are available.  Trying transcal. ----\n";  
			    if($LST_start < $LST_end)
			    {
				if((LST() < $LST_start) || (LST() > $LST_end))
				{
				    last;
				}
			    }
			    else
			    {
				if((LST() < $LST_start) && (LST() > $LST_end))
				{
				    last;
				}
			    }
			    &Observe_Transcal; 
			}
		    }
		}
		#elsif($nloop % 1 == 0) replaced, because else has the same effect with less effort.
		else
		{
		    $icount = $i;
		    $total_time = 0;
		    #print "The first source variable is: $obs_loop[$icount]\n";
		    while(($obs_loop[$icount] !~ /^c/) && ($icount != $obs_loop_length))
		    {
			$temp = "n$obs_loop[$icount]";
			if(${$temp} == "")
			{
			    if($temp =~ /\[/)
			    {
				@temparray = split (/targ/, $temp);
				#print "temparray has the following: @temparray\n";
				@scans = split /\[/, $temparray[1];
				#print "scans has the following: @scans\n";
			        $temp = "ntarg" . $scans[0];
			    }
			}
			#print "The number of scans for n$obs_loop[$icount] should be in this variable: ${$temp}.\n";
			$total_time += "${$temp}";
			$total_time += 1;
			$icount++;
		    }
		    $total_time *= $inttime;
		    if($simulateMode)
		    {
			$future_time = $unixTime + $total_time;
			@lookup_time = localtime($future_time);
		    }
		    else
		    {
			$future_time = time + $total_time;
			@lookup_time = gmtime($future_time);
		    }
		    #print "The next cal will be observed in $total_time seconds.\n";
		    $month = (qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec))[$lookup_time[4]];
		    $year = $lookup_time[5] + 1900;
		    if($obs_loop[$icount] =~ /^c/)
		    {
			$temp_source = "${$obs_loop[$icount]}";
		    }
		    else
		    {
			$temp_source = "${$obs_loop[0]}";
		    }
		    $future_time = " -t \"$lookup_time[3] $month $year $lookup_time[2]:$lookup_time[1]:$lookup_time[0]\"";
		    #print "$future_string\n";
		    $check = &callookup;
		    if($check == 0)
		    {
			&findothercal;
		    }
		    $future_string = "";
		    #print "before the if check is $check\n";
		    if($check == 1)#check to see if the next cal will be up by the end of the source(s)
		    {
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
			$lastsource = ${$obs_loop[$i]};
		    }
		    else
		    {
			$EXIT = 1;
			#print "The calibrator will be too low after this source loop, skipping to the final gain cal.  The last target is currently at $currentel degrees.\n";
			print "THE CALIBRATOR WILL BE TOO LOW AFTER THIS SOURCE LOOP, SKIPPING TO THE FINAL GAIN CAL. \nTHE LAST TARGET IS CURRENTLY AT $currentel DEGREES.\n";
			last;
		    }
		}
	    }	
	}
    }
    if($EXIT != -1 || $obs_targ >= 1)
    {
#	print "The value of EXIT is $EXIT\n";
	print "\n";
	print "##########################\n";
	print "# Final Gain Calibration #\n";
	print "##########################\n";
	print "\n";
	LST(); 
	$count= @obs_loop;
        for($j=0;$j<$count;$j++)
	{
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "c")
	    {
		$targel=checkEl(${$obs_loop[$j]});
		if(${$obs_loop[$j]} eq $lastsource)
		{
		    print "The current calibrator was just observed, skipping to the next source.\n";
		}
	        elsif($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN)
		{
		    $current_source = ${$obs_loop[$j]};
	            $name = "n$obs_loop[$j]";
	            command("observe -s ${$obs_loop[$j]}");
	            command("tsys");
	            command("integrate -s ${$name} -w");
		    $lastsource = ${$obs_loop[$i]};
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


sub callookup
{
    $check = 0;
    #print "starting callookup!\n";
    @temp_array = split('\|', $obs_loop[$i]);
    $length = @temp_array;

    if($length <= 1){ $current_source = $obs_loop[$i];}
    if($length > 1)
    {
	print "---- Multiple calibrators possible ----\n";
	print "---- Selecting between ${$temp_array[0]} "; 
	for($j=1;$j<$length;$j++) { print "| ${$temp_array[$j]} ";}
	print "\n";
	$current_source = selectSource($obs_loop[$i]);
    }
    $int_time =  "n$current_source";
    LST();
    $future_string = "lookup -s $temp_source" .$future_time;
    #print "Future_string in callookup is: $future_string.\n";
    if($future_string ne "")
    {
	my ($sourcename)= ${$current_source};
	my ($silent)    =(defined($_[1]) and $_[1]);
	my ($sourceCoordinates,$sourceAz,$sourceEl,$sunDistance);
	#print "Passed the if\n";
	unless($thisMachine =~ /ulua/)
	{
	    $sourceCoordinates=`$future_string`;
	}
	else
	{
	    $sourceCoordinates=`\./$future_string`;
	}
	chomp($sourceCoordinates);
	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
	if ($sourceAz eq 'error_flag') 
	{
	    print "##########################################\n";
	    print "######## WARNING WARNING WARNING #########\n";
	    print "##### source $sourcename not found. ######\n";
	    print "##########################################\n";
	    for ($i=0; $i<7;$i++) {print "\a";sleep 1;}
	    die   " quiting from the script \n";
	}
	#print "The lookup command is: $sourceCoordinates.\n";
	#print "The elevation is: $sourceEl.\n";
	#print "Minimum elevation: $MINEL_GAIN\n";
	
	#if (not $silent) 
	#{
	    #printf("%s will be at %4.1f degrees elevation the next time it is observed.\n",$temp_source,$sourceEl);
	#}
	$targel = $sourceEl;
    }
    else
    {
	#print "passed the else.\n";
	$targel=checkEl(${$current_source});
    }
    #print "targel is $targel\n";
    if($targel > $MINEL_GAIN)
    {
	$check = 1;
	#printf("%s will be at %3.1f degrees the next time it's observed.\n",$temp_source,$targel);
    }
    return $check;
}


sub selectSource
{
    @temp_array = split('\|', $_[0]);
    $length = @temp_array;
    #print "In selectSource\n";
    for($j=0; $j<$length; $j++)
    {
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
    for($i=0; $i<$pol_loop_length; $i++)
    {
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
 
    for($i=0;$i<@pol_loop;$i++)
    {
	@temp = split('\[',$pol_loop[$i]);
	$current_targ=${$temp[$0]};
	@pol_commands=split(',',substr($temp[1], 0, -1));
	print "#   Source: $current_targ\n";
	for($j=0;$j<@pol_commands;$j++)
	{
	    print "#     do: ${$pol_commands[$j]}\n";
	}
    }
    print "# Repeat over LST range and target availability.\n";
    
    while($EXIT==0)
    {
	printPID(); 
	$nloop=$nloop+1;
        print "\nLoop No.= $nloop\n";
        print "\n";
	for($i=0;$i<@pol_loop;$i++)
	{
	    #TARGET:
	    if(&checkLST==1)
	    {
		$EXIT=1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    if($nloop > 80)
	    {
		$EXIT=1;
		print "---- Maximum number of loops achieved.  Exiting.. ----\n";
		last;
	    }
	    
	    if($initial==0)
	    {
		print "\n";
		print "###################\n";
		print "# Elevation Check #\n";
	        print "###################\n";
		print "\n";
		LST();
		if(&Check_Target_El && &Check_Gain_El)
		{
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
	    #print "temp array contains:\n @temp\n";
	    $current_targ=${$temp[$0]};
	    @pol_commands=split(',',substr($temp[1], 0, -1));

	    $targel =  $targel=checkEl($current_targ);
	    if($targel < $MINEL_TARG || $targel > $MAXEL_TARG)
	    {
		print "Target $current_targ is out of elevation range $MINEL_TARG - $MAXEL_TARG.\n";


		@array = split(//, $temp[0]);
		if($array[0] eq "t")
		{
		    print "Source $current_targ is a target, skipping...\n";
		    $FOUND_REPLACEMENT = -1;
		}
		elsif($array[0] eq "c")
		{
		    print "Source $current_targ is a calibrator, searching for an alternate...\n";
		    $FOUND_REPLACEMENT = 0;
		    for($k=0;$k<@pol_loop;$k++)
		    {
			if($FOUND_REPLACEMENT == 0)
			{
			    @temp2 = split('\[',$pol_loop[$k]);
			    @array2 = split(//, $temp2[0]);
			    if($array2[0] eq "c")
			    {
				$test_targ=${$temp2[0]};
				@pol_command_test=split(',',substr($temp2[1], 0, -1));
				$chktargel = checkEl($test_targ);
				if($chktargel > $MINEL_GAIN && $chktargel < $MAXEL_GAIN)
				{
				    print "Will use $test_targ instead (el: $chktargel).\n";
				    $FOUND_REPLACEMENT = 1;
				    $current_targ = $test_targ;
				    @pol_commands=@pol_command_test;
				}
			    }
			}
		    }
		    if($FOUND_REPLACEMENT == 0)
		    {
			print "No suitable replacement calibrator found... exiting.\n";
			$EXIT = 1;
			last;
		    }
		}
#.new
		
		#command("standby");
	    }
	    
	    if(($targel > $MINEL_GAIN && $targel < $MAXEL_GAIN) || ($FOUND_REPLACEMENT == 1))
	    {
		if($prev_targ ne $current_targ)
		{
		    command("observe -s $current_targ");
		    command("antennaWait");
		}
		#rint "@pol_commands\n";
		for($j=0;$j<@pol_commands;$j++)
		{
		    command("${$pol_commands[$j]}");
		}
		if(&checkLST==1)
		{
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

#this loop will run the dual polar mode. It will accept 4 new calibration variables:
#$targpol#, $calpol#, $bpasspol#, $fluxpol#
#this way the sources can be re-named by the PI so they know what's going on.
#These vailable will be exactly the same as their non-pol counterparts, except
#they will run a second rotateWaveplate for the odd antennas out. For now
#we'll call them 2 and 3, but there will be a way to figure it out automatically
#to avoid the nants problem. This variable will be called $pants (because $polants isn't as cool). 

#Do we make a dual polarization mossaic mode? Then when do we do the rotating and such?
#Adding to every loop will be as problimatic as the tsys thing.

sub DualPolLoop
{
    @obs_loop = @_;
    $nloop = 0;
    $init = 0;
    $obs_targ = 0;
    @cals = ();
    foreach $object (@obs_loop)
    {
	if( $object =~ /^c/)
	{
	    if(@cals == ())
	    {
		push @cals, ${$object};
	    }
	    else
	    {
		$yes = 0;
		foreach $thing (@cals)
		{
		    if($thing eq ${$object})
		    {
			$yes = 1;
		    }
		}
		if($yes == 0)
		{
		    push @cals, ${$object};
		}
	    }
	}
    }
    #print "cals is: \n @cals \n";
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
	if($nloop == 2 && $thisMachine =~ /hal/)
	{
	    scriptcopy();
	}
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
	    if(&checkLST==1)
	    {
		$EXIT=-1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    if($nloop >= 80)
	    {
		$EXIT=1;
		print "---- Maximum number of loops achieved.  Exiting.. ----\n";
		last;
	    }
	    if($init==0)
	    {
		print "\n";
		print "###########################\n";
		print "# Initial Elevation Check #\n";
		print "###########################\n";
		print "\n";
		
		LST(); 
		if(&Check_Target_El)
		{
		    print "All target sources too low.  Exiting..\n";
		    command("standby");
		    exit;	
		}
		print "---- At least one target is available. ----\n";
		if(&Check_Gain_El)
		{
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
		LST(); 
		$targel=checkEl(${$obs_loop[$i]});
		$targelcheck = $MINEL_TARG;
		if($sourceAz < 360 && $sourceAz > 180)
		{
		    $targelcheck = $MINEL_CHECK;
		}
		$currentel = $targel;
	        if($targel < $targelcheck)
		{
		    if(&Check_Target_El)
		    {
			print "All target sources too low.\n";
		        $EXIT = 1;
			last;
		    }
		}
		elsif($targel > $MAXEL_TARG)
		{
		    while(!&Source_Too_High_or_Low)
		    {
			print "---- None of the Targets are available.  Trying transcal. ----\n";
			if($LST_start < $LST_end)
			{
			    if((LST() < $LST_start) || (LST() > $LST_end))
			    {
				last;
			    }
			}
			else
			{
			    if((LST() < $LST_start) && (LST() > $LST_end))
			    {
				last;
			    }
			}
			&Observe_Transcal; 
		    }
		}
		#elsif($nloop % 1 == 0)  replaced because this check is retarded.
		else
		{
		    $icount = $i;
		    $total_time = 0;
		    #print "The first source variable is: $obs_loop[$icount]\n";
		    while(($obs_loop[$icount] !~ /^c/) && ($icount != $obs_loop_length))
		    {
			$temp = "n$obs_loop[$icount]";
			if(${$temp} == "")
			{
			    if($temp =~ /\[/)
			    {
				@temparray = split (/targ/, $temp);
				#print "temparray has the following: @temparray\n";
				@scans = split /\[/, $temparray[1];
				#print "scans has the following: @scans\n";
			        $temp = "ntarg" . $scans[0];
			    }
			}
			#print "The number of scans for n$obs_loop[$icount] should be in this variable: ${$temp}.\n";
			$total_time += "${$temp}";
			$total_time += 1;
			$icount++;
		    }
		    $total_time *= $inttime;
		    if($simulateMode)
		    {
			$future_time = $unixTime + $total_time;
			@lookup_time = localtime($future_time);
		    }
		    else
		    {
			$future_time = time + $total_time;
			@lookup_time = gmtime($future_time);
		    }
		    #print "The next cal will be observed in $total_time seconds.\n";
		    $month = (qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec))[$lookup_time[4]];
		    $year = $lookup_time[5] + 1900;
		    if($obs_loop[$icount] =~ /^c/)
		    {
			$temp_source = "${$obs_loop[$icount]}";
		    }
		    else
		    {
			$temp_source = "${$obs_loop[0]}";
		    }
		    $future_time = " -t \"$lookup_time[3] $month $year $lookup_time[2]:$lookup_time[1]:$lookup_time[0]\"";
		    #print "$future_string\n";
		    $check = &callookup;
		    if($check == 0)
		    {
			&findothercal;
		    }
		    $future_string = "";
		    #print "before the if check is $check\n";
		    if($check == 1)#check to see if the next cal will be up by the end of the source(s)
		    {
			unless(${$int_time})
			{
			    print "The current source \(${$obs_loop[$i]}\) does not have any scans scheduled. The number of scans is probably not set in the script.\n";
			} 
			$obs_targ = 1;
			$current_source = ${$obs_loop[$i]};
			command("observe -s ${$obs_loop[$i]}");
			command("tsys");
			command("rotateWaveplate -s L");
			if($obs_loop[$i] =~ /pol/)
			{
			    command("rotateWaveplate -a $pants -s R");
			}
			command("integrate -s ${$int_time} -w");
			$lastsource = ${$obs_loop[$i]};
		    }
		    else
		    {
			$EXIT = 1;
			#print "The calibrator will be too low after this source loop, skipping to the final gain cal.  The last target is currently at $currentel degrees.\n";
			print "THE CALIBRATOR WILL BE TOO LOW AFTER THIS SOURCE LOOP, SKIPPING TO THE FINAL GAIN CAL. \nTHE LAST TARGET IS CURRENTLY AT $currentel DEGREES.\n";
			last;
		    }
		}
	    }
	    elsif($array[0] eq "c")
            {
		print "\n";
		print "---- Observing Gain Calibrator (Loop = $nloop) ----\n";
		#print "\n";
		
		@temp_array = split('\|', $obs_loop[$i]);
		#print "temp array contains the following:\n @temp_array\n";
		$length = @temp_array;
		
		if($length <= 1){ $current_source = $obs_loop[$i];}
		if($length > 1)
		{
		    print "---- Multiple calibrators possible ----\n";
		    print "---- Selecting between ${$temp_array[0]} "; 
		    for($j=1;$j<$length;$j++) { print "| ${$temp_array[$j]} ";}
		    print "\n";
		    $current_source = selectSource($obs_loop[$i]);
		}
		
		$int_time =  "n$current_source";
	        LST(); $targel=checkEl(${$current_source});
                if($targel < $MINEL_GAIN)
		{
		    if(!&Cal_Too_High_or_Low)
		    {
			print "None of the calibrators are above $MINEL_GAIN degrees\n";
			$EXIT = 1;
		    }
		}
		elsif($targel > $MAXEL_GAIN)
		{
		    if(!&Cal_Too_High_or_Low)
		    {
			print "None of the calibrators are available.  Trying transcal.\n";
			while(checkEl($cal0) > $MAXEL_GAIN)
			{
			    LST(); 
			    &Observe_Transcal; 
			}
		    }
		}
		#elsif($nloop % 1 == 0) another one, really?
#		elsif(${$current_source} eq $lastsource)
#		{
#		    print "The current calibrator was just observed, skipping to the next source.\n";
#		}
		else
		{
		    unless(${$int_time})
		    {
			print "The current source \(${$current_source}\) does not have any scans scheduled. The number of scans is probably not set in the script.\n";
		    }
		    command("observe -s ${$current_source}");
                    command("tsys");
		    command("rotateWaveplate -s L");
		    if($obs_loop[$i] =~ /pol/)
		    {
			command("rotateWaveplate -a $pants -s R");
		    }
                    command("integrate -s ${$int_time} -w");
		    $lastsource = ${$current_source};
		}
            }
	    elsif($array[0] eq "m")
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
		$targelcheck = $MINEL_TARG;
		if($sourceAz < 360 && $sourceAz > 180)
		{
		    $targelcheck = $MINEL_CHECK;
		}

		if($T_el < $targelcheck)
		{
		    print "Target too low, checking other targets...\n";
		    if(&Check_Target_El)
		    {
			print "All target sources too low.\n";
		        $EXIT = 1;
			last;
		    }
		    print "At least one target is still above El limit.\n";
		}
		elsif($T_el > $MAXEL_TARG)
		{
		    if(!&Source_Too_High_or_Low)
		    {
			print "---- None of the Targets are available.  Trying transcal. ----\n";
			while(!&Source_Too_High_or_Low)
			{
			    print "---- None of the Targets are available.  Trying transcal. ----\n";  
			    if($LST_start < $LST_end)
			    {
				if((LST() < $LST_start) || (LST() > $LST_end))
				{
				    last;
				}
			    }
			    else
			    {
				if((LST() < $LST_start) && (LST() > $LST_end))
				{
				    last;
				}
			    }
			    &Observe_Transcal; 
			}
		    }
		}
		#elsif($nloop % 1 == 0) replaced, because else has the same effect with less effort.
		else
		{
		    $icount = $i;
		    $total_time = 0;
		    #print "The first source variable is: $obs_loop[$icount]\n";
		    while(($obs_loop[$icount] !~ /^c/) && ($icount != $obs_loop_length))
		    {
			$temp = "n$obs_loop[$icount]";
			if(${$temp} == "")
			{
			    if($temp =~ /\[/)
			    {
				@temparray = split (/targ/, $temp);
				#print "temparray has the following: @temparray\n";
				@scans = split /\[/, $temparray[1];
				#print "scans has the following: @scans\n";
			        $temp = "ntarg" . $scans[0];
			    }
			}
			#print "The number of scans for n$obs_loop[$icount] should be in this variable: ${$temp}.\n";
			$total_time += "${$temp}";
			$total_time += 1;
			$icount++;
		    }
		    $total_time *= $inttime;
		    if($simulateMode)
		    {
			$future_time = $unixTime + $total_time;
			@lookup_time = localtime($future_time);
		    }
		    else
		    {
			$future_time = time + $total_time;
			@lookup_time = gmtime($future_time);
		    }
		    #print "The next cal will be observed in $total_time seconds.\n";
		    $month = (qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec))[$lookup_time[4]];
		    $year = $lookup_time[5] + 1900;
		    if($obs_loop[$icount] =~ /^c/)
		    {
			$temp_source = "${$obs_loop[$icount]}";
		    }
		    else
		    {
			$temp_source = "${$obs_loop[0]}";
		    }
		    $future_time = " -t \"$lookup_time[3] $month $year $lookup_time[2]:$lookup_time[1]:$lookup_time[0]\"";
		    #print "$future_string\n";
		    $check = &callookup;
		    if($check == 0)
		    {
			&findothercal;
		    }
		    $future_string = "";
		    #print "before the if check is $check\n";
		    if($check == 1)#check to see if the next cal will be up by the end of the source(s)
		    {
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
			$lastsource = ${$obs_loop[$i]};
		    }
		    else
		    {
			$EXIT = 1;
			#print "The calibrator will be too low after this source loop, skipping to the final gain cal.  The last target is currently at $currentel degrees.\n";
			print "THE CALIBRATOR WILL BE TOO LOW AFTER THIS SOURCE LOOP, SKIPPING TO THE FINAL GAIN CAL. \nTHE LAST TARGET IS CURRENTLY AT $currentel DEGREES.\n";
			last;
		    }
		}
	    }	
	}
    }
    if($EXIT != -1 || $obs_targ >= 1)
    {
#	print "The value of EXIT is $EXIT\n";
	print "\n";
	print "##########################\n";
	print "# Final Gain Calibration #\n";
	print "##########################\n";
	print "\n";
	LST(); 
	$count= @obs_loop;
	$finished = 0;
        for($j=0;$j<=$count;$j++)
	{
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "c")
	    {
		$targel=checkEl(${$obs_loop[$j]});
#		if(${$obs_loop[$j]} eq $lastsource)
#		{
#		    print "The current calibrator was just observed, skipping to the next source.\n";
#		}
	        if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN)
		{
		    $current_source = ${$obs_loop[$j]};
	            $name = "n$obs_loop[$j]";
	            command("observe -s ${$obs_loop[$j]}");
	            command("tsys");
		    command("rotateWaveplate -s L");
		    if($obs_loop[$j] =~ /pol/)
		    {
			command("rotateWaveplate -a $pants -s R");
			$finished = 1;
		    }
	            command("integrate -s ${$name} -w");
		    $lastsource = ${$obs_loop[$i]};
		    if($finished == 1)
		    {
			last;
		    }
		}
	    }
	}
    }
    print "##########################\n";
    print "# End of Observing Loop  #\n";
    print "##########################\n";
    #---------- End of Observing loop -----------
}



sub checkLST
{
    $check=LST();
    #print "The LST time is: $check\n";
    if($LST_start > $LST_end)
    {
	if(($check < $LST_start) and ($check > $LST_end))
	{
	    return 1;
	}
    }
    if($LST_start < $LST_end)
    {
	if(($check < $LST_start) or  ($check > $LST_end))
	{
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
        for($j=0;$j<$count;$j++)
	{
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "c")
	    {
		@temp_array = split('\|', $obs_loop[$j]);
		$length = @temp_array;
		#print "temp array in the cal select contains:\n @temp_array\n";
		if($length <= 1) {$current_source = $obs_loop[$j];}
		if($length > 1)
		{
		    $current_source = selectSource($obs_loop[$j]);
		}  
		$targel=checkEl(${$current_source});
		if(${$current_source} eq $lastsource)
		{
		    print "The current calibrator was just observed, skipping to the next source.\n";
		    return 1;
		}
		elsif($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN)
		{
		    #$current_source = ${$obs_loop[$j]};
	            $name = "n$obs_loop[$j]";
	            command("observe -s ${$obs_loop[$j]}");
	            command("tsys");
	            command("integrate -s ${$name} -w");
		    $lastsource = ${$obs_loop[$j]};
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
        for($j=0;$j<$count;$j++)
	{
	    @array = split(//, $obs_loop[$j]);
	    if($array[0] eq "t")
	    {
		$targel=checkEl(${$obs_loop[$j]});
	        if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN)
		{
		   # $current_source = ${$obs_loop[$j]};
	           # $name = "n$obs_loop[$j]";
	           # command("observe -s ${$obs_loop[$j]}");
	           # command("tsys");
	           # command("integrate -s ${$name} -w");
	            $TOO_HIGH=1;
		    last;
                }
	    }
	    elsif($array[0] eq "m")
	    {
		@array2= lookupMosaicEl($obs_loop[$j]);
		if($array2[0] <  $MAXEL_GAIN and $array2[1] > $MINEL_GAIN)
		{
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
    for($j=0;$j<$count;$j++)
    {
	@array = split(//, $obs_loop[$j]);
	#print "$array[0] - ${$obs_loop[$j]}\n";
	if($array[0] eq "t")
	{
	    $targel=checkEl(${$obs_loop[$j]});
	    #print "${$obs_loop[$j]} is at $targel degrees.\n";
	    #print "The target az is currently $sourceAz\n";
	    if($targel > $targelcheck)
	    { 
		$TARG_TOO_LOW=0;
		last;
	    }   
        }
	elsif($array[0] eq "m")
	{
	    @array2= lookupMosaicEl($obs_loop[$j]);
	    if($array2[0] > $targelcheck)
	    {
		$TARG_TOO_LOW=0;
		last;
	    }
	}
	elsif($array[0] eq "b")
	{
	    $targel=checkEl(${$obs_loop[$j]});
	    if($targel > $MINEL_BPASS)
	    { 
		$TARG_TOO_LOW=0;
		last;
	    }   
	}
	elsif($array[0] eq "f")
	{
	    $targel=checkEl(${$obs_loop[$j]});
	    if($targel > $MINEL_FLUX)
	    { 
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
    for($j=0; $j<$count; $j++)
    {
	@array = split(//, $obs_loop[$j]);
	if($array[0] eq "c")
	{
	    @temp_array = split('\|', $obs_loop[$j]);
	    $length = @temp_array;
	    	    
	    if($length <= 1)
	    {
		$current_source = $obs_loop[$j];
	    }
	    if($length > 1)
	    {
		$current_source = selectSource($obs_loop[$j]);
	    }  

	    $targel=checkEl(${$current_source});
	    if($targel > $MINEL_GAIN)
	    { 
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
    if($fluxel < $MINEL_FLUX)
    {
	print "too low, skipping.\n";
    }
    elsif($fluxel > $MAXEL_FLUX)
    {
	print "too high, skipping.\n";
    }
    elsif($fluxel > $MINEL_FLUX and $fluxel < $MAXEL_FLUX)
    {
	print "\n";
	print "#######################\n";
	print "# FLUX Calibration    #\n";
	print "#######################\n";
	print "\n";
	print "---- Observing $flux. ----\n"; 
	print "\n";
	
	$current_source = ${$flux};
	until(${$nflux} <= $total_scans)
	{
	    LST(); $fluxel=checkEl(${$flux});
	    if($fluxel < $MINEL_FLUX or $fluxel > $MAXEL_FLUX)
	    {
		print "Source elevation limit reached.\n";
		print "Exiting ...\n";
		command("standby");
		exit;
	    }
	    else
	    {
		command("observe -s ${$flux} -t flux");
		if($total_scans == 0)
		{
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

sub DoPolFlux 
{
 # ---------- Start of $flux1 loop -----------
    $total_loops = 0;
    $EXIT = 0;
    $flux = $_[0];
    $nflux = $_[1];
    $fluxpol = $_[2];
    $nfluxpol = $_[3];
    print "Checking elevation of Flux Cal ${$flux}...\n";
    LST(); $fluxel=checkEl(${$flux});
    if($fluxel < $MINEL_FLUX)
    {
	print "too low, skipping.\n";
    }
    elsif($fluxel > $MAXEL_FLUX)
    {
	print "too high, skipping.\n";
    }
    elsif($fluxel > $MINEL_FLUX and $fluxel < $MAXEL_FLUX)
    {
	print "\n";
	print "#######################\n";
	print "# FLUX Calibration    #\n";
	print "#######################\n";
	print "\n";
	print "---- Observing $flux. ----\n"; 
	print "\n";
	
	$current_source = ${$flux};
	until($EXIT == -1)
	{
	    if(&checkLST==1)
	    {
		$EXIT=-1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    LST(); 
	    $fluxel=checkEl(${$flux});
	    if($fluxel < $MINEL_FLUX or $fluxel > $MAXEL_FLUX)
	    {
		print "Source elevation limit reached.\n";
		print "Exiting ...\n";
		command("standby");
		exit;
	    }
	    else
	    {
		if(($total_loops % 2) == 0)
		{
		    if($fluxpol)
		    {
			command("observe -s ${$fluxpol} -t flux");
		    }
		    else
		    {
			command("observe -s ${$flux} -t flux");
		    }
		    command("rotateWaveplate -a $pants -s R");
		    if($total_scans == 0)
		    {
			command("tsys");
		    }
		    if($nfluxpol)
		    {
			command("integrate -s ${$nfluxpol} -w");
		    }
		    else
		    {
			command("integrate -s ${$nflux} -w");
		    }
		}
		else
		{
		    command("observe -s ${$flux} -t flux");
		    command("rotateWaveplate -s L");
		    command("integrate -s ${$nflux} -w");
		}
	    }
	    $total_loops++;
	}
	if(($total_loops % 2) == 0)
	{
	    if($fluxpol)
	    {
		command("observe -s ${$fluxpol} -t flux");
	    }
	    else
	    {
		command("observe -s ${$flux} -t flux");
	    }
	    command("rotateWaveplate -a $pants -s R");
	    if($nbpasspol)
	    {
		command("integrate -s ${$nfluxpol} -w");
	    }
	    else
	    {
		command("integrate -s ${$nflux} -w");
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
    LST(); 
    $bpel=checkEl(${$bpass});
    if($bpel < $MINEL_BPASS)
    {
	print "too low, skipping.\n";
    }
    elsif($bpel > $MAXEL_BPASS)
    {
	print "too high, skipping.\n";
    }
    elsif($bpel > $MINEL_BPASS and $bpel < $MAXEL_BPASS)
    {
	print "\n";
	print "###########################\n";
	print "# BANDPASS Calibration    #\n";
        print "###########################\n";
	print "\n";
	print "----  Observing $bpass. ----\n"; 
	print "\n";
	
	$current_source = ${$bpass};
	until(${$nbpass} <= $total_scans)
	{
	    LST(); 
	    $bpel=checkEl(${$bpass});
	    if($bpel < $MINEL_BPASS or $bpel > $MAXEL_BPASS)
	    {
		print "Source elevation limit reached.\n";
		print "Exiting ...\n";
		command("standby");
		exit;
	    }
	    else
	    {
		command("observe -s ${$bpass} -t bandpass");
		if($total_scans == 0)
		{
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

sub DoPolPass
{
    # ---------- Start of $bpass1 loop -----------
    $total_loops = 0;
    $EXIT = 0;
    $bpass = $_[0];
    $nbpass = $_[1];
    $bpasspol = $_[2];
    $nbpasspol = $_[3];
    print "Checking elevation of Bandpass Cal ${$bpass}...\n";
    LST(); 
    $bpel=checkEl(${$bpass});
    if($bpel < $MINEL_BPASS)
    {
	print "too low, skipping.\n";
    }
    elsif($bpel > $MAXEL_BPASS)
    {
	print "too high, skipping.\n";
    }
    elsif($bpel > $MINEL_BPASS and $bpel < $MAXEL_BPASS)
    {
	print "\n";
	print "###########################\n";
	print "# BANDPASS Calibration    #\n";
        print "###########################\n";
	print "\n";
	print "----  Observing $bpass. ----\n"; 
	print "\n";
	
	$current_source = ${$bpass};
	command("rotateWaveplate -s L");
	until($EXIT == -1)
	{
	    if(&checkLST==1)
	    {
		$EXIT=-1;
		print "Outside of LST range $LST_start - $LST_end\n";
		last;
	    }
	    LST();
	    $bpel=checkEl(${$bpass});
	    if($bpel < $MINEL_BPASS or $bpel > $MAXEL_BPASS)
	    {
		print "Source elevation limit reached.\n";
		print "Exiting ...\n";
		command("standby");
		exit;
	    }
	    else
	    {
		if(($total_loops % 2) == 0)
		{
		    if($bpasspol)
		    {
			command("observe -s ${$bpasspol} -t bandpass");
		    }
		    else
		    {
			command("observe -s ${$bpass} -t bandpass");
		    }
		    command("rotateWaveplate -a $pants -s R");
		    if($total_scans == 0)
		    {
			command("tsys");
		    }
		    if($nbpasspol)
		    {
			command("integrate -s ${$nbpasspol} -w");
		    }
		    else
		    {
			command("integrate -s ${$nbpass} -w");
		    }
		}
		else
		{
		    command("observe -s ${$bpass} -t bandpass");
		    command("rotateWaveplate -s L");
		    command("integrate -s ${$nbpass} -w");
		}
	    }
	    $total_loops++;
	}
	#print "Exited the loop! total_loops is $total_loops\n";
	if(($total_loops % 2) == 0)
	{
	    if($bpasspol)
	    {
		command("observe -s ${$bpasspol} -t bandpass");
	    }
	    else
	    {
		command("observe -s ${$bpass} -t bandpass");
	    }
	    command("rotateWaveplate -a $pants -s R");
	    if($nbpasspol)
	    {
		command("integrate -s ${$nbpasspol} -w");
	    }
	    else
	    {
		command("integrate -s ${$nbpass} -w");
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
     if($calel > $MINEL_BPASS and $calel < $MAXEL_BPASS)
     {
	 print "\n";
	 print "###########################\n";
	 print "# BANDPASS Calibration    #\n";
	 print "###########################\n";
	 print "\n";
	 print "----  Observing $cal. ----\n"; 
	 print "\n";
	 
          $current_source = $cal;
          until($ncal <= $total_scans)
	  {
               LST(); $calel=checkEl($cal);
               if($calel < $MINEL_BPASS or $calel > $MAXEL_BPASS)
	       {
                    print "Source elevation limit reached.\n";
                    print "Exiting ...\n";
                    command("standby");
                    exit;
               }
	       else
	       {
                    command("observe -s $cal ");
                    if($total_scans == 0)
		    {
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
    LST(); 
    $transel=checkEl($transcal);
    if($transel > $MINEL_GAIN and $transel < $MAXEL_GAIN)
    {
	$current_source = $transcal;
	print "---- Observing transit calibrator ----\n";
	command("observe -s $transcal");
	command("tsys");
	command("integrate -s 10 -w");
	$lastsource = $transcal;
    }
    else
    {
	print "---- Transcal not available.  Exiting.. ----\n";
	command("standby");
	exit;
    }
}

sub findothercal
{
    #print "starting findothercal.\n";
    #@cals will contain all of the cals in the source loop
    #$nextcal will be the next cal that was already checked
    $nextcal = $temp_source;
    foreach $tempcal (@cals)
    {
	if($tempcal ne $nextcal)
	{
	    $temp_source = $tempcal;
	    #print "checking $temp_source\n";
	    $check = callookup;
	    if($check == 1)
	    {
		last;
	    }
	}
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
    for($k=0; $k<$length; $k++)
    {
	if($temp_array[$k] eq '-r')
	{
	    $k++;
	    @temp_array2 = split(":",$temp_array[$k]);
	    $coordinates[0]= int(1000*($temp_array2[0] + ($temp_array2[1]/60) + ($temp_array2[2]/3600)))/1000;
	}
	elsif($temp_array[$k] eq '-d')
	{
	    $k++;	
	    @temp_array2 = split(":",$temp_array[$k]);
	    if(length($temp_array2[0]) > 2)
	    { 
		$sign=substr($temp_array2[0],0,1);
		$temp_array2[0]=substr($temp_array2[0],1);
	    }
	    $coordinates[1]=int(1000*($temp_array2[0] + ($temp_array2[1]/60) + ($temp_array2[2]/3600)))/1000;
	    if($sign eq "-"){$coordinates[1]=$sign . $coordinates[1];}
	}
	elsif($temp_array[$k] eq '-e')
	{
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
