#!/usr/bin/perl
#
# chart6.pl
# Plots both the low-freq and high-freq receiver total-power data on the 
# same gnuplot page.  There are 6 panels, 1 for each antennas 1..6.
# This script should be run on a Solaris machine, such as smadata.
# Use -A to prevent auto-scaling on the basis of the previous scans.
# 
# - Todd Hunter 

#####################################################
$filetype=".rpoint";
#####################################################
$scale345[0] = 0; # multiply the 650 voltage by this value (all 6 antennas, if != 0)
$scale345[1] = 250; # multiply the 650 voltage by this value
$scale345[2] = 1; # multiply the 650 voltage by this value
$scale345[3] = 2; # multiply the 650 voltage by this value
$scale345[4] = 1; # multiply the 650 voltage by this value
$scale345[5] = 0.05; # multiply the 650 voltage by this value
$scale345[6] = 2.5; # multiply the 650 voltage by this value

$scale230[0] = 0; # multiply the 650 voltage by this value (all 6 antennas, if != 0)
$scale230[1] = 1; # multiply the 650 voltage by this value
$scale230[2] = 1; # multiply the 650 voltage by this value
$scale230[3] = 1; # multiply the 650 voltage by this value
$scale230[4] = 0.3; # multiply the 650 voltage by this value
$scale230[5] = 1; # multiply the 650 voltage by this value
$scale230[6] = 1; # multiply the 650 voltage by this value
$offset = 0;
for ($i=0; $i<=6; $i++) {
  $offset230[$i] = 0.0;
  $offset345[$i] = 0.0;
  $lowfreqmin[$i] = 0.0;
  $lowfreqmax[$i] = 0.0;
  $highfreqmin[$i] = 0.0;
  $highfreqmax[$i] = 0.0;
  $avg230[$i] = $mid230[$i] = 0.0;
  $avg345[$i] = $mid345[$i] = 0.0;
  $avg690[$i] = $mid690[$i] = 0.0;
  $rms230[$i] = 0.0;
  $rms345[$i] = 0.0;
  $rms690[$i] = 0.0;
}
$antenna = 0;
$freqlow = 0;
$freqhigh = 0;
$ncols = 2.0;
$nrows = 3.0;
$autoscale = 1;
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
            $xmargin, $xmargin+$xsize+$xgap);
# (0.02,0.5,0.02,0.5,0.02,0.5);
# @yorigin = (0.66,0.66,0.33,0.33,0.02,0.02);
@yorigin = ($fullheight-$ymargin-$ysize,
            $fullheight-$ymargin-$ysize,
            $fullheight-$ymargin-2*$ysize-$ygap, 
            $fullheight-$ymargin-2*$ysize-$ygap, 
            $fullheight-$ymargin-3*$ysize-2*$ygap,
            $fullheight-$ymargin-3*$ysize-2*$ygap);
#for ($i=0; $i<6; $i++) {
#  print "$xorigin[$i], $yorigin[$i]\n";
#}
@latestFile = ("NONE","NONE","NONE","NONE","NONE","NONE");

$i = 0;
while ($ARGV[$i] ne "") {
    if ($ARGV[$i] eq "-h" || $ARGV[$i] eq "--help") {
	print "Usage: chart6.pl [-a <antennaList:comma-delimited>] -s <scaleFor690/345GHzAllAntennas>\n";
        print "                 -s1 <scaleFor690/345GHzAntenna1>\n";
        print "                 -s2 <scaleFor690/345GHzAntenna2> etc.\n";
        print "                 -r  <scaleFor690/230GHzAllAntennas> etc.\n";
        print "                 -r1 <scaleFor690/230GHzAntenna1> etc.\n";
        print "                 -r2 <scaleFor690/230GHzAntenna2> etc.\n";
	print "       -A (to prevent autoscaling based on previous scan)\n";
	exit(0);
    }
    if ($ARGV[$i] eq "-A") {
	$autoscale = 0;
	print "Will not do autoscaling based on previous scan's data\n";
    }
    if ($ARGV[$i] eq "-a") {
	$i++;
	print "parsing -a argument: $ARGV[$i]\n";
	if ($ARGV[$i] eq "") {
	    printf "-a needs an argument (comma-delimited and NOT ellipsis-delimited)\n";
	    exit(0);
	} else {
	    @antennaList = split(',',$ARGV[$i]);
	    print "antennaList = $antennaList\n";
	}
    }
    if ($ARGV[$i] eq "-s") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s needs an argument\n";
	    exit(0);
	} else {
	    $scale345[0] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-s1") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s1 needs an argument\n";
	    exit(0);
	} else {
	    $scale345[1] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-s2") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s2 needs an argument\n";
	    exit(0);
	} else {
	    $scale345[2] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-s3") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s3 needs an argument\n";
	    exit(0);
	} else {
	    $scale345[3] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-s4") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s4 needs an argument\n";
	    exit(0);
	} else {
	    $scale345[4] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-s5") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s5 needs an argument\n";
	    exit(0);
	} else {
	    $scale345[5] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-s6") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-s6 needs an argument\n";
	    exit(0);
	} else {
	    $scale345[6] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r needs an argument\n";
	    exit(0);
	} else {
	    $scale230[0] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r1") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r1 needs an argument\n";
	    exit(0);
	} else {
	    $scale230[1] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r2") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r2 needs an argument\n";
	    exit(0);
	} else {
	    $scale230[2] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r3") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r3 needs an argument\n";
	    exit(0);
	} else {
	    $scale230[3] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r4") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r4 needs an argument\n";
	    exit(0);
	} else {
	    $scale230[4] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r5") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r5 needs an argument\n";
	    exit(0);
	} else {
	    $scale230[5] = $ARGV[$i];
	}
    }
    if ($ARGV[$i] eq "-r6") {
	$i++;
	if ($ARGV[$i] eq "") {
	    printf "-r6 needs an argument\n";
	    exit(0);
	} else {
	    $scale230[6] = $ARGV[$i];
	}
    }
    $i++;
}

$badAntExists = 0;

foreach $ant (@antennaList) {
    if ($ant > 6 || $ant < 1) {
	$badAntExists = 1;
	@badAntList   = (@badAntList, $ant);
    }
}
$numberAntennas = $#antennaList+1;
if ($numberAntennas == 0) {
# read it from the antennasInArray file
    open(PROJ,"/global/projects/antennasInArray") || die "You must either give a project command, or specify one or more antennas with -a (comma-delimited)";
    $_ = <PROJ>;
    @antennaList = split(' ',$_);
    $numberAntennas = $#antennaList+1;
    if ($numberAntennas > 0) {
	print "Using all $numberAntennas antennas in the project: @antennaList\n";
    } else {
	print "You must either give a project command, or specify one or more antennas with -a (comma-delimited)\n";
	exit(0);
    }
} elsif ($badAntExists) {
    print "ERROR: Input parameter error\n";
    print "You can only view antennas 1-6\n";
    print "but you tried to view antenna[s]\n";
    foreach $badAntNumber (@badAntList) {
	print "   $badAntNumber\n";
    }
    exit(0);
} else {
    print "Using antenna list = @antennaList\n";
}
#exit 0;
if ($scale230[0] == 0) {
 "When plotting 230 data, will scale the high-frequency data by: $scale230[1] $scale230[2] $scale230[3] $scale230[4] $scale230[5] $scale230[6]\n";
 "When plotting 230 data, will scale the high-frequency data by: $scale230[1] $scale230[2] $scale230[3] $scale230[4] $scale230[5] $scale230[6]\n";
} else {
 "When plotting 230 data, will scale the high-frequency data by $scale230[0]\n";
}
if ($scale345[0] == 0) {
 "When plotting 345 data, will scale the high-frequency data by: $scale345[1] $scale345[2] $scale345[3] $scale345[4] $scale345[5] $scale345[6]\n";
 "When plotting 345 data, will scale the high-frequency data by: $scale345[1] $scale345[2] $scale345[3] $scale345[4] $scale345[5] $scale345[6]\n";
} else {
 "When plotting 345 data, will scale the high-frequency data by $scale345[0]\n";
}
print "Will use antennas: @antennaList\n";
foreach $antenna (@antennaList) {
    $temp[$antenna-1] = 0; 			
    $latest_mtime[$antenna-1] = 0;		# used for file modification time.
    $latestTime[$antenna-1] = "";	
    $latestSource[$antenna-1] = "";
}

killoldprocs();

while (1) {			# begin infinite loop.
    $newfile = 0;
# slurp in all the file names in this area.
    foreach $antenna (@antennaList) {
	print "Waiting for a new file to appear in /data/engineering/rpoint/ant$antenna\n";
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
		print "\nPrevious latest file was $latestFile[$antenna-1] ";
		print "in antenna $antenna of type $filetype\n";
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
		$latestAntenna = $antenna;
	    }
	    $temp[$antenna-1] = $#a;		# reset the number of files.
	}
    }  # end of loop over antennas
    # okay
    $antenna = $latestAntenna;  # added on 10 Mar 2004
    if ($newfile == 1) {
	print "\n@latestFile\n";
	if ($startedGnuPlot==1 && $newfile==1) { 
	    print "killing previous gnuplot pid=$gnuplotPid\n";
	    $result = `kill -9 $gnuplotPid`;
	    print "$result\n";
	    print "Now running ps | grep gnuplot_\n";
	    killoldprocs();
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
	
	print "Opening data file = $latestFile[$antenna-1]\n";
	open(DATAFILE,$latestFile[$antenna-1]);
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
	    $freqhigh = 650;
	} else {
	    $freqhigh = 0;
	}
	print "This is the $freqlow and $freqhigh receivers\n";
	close(DATAFILE);

# look for NaNs that will cause the gnuplot process to die
	foreach $ant (@antennaList) {
	    $dropped[$ant] = 0;
	}
	$ctr = 0;
	foreach $ant (@antennaList) {
	    print "Opening data file to check for NaNs = $latestFile[$ant-1]\n";
	    open(DATAFILE,$latestFile[$ant-1]);
	    $_=<DATAFILE>;
	    $_=<DATAFILE>;
# from here on is valid data
	    $_=<DATAFILE>;
	    @dataline = split(' ',$_);
	    print "Checking line = $_\n";
	    if ($dataline[5] =~ /[^0-9\.\-]/ || $dataline[6] =~ /[^0-9\.\-]/) {
		# remove this antenna from the list
		splice @antennaList, $ctr, 1;
		print "Dropping antenna $ant from the list. New list = @antennaList\n";
		$dropped[$ant] = 1;
	    }
	    if ($autoscale != 0) {
		$lowfreqmax[$ant] = -1000000.;
		$lowfreqmin[$ant] =  1000000.;
		$highfreqmax[$ant] = -1000000.;
		$highfreqmin[$ant] =  1000000.;
		$lowfreqsum = 0;
		$highfreqsum = 0;
		$lowfreqsumsq = 0;
		$highfreqsumsq = 0;
		$npts = 0;
		do {
		    @dataline = split(' ',$_);
		    if (($dataline[5] =~ /[^0-9\.\-]/ || $dataline[6] =~ /[^0-9\.\-]/) && $dropped[$ant]==0){
			$dropped[$ant] = 1;
			# remove this antenna from the list
			splice @antennaList, $ctr, 1;
			print "Dropping antenna $ant from the list. New list = @antennaList\n";
			break;
		    } else {
			$npts++; 
			$lowfreqsum += $dataline[5];
			$highfreqsum += $dataline[6];
			$lowfreqsumsq += $dataline[5]*$dataline[5];
			$highfreqsumsq += $dataline[6]*$dataline[6];
			if ($dataline[5] > $lowfreqmax[$ant]) {
			    $lowfreqmax[$ant] = $dataline[5];
			}
			if ($dataline[5] < $lowfreqmin[$ant]) {
			    $lowfreqmin[$ant] = $dataline[5];
			}
			if ($dataline[6] > $highfreqmax[$ant]) {
			    $highfreqmax[$ant] = $dataline[6];
			}
			if ($dataline[6] < $highfreqmin[$ant]) {
			    $highfreqmin[$ant] = $dataline[6];
			}
		    }
		    $_=<DATAFILE>;
		    @parse = split(' ',$_);
# the final line in the file often has a pound sign out front and nowhere else
		} while (defined($_) && defined($parse[2]));
#		print "Skipped line = $_\n";
		$mid690[$ant] = 0.5*($highfreqmin[$ant]+$highfreqmax[$ant]);
		$denominator = ($highfreqmax[$ant]-$highfreqmin[$ant]);
		if ($freqlow == 230) {
		    if ($denominator != 0) {
			$scale230[$ant]=($lowfreqmax[$ant]-$lowfreqmin[$ant])/$denominator;
		    } else {
			$scale230[$ant] = 1;
		    }
		    $mid230[$ant] = 0.5*($lowfreqmin[$ant]+$lowfreqmax[$ant]);
		    if ($npts>0) {
			$avg230[$ant] = $lowfreqsum/$npts;
			$avg690[$ant] = $highfreqsum/$npts;
			$rms230[$ant] = $lowfreqsumsq/$npts-($avg230[$ant]**2);
			if ($rms230[$ant] > 0) {
			    $rms230[$ant] = sqrt($rms230[$ant]);
			} 
#			$offset230[$ant] = $avg230[$ant]-($avg690[$ant]*$scale230[$ant]);
			$offset230[$ant] = $mid230[$ant]-($mid690[$ant]*$scale230[$ant]);
		    }
		    print "New scale factor for 650/230 on antenna $ant = $scale230[$ant], offset = $offset230[$ant]\n";
#                    print "npts = $npts, lowfreqsum=$lowfreqsum\n";
		} else {
		    if ($denominator != 0) {
			$scale345[$ant]=($lowfreqmax[$ant]-$lowfreqmin[$ant])/$denominator;
		    } else {
			$scale345[$ant] = 1;
		    }
		    $mid345[$ant] = 0.5*($lowfreqmin[$ant]+$lowfreqmax[$ant]);
		    if ($npts > 0) {
			$avg345[$ant] = $lowfreqsum / $npts;
			$avg690[$ant] = $highfreqsum / $npts;
			$rms345[$ant] = $lowfreqsumsq/$npts-($avg345[$ant]**2);
			if ($rms345[$ant] > 0) {
			    $rms345[$ant] = ($rms345[$ant]**0.5);
			} 
#			$offset345[$ant] = $avg345[$ant]-($avg690[$ant]*$scale345[$ant]);
			$offset345[$ant] = $mid345[$ant]-($mid690[$ant]*$scale345[$ant]);
		    }
		    print "New scale factor for 650/345 on antenna $ant = $scale345[$ant], offset = $offset345[$ant]\n";
#                    print "npts = $npts\n";
                    print "     lowfreqsum = $lowfreqsum   highfreqsum = $highfreqsum  avg345 = $avg345[$ant]  avg690 = $avg690[$ant]\n";
		}
		if ($npts > 0) {
		    $rms690[$ant] = $highfreqsumsq/$npts - ($avg690[$ant]**2);
		    if ($rms690[$ant] > 0) {
			$rms690[$ant] = ($rms690[$ant]**0.5);
		    } 
		}
		if ($freqlow == 230) {
		    print "antenna $ant min/max: 230: ($lowfreqmin[$ant],$lowfreqmax[$ant])   690: ($highfreqmin[$ant],$highfreqmax[$ant])\n";
		    print "antenna $ant RMS: 230: $rms230[$ant]   690: $rms690[$ant]\n";
		} else {
		    print "antenna $ant min/max: 345: ($lowfreqmin[$ant],$lowfreqmax[$ant])   690: ($highfreqmin[$ant],$highfreqmax[$ant])\n";
		    print "antenna $ant RMS: 345: $rms345[$ant]   690: $rms690[$ant]\n";
		}
	    }  # end if/else autoscale 
	    $ctr = $ctr+1;
	    close(DATAFILE);
	} # end 'for' loop over antennas
	open(GNUPLOT,$makegnuname) || die "Could not open file = $makegnuname";
	print "creating file with gnuname = $makegnuname\n";
		    
	##################
	## AZIMUTH SCAN ##
	##################
		    
	if ($a[$index] =~ m/azscan/) {
	    print GNUPLOT "T=1\n";
	    print GNUPLOT "set noytics\n";
	    print GNUPLOT "loop=1\n\n";
	    print GNUPLOT "set size 0.95, 0.95\n";
	    print GNUPLOT "set origin 0.02, 0.02\n";
	    print GNUPLOT "set multiplot\n\n";
	    print "entering 'for' loop with list = @antennaList\n";
	    foreach $antenna (@antennaList) {
		if ($freqlow == 230) {
		    if ($scale230[0] == 0) {
			$scalefactor = $scale230[$antenna];
			$offset = $offset230[$antenna];
		    } else {
			$scalefactor = $scale230[0];
			$offset = 0;
		    }
		} else {
		    if ($scale345[0] == 0) {
			$scalefactor = $scale345[$antenna];
			$offset = $offset345[$antenna];
		    } else {
			$scalefactor = $scale345[0];
			$offset = 0;
		    }
		}
		print "offset = $offset, offset230[$antenna] = $offset230[$antenna], offset345[$antenna] = $offset345[$antenna]\n";
		print GNUPLOT "
set size   $xsize, $ysize
set origin $xorigin[$antenna-1], $yorigin[$antenna-1]
set autoscale y
set yrange [] writeback
#
set xlabel \"Ant$antenna Az Offset (arcsec) $latestSource[$antenna-1] $latestTime[$antenna-1] \" 0,0.8
#set title \"Ant$antenna Az scan  $latestSource[$antenna-1] $latestTime[$antenna-1]\" 
plot '$latestFile[$antenna-1]' u (\$4):(\$6) title \"$freqlow GHz\" with lines
#
set noautoscale y
#
replot '$latestFile[$antenna-1]' using (\$4):(\$7*$scalefactor+$offset) title \"$freqhigh GHz\" with lines
";
	    } # end of 'for' loop
	    print GNUPLOT "set autoscale y\n";
	    print GNUPLOT "set nomultiplot\n";
	    print GNUPLOT "set size\n";
	    print GNUPLOT "set origin\n";
	    print GNUPLOT "pause T\n";
	    print GNUPLOT "if (loop==1) reread\n";
	}  # end of macro definition for az scan
		
	####################
	## ELEVATION SCAN ##
	####################
    
	if ($a[$index] =~ m/elscan/) {
	    print GNUPLOT "T=1\n";
	    print GNUPLOT "set noytics\n";
	    print GNUPLOT "loop=1\n\n";
	    print GNUPLOT "set size 0.95, 0.95\n";
	    print GNUPLOT "set origin 0.02, 0.02\n";
	    print GNUPLOT "set multiplot\n\n";
	    print "entering 'for' loop with list = @antennaList\n";
	    foreach $antenna (@antennaList) {
		if ($freqlow == 230) {
		    if ($scale230[0] == 0) {
			$scalefactor = $scale230[$antenna];
			$offset = $offset230[$antenna];
		    } else {
			$scalefactor = $scale230[0];
			$offset = 0;
		    }
		} else {
		    if ($scale345[0] == 0) {
			$scalefactor = $scale345[$antenna];
			$offset = $offset345[$antenna];
		    } else {
			$scalefactor = $scale345[0];
			$offset = 0;
		    }
		}
		print "offset = $offset, offset230[$antenna] = $offset230[$antenna], offset345[$antenna] = $offset345[$antenna]\n";
		print GNUPLOT "
set size   $xsize, $ysize
set origin $xorigin[$antenna-1], $yorigin[$antenna-1]
set autoscale y
set yrange [] writeback
#
set xlabel \"Ant$antenna El Offset (arcsec) $latestSource[$antenna-1] $latestTime[$antenna-1]\" 0,0.8
#set title \"Ant$antenna El scan  $latestSource[$antenna-1] $latestTime[$antenna-1]\" 
plot '$latestFile[$antenna-1]' u (\$5):(\$6) title \"$freqlow GHz\" with lines
#
set noautoscale y
#
replot '$latestFile[$antenna-1]' using (\$5):(\$7*$scalefactor+$offset) title \"$freqhigh GHz\" with lines 
#
";
		print "latestFile[$antenna] = $latestFile[$antenna-1]\n";
	    } # end of 'for' loop
	    print GNUPLOT "set autoscale y\n";
	    print GNUPLOT "set nomultiplot\n";
	    print GNUPLOT "set size\n";
	    print GNUPLOT "set origin\n";
	    print GNUPLOT "pause T\n";
	    print GNUPLOT "if (loop==1) reread\n";
	} # end of macro definition for el scan
	print "Closing gnuplot macro file\n";	
	close(GNUPLOT);
	
	if ($startedGnuPlot==0) {
	    $gnucommand = "|gnuplot ".$gnuname." &";
	    print "running gnu command = $gnucommand\n";
	    $gnuplotPid=open(F,$gnucommand);
	    $gnuplotPid++;
	    print "gnuplot pid = $gnuplotPid\n";
	    $startedGnuPlot=1;
	}
    } # foreach antenna loop
    sleep 10;
} # while (1) loop

sub killoldprocs {
    print "killing any previous gnuplots (that I can)\n";
    do {
	$ps = `ps | grep gnuplot_`;
	@lines = split('\n',$ps);
	$kill = 0;
	foreach $line (@lines) {
	    print "$line\n";
	    ($pid,$terminal,$state,$cputime,$c,$d,$e) = split(' ',$line);
	    if ($e eq "ps" || $c == "grep") {
		print "not killing the grep process\n";
	    } else {
		print "killing previous gnuplot pid=$pid\n";
		$result = `kill -9 $pid`;
		print "$result\n";
		$kill++;
	    }
	}
    } while ($kill != 0);
    $startedGnuPlot=0;
}
