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
# The observer should edit the lines below to match the list of 
# antennas in the project.  This script should require about 1.5 hours
# per antenna to complete.  So with 4 antennas, that makes 6 hours.
#
$antenna=shift(@ARGV);
$numants = 4; 
$ant[0] = 1;
$ant[1] = 2;
$ant[2] = 5;
$ant[3] = 6;
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

# check participating antennas.
#checkANT();
#

#
# load the sma library of functions used in this observing script.
do 'sma.pl';
#
$SIG{INT}=\&Pause;

$startazoff = `value -a $antenna -v azoff`;
$starteloff = `value -a $antenna -v eloff`;
printf "The starting offsets for antenna %d are (%.1f,%.1f)\n", $antenna, 
    $startazoff, $starteloff;

$inttime = 30.0;
$scans = 8; # with one bad scan, this gives 3.5 minutes integration per antenna pointing
$maxoff =  111.1;  # arcsec
$minoff = -111.1; # arcsec
$arcsec = 3;  # acquisition limit requirement in arcsec
$start = 30;
$step = 20;
# pattern will be 0,+30,+50,+70,+90,+110
#                 0,-30,-50,-70,-90,+110
# in both azim and elev, so 24 steps per antenna * 4min = 96 min = 100 min with overhead
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


sub Pause()
{
    print "beamphasetest interrupted.  Restoring antenna offsets.\n";
    sleep 1;
    command("eloff -a $antenna -s $starteloff");
    sleep 1;
    command("azoff -a $antenna -s $startazoff");
    sleep 1;
    exit(0);
}
