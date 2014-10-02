#!/usr/bin/perl -I/application/lib -w
do 'sma.pl';
#
#
############### SOURCE, CALIBRATOR and LIMITS ############## 

$inttime="15"; 
$ntarg="6";
$targ1="0418+380"; 
$targ2="0428-379";

$MINEL = 20;
$MAXEL = 85;

######################################################### 

command("radio");
command("integrate -t $inttime");
command("project -r -p 'Mark Gurwell' -d 'Elevation Effects Tests'");
print "----- initialization done, starting script -----\n";

#keep checking elevation of target to make sure it isn't up

$loop = 0; 
while($loop <= 100) {
    $loop = $loop+1;
    
#target 1

    LST(); $targel = checkEl($targ1);
    if($targel > $MINEL) {
	command("observe -s $targ1");
	command("tsys");
#	if( $loop % 2 == 1 ) {
#	    print "Updating pointing using $targ1.\n";
#	    command("ipoint -i 40 -r 1");
#	}
	command("integrate -s $ntarg -w");
    }

#target 2

    LST(); $targel = checkEl($targ2);
    if($targel > $MINEL) {
	command("observe -s $targ2");
	command("tsys");
	command("integrate -s $ntarg -w");
    }

}

print,"Okay, all done...\n";
print "\n";
