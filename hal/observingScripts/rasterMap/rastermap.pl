#!/usr/local/bin/perl
# rastermap iterates of el, performing an az scan at each step and running the data collector continuously
#adapted from cpoint by Kieran Finn 07/03/2012
#contact kieran.finn@hotmail.com


#Initialization
$azminoffset=-520.; #CHANGE DEFAULT VALUES HERE
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
    print "rastermap is a script for collecting data over a predefined grid. The following options are available.\n";
    print "--source: source name. If given, antennas will slew to the source before scanning.\n";
    print "--antenna: Antennas to use. Defaults to those used in the project.\n";
    print "--offsetunit: The resolution (in arcsec) of the grid. Default is 40\n"; #REMEMBER TO UPDATE THE USAGE FILE IF YOU CHANGE THE DEFAULTS
    print "--x,--y: the resolution (in arcsec) for the azimuth and elevation respectively of the grid. If not provided will use offsetunit.\n";
    print "--beginazoff,--endazoff: The beginning and end of the azimuthal grid (in arcsec). Default is -520 to 520.\n";
    print "--initialeloff, --finaleloff: The beginning and end of the elevational grid (in arcsec). Default is -80 to 80.\n";
    print "--w: error to pass to antennawait (in arcsec). Default is 4.\n";
    print "--chopping: Turn chopper on. (note that this will cause the resulting plot to look like a sine wave in azimuth\n";
    print "--q: (sQuare) use a square grid going from -q to +q in both az and el.This takes priority over –b,-e,-i and –f.\n\n";
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
'q=f'=>\$square, 'chopping'=>\$chopflag); #may want to alter this so the default is chopping

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
    die "offsetUnit should be an integer > 0.\n";
}
open(logfile,">>/rootfs/logs/SMAshLog");
$line = `date -u +"%a %b %d %X %Y"`;
chop($line);
$user = $ENV{"USER"};
printf logfile "%s ($user): $0 %s\n", $line, $args;
close(logfile);

@antennalist = ();

# if no antenna is specified, get the antennas from project command.
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
    chop $antString; #builds $antString from antennas in the project if -a option is not specified
}

sub writeData() {#takes the current azoff, eloff and continuum power (syncdet2) and writes it to the file raster.dat to be used in plotting
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

sub Pause() {#this section doesn't work as it should. I can't send commands to the antennas after a ctrl-C event. Possibly buffer issues
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
    chomp ($azdrv=`prio 50 value -a $antenna -v az_drv_state`);
    if ($azdrv*$eldrv==0){
	$test=1;
	$closedstring.=" $antenna ";
    }
    chop($closedstring);
}  
if ($test){
    print "The drives are turned off for the following antennas:$closedstring. Please turn them on before running.\n ";
    exit(1); #I tried to add an automated command `resume -a antString` here but Hal didn't like it
}else{
    print "The drives are turned on.\n";
}

if ($source ne ""){
    print "giving the observe command ($source)\n";
    `observe -a $antString -s $source`;
}
sleep 2;
`radio -a $antString`;

print "running antennaWait -a $antString -e $antennawaitarcsec\n";
`antennaWait -a $antString -e $antennawaitarcsec`;

#check if the M3 doors are closed and prompts user to open them
$test=0;
$closedstring="";
foreach $antenna (@antennalist){
    chomp ($open=`prio 50 value -a $antenna -v m3state`);
    if ($open==1){
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
	`openM3 -a $antString`;
    }else{
	print "Leaving M3 doors closed. WARNING data will not be useful\n"; #can't imagine why you'd want to run with the M3 doors closed but just in case
	#might be worth adding an option to quit at this point
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
#the actual grid is still specified by the options and is indipendant of the original offsets. These are just used to return the array to its original position after completion
print "collecting reset values\n";
foreach $antenna (@antennalist){
        $azoffreset[$antenna]=`prio 50 value -a $antenna -v azoff`;
	chomp($azoffreset[$antenna]);
        $eloffreset[$antenna]=`prio 50 value -a $antenna -v eloff`;
	chomp($eloffreset[$antenna]);
	`rm /data/engineering/cpoint/ant${antenna}/raster.dat`;
}

$eloff=$elminoffset;
$eloffsetUnit=abs($eloffsetUnit);
#MAIN LOOP
while(42){
	foreach $antenna (@antennalist){
	    print "incrementing the el on antenna $antenna\n"; 
	    `eloff -a $antenna -s $eloff`;
	}
        print "running antennaWait -a $antString -e $antennawaitarcsec\n"; #not running this here caused the following check to lag behind and an additional azscan to be run
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
	azScan($azminoffset,$azmaxoffset);
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

sub azScan {#increases the az in increments of offsetUnit and writes data at each point
#NOTE: I have a couple of ideas to speed this up if it's too slow but they are untested. try rastermap_scanning.pl or see the end of the wiki page
    my $minoffset=shift @_;
    my $maxoffset=shift @_;
    $azoffsetUnit = abs($azoffsetUnit);
    $azoff=$minoffset;
    while(42){
	foreach $antenna (@antennalist){
		print "incrementing the az on antenna $antenna\n"; 
		`azoff -a $antenna -s $azoff`;
	}
        print "running antennaWait -a $antString -e $antennawaitarcsec\n";
	`antennaWait -a $antString -e $antennawaitarcsec`;
	my @temp;
	foreach $antenna (@antennalist){
		push(@temp, `prio 50 value -a $antenna -v azoff`);
	}
	@temp = sort {$a <=> $b} @temp; #want to continue the scan until ALL antennas have done the full scan so check the smallest
	if ($temp[0]>$maxoffset){
		last;
	}
	writeData();
	$azoff+=$azoffsetUnit;
    }
}
