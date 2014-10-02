#!/usr/bin/perl -w
#
# Typical prerequisite commands:
# activate -l 0 -h 6 -g
# dopplerTrack -tco2-1 -u -s12
# setFeedOffset -f 230
# restartCorrelator -s 32
# 
# Before running this script, "point" until you have consistent results.
#
#
# The observer should edit the lines below to match the list of 
# antennas in the project.  This script should require about 1.5 hours
# per antenna to complete.  So with 4 antennas, that makes 6 hours.
#
$antenna=shift(@ARGV);
$numants = 6; 
$ant[0] = 1;
$ant[1] = 2;
$ant[2] = 3;
$ant[3] = 5;
$ant[4] = 6;
$ant[5] = 7;
$a = -1;
for ($i=0; $i<$numants; $i++) {
    if ($ant[$i] == $antenna) {
	$a = $i;
    }
}
if ($a == -1) {
    printf "You must give a valid antenna number as an argument.  Choices are:\n";
    for ($i=0; $i<$numants; $i++) {
	printf "%d ", $ant[$i];
    }
    printf "\n";
    exit(0);
}

#
# load the sma library of functions used in this observing script.
do 'sma.pl';
#
# check participating antennas.
checkANT();
#
$inttime = 30.0;
$scans = 8; # with one bad scan, this gives 3.5 minutes integration per antenna pointing
$maxoff =  60.1;  # arcsec
$minoff = -60.1; # arcsec
$arcsec = 3;  # acquisition limit requirement in arcsec
$start = 40;
$step = 5; # 1/8 of a beam
# pattern will be 0,30,40,50,55,60
#                 0,-30,-40,-55,-60
# in both azim and elev, so 20 steps per antenna * 4min = 80 min = 90 min with overhead
#
# 
$cal1 = "3c454.3";
$cal2 = "3c279";
$cal3 = "1924-292";
    if (checkEl($cal1) > 20) {
#       in June, 3c454 will always be rising if this is true
	command("observe -s $cal1");
    } else {
	if (checkEl($cal2) > 30) {
#           in June, 3c279 will always be setting if this is true,
#           so we need to leave it early if we cannot finish a full run
#           before it sets
	    command("observe -s $cal2");
	} else {
	    command("observe -s $cal3");
	}
    }
    command("sleep 2");
    command("tsys");
    $startazoff = `value -a $antenna -v azoff`;
    $starteloff = `value -a $antenna -v eloff`;
    for ($i=$start; $i<$maxoff; $i+=$step) {
	if ($i==$start) {
	    $offset = $startazoff;
	} else {
	    $offset = $i+$startazoff;
	}
	command("azoff -a $antenna -s $offset");
	command("antennaWait -a $antenna -e $arcsec");
	command("integrate -t $inttime -s $scans -w");
    }
    command("tsys");
    command("sleep 2");
    for ($i=-$start; $i>$minoff; $i-=$step) {
	if ($i==-$start) {
	    $offset = $startazoff;
	} else {
	    $offset = $i+$startazoff;
	}
	command("azoff -a $antenna -s $offset");
	command("antennaWait -a $antenna -e $arcsec");
	command("integrate -t $inttime -s $scans -w");
    }
    command("azoff -a $antenna -s $startazoff");
    command("tsys");
    command("sleep 2");
    for ($i=$start; $i<$maxoff; $i+=$step) {
	if ($i==$start) {
	    $offset = $starteloff;
	} else {
	    $offset = $i+$starteloff;
	}
	command("eloff -a $antenna -s $offset");
	command("antennaWait -a $antenna -e $arcsec");
	command("integrate -t $inttime -s $scans -w");
    }
    command("tsys");
    command("sleep 2");
    for ($i=-$start; $i>$minoff; $i-=$step) {
	if ($i==-$start) {
	    $offset = $starteloff;
	} else {
	    $offset = $i+$starteloff;
	}
	command("eloff -a $antenna -s $offset");
	command("antennaWait -a $antenna -e $arcsec");
	command("integrate -t $inttime -s $scans -w");
    }
    command("eloff -a $antenna -s $starteloff");
    printf "***************************\n";

    printf "Done antenna $antenna.  \n";
    printf "When you restart the script, give the next antenna number as an argument.\n";
    printf "For example:  beamphasetest.pl %d\n", $ant[$a+1];

