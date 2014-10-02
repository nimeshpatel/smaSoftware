#!/usr/local/bin/perl
# rastermap iterates of el, performing an az scan at each step and running the data collector continuously
#adapted from cpoint by Kieran Finn 07/03/2012
#contact kieran.finn@hotmail.com


#Initialization
$azminoffset=-520.;
$azmaxoffset=520.;
$elminoffset=-80.;
$elmaxoffset=80.;
$offsetUnit=40.0;
$azoffsetUnit=0.0;
$eloffsetUnit=0.0;
$chopflag="";
$pid=$$;
$source="";
$antennawaitarcsec=4.0;
%positiveResponses=('yes'=>1,'y'=>1,'Y'=>1, 'Yes'=>1, 1=>1);
print "process id = $pid\n";
#interrupt
$SIG{INT}=\&Pause;
use Getopt::Long;
$Getopt::Long::autoabbrev=1;

sub Usage(){
    print "\n\nUsage:\n\n";
    print "rastermap is a script for collecting data over a predefined grid. The following options are available.\n\n";
    print "--source: source name. If given, antennas will slew to the source before scanning.\n\n";
    print "--antenna: Antennas to use. Defaults to those used in the project.\n\n";
    print "--offsetunit: The resolution (in arcsec) of the grid. Default is 40\n\n";
    print "--x,--y: the resolution (in arcsec) for the azimuth and elevation respectively of the grid. If not provided will use offsetunit.\n\n";
    print "--beginazoff,--endazoff: The beginning and end of the azimuthal grid (in arcsec). Default is -520 to 520.\n\n";
    print "--initialeloff, --finaleloff: The beginning and end of the elevational grid (in arcsec). Default is -80 to 80.\n\n";
    print "--w: error to pass to antennawait (in arcsec). Default is 4.\n\n";
    print "--chopping: Turn chopper on. (note that this will cause the resulting plot to look like a sine wave in azimuth\n\n";
    print "--q: (sQuare) use a square grid going from -q to +q in both az and el.\n\n";
}
#main start

foreach $i ( @ARGV ) {
	if ($i eq "-h"||$i eq "--help"){
	    Usage();
	    die;
	}
    $args = $args . $i . " ";
}



GetOptions('antenna=s'=>\$antString,'source=s'=>\$source,
'offsetunit=i'=>\$offsetUnit,'beginazoff=i'=>\$beginazoff,
'endazoff=i'=>\$endazoff,'initialeloff=i'=>\$Begineloff,'finaleloff=i'=>\$Endeloff,
'x=f'=>\$azoffsetUnit,'y=f'=>\$eloffsetUnit,'w=f'=>\$antennawaitarcsec,
'q=f'=>\$square, 'chopping'=>\$chopflag);

if ($beginazoff ne "") { 
    $azminoffset = $beginazoff;
}
if ($endazoff ne "") { 
    $azmaxoffset = $endazoff;
}
if ($Begineloff ne "") { 
    $elminoffset = $Begineloff;
}
if ($Endeloff ne "") { 
    $elmaxoffset = $Endeloff;
}
if ($azoffsetUnit == 0){
    $azoffsetUnit=$offsetUnit;
}
if ($eloffsetUnit == 0){
    $eloffsetUnit=$offsetUnit;
}
if ($square ne ""){
    $square=abs($square);
    $azminoffset=-$square;
    $azmaxoffset=$square;
    $elminoffset=-$square;
    $elmaxoffset=$square;
}

if ($offsetUnit < 0 || ($offsetUnit==0 && $azoffsetUnit==0 && $eloffsetUnit==0)) {
    &Usage;
    die "offsetUnit (i.e the scan rate in arcsec/sec) should be an integer > 0.\n";
}
print "offsetUnit=$offsetUnit\n";
open(logfile,">>/rootfs/logs/SMAshLog");
$line = `date -u +"%a %b %d %X %Y"`;
chop($line);
$user = $ENV{"USER"};
printf logfile "%s ($user): $0 %s\n", $line, $args;
close(logfile);

@antennalist = ();

# if no antenna is specified, get the antennas from project command.
print "antString = $antString\n";
$spaceDelimited = `parseAntennaList $antString`;
@antennalist = sort(split(' ',$spaceDelimited));
#print "@antennalist\n";
$numberAntennas = $#antennalist+1;
printf "number of antennas = %d\n", $numberAntennas;
if ($numberAntennas == 0) {
    print "Did not see -a, will search the project\n";
    for ($i=1; $i<=8; $i++) {
	$antenna_status=`value -a $i -v antenna_status`;
	if ($antenna_status==1) { 
	    push(@antennalist,$i); 
	}       
    }
    
    $spaceDelimited = `getAntList`;
    $antString="";
    foreach $antenna (@antennalist){
	    $antString.="$antenna,";
    }
    chop $antString;
} else {
# now compare the antenna list with the project list
    print "Comparing antenna list with the current project\n";
    for ($i=1; $i<=8; $i++) {
	$antenna_status=`value -a $i -v antenna_status`;
	if ($antenna_status==1) { 
	    push(@projectlist,$i); 
	    
	}       
    }
    if ($#projectlist == $#antennalist) {
	$match = 1;
	for ($i=1; $i<8; $i++) {
	    if ($projectlist[$i] != $antennalist[$i]) {
		$match = 0;
	    }
	}
	if ($match == 1) {
	    print "The specified antennas match those in the project.  Ignoring the -a option.\n";
	    $numberAntennas = 0;
	    $spaceDelimited = `getAntList`;
	} else {
	    print "The specified antennas do not match those in the project.  Using the -a option.\n";
	}
    }
}

sub writeData() {
    print "writing data\n";
    foreach $ant (@antennalist) {
	$writeaz=`prio 50 value -a $ant -v azoff`;
	chomp $writeaz;
	$writeel=`prio 50 value -a $ant -v eloff`;
	chomp $writeel;
	$writepow=`prio 50 value -a $ant -v syncdet2_channels_v2`;
	chomp $writepow;
	open(OUTFILE, ">>/data/engineering/cpoint/ant${ant}/raster.dat");
	print OUTFILE "$writeaz $writeel $writepow\n";
	close OUTFILE;	
    }
}

sub Pause() {#this section doesn't work as it should. I can't send commands to the antennas
    print "rastermap interrupted\n";
    foreach $antenna (@antennalist) {
	print "resetting antenna $antenna\n";
	`prio 100 azoff -a $antenna -s $azoffreset[$antenna]`;
	sleep 1;
	`prio 100 eloff -a $antenna -s $eloffreset[$antenna]`;
	sleep 1;
    } # for all antennas
    if ($chopflag ne ""){
	    print "stopping the chopper\n";
	    `stopChopping -a $antString`;
    }
    print "interrupted; end\n";
    exit(0);
}


print "process id = $pid \n";
$offsetUnit=abs($offsetUnit);

#set up antennas
print "antennas to be used...\n";
foreach $antenna (@antennalist) {
    print "$antenna ";
}
print "\n";

#checks if the drives are turned off and prompts user to turn them on
$test=0;
$closedstring="";
foreach $antenna (@antennalist){
    chomp ($eldrv=`prio 50 value -a $antenna -v el_drv_state`); 
    chomp ($azdrv=`prio 50 value -a $antenna -v az_drv_state`); # CHECK THESE
    if ($azdrv*$eldrv==0){						#TWO LINES
	$test=1;
	$closedstring.=" $antenna ";
    }
    chop($closedstring);
}  
if ($test){
    print "The drives are turned off for the following antennas:$closedstring. Please turn them on before running.\n ";
    exit(1);
}else{
    print "The drives are turned on.\n";
}

if ($source ne ""){
    print "giving the observe command ($source)\n";
    `observe -a $antString -s $source`;
}
sleep 2;
`radio -a $antString`;
$azoffsetUnit = abs($azoffsetUnit);
`offsetUnit -a $antString -s $azoffsetUnit`

print "running antennaWait -a $antString -e $antennawaitarcsec\n";
`antennaWait -a $antString -e $antennawaitarcsec`;

#check if the M3 doors are closed and prompts user to open them
$test=0;
$closedstring="";
foreach $antenna (@antennalist){
    chomp ($open=`prio 50 value -a $antenna -v m3state`); 	# CHECK THESE
    if ($open==1){						#TWO LINES
	$test=1;
	$closedstring.=" $antenna ";
    }
    chop($closedstring);
}  
if ($test){
    print "The M3 doors are closed for the following antennas:$closedstring. Would you like to open them?";
    chomp ($open=<>);
    if (exists $positiveResponses{$open}) {
	print "Opening M3 doors\n";
	`openM3 -a $antString`;	#OPEN THE POD BAY DOORS HAL
    }else{
	print "Leaving M3 doors closed. WARNING data will not be useful\n";
    }
}else{
    print "The M3 doors are open.\n";
}

if ($chopflag ne ""){
    print "Chopping requested, starting the chopper.\n";
    `startChopping -a $antString`;
}else{
    print "chopping not requested\n";
}
print "running antennaWait -a $antString -e $antennawaitarcsec\n";
`antennaWait -a $antString -e $antennawaitarcsec`;

#collect original offsets (to be used in resetting)
print "collecting reset values\n";
foreach $antenna (@antennalist){
        $azoffreset[$antenna]=`prio 50 value -a $antenna -v azoff`;
	chomp($azoffreset[$antenna]);
        $eloffreset[$antenna]=`prio 50 value -a $antenna -v eloff`;
	chomp($eloffreset[$antenna]);
	`rm /data/engineering/cpoint/ant${antenna}/raster.dat`;
}
print "azoffreset=@azoffreset\n";
print "eloffreset=@eloffreset\n";
$eloff=$elminoffset;
$eloffsetUnit=abs($eloffsetUnit);
while(42){
	foreach $antenna (@antennalist){
	    print "incrementing the el and resetting the az on antenna $antenna\n"; 
	    `eloff -a $antenna -s $eloff`;
	    `azoff -a $antenna -s $azminoffset`;
	}
        print "running antennaWait -a $antString -e $antennawaitarcsec\n";
	`antennaWait -a $antString -e $antennawaitarcsec`;
	my @temp;
	foreach $antenna (@antennalist){
	    push(@temp, `prio 50 value -a $antenna -v eloff`);
	}
	@temp = sort {$a <=> $b} @temp; #want to continue the scan until ALL antennas have done the full scan so check the smallest
	if ($temp[0]>$elmaxoffset){
	    last;
	}
	print "running azscan\n";
	azScan();
	sleep 1;
	$eloff+=$eloffsetUnit;
}
#reset offsets to original values
foreach $antenna (@antennalist){
	print "resetting antenna $antenna\n";
	`eloff -a $antenna -s $eloffreset[$antenna]`;
	sleep 1;
	`azoff -a $antenna -s $azoffreset[$antenna]`;
	sleep 1;
}
if ($chopflag ne ""){
	print "stopping the chopper\n";
	`stopChopping -a $antString`;
}
print "finished\n";	
sleep 1;

#main end

sub azScan {
    `azscan -a $antString`;
    while(42){
	$start=time();
	writeData();
	my @temp;
	foreach $antenna (@antennalist){
		push(@temp, `prio 50 value -a $antenna -v azoff`);
	}
	@temp = sort {$a <=> $b} @temp; #the most recently checked will have moved farthest so we should check the largest value
	if ($temp[-1]>$maxoffset){
		last;
	}
	while ((time()-$start)<1){} #use this instead of sleep to avoid overheads
    }
    print "Antenna $antenna $scantype is completed\n";
    `stopScan -a $antString`;
}
