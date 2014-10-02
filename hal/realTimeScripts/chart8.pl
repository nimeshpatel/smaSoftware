#!/usr/bin/perl
#
# Plots both the low-freq and high-freq receiver data on the same screen
# Should be run separately for each antenna using -a.
# - Todd Hunter 11/10/2003

#####################################################
$filetype=".rpoint";
#####################################################

###############################
## SET UP THE GUI PARAMETERS ##
###############################
$scale = 50; # multiply the 650 voltage by this value
$antenna = 0;
$freqlow = 0;
$freqhigh = 0;
$ncols = 2.0;
$nrows = 4.0;
$fullwidth = 1;
$fullheight = 1;
$xmargin = 0.02;
$ymargin = 0.02;
$xgap = $xmargin;
$ygap = $ymargin;
$xsize = ($fullwidth-$xgap*($ncols-1)-2*$xmargin)/$ncols;
$ysize = ($fullheight-$ygap*($nrows-1)-2*$ymargin)/$nrows;

@xorigin = ($xmargin, $xmargin+$xsize+$xgap, 
            $xmargin, $xmargin+$xsize+$xgap, 
            $xmargin, $xmargin+$xsize+$xgap,
            $xmargin, $xmargin+$xsize+$xgap);
@yorigin = ($fullheight-$ymargin-$ysize,
            $fullheight-$ymargin-$ysize,
            $fullheight-$ymargin-2*$ysize-$ygap, 
            $fullheight-$ymargin-2*$ysize-$ygap, 
            $fullheight-$ymargin-3*$ysize-2*$ygap,
            $fullheight-$ymargin-3*$ysize-2*$ygap,
            $fullheight-$ymargin-4*$ysize-3*$ygap,
            $fullheight-$ymargin-4*$ysize-3*$ygap);
#################################
### done setting up gui params ##
#################################


@latestFile = ("NONE","NONE","NONE","NONE","NONE","NONE","NONE","NONE");

$i = 0;
while ($ARGV[$i] ne "") {
    if ($ARGV[$i] eq "-a") {
	$i++;
	print "[$ARGV[$i]]\n";
	if ($ARGV[$i] eq "") {
	    printf "-a needs an argument (comma-delimited)\n";
	    exit(0);
	} else {
	    @antennaList = split(',',$ARGV[$i]);
	}
    }
    if ($ARGV[$i] eq "-s") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s needs an argument\n";
	    exit(0);
	} else {
	    $scale = $ARGV[$i];
	}
    }
    $i++;
}

$badAntExists = 0;

foreach $ant (@antennaList) {
    if ($ant > 8 || $ant < 1) {
	$badAntExists = 1;
	@badAntList   = (@badAntList, $ant);
    }
}
$numberAntennas = $#antennaList+1;
if ($numberAntennas == 0) {
    print "You must specify one or more antennas with -a (comma-delimited)\n";
    exit(0);
} elsif ($badAntExists) {
    print "ERROR: Input parameter error\n";
    print "You can only view antennas 1-8\n";
    print "but you tried to view antenna[s]\n";
    foreach $badAntNumber (@badAntList) {
	print "   $badAntNumber\n";
    }
    exit(0);
}

print "Will scale the high-frequency data by $scale\n";
print "Will use antennas: @antennaList\n";
foreach $antenna (@antennaList) {
    $temp[$antenna-1] = 0; 			
    $latest_mtime[$antenna-1] = 0;		# used for file modification time.
    $latestTime[$antenna-1] = "";
    $latestSource[$antenna-1] = "";
}
$startedGnuPlot=0;
while (1) {			# begin infinite loop.
    $newfile = 0;
    print "Waiting for a new file to appear\n";
# slurp in all the file names in this area.
    foreach $antenna (@antennaList) {
	@a=</data/engineering/rpoint/ant$antenna/*.rpoint>;
#	print "$#a, temp[$antenna-1] = $temp[$antenna-1]\n";
	if ($#a > $temp[$antenna-1]) {	# a new file has been added?
# then get the file statistics and find the
# latest file!
	    for ($i=0;$i<=$#a;$i++) {  
		($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
		 $atime,$mtime,$ctime,$blksize,$blocks)
		    = stat $a[$i];
		if ($mtime > $latest_mtime[$antenna-1]) {
		    $index=$i; 
		    $latest_mtime[$antenna-1]=$mtime;
		}
	    }
	    # if this new file is a fits file, 		
	    # wait while the file is still being written,
	    # pausing every second.
	    if ($a[$index] =~ m/$filetype$/) {
		print "Previous latest file was $latestFile[$antenna-1] in antenna $antenna of type $filetype\n";
		$latestFile[$antenna-1] = $a[$index];
		@fileNameBits = split('\.',$latestFile[$antenna-1]);
		foreach (@fileNameBits) {
		    # match time of the form DDMMMYY_HHMMSS
		    if ($_ =~ /\d{2}\w{3}\d{2}_\d{6}/) {
			$latestTime[$antenna-1] =$_;
		    } 
		}

		## open the file, parse the header and get the source name
		open FH, "$latestFile[$antenna-1]";
		#the first line is trash
		$line = <FH>;
		#read the useful header
		@headerVals = split(",", <FH>);
		@sourceAndSpaces = split(" ",$headerVals[2]);
		$latestSource[$antenna-1] = $sourceAndSpaces[0];
		print "Saw file $latestFile[$antenna-1] in antenna $antenna ";
		print "of type $filetype\n";
		print "and source = $latestSource[$antenna-1]\n";
		$newfile = 1;
	    }
	    $temp[$antenna-1] = $#a;		# reset the number of files.
	}
    }
# okay
    if ($newfile == 1) {
	print "\n@latestFile\n";
	if ($startedGnuPlot==1 && $newfile==1) { 
	    #kill the previous gnuplot 
	    print "killing previous gnuplot\n";
	    #kill -9 => $gnuplotPid;
	    `kill -9 $gnuplotPid`;
	    sleep 2;
	    $ps = `ps -a | grep gnuplot_`;
	    ($pid,$junk)=split(' ',$ps);
	    `kill -9 $pid`;
	    sleep 2;
	    print "killed previous gnuplot\n";
	    $startedGnuPlot=0;
	}
	
	## make a unique gnuplot macro filename
	$localtime = `date`;
	chop($localtime);   #local time is like: Wed Jan 28 21:10:19 HST 2004
	@tNug = split(" ",$localtime);  
	#dateString format = 2004Jan28_21:12:18
	$dateString = $tNug[5].$tNug[1].$tNug[2]."_".$tNug[3];
	if ($a[$index] =~ m/azscan/) {
	    $label = "azscan";
	} elsif ($a[$index] =~ m/elscan/) {
	    $label = "elscan";
	}

	$gnuname= "/data/engineering/rpoint/gnuplotmacros/gnuplot.macro.".$dateString.".".$label;
	$makegnuname = ">".$gnuname;
	#done with filename
	
	print "Creating macro file = $gnuname\n";
	print "Opening data file = $a[$index]\n";
	open(DATAFILE,$a[$index]);

	# look for receiver bands in the header
	$_=<DATAFILE>;
	$_=<DATAFILE>;
	@items = split(',',$_);
	$receiverhigh = $items[88];
	$receiverlow = $items[89];
	if ($receiverlow eq "A1") {
	    $freqlow = 230;
	} else {
	    $freqlow = 345;
	}
	if ($receiverhigh eq "E") {
	    $freqhigh = 658;
	} else {
	    $freqhigh = 0;
	}
	close(DATAFILE);
	open(GNUPLOT,$makegnuname);
	print "gnuname = $makegnuname\n";
		    
#  AZIMUTH SCAN
		    
	if ($a[$index] =~ m/azscan/) {
	    print GNUPLOT "T=1\n";
	    print GNUPLOT "set noytics\n";
	    print GNUPLOT "loop=1\n\n";
	    print GNUPLOT "set size 0.95, 0.95\n";
	    print GNUPLOT "set origin 0.02, 0.02\n";
	    print GNUPLOT "set multiplot\n\n";
	    print "entering 'for' loop with list = @antennaList\n";
	    foreach $antenna (@antennaList) {
		print GNUPLOT "
set size   $xsize, $ysize
set origin $xorigin[$antenna-1], $yorigin[$antenna-1]
set autoscale y
set yrange [] writeback
#
set xlabel \"Ant$antenna Az Offset (arcsec) $latestSource[$antenna-1] $latestTime[$antenna-1]\" 0,0.8
#set title \"Antenna $antenna Azimuth scan\" # : $latestFile[$antenna-1]\"
plot '$latestFile[$antenna-1]' u (\$4):(\$6) title \"$freqlow GHz\" with lines
#
set noautoscale y
#
replot '$latestFile[$antenna-1]' using (\$4):(\$7*$scale) title \"$freqhigh GHz\" with lines
";
	    } # end of 'for' loop
	    print GNUPLOT "set autoscale y\n";
	    print GNUPLOT "set nomultiplot\n";
	    print GNUPLOT "set size\n";
	    print GNUPLOT "set origin\n";
	    print GNUPLOT "pause T\n";
	    print GNUPLOT "if (loop==1) reread\n";
	}  # end of macro definition for az scan
		    
#  ELEVATION SCAN
	    
	if ($a[$index] =~ m/elscan/) {
	    print GNUPLOT "T=1\n";
	    print GNUPLOT "set noytics\n";
#			print GNUPLOT "set rmargin 1\n";
#			print GNUPLOT "set bmargin 1\n";
	    print GNUPLOT "loop=1\n\n";
	    print GNUPLOT "set size 0.95, 0.95\n";
	    print GNUPLOT "set origin 0.02, 0.02\n";
	    print GNUPLOT "set multiplot\n\n";
	    print "entering 'for' loop with list = @antennaList\n";
	    foreach $antenna (@antennaList) {
#		print "working on antenna $antenna\n";
#		print "xorigin = $xorigin[$antenna-1], yorigin = $yorigin[$antenna-1]\n"; 
		print GNUPLOT "
set size   $xsize, $ysize
set origin $xorigin[$antenna-1], $yorigin[$antenna-1]
set autoscale y
set yrange [] writeback
#
set xlabel \"Ant$antenna El Offset (arcsec) $latestSource[$antenna-1] $latestTime[$antenna-1]\" 0,0.8
#set title \"Antenna $antenna Elevation scan\" # : $latestFile[$antenna-1]\"
plot '$latestFile[$antenna-1]' u (\$5):(\$6) title \"$freqlow GHz\" with lines
#
set noautoscale y
#
replot '$latestFile[$antenna-1]' using (\$5):(\$7*$scale) title \"$freqhigh GHz\" with lines 
#
";
	    } # end of 'for' loop
	    print GNUPLOT "set autoscale y\n";
	    print GNUPLOT "set nomultiplot\n";
	    print GNUPLOT "set size\n";
	    print GNUPLOT "set origin\n";
	    print GNUPLOT "pause T\n";
	    print GNUPLOT "if (loop==1) reread\n";
	} # end of macro definition for el scan
	
	close(GNUPLOT);
#    }

	
    if ($startedGnuPlot==0) {
	$gnucommand = "|gnuplot ".$gnuname." &";
	print "running gnu command = $gnucommand\n";
	$gnuplotPid=open(F,$gnucommand);
	$gnuplotPid++;
	print "gnuplot pid = $gnuplotPid\n";
	print "\nmacro is saved in:\n";
	print "     $gnuname\n\n";
	$startedGnuPlot=1;
    }
}
    sleep 10;
} # while (1) loop

