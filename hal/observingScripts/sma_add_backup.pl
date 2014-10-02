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



sub initialize() 
{
    print "initializing.....\n";
    $garrus = 0;
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
    GetOptions('loop=i' => \$nloop, 'time=s' => \$givenUTC, 'simulate', 'restart', 'mosaic', 'figure','antennas=s'=>\$antList, 'pointing');
    $nloop--;
    if($givenUTC) 
    {
	($mn,$d,$year,$gh,$gm)=split(' ',$givenUTC);
	$mn=$mn-1;
	$unixTime=timelocal(0,$gm,$gh,$d,$mn,$year);
	$prevUnixTime=$unixTime;
    }
    #print "The current time is $unixTime\n";
    if($opt_simulate) {$simulateMode=1;} else {$simulateMode=0;}
    if($opt_restart || $opt_figure) {$restart=1;} else {$restart=0;}
    
#Ctrl-C interrupt handler;
    $SIG{INT}=\&finish;

    $unameResponse = `uname -a`;
    ($thisOS,$thisMachine,$otherStuff)=split(' ',$unameResponse);
    
    if($thisMachine ne "hal9000") 
    {
	print "Not running on hal9000, entering simulation mode.\n";
	$simulateMode=1;
    } 

    $mypid = $$;
    $myname = ${0};
    if(not $simulateMode) 
    {
	command("radecoff -r 0 -d 0");
	command("project -r -i $mypid -f $myname");
	printPID();
    } 
    else 
    {
	print "Script: $myname.\n";
    }

    $ants = '';
    #this bit isn't needed
    if($antList)
    {
	$space = `parseAntennaList $antList`;
	chomp($space);
	#print "The parsed list is $space\n";
	@tsys = sort(split(/ /, $space));
	$list = `getAntList`;
	chomp($list);
	@project = split / /, $list;
	foreach $ant (@project)
	{
	    $onlist = 0;
	    foreach $tant (@tsys)
	    {
		if($tant == $ant)
		{
		    $onlist = 1;
		}
	    }
	    if($onlist)
	    {
		$ants = $ants . $ant . " ";
	    }
	}
	chop($ants);
	$ants =~ s/ /,/g;
	#print "antList is $antList\n";
	#print "ants is $ants\n";
    }


#here's the new bit moved here, from sma_add.pl, so it can be executed in the correct order.
    if($opt_figure)
    {
	#this part gets the loop data from later in the script.
	$d = 0;
	@loop = ();
	@allsources = ();
	$check = 0;
	$longline = "";
	open(FILE, "$0") or die "file couldn't be opened\n";
	#print "The file that was just openned is $0\n";
	while(<FILE>)
	{ 
	    $line = $_;
	    #if($line !~ /^\#/ && $line !~ /^\s/)
	    if($line !~ /^\#/)
	    {
		#print "The line is $line";
		if($line =~ /restart/)
		{
		    $d = 1;
		    #print "d is $d\n";
		}
		if($d == 1 && $line =~ /\}/)
		{
		    $d = 0;
		}
		if($line =~ /$\;/)
		{
		    #print  "The line $line, ended with a semicolon.\n";
		    if($longline ne "")
		    {
			$longline = $longline . $line;
			$line = $longline;
			#print "The multi-line loop is $longline.\n";
		    }
		    if($line =~ /ObsLoop/ || $line =~ /DoFlux/ || $line =~ /DoPass/)
		    {
			push @loop, $line;
			#print "Pushing the above line into the array loop.\n";
		    }
		    if($line =~ /checkANT/)
		    {
			$check = 1;
		    }
		    if($d == 1 && ($line =~ /ObsLoop/ || $line =~ /DoFlux/ || $line =~ /DoPass/))
		    {
			#print "This line is in the restart: $line";
			$garrus++;
		    }
		    unless($check)
		    {
			if($line =~ /^\$targ/)
			{
			    @frell = split /\"/, $line;
			    @dren = split / /, $frell[1];
			    push @allsources, $dren[0];
			}
			elsif($line =~ /^\$cal/)
			{
			    @frell = split /\"/, $line;
			    push @allsources, $frell[1];
			}
			elsif($line =~ /^\$flux/)
			{
			    @frell = split /\"/, $line;
			    push @allsources, $frell[1];
			}
			elsif($line =~ /^\$bpass/)
			{
			    @frell = split /\"/, $line;
			    push @allsources, $frell[1];
			}
		    }
		    $longline = "";
		}
		elsif($line !~ /\}/ && $line !~ /restart/)
		{
		    $line =~ s/\s//g;
		    #print "The line is now:$line.\n";
		    $longline .= $line;
		    #print "long line is currently $longline\n";
		}
	    }
        }
	close(FLIE);
	#exit;
	#this part reads the mir data file.
        unless($simulateMode)
	{
	    $thename = "";
	    $machine = `shmValue -m hal9000 DSM_HAL_HAL_STORAGE_SERVER_C40`;
	    chomp ($machine);
	    if($machine !~ /\w/)
	    {
		$machine = 'hcn';
	    }
	    $datafile = `shmValue -m $machine DSM_AS_FILE_NAME_C80`; 
	    print "The datafile is $datafile\n";
	    @files = `ls $datafile`;
	    foreach $file (@files)
	    {
		if($file =~ /plot_me/)
		{
		    $thename = $file;
		}
	    }
	    chomp($datafile);
	    $datafile =~ s/\s+$//;
	    chomp($thename);
	    $datafile = $datafile . $thename;
	    #print "The datafile should be: $datafile.\n";
	    open(LOG, "<$datafile") or die "the input file couldn't be opened\n"; #for hal
	}
	else
	{
	    open(LOG, "<plot_me_5_rx0") or die "the input file couldn't be opened.\n"; #for testing
	}	
	
	@list = ();
	@times = ();
	#print "Reading the datafile.\n";
	while(<LOG>)
	{
	    $line = $_;
	    @stuff = split(" ", $line);
	    if($stuff[8] == 1)
	    {
		push @list, $stuff[0];
		push @times, $stuff[1];
	    }
	    #print "reading data for $stuff[0].\n";
	}
	#print "The last line is: $line";
	close(LOG);
	#print "Finished with the datafile.\n";
	
#open(DATA, ">datafile.txt");
	
	$lastitem = "";
	$sourcecount = -1;
	@sourcelist = ();
	@sourcescans = ();
	@sourcetime = ();
	$counter = 0;
	#print "Setting up the arrays with the source data.\n";
	foreach $item (@list)
	{
	    if($lastitem ne $item)
	    {
		#print DATA "$item\n";
		$sourcecount++;
		$sourcelist[$sourcecount] = $item;
		$sourcescans[$sourcecount] = 1;
		$sourcetime[$sourcecount] = $times[$counter];
	    }
	    else
	    {
		$sourcescans[$sourcecount]++;
	    }
	    $lastitem = $item;
	    #print "The item is: $item\n";
	    $counter++;
	}
#close(DATA);
	#exit;
	for($c = 0; $c <= $#sourcelist; $c++)
	{
	    #print "$sourcelist[$c], $sourcescans[$c]\n";
	    if($sourcelist[$c] =~ /\+/)
	    {
		@plus = split('\+',$sourcelist[$c]);
		$sourcelist[$c] = $plus[0] . '\+' . $plus[1];
	    }
	}
	#print "Printing the contents of the array loop:\n";
	#foreach $item (@loop)
	#{
	#    print $item;
	#}

#this contains the completion status for each loop in order. 0 is not started, at the start of a loop,
#or the loop was skipped, 1 is complete, and 2 is partially finished.
	@theword = ();

	$partialscans = 0; #this has the number of scans that need to be observe for flux or bp cals
	@partialobsloop = (); #this has the rest of the observing loop.
	@callist = (); #this has the flux and bandpass cals in the script.
	@obscheck = (); 
	$a = 0;

	foreach $line (@loop)
	{
	    #print "checking $line\n";
	    if($line =~ /DoFlux/)
	    {
		@frell = split /\(/, $line;
		@dren = split /\)/, $frell[1];
		@targetlist = split /,/, $dren[0];
		push @callist, $targetlist[0];
	    }
	    elsif($line =~ /DoPass/)
	    {
		@frell = split /\(/, $line;
		@dren = split /\)/, $frell[1];
		@targetlist = split /,/, $dren[0];
		push @callist, $targetlist[0];
	    }
	    elsif($line =~ /ObsLoop/)
	    {
		@frell = split /\(/, $line;
		@dren = split /\)/, $frell[1];
		@targetlist = split /,[a-z]|,\"/, $dren[0];
		#print "The split target list is:\n";
		#foreach $item (@targetlist)
		#{
		#    print "$item\n";
		#}
		$d = 0;
		foreach $target (@targetlist)
		{
		    if($target =~ /^al/)
		    {
			$target = "c" . $target;
		    }
		    elsif($target =~ /^arg/)
		    {
			$target = "t" . $target;
		    }
		    elsif($target =~ /^rans/)
		    {
			$target = "t" . $target;
		    }
		    elsif($target =~ /mtarg/)
		    {
			$target =~ s/\"+//;
			@frell = split /\[/, $target;
			@dren = split /,/, $frell[1];
			chop $dren[1];
			$frell[0] =~ s/m+//;
			@s = split / /, ${$frell[0]};
			$target =~ s/\[+/b/;
			$target =~ s/,+/d/;
			$target =~ s/-+/n/;
			$target =~ s/-+/n/;
			$target =~ s/\]+//;
			#${target} = $s[0] . "_" . $dren[0] . "_" . $dren[1];
			$temp = $s[0] . "_" . $dren[0] . "_" . $dren[1];
			#print "Target is $target\n";
			#print "Frell is @frell\n";
			#print "Dren is @dren\n";
			#print "The variable name should be $target.\n";
			#print "The target name should be $temp.\n";
			#use strict 'refs';
			$$target = $temp;
			#print "The variable should be saved here $$target.\n";
			push @allsources, $temp;
		    }
		    $target =~ s/\"+//;
		    $target =~ s/,+//;
		    $targetlist[$d] = $target;
		    $d++;
		}
		unshift @targetlist, "&ObsLoop(";
		$holder = "";
		foreach $object (@targetlist)
		{
		    $holder = $holder . "$object ";
		}
		$loop[$a] = $holder;
		#print "The new target list is:\n";
		#foreach $item (@targetlist)
		#{
		#    print "$item\n";
		#}
	    }
	    $a++;
	}
	#print "The loop is:\n";
	#foreach $item (@loop)
	#{
	#    print $item;
	#}
	#print "Now referencing the new variable $mtarg0rn125d0.\n";
	#exit;
#remove all soures taken manually.
	#print "removing sources taken manually.\n";
	for($c = 0; $c <= $#sourcelist; $c++)
	{
	    $tali[$c] = 0;
	}
	foreach $target (@allsources)
	{
	    for($c = 0; $c <= $#sourcelist; $c++)
	    {
		#print "$sourcelist[$c] vs $target\n";
		if($target =~ /$sourcelist[$c]/i)
		{
		    $tali[$c] = 1;
		    #print "check\n";
		}
	    }
	}
	#exit;
	$d = 0;
	#print "The checklist is:\n";
	#for($c = 0; $c <= $#tali; $c++)
	#{
	#    print "$tali[$c] $sourcelist[$c]\n";
	#}
	while($d <= $#sourcelist)
	{
	    #print "$sourcelist[$d] has a $tali[$d]\n";
	    if($tali[$d] == 0)
	    {
		splice @sourcelist, $d, 1;
		splice @sourcescans, $d, 1;
		splice @sourcetime, $d, 1;
		splice @tali, $d, 1;
	    }
	    else
	    {
		$d++;
	    }
	}
	$e = 0;
	while($e < $#sourcelist)
	{
	    #print "Checking $sourcelist[$e] against $sourcelist[($e+1)].\n";
	    if($sourcelist[$e] eq $sourcelist[($e+1)])
	    {
		#print "$sourcelist[$e] matched the next source, removing it.\n";
		$sourcescans[($e+1)] += $sourcescans[$e];
		splice @sourcelist, $e, 1;
		splice @sourcescans, $e, 1;
		splice @sourcetime, $e, 1;
	    }
	    else
	    {
		$e++;
	    }
	}

	#print "Here's the cleaned list of observed sources:\n";
	#for($c = 0; $c <= $#sourcelist; $c++)
	#{
	#    print "$sourcelist[$c], $sourcescans[$c], $sourcetime[$c]\n";
	#}
	#exit;
	#print "Here's the cal list just for the hell of it:\n";
	#foreach $cal (@callist)
	#{
	#    print "${$cal}\n";
	#}
	#print "and the source list:\n";
	#foreach $target (@allsources)
	#{
	#    print "$target\n";
	#}

	$bo = 0;
	#foreach $line (@loop)
	#print "Starting the smart part of the loop.\n";
	while(($#sourcelist >= 0)  && ($bo <= $#loop))
	{
	    #print "The sourcelist has $#sourcelist sources. Bo is set to $bo. The number of loops is $#loop.\n";
	    $line = $loop[$bo];
	    #print "The line in question is $line.\n";
	    if($line =~ /DoFlux/)
	    {
		@frell = split /\(/, $line;
		@dren = split /\)/, $frell[1];
		@targetlist = split /,/, $dren[0];
		#print "The target is ${$targetlist[0]}, and the actual target is $sourcelist[0]\n";
#		for($c = 1; $c <= 5; $c++)
#		{
#		    if($sourcelist[$c] !~ /${$targetlist[0]}/i)
#		    {
#			print "Found a match with a c of $c.\n";
#			for($d = 0; $d < $c; $d++)
#			{
#			    shift @sourcelist;
#			    shift @sourcescans;
#			    print "Removed one source.\n";
#			}
#		    }
#		}
		#if($sourcelist[0] =~ /${$targetlist[0]}/i)
		if(${$targetlist[0]} =~ /$sourcelist[0]/i)
		{
		    #print "The number of scans that have been taken is $sourcescans[0]\n";
		    #print "The total number of scans are ${$targetlist[1]}\n";
		    $total = ${$targetlist[1]} - $sourcescans[0];
		    if($total <= 0)
		    {
			push @theword, 1;
			shift @sourcelist;
			shift @sourcescans;
			shift @sourcetime;
			#print "DoFlux set the word to 1.\n";
		    }
		    else
		    {
			push @theword, 2;
			$partialscans = $total;
			shift @sourcelist;
			shift @sourcescans;
			shift @sourcetime;
			#print "DoFlux set the word to 2.\n";
		    }
		}
		else
		{
		    push @theword, 0;
		    #print "DoFlux set the word to 0.\n";
		}
		#print "The total number of scans on $targetlist[0] was $total\n";
		#print "The target list is @targetlist\n";
	    }
	    elsif($line =~ /DoPass/)
	    {
		@frell = split /\(/, $line;
		@dren = split /\)/, $frell[1];
		@targetlist = split /,/, $dren[0];
#		for($c = 1; $c <= 5; $c++)
#		{
#		    if($sourcelist[$c] !~ /${$targetlist[0]}/i)
#		    {
#			print "Found a match with a c of $c.\n";
#			for($d = 0; $d < $c; $d++)
#			{
#			    shift @sourcelist;
#			    shift @sourcescans;
#			    print "Removed one source.\n";
#			}
#		    }
#		}
		if(${$targetlist[0]} =~ /$sourcelist[0]/i)
		#if($sourcelist[$c] =~ /${$targetlist[0]}/i)
		{
		    $total = ${$targetlist[1]} - $sourcescans[0];
		    if($total <= 0)
		    {
			push @theword, 1;
			shift @sourcelist;
			shift @sourcescans;
			shift @sourcetime;
			#print "DoPass set the word to 1.\n";
		    }
		    else
		    {
			push @theword, 2;
			$partialscans = $total;
			shift @sourcelist;
			shift @sourcescans;
			shift @sourcetime;
			#print "DoPass set the word to 2.\n";
		    }
		}
		else
		{
		    push @theword, 0;
		    #print "DoPass set the word to 0.\n";
		}
		#print "The total number of scans on $targetlist[0] was $total\n";
		#print "The target list is @targetlist\n";
		#print "The sourcelist after removing the bandpass cal is:\n";
		#foreach $item (@sourcelist)
		#{
		#    print "$item\n";
		#}
	    }
	    elsif($line =~ /ObsLoop/)
	    {
		$sourcedelete = 0;
		$same = 0;
		$endloop = 0;
		#print "Started looking at obsLoop\n";
		#@frell = split /\(/, $line;
		#@dren = split /\)/, $frell[1];
		#@targetlist = split /,/, $dren[0];
		@targetlist = split / /, $line;
		shift @targetlist;
		#print "The target list is:\n";
		#foreach $targ (@targetlist)
		#{
		#    print "$targ\n";
		#}
		#if("M82-5 -r 09:55:47.88 -d +69:40:46.9 -e 2000 -v 203" =~ /M82-5/i)
		#{
		#    print "The check worked.\n";
		#}
		#print "The target list is: @targetlist\n";
		#while(($#sourcelist > 0) || ($endloop == 0))
		while($endloop == 0)
		{
		    print "in the endloop while\n";
		    $endcheck = 0;
		    foreach $cal (@callist)
		    {
			#print "Checking $cal against $sourcelist[$#sourcelist].\n";
			if(${$cal} =~ /$sourcelist[$#sourcelist]/i)
			{
			    $endcheck = 1;
			    $endloop = 1;
			}
		    }
		    #if($sourcelist[0] !~ ${$targetlist[0]})
		    #{
			#shift @sourcelist;
			#shift @sourcescans;
		    #}
#		    foreach $target (@targetlist)
#		    {
			#print "checking ${$target} against $sourcelist[$#sourcelist]\n";
			#if("${$target}" =~ /$sourcelist[$#sourcelist]/i)
#			if($sourcelist[$#sourcelist] =~ /${$target}/i)
#			{
			    #$lasttarget = ${$target};
#			    $endloop = 1;
			    #print "ding ding\n";
#			}
#		    }
#		    print "endloop is $endloop\n";
#		    if($endloop == 0)
#		    {
#			$endloop = 1;
#			foreach $cal (@callist)
#			{
#			    if($sourcelist[$#sourcelist] =~ /${$cal}/i)
#			    {
#				$endloop = 0;
#			    }
#			}
#			if($endloop == 1)
#			{
			    #this part is probably fine, but it might need to be changed.
#			    push @theword, 0;
			    #print "ObsLoop set the word to 0. #1\n";
#			}
#			else
#			{
#			    push @theword, 1;
			    #print "ObsLoop set the word to 1. #2\n";
#			}
#			$endloop = 1;
#			$sourcedelete = 1;
#		    }
#		    else
		    unless($endcheck == 1)
		    {
			@templist = ();
			@checklist = ();
			$counter = 0;
			#for($w = 0; $w <= $#sourcelist; $w++)
			#{
			#    $e = 0;
			#    $h = 0;
			#    while($h == 0)
			#    {
			#	if(${$targetlist[$e]} =~ /$sourcelist[$w]/i)
			#	#if($sourcelist[$w] =~ /$targetlist[$e]/i)
			#	{
			#	    push @templist, $sourcelist[$w];
			#	    $h = 1;
			#	}
			#	if($e > $#targetlist)
			#	{
			#	    $h = 1;
			#	}
			#	$e++;
			#    }
			#}
			#print "The target list is: @targetlist\n";
			#print "The unedited list is: @templist\n";
			$v = 0;
			$w = 0;
			$x = 0;
			@checklist2 = ();
			@targetlist2 = @targetlist;
			#print "The second target list is: @targetlist2\n";
			@mastertargetlist = @targetlist;
			for($r = 0; $r <= $#sourcelist; $r++)
			{
			    #print "Checking $sourcelist[$r].\n";
			    if(${$targetlist[$v]} =~ /$sourcelist[$r]/i)
			    {
				#print "${$targetlist[$v]} did match $sourcelist[$r]\n";
				$v++;
				push @checklist, 1;
				$w = 0;
                                #print "The checklist has $#checklist elements.\n";
#				if($targetlist[($v-1)] =~ /cal/i)
#				{
#				    #print "passed 1.\n";
#				    if($targetlist[$v] =~ /cal/i)
#				    {
#					#print "passed 2.\n";
#					if(${$targetlist[$v]} !~ /$sourcelist[($r+1)]/i)
#					{
#					    #print "passed 3.\n";
#					    $v++;
#					    push @checklist, 1;
#					    $w = 0;
#					    #print "The checklist has $#checklist elements.\n";
#					}
#				    }
#				}
			    }
			    elsif($targetlist[$v] =~ /cal/i)
			    {
				#print "${$targetlist[$v]} replaced $sourcelist[$r]\n";
				$v++;
				push @checklist, 1;
				$r--; #dangerous?
				$w = 0;
				#print "The checklist has $#checklist elements.\n";
			    }
			    else
			    {
				#print "${$targetlist[$v]} didn't match $sourcelist[$r]\n";
				#print "${$targetlist2[$w]}\n";
				if($x == 0)
				{
				    #exit;
				    while(${$targetlist2[$x]} !~ /$sourcelist[$r]/i && $x <= $#targetlist2)
				    {
					#print "${$targetlist2[$x]} didn't match $sourcelist[$r]\n";
					$x++;
				    }
				    #print "x is $x\n";
				    #print "The checklist has $#checklist values.\n";
				    $w = $x;
				    $y = $x - $#checklist;
				    #print "The number of sources skipped is $y\n";
				    for($c = 0; $c < ($x - $#checklist); $c++)
				    {
					push @checklist2, 1;
				    }
				    #print "The checklist now has $#checklist2 values.\n";
				}
				if(${$targetlist2[$w]} =~ /$sourcelist[$r]/i)
				{
				    #print "it worked!\n";
				    $w++;
				    push @checklist2, 1;
				    #print "The checklist has $#checklist2 values.\n";
				}
				elsif($targetlist2[$v] =~ /cal/i)
				{
				    $w++;
				    push @checklist2, 1;
				    #print "The checklist has $#checklist2 values.\n";
				}
			    }
			    if($v > $#targetlist)
			    {
				$v = 0;
			    }
			    if(($w - $x) == 2)
			    {
				$v = $w;
				push @checklist, @checklist2;
				#print "The checklist now has $#checklist elements.\n";
				#@targetlist = @targetlist2;
				$w = 0;
				$x = 0;
				@checklist2 = ();
				@targetlist2 = @mastertargetlist;
			    }
			    if($w > $#targetlist2)
			    {
				$w = 0;
			    }
			}
			#print "The checklist has $#checklist elements.\n";
			if($opt_pointing)
			{
			    pop @checklist;
			    #print "Repeating the last source.\n";
			}
			@templist = @targetlist;
			for($t = 0; $t <= $#checklist; $t++)
			{
			    $wrex = "n$templist[0]";
			    #print "$n{$wrex}\n";
			    unless(($#checklist < $t) && (${$wrex} >= ($sourcescans[$t] + 1)))
			    {
				#print "Removing $templist[0]\n";
				shift @templist;
			    }
			    if(@templist == ())
			    {
				@templist = @targetlist;
			    }
			}
			print "The partial loop is: @templist\n";
			#print "The last source observed was $sourcelist[$#sourcelist]\n";
			if(@templist == ())
			{
			    push @theword, 0;
			    $endloop = 1;
			    $sourcedelete = 1;
			    #print "ObsLoop set the word to 0. #3\n";
			}
			else
			{
			    push @theword, 2;
			    $partialobsloop = $#mastertargetlist - $#templist;
			    #print "The total number of sources is $#mastertargetlist, the script was $#templist through the loop, and the total remaining sources are $partialobsloop.\n";
			    $endloop = 1;
			    $sourcedelete = 1;
			    #print "ObsLoop set the word to 2. #4\n";
			    #print "The partial loop is: @partialobsloop\n";
			}
		    }
		    else
		    {
			push @theword, 1;
			$sourcedelete = 1;
			#print "ObsLoop set the word to 1. #2\n";
		    }
		}
		if($sourcedelete)
		{
		    #print "Before deleting the source list is: @sourcelist\n";
		    #print "Deleting sources.\n";
		    foreach $target (@targetlist)
		    {
			foreach $cal (@callist)
			{
			    if(${$target} =~ ${$cal})
			    {
				$same = 1;
			    }
			}
		    }
		    #print "same is $same.\n";
		    if($same)
		    {
			$q = 0;
			while($q <= $#sourcelist)
			{
			    foreach $target (@targetlist)
			    {
				if(${$target} =~ /$sourcelist[$q]/i)
				{
				    foreach $cal (@callist)
				    {
					if(${$target} =~ /${$cal}/i)
					{
					    $grunt = "n$target";
					    if(${$grunt} >= ($sourcescans[$q] + 2))
					    {
						shift @sourcelist;
						shift @sourcescans;
						shift @sourcetime;
					    }
					    else
					    {
						$q++;
					    }
					}
				    }
				}
				else
				{
				    $q++;
				}
			    }
			}	
		    }
		    else
		    {
			#print "Checking sources to delete.\n";
			$miranda = 0;
			$ashely = 0;
			$q = 0;
			while($q <= $#sourcelist)
			{
			    $ash = 0;
			    foreach $target (@targetlist)
			    {
				#print "The target is ${$target} and the observed source is $sourcelist[$q].\n";
				#if($sourcelist[$q] =~ ${$target})
				if(${$target} =~ /$sourcelist[$q]/i)
				{
				    $ash = 1;
				}
			    }
			    if($ash)
			    {
				shift @sourcelist;
				shift @sourcescans;
				shift @sourcetime;
				$ashely++;
			    }
			    else
			    {
				$miranda++;
				$q++;
			    }
			    if($ashely > 0 && $miranda > 0 && $ash == 0)
			    {
				for($c = 0; $c < $miranda; $c++)
				{
				    shift @sourcelist;
				    shift @sourcescans;
				    shift @sourcetime;
				}
				$ashely = 0;
				$miranda = 0;
			    }
			    #print "Ash is $ash. \n";
			    #print "The last source in sourcelist is numbered: $#sourcelist.\n";
			    #print "q is currently at $q.\n";
			}
		    }
		}
		#print "The remaining sources are @sourcelist\n";
	    }
	    $bo += 1;
	    #print "The counter is at $bo of $#loop.\n";
	}
    }
    #print "The final source list is:\n";
    #for($c = 0; $c <= $#sourcelist; $c++)
    #{
#	print "$sourcelist[$c], $sourcescans[$c]\n";
    #}
    #print "The final word is @theword\n";
    $n7 = 0;
    for($s = $#theword; $s >= 0; $s--)
    {
	#print "N7 is $n7.\n";
	if($theword[$s] == 1)
	{
	    $n7 = 1;
	}
	elsif($theword[$s] == 2 && $n7 != 1)
	{
	    $n7 = 2;
	}
	if($n7 == 1)
	{
	    $theword[$s] = 1;
	}
	elsif($n7 == 2)
	{
	    $theword[$s] = 2;
	    $n7 = 1;
	}
    }
    #print "The corrected word is @theword\n";
    #print "Garrus is at: $garrus\n";
#    for($c = 0; $c <= $#partialobsloop; $c++)
#    {
#	if($partialobsloop[$c] =~ /mtarg/)
#	{
#	    $partialobsloop[$c] =~ s/b+/\[/;
#	    $partialobsloop[$c] =~ s/d+/,/;
#	    $partialobsloop[$c] =~ s/n+/-/;
#	    $partialobsloop[$c] =~ s/n+/-/;
#	    $partialobsloop[$c] = $partialobsloop[$c] . "\]";
#	}
#    }
    #exit;
 }

# --- interrupt handler for CTRL-C ---
sub finish 
{
    command("radecoff -r 0 -d 0");
    sleep(1);
    print "Please remember to stow the antennas safely if you are leaving.\n";
    exit(1);
}

# --- print PID ---
# usage: printPID();
sub printPID 
{
    if(not $simulateMode) 
    {
	print "The process ID of this $0 script is $mypid\n";
    }
}


# --- check antennas ---
# usage: checkANT();
# This subroutine checks active antennas and stores them
# as an array @sma.
sub checkANT 
{
    initialize();
    my ($i,$exist);
    if(not $simulateMode) {print "Checking antenna status ... \n";}
    for ($i = 1; $i <=8; $i++) 
    {
	if($simulateMode==0) 
	{ 
	    $exist = `value -a $i -v antenna_status`;
	    chomp($exist);
	} 
	else 
	{
	    $exist=1;
	}
	if ($exist) 
	{
	    if(not $simulateMode) {print "Found antenna ",$i," in the array!\n";}
	    @sma = (@sma,$i);
	}
    }

    $nants = scalar(@sma);
    print "nants = $nants\n";
    if($nants eq "8") 
    {
	$ps="polarPattern -p 7 -w -c 1";
	$pc="polarPattern -p 8 -w -c 1";
    }
    
    if($nants eq "7") 
    {
	$ps="polarPattern -p 1 -w -c 1";
	$pc="polarPattern -p 2 -w -c 1";
    }
    
    if($nants eq "6")
    {
	$ps="polarPattern -p 3 -w -c 1";
	$pc="polarPattern -p 4 -w -c 1";
    }

    if($nants eq "5") 
    {
	$ps="polarPattern -p 5 -w -c 1";
	$pc="polarPattern -p 6 -w -c 1";
    }


    if(not $simulateMode) 
    { 
	print "Antennas @sma are going to be used.\n";
	scriptcopy();
    }
}

sub scriptcopy
{
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
  $directoryName=`readDSM -v DSM_AS_FILE_NAME_C80 -m hcn -l`;
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
#      $a=checkEl('your_source);
# will give you the eleveation in $a and also prints it on screen.
# Also,
#      $a=checkEl('your_source',1);
# will give $a the elevation but does not print the value.
# The second argument, 'silent flag', is optional.
sub checkEl 
{
    my ($sourcename)=$_[0];
    my ($silent)    =(defined($_[1]) and $_[1]);
#    my ($sourceCoordinates,$sourceAz,$sourceEl,$sunDistance);

    if((not $simulateMode) or ($thisMachine eq "hal9000")) 
    {
	$sourceCoordinates=`lookup -s $sourcename`;
   	chomp($sourceCoordinates);
   	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
	if ($sourceAz =~ /Source/) 
	{
	    print "##########################################\n";
	    print "######## WARNING WARNING WARNING #########\n";
	    print "##### source $sourcename not found. ######\n";
	    print "##########################################\n";
	    die   " quiting from the script \n";
	}
    } 
    else 
    {
	$lookupTime="$mn $d $year $hour:$min";
	# check sourcename for ra dec input - if so, parse it
	if($sourcename =~ /-r/) 
	{
#	print "got ra/dec arguments...parsing them...\n";
	    if(!($sourcename =~ /-d/)) { die "both ra and dec are required\n";}
	    @sourcenameArgs=split(' ',$sourcename);
	    $iarg=0;
	    $sourceNameArgindex=0;
	    foreach $arguments (@sourcenameArgs) 
	    {
		if($arguments eq '-s') {$sourceNameArgindex=$iarg+1;}
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
	    if($decd<0.) {$decsign=-1;$decd=-$decd}
	    $givenDEC=$decd+$decm/60.+$decs/3600.;
	    if($decsign==-1){$givenDEC=-$givenDEC;}
	    $parsedSourcename="$sourceNameString -r $givenRA -d $givenDEC -e $givenEpoch ";
	    $newSourceFlag=1;
	} 
	else 
	{
	    $newSourceFlag=0;
	}
	
	if($newSourceFlag==1) 
	{
	    if($thisMachine eq 'ulua')
	    {
		$sourceCoordinates=`\./lookup -s $parsedSourcename -t "$lookupTime"`;
	    }
	    else
	    {
		$sourceCoordinates=`lookup -s $parsedSourcename -t "$lookupTime"`;
	    }
	} 
	else 
	{
	    if($thisMachine eq 'ulua')
	    {
		print "lookup time: $lookupTime\n";
		$sourceCoordinates=`\./lookup -s $sourcename -t "$lookupTime"`;
	    }
	    else
	    {
		print "lookup time: $lookupTime\n";
		$sourceCoordinates=`lookup -s $sourcename -t "$lookupTime"`;
	    }
	}
	#print "The parsed source name is $parsedSourcename\n";
   	chomp($sourceCoordinates);
   	($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
	if ($sourceAz =~ /Source/) 
	{
	    print "##########################################\n";
	    print "######## WARNING WARNING WARNING #########\n";
	    print "##### source $sourcename not found. ######\n";
	    print "##########################################\n";
	    die   " quiting from the script \n";
	}
   	if (not $silent) 
	{
	    if($newSourceFlag==1) 
	    {
		printf("%s is at %4.4f degrees elevation\n",$sourceNameString,$sourceEl);
	    } 
	    else 
	    {
		printf("%s is at %4.4f degrees elevation\n",$sourcename,$sourceEl);
	    }
   	}
    }   
    return $sourceEl;
}
sub mainLoop () 
{
    @inputArgs=@_;
    $narg=$#inputArgs+1;
    if(($narg%2)!=0) {die "Incorrect number of arguments.\n";}
    for ($i=1;$i<$narg;$i=$i+2) 
    {
        $numbercheck=$inputArgs[$i];
	if(!($numbercheck !~ /\D/)) 
	{
	    $j=$i+1;die "Argument $j is Not a number\n";
	}
    }
    
    for ($i=0;$i<$narg;$i=$i+2) 
    {
	#LST(); 
	$targel=checkEl($inputArgs[$i]);
        $current_source = $inputArgs[$i];
	
	if($targel>$MINEL_CHECK) 
	{
	    command("observe -s $inputArgs[$i]");
	    command("tsys");
	    command("integrate -s $inputArgs[$i+1] -w");
	}
	else 
	{
	    print "Source $inputArgs[$i] is too low: $targel deg.\n";
	    print "Skipping it.\n";
        }
	
    }
    
}

sub mainLoopMosaic () 
{
    @inputArgs=@_;
    $narg=$#inputArgs+1;
    if(($narg%3)!=0) {die "Incorrect number  of arguments.\n";}
    for ($i=1;$i<$narg;$i=$i+3) 
    {
        $numbercheck=$inputArgs[$i];
	if(!($numbercheck !~ /\D/)) 
	{
	    $j=$i+1;die "Argument $j is Not a number\n";
	}
    }
    
    for ($i=0;$i<$narg;$i=$i+3) 
    {
	#LST(); 
	$targel=checkEl($inputArgs[$i]);
        $current_source = $inputArgs[$i];
	
	if($targel>$MINEL_CHECK) 
	{
	    command("observe -s $inputArgs[$i]");
	    ($raoff,$decoff)=split(',',$inputArgs[$i+2]);
	    command("radecoff -r $raoff -d $decoff");
	    command("tsys");
	    command("integrate -s $inputArgs[$i+1] -w");
	    command("radecoff -r 0 -d 0");
	} 
	else 
	{
	    print "Source $inputArgs[$i] is too low: $targel deg.\n";
	    print "Skipping it.\n";
        }
	
    }
}


# --- performs a shell task with delay and printing. ---
sub command 
{
    print "@_\n";       				# print the command
    my ($givenCommand)=$_[0];
    if($antenna && $givenCommand =~ /tsys/)
    {
	$givenCommand = "tsys -a $antenna";
    }
    if($simulateMode==0) 
    {
	system("$givenCommand");
    }	# execute the command
                          	# sleep 1 sec
    # if simulation, then increment time as given by integrate command.
    if($simulateMode==1) 
    {
	$prevUnixTime=$unixTime;
	if($givenCommand=~/integrate/) 
	{
	    ($intcommand,$tors,$timeorscans)=split(' ',$givenCommand);
	    if($tors =~/t/) 
	    {
		$integrationTime=$timeorscans;
	    }
	    if($tors =~/s/) 
	    {
		$numberOfScans=$timeorscans;
		$totalIntegrationTime=$integrationTime*$numberOfScans;
	    }
	    $unixTime=$unixTime+$totalIntegrationTime;
	}
	if($givenCommand=~/tsys/) 
	{
	    $unixTime=$unixTime+$tsysDelay;
	}
	if($givenCommand=~/observe/) 
	{
	    $unixTime=$unixTime+$observeDelay;
	}
	if($givenCommand=~/point/) 
	{
	    @pointArgs=split(' ',$givenCommand);
	    for($iarg=0;$iarg<=$#pointArgs;$iarg++) 
	    {
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
    } 
    else {sleep 1;} 
}

# --- get LST in hours ---
# usage: LST(); or LST(1);
# The return value of this subroutine is LST in hours.
# The time is taken from the Reflective Memory. However, when in $simulatemode,
# then the return value is simulated LST in hours.
# It can print the current LST if used as LST(1);
sub LST 
{
    my ($LST);
    if($simulateMode) {$LST=simlst();}
    else {$LST= `value -a $sma[0] -v lst_hours`;}
    chomp($LST);
    #if (not $simulateMode and $_[0])  {printf("LST [hr]= %6.2f\n",$LST);}
    printf("LST [hr]= %6.2f\n",$LST);
    
    return $LST;
}

# --- simulate LST ------
sub simlst 
{
#$longitude = -4.76625185186; # hours (71d29'37.6s W) Haystack
    $longitude = -10.365168199815; # hours (-155.477522997222 W, pad-1) MaunaKea
    
    if($givenUTC eq "") 
    {
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

    if($givenUTC) 
    {
	if($firstTime==1) 
	{ 
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
    
    $epsilon0 = 21.448-46.8150 * $T - 0.00059 * $T * $T+ 0.001813 * $T * $T * $T;
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


