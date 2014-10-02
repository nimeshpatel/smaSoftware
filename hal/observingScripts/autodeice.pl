#!/usr/bin/perl


#$rawdeice = `shmValue -a 5 RM_ANTENNA_SURFACE_MINMAX_TEMPS_V24_S`;
#values 0-5 are the min temps for the dish surface zones
#it updates every 2 minutes.
#the values are 100 times larger then they should be

#SMA_METEOROLOGY_X:TEMP_F
#SMA_METEOROLOGY_X:HUMIDITY_F

use Getopt::Long;
$Getopt::Long::autoabbrev=1;

GetOptions('help');

if($opt_help)
{
    &usage; die "\n";
}

@average = (); #conatins a list of the last 10 avereges
#@low = ();     #contains a list of the last 10 lowest values
$count = 0;
$sum = 0;
$test = 0;
$limit = 0;
$stop = 0;
$loop = 0;
print "Do you wish to make the lower limit on deice 1?\n";
$input = <STDIN>;
if($input =~ /yes/i || $input =~ /y/i)
{
    $limit = 1;
}
while($test == 0)
{
    print "What level is deice currently using (0 - 5)?\n";
    print "Enter 'search' to have this program check the log.\n";
    print "Enter 'set #' to set deice to # (0-3).\n";
    $in = <STDIN>;
    chomp($in);
    if($in lt 6 && $in gt -1)
    {
	print "Thank you.  autodeice is starting to monitor deice.  Use ctrl + c to quit.\n";
	$test = 1;
	$level = $in;
    }
    elsif($in =~ /search/i)
    {
	print "Searching...\n";
	currentLevel();
	if($foundvalue && $dlevel != -1)
	{
	    print "The current deice level is $dlevel.\n";
	    $level = $dlevel;
	    $test = 1;
	}
	else
	{
	    print "The search couldn't find the current deice level, please enter the value.\n";
	}
    }
    elsif($in =~ /set/i)
    {
	$check = 0;
	@spin = split(" ", $in);
	if($spin[1] lt 4 && $spin[1] gt -1)
	{
	    print "Setting deice to $spin[1].  autodeice is starting to monitor deice.  Use ctrl + c to quit.\n";
	    `deice $spin[1]`;
	    $check = 1;
	    $test = 1;
	    $level = $spin[1];
	}
	while($check == 0)
	{
	    print "There was and error, please re-enter the value you would like deice set to.\n";
	    $in = <STDIN>;
	    chomp($in);
	    if($in lt 4 && $in gt -1)
	    {
		$check = 1;
		$test = 1;
		$level = $in;
		`$deice $in`;
		print "Setting deice to $in.  autodeice is starting to monitor deice.  Use ctrl + c to quit.\n";
#		open(FILE, ">>fakelog.txt");
#		print FILE "Wed Mar 18 01:30:07 2009 (rhowie): deice $level\n";
#		close FILE;
	    }
	}
    }
    else
    {
	print "That selection is invalid, please try again.\n";
    }
    if($limit && $level == 0)
    {
	print "Deice is off, turning deice on to follow the level limit of 1.\n";
#	open(FILE, ">>fakelog.txt");
#	print FILE "Wed Mar 18 01:30:07 2009 (rhowie): deice 1\n";
#	close FILE;
	`deice 1`;
	$level = 1;
    }
}
#open(FILE, ">>fakelog.txt");
#print FILE "Wed Mar 18 01:30:07 2009 (rhowie): deice $level\n";

while(1)
{
    for($n = 0; $n < 6; $n++)
    {
#	print "The current deice level is $level.\n";
	$loop++;
	$sum = 0;
#	open(LOG, "deice.txt");
#	$rawdeice = readline(LOG);
#	close LOG;
#	$rawdeice = <STDIN>;
	$rawdeice = `shmValue -a 5 RM_ANTENNA_SURFACE_MINMAX_TEMPS_V24_S`;
	$temp = `shmValue -a 1 RM_WEATHER_TEMP_F`;
	$humidity = `shmValue -a 1 RM_WEATHER_HUMIDITY_F`;
#	$temp = 6;
#	$humidity = 5;
	@deice = split(" ", $rawdeice);
	splice @deice, 6;
	@sorted = sort(@deice);
#	print "The current min values are: \n";
#	for($x = 0; $x < 6; $x++)
#	{
#	    print "$deice[$x] ";
#	}
#	print "The sorted min values are: \n";
#	for($x = 0; $x < 6; $x++)
#	{
#	    print "$sorted[$x] ";
#	}
	for($z = 0; $z < 6; $z++)
	{
	    $sum += $deice[$z];
	    $sorted[$z] = $sorted[$z] / 100;
	}
	$sum = $sum / 600;
	$sum = sprintf "%.1f", $sum;
#	print "\nThe average is: $sum\n";
	if($count < 10)
	{
	    $average[$count] = $sum;
#	    $low[$count] = $sorted[0];
	}
	else
	{
	    push @average, $sum;
	    shift @average;
#	    push @low, $sorted[0];
#	    shift @low;
	}
#	print "The loop number is $loop\n";
	if($loop > 1)
	{
	    #this may still be a problem
#	    unless(($average[($#average - 2)] == $average[($#average - 1)]) && ($average[($#average - 1)] == $average[($#average)]))
	    unless(1 != 1)	
	    {
		unless($temp gt 1.5 && $humidity lt 80)
		{
		    if($sum < .8 && ($average[$#average] < ($average[$#average - 1] * 1.1)))
		    {
			$change = 1;
			levelChange($change);
		    }
		    elsif($sorted[0] < -.2 && $sum < 1.5 && ($average[$#average] <= $average[$#average - 1]))
		    {
			$change = 1;
			levelChange($change);
		    }
#	    elsif($sum < 0 && $change == 0)
#	    {
#		$change = 1;
#		levelChange($change);
#	    }
		    #This will increase deice if the average values are dropping rapidly
		    elsif($count > 2 && $sum < 3 && ($average[($#average - 2)] < ($average[($#average - 1)] * 1.2)) && ($average[($#average - 1)] < ($average[($#average)] * 1.2)))
		    {
			$change = 1;
			levelChange($change);
		    }
		    elsif($sum > 4 && $sum < 15)
		    {
			$change = -1;
			levelChange($change);
		    }
		    #This will decrease deice if the average values are rising rapidly
		    elsif($count > 2 && $sum > 3 && (($average[($#average - 2)] * 1.2) < $average[($#average - 1)]) && (($average[($#average - 1)] * 1.2) < $average[($#average)]))
		    {
			$change = -1;
			levelChange($change);
		    }
		    elsif($sum >= 15 && $level != 0 && $limit != 1) 
		    {
			$level = 0;
			print "Temperatures are very high, deice will be turned off for a little while.  Waiting 5 minutes for next update.\n";
			`deice 0`;
			$change = 0;
		    }
		    elsif($sum >= 15 && $level != 0 && $limit == 1)
		    {
			print "Temperatures are very high, deice will be turned to 1 for a little while.  Waiting 5 minutes for next update.\n";
			`deice 1`;
			$level = 1;
			$change = 0;
		    }
		    else
		    {
			print "Deice will be left at $level.  Waiting 5 minutes for next update.\n";
			$change = 0;
		    }
		}
		else
		{
		    if($level != 0 && $limit == 0)
		    {
			print "The weather conditions have improved, so deice probably isn't necessary.  Deice has been turned off for now.  Waiting 5 minutes for next update.\n";
			$level = 0;
			`deice 0`
			}
		    elsif($level != 1 && $limit == 1)
		    {
			print "The weather conditions have improved, so deice probably isn't necessary.  However, the lower limit is one, so deice will be turned to level 1.  Waiting 5 minutes for next update.\n";
			`deice 1`;
			$level = 1;
		    }
		    else
		    {
			print "The weather conditions are better, leaving deice at level $level.  Waiting 5 minutes for the next update.\n"
			}
		}
	    }
	    else
	    {
		print "The deice values have gone stale.  autodeice won't change the deice values until they return.\n";
	    }
	}
	$count++;
#	print "Here is the list of the last 10 avereges:";
#	for($a = 0; $a < 10; $a++)
#	{
#	    print "$average[$a] ";
#	}
#	print "\nHere is a list of the last 10 lowest values:";
#	for($a = 0; $a < 10; $a++)
#	{
#	    print "$low[$a] ";
#	}
	print "\n";
	sleep(3);
	if($level > 3)
	{
	    currentLevel();
	    if($dlevel != $level) 
	    {
		print "The level wasn't changed to $level, using $currentlevel as the current level.\n";
		$level = $currentlevel;
	    }
	}
    }
    print "Pausing to check if the deice value is the same as the last one in the SMAshLog.\n";
    currentLevel();
    if($foundvalue && $dlevel == -1)
    {
	print "Extra commands were given with the deice command and the current level wasn't found.  Continuing to use $level\n";
    }
    elsif($foundvalue && $dlevel == $level)
    {
	print "The deice level $level is correct.\n";
    }
    elsif(!$foundvalue)
    {
	print "Couldn't find a deice entry in the SMAshLog, using $level.\n";
    }
    elsif($foundvalue && $dlevel != $level)
    {
	print "A different value was found in the SMAshLog, using the found value: $dlevel.\n";
	$level = $dlevel;
    }
}
#close FILE;

sub levelChange($change)
{
#    open(FILE, ">>fakelog.txt");
#    print "The current level is: $level\n";
#    print "The change required is: $change\n";
    if($stop == 0)
    {
	$currentlevel = $level;
	$level += $change;
	if($limit == 1 && $level == 0)
	{
	    $level = 1;
	    print "Deice was left at 1, since the limit is in place.  ";
	}
	elsif($level > 5)
	{
	    print "Deice is at it's maximum level and the dishes are still cold.  Keeping deice at level 5.  ";
	    $level = 5;
	}
	elsif($level > 3 && $level < 6) 
	{
	    print "Deice should be turned up to level $level, however, this program doesn't have the password.  The user will have to turn it up manually.  ";
#	print FILE "Wed Mar 18 01:30:07 2009 (rhowie): deice $level\n";	
	}
	elsif($level < 0)
	{
	    $level = 0;
	    print "Deice has been left off.  ";
	}
	elsif($level == 0)
	{
	    print "Deice has been turned off.  ";
	    `deice 0`;
	}
	else
	{
	    print "Deice has been turned to $level.  ";
	    `deice $level`;
	    if($change > 0)
	    {
		$stop = 1;
	    }
	}
    }
    else
    {
	$stop = 0;
    }
    print "Waiting 5 minutes for next update.\n";
#    close FILE;
}

sub currentLevel
{
    $path = "/global/logs/SMAshLog";
    $foundvalue = 0;
    $counter = 0;
    $dlevel = 0;
#    print "in the function\n";
    $raw = `tail -10 $path`;
    @lines = split("\n", $raw);
    while($foundvalue == 0 && $counter < 10) 
    {
#	print "in the while\n";
	@line = split(" ", $lines[($#lines - $counter)]);
	print "The 6th part is: $line[6]\n";
	print "The 7th part is: $line[7]\n";
	if($line[6] =~ /deice/)
	{
	    if($#line == 7 && $line[7] =~ \d)
	    {
		$dlevel = $line[7];
		$foundvalue = 1;
		print @line;
		print "\n";
		print "dlevel is $dlevel\n";
	    }
	    elsif($line[7] =~ /a/ && $#line == 8 && $line[8] =~ \d)
	    {
		$dlevel = $line[8];
		$foundvalue = 1;
		print @line;
		print "\n";
	    }
	    elsif($line[7] =~ /a/ && $#line == 9 && $line[9] =~ \d)
	    {
		$dlevel = $line[9];
		$foundvalue = 1;
		print @line;
		print "\n";
	    }
	    else
	    {
		$foundvalue = 1;
		$dlevel = -1;
	    }
	}
	$counter++;
    }
    unless($foundvalue)
    {
#	print "checking the last 100\n";
	$counter = 0;
	$raw = `tail -100 $path`;
	@lines = split("\n", $raw);
	while($foundvalue == 0 && $counter < 100) 
	{
#	    print "in the while\n";
	    @line = split(" ", $lines[($#lines - $counter)]);
#	    print "The 6th part is: $line[6]\n";
	    if($line[6] =~ /deice/)
	    {
		if($#line == 7)
		{
		    $dlevel = $line[7];
		    $foundvalue = 1;
#		    print @line;
#		    print "\n";
		}
		elsif($line[7] =~ /a/ && $#line == 8)
		{
		    $dlevel = $line[8];
		    $foundvalue = 1;
#		    print @line;
#		    print "\n";
		}
		elsif($line[7] =~ /a/ && $#line == 9)
		{
		    $dlevel = $line[9];
		    $foundvalue = 1;
#		    print @line;
#		    print "\n";
		}
		else
		{
		    $foundvalue = 1;
		    $dlevel = -1;
		}
	    }
	    $counter++;
	}
    }
    unless($foundvalue)
    {
#	print "checking the last 1000\n";
	$counter = 0;
	$raw = `tail -1000 $path`;
	@lines = split("\n", $raw);
	while($foundvalue == 0 && $counter < 1000) 
	{
#	    print "in the while\n";
	    @line = split(" ", $lines[($#lines - $counter)]);
#	    print "The 6th part is: $line[6]\n";
	    if($line[6] =~ /deice/)
	    {
		if($#line == 7)
		{
		    $dlevel = $line[7];
		    $foundvalue = 1;
#		    print @line;
#		    print "\n";
		}
		elsif($line[7] =~ /a/ && $#line == 8)
		{
		    $dlevel = $line[8];
		    $foundvalue = 1;
#		    print @line;
#		    print "\n";
		}
		elsif($line[7] =~ /a/ && $#line == 9)
		{
		    $dlevel = $line[9];
		    $foundvalue = 1;
#		    print @line;
#		    print "\n";
		}
		else
		{
		    $foundvalue = 1;
		    $dlevel = -1;
		}
	    }
	    $counter++;
	}
    }
}

sub usage
{
    print "autodeice first asks the user if they want to set the deice 
limit to 1. This will disable the program's ability to turn deice off 
completely. This option should be used when it's actively snowing to 
prevent buildup. After that, it will ask the user what level deice is 
currently set to. Entering 0-5 will tell the program that deice is 
currently at the given level. Entering 'search' will search the SMAshLog
for the last deice command. The search is limited to 1000 entries in the
log.  If searching fails, using 'set #' will send a deice command for the
number given (only 0-3 are allowed).  The program will then monitor deice,
checking every 2 minutes for temperature changes.  The program does not
have the password for deice levels 4 and 5. If the program decides to set
deice to 4 or 5, it will ask the user to do so manually. Issue 
'deice 4 (or 5)' in a hal window and enter the password (found in the
deice document).\n";
}
