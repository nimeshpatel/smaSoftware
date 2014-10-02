#!/usr/local/bin/perl
# usage: read_refl_mem.pl values_file duration_seconds samples_per_sec output_to_screen output_file

#Hawaii computers don't have this module!!!

#$hiResExists = 0;
#if ($hiResExists) {
#    print "[hiResExists ==> [$hiResExists] ... ";
#    print "Time::HiRes EXISTS\n";
#    use Time::HiRes qw(usleep gettimeofday);
#    print "Using Time::HiRes Module\n";
#} else {
    $systemCallToGetTime = "getunixtime";
    print "Time::HiRes not present, using 'getunixtime'\n";
#}

#### TO DO ############
#
# improve the usleep function...
#   - read abs time
#     compute desired wait time
#     set alarm
#
# change regex search from if ($ =~ /regex/ ) {then do nothing} else{dosomething} to:
# if ($ !=~ /regex/) {then do something}
# 
# parse the command line in a GOOD way
#
#######################

# written AUG-15-2002 by James Battat (jbattat@cfa.harvard.edu)
# heavily modified by Battat on AUG-16-2002
# A program to read a list of values from reflective memory at the SMA
# the user can decide:
#   1.) which values to read
#   2.) which antenna to read from
#   3.) how long to collect data
#   4.) data sampling interval (samples per sec)
#
#   (1.) and (2.) are defined in a ASCII text file of the following format:
#      antenna_number     value
#    e.g.
#        4            temperature_volt
#        4            dewar_temperature
#        3            temperature_volt2

# -h for help (syntactical)

$verbose  = 0;
$verbose2 = 0;

&Main();

sub Main() {
    $SIG{INT} = \&goodExit;
    &interpretInput();
    &readAndStoreCommands();
    &executeCommands();
    print "Successful completion \n\n";
}

sub goodExit {
   print "\nReceived a ctrl-c command, exiting nicely\n";
   close(OUT) or die "cannot close";
   exit 0;	
}

sub defineDefaults {
    # define the default values that will be used if
    # the user calls read_refl_mem.pl with no arguments

    @n = split(' ', `date`);
#    print "$n[$#n]\n";
    ($hour, $min) = split(':', $n[3]);

    print "Defining the default input values:\n" if ($verbose);
#    $default_values_file           = "default_values.inp";
    $default_duration              = 6;
    $default_samples_per_sec       = 1;
    $default_output_file_root      = "/global/data/engineering/totalpower/";

    $default_output_file         = $default_output_file_root."refMemOut_".$n[1].$n[2].$n[$#n]."_".$hour.$min.".pow";
    $default_output_file     .= time();
    $default_output_to_screen = 1;

    print "Done Defining default values. \n\n" if ($verbose);
}

sub printInputs {
    print "output_file      = [$output_file]\n";
    print "values_file      = [$values_file]\n";
    print "duration  [sec]  = [$duration]\n";
    print "samples_per_sec  = [$samples_per_sec]\n";
    print "increment [sec]  = [$increment]\n";

    print "\n";
}

sub interpretInput {
    #if the user requests help
    if($ARGV[0] EQ "-h") { 
	print "usage: read_refl_mem.pl values_file duration samples_per_sec output_to_screen output_file\n";
	print "where:\n";
	print "   values_file: an ascii file with  antenna_number and value\n";
	print "   duration is the total data collection time (seconds)\n";
	print "   samples_per_sec is the number of samples per second \n";
	print "   if the user omits output_file, then the default is chosen\n";
	print "\n";
	print "an example of a values_file is:\n";
	print " 4        total_power_volts  \n";
	print " 4        cabin_temperatue   \n";
	print " 3        total_power_volts2 \n";
	print "\n";
    exit(1);
    }
    
    &defineDefaults();
    
    #if no inputs are specified then go with the default values
    # otherwise, take values from input line
    if($#ARGV<0) { 
	$values_file     = $default_values_file;        #
	$duration        = $default_duration;           # 1 minute
	$samples_per_sec = $default_samples_per_sec;    # 1 Hz sampling
	$increment       = 1.0/$samples_per_sec;        #
	$output_file     = $default_output_file;
	$output_to_screen= $default_output_to_screen;
    } else {       
	$values_file     = $ARGV[0];
	$duration        = $ARGV[1];
	$samples_per_sec = $ARGV[2];
	$increment       = 1.0/$samples_per_sec;
	$output_to_screen= $ARGV[3];
	if ($ARGV[4] ne "") {
           print "i'm in NOT!!\n";
	    $output_file     = $default_output_file_root.$ARGV[4];
	} else {
            print "i'm in !!!!!!!\n";
            print " defvals file = $default_values_file\n";
            print "   vals file = $values_file\n";
    
	    &seconds2hst();
	    if ($values_file) {
               $values_string = $values_file;
            } else {
               $values_string = $default_values_file;
            }
            $values_string =~ s/.inp$//;
            $output_file   = $default_output_file_root.$file_dateString.$values_string.".dat";	
        }
    }

    &confirmFilename();
    &printInputs();
    $increment_us = $increment * 1_000_000;
	
}

sub readAndStoreCommands() {
    #open the file that contains antenna_num and values
    #and store contents into an array
    open FILE, $values_file or die "can't open values file\n";
    @lines = <FILE>;
    close(FILE);
    $maxOfLines = $#lines;
    print "there are $maxOfLines LINES IN THIS FILE\n";
    
    #operate on each line of the file to get the
    #appropriate system call
    
    # - the "#" symbol is a comment character anywhere in the line
    # - lines that have only ant_num or only value are printed to screen
    #   with an error msg and subsequently ignored

    for ($i=0; $i<=$maxOfLines; $i++) {
	print "$i ==> $lines[$i]" if ($verbose);
	$line = $lines[$i];
	
	#honor the comment character (#)
	($line) = split(/\#/,$line);

	#if a line is all whitespace, skip it
	# WOULD BE NICER AS if($line !=~ /regex/) {then do something}
	# but i cant get that to work!
	#perhaps if (!($line =~/regex/)) { then do something...}
	if ($line =~ /^[\/s]*$/) { 
	    print "encountered an all-whitespace line in $values_file\n" if ($verbose);
	} else { 
	    #extract antenna number and value name from the line
	    ($col1, $col2) = split(/\s+/, $line);
	    
	    #If BOTH antenna and value are present then store in hash
	    if (($col1 ne "") && ($col2 ne "")) {
		($antenna[$i], $value[$i]) = ($col1, $col2);
		print "[$antenna[$i]]::[$value[$i]]\n\n" if ($verbose);
		$key   = "A".$antenna[$i]."_".$value[$i];
		$val   = $antenna[$i].":".$value[$i];
		$valueOfAnt{$key} = $val;
	    } else {
		if ($verbose) {
		    print "\nthere is a missing value here...";
		    print "\n[$col1]::[$col2]\n";
		    print "Omitting this line\n\n";
		}
	    }
	}    
    }
    
    #create and store the values commands.
    print "The following values will be called: \n";
    $n = 0;
    while (($key,$val) = each(%valueOfAnt)) {
	($antenna, $value) = split(/:/,$val);
	print "[$antenna] ==> [$value]\n" if ($verbose2);
	
	if (($antenna ne "") && ($value ne "")) {
	    $commands[$n] = "value -a$antenna -v $value";
	    $labels[$n]   = "A".$antenna."_".$value;
	    print "$commands[$n]\n";
	    $n++;
	}
    }
    @timeLabels = ("sec_since_1970", "elap_sec");
    @labels     = (@timeLabels, @labels);
    return(@commands);
}


#now execute the commands and write data to file...
sub executeCommands() {
    my($start, $t, $tempTime, $m, $w);
    my(@n);
    my(@timeStamp);

    if ($hiResExists) {
	$start = gettimeofday;
    } else {
	$start = `$systemCallToGetTime`;
    }
    print "start time = $start\n\n";
    if (!$output_to_screen) {
	print "You have chosen NO OUTPUT TO SCREEN ... \n";
	print "see the output file, [$output_file] for data\n";
    }

    open(OUT, ">$output_file") or die "can't open output file\n";

    foreach (@labels) {
	print "[$_]\t" if ($output_to_screen);
	print OUT "$_\t";
    }
    print "\n" if ($output_to_screen);
    print OUT "\n";

    $t = 0;
    while ($t <= $duration) {
	undef(@output);
	#execute the commands and store output into @output
	$m = 0;
	foreach (@commands) {
	    $output[$m] = `$_`;
	    $m++;
	}

	#get a timestamp (to microsecond accuracy)
	if ($hiResExists) {
	    $presentTime  = gettimeofday;
	} else {
	    $presentTime  = `$systemCallToGetTime`;
	}

	$t            =  $presentTime - $start;
      	@timeStamp    = ($presentTime, $t);
	@output       = (@timeStamp, @output);
      	chomp(@output);

	#print the data to screen and to file (as requested)
	$w = 0;
	foreach (@output) {
	    if ($w <= $#timeLabels) {
		printf "[%.3f]\t", $_ if ($output_to_screen);
		printf OUT "%.3f\t", $_;
	    } else {
		print "[$_]\t" if ($output_to_screen);
		print OUT "$_\t";
	    }
	    $w++;
	}
	print "\n" if ($output_to_screen);
	print OUT "\n";

	#pause for $increment seconds ($increment can be decimal)
	if ($hiResExists) {
	    usleep($increment_us);
	} else {
	    select(undef, undef, undef, $increment);
	}
    }    
    close(OUT);
    print "\n\n";
}

sub confirmFilename() {

    my $unsure = 1;

    while ($unsure) {
        if (-e $output_file) {
            $temp_output_file = $output_file . time();
            print "The output file name you chose already exists\n";
            print "I suggest using $temp_output_file\n";
            print "Type:\n \t'y' to accept my suggestion\n\t'n' to overwrite\n\t'r' to choose another file name (you will be prompted for the new name)\n";
            chomp( my $decision = <STDIN> );

            if ($decision eq 'y') {
                $output_file = $temp_output_file;
                print "OK, you chose to use my suggested output file: $output_file\n";
                $unsure = 0;
            } elsif ($decision eq 'n') {
                print "OK, you have instructed me to overwrite the existing file named:\n $output_file\n";
            $unsure = 0;
            } elsif ($decision eq 'r') {
                print "OK, you can choose a new filename.  (include entire path...)\n";
                chomp ( $output_file = <STDIN> );
                #$unsure still equals  0
            } else {
                print "\nYou have entered an invalid choice:\n";
                print "'y', 'n', or 'r' please\n";
            }

        } else {
            $unsure = 0;
            print "Good.  You have chosen an unused filename\n";
        }
    }
}

sub seconds2hst {
# written: 02sep06 James Battat (jbattat@cfa.harvard.edu)
# converts getunixtime (in seconds) into the following format:
# [yr][mon][date]hst[hr][min]
# e.g. 02sep06hst1145
# because it's running on hal, which uses UTC, we must subtract 10 from 
# the hour given

# ideally, you could just set the $ENV{TZ} variable to be HST, 
# but i dont know how and i dont want to mess with hal.

use Time::Local;
my $verbose = 0;
my $utc2hst = 10;
my @monthnames = ( qw(Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec) );
my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday);

$TimeInSeconds = `getunixtime`;

#for testing only
#$TimeInSeconds -= 40000;

($sec, $min, $hour, $mday, $mon, $year, $wday, $yday) = localtime($TimeInSeconds);

$year += 1900;
$year_print_str = sprintf("%02d", $year % 100);
#note this is UTC date, it will be changed below....
$UTCdate           = $year_print_str.$monthnames[$mon].$mday_str;

if ($verbose) {
   print "UTC\n";
   print "[$UTCdate]\n";
   print "$hour:$min\n";
}

$hour -= $utc2hst;

if ($hour<0) {
   $hour += 24;
   $mday -= 1;
}

$mday_str = sprintf("%02d", $mday);
$date = $year_print_str.$monthnames[$mon].$mday_str;

if ($verbose) {
   print "\nHST\n";
   print "[$date]\n";
   print "$hour:$min\n";
}


$hour_str = sprintf("%02d", $hour);
$min_str  = sprintf("%02d", $min);

$file_dateString = $date."hst".$hour_str.$min_str;

print "$file_dateString\n";

}


sub dewarTemps {
  my ($dewarValue, $controlString, $verbose);
  $dewarValue = $_[0];
  $verbose = 0;

  print "the value $dewarValue is a dewar temperature value\n" if ($verbose);
  $ant_prefix = substr($dewarValue, 0, 3);
  print "ant_prefix ==> [$ant_prefix]\n" if ($verbose);

  $ant        = substr($ant_prefix, 1, 1);
  $value      = substr($dewarValue, 3);
  $controlString = "value -a$ant -v $value";

  print "control string = [$controlString]\n" if ($verbose);

  $dewarTemp  = `$controlString`;
  print "$dewarTemp\n" if ($verbose);
  @dewarTemps = split(/\s+/, $dewarTemp);

  @goodValues     = (0,1,2,3,9);
  @goodDewarTemps = @dewarTemps[@goodValues];

  $i = 0;
  foreach (@goodDewarTemps) {
    $tempIndex = @goodValues[$i] + 1;
    $suffix = "dewar_temp_".$tempIndex."_extracted";
    $tempsOfDewar{$ant_prefix.$suffix} = $goodDewarTemps[$i];
    $i += 1;
  }

  if ($verbose) {
    foreach (keys(%tempsOfDewar)) {
      print "tempsOfDewar{$_} ==> $tempsOfDewar{$_}\n";
    }
  }

}
