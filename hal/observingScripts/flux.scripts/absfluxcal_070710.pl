#!/usr/bin/perl -w
#
#
############### SOURCE, CALIBRATOR and LIMITS ############## 

$inttime="15"; 
$ntarg="10";
$targ1="uranus"; 
$targ2="neptune";
$targ3="2253+161";
$targ4="mars";
$targ5="bllac";
$targ6="mwc349";

$MINEL = 20;
$MAXEL = 85;

######################################################### 

do 'sma.pl';
checkANT();
command("radio");
command("integrate -t $inttime");
$myPID=$$;
command("project -r -p 'Mark Gurwell' -d 'Flux Cal Tests'");
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
	if( $loop % 2 == 1 ) {
	    print "Updating pointing using $targ1.\n";
	    command("ipoint -i 40 -r 1");
	}
	command("integrate -s $ntarg -w");
    }

#target 2

    LST(); $targel = checkEl($targ2);
    if($targel > $MINEL) {
	command("observe -s $targ2");
	command("tsys");
	command("integrate -s $ntarg -w");
    }

#target 3

    LST(); $targel = checkEl($targ3);
    if($targel > $MINEL) {
	command("observe -s $targ3");
	command("tsys");
	command("integrate -s $ntarg -w");
    }


#target 4

    LST(); $targel = checkEl($targ4);
    if($targel > $MINEL) {
	command("observe -s $targ4");
	command("tsys");
	command("integrate -s $ntarg -w");
    }

#target 5

    LST(); $targel = checkEl($targ5);
    if($targel > $MINEL) {
	command("observe -s $targ5");
	command("tsys");
	command("integrate -s $ntarg -w");
    }
#target 6

    LST(); $targel = checkEl($targ6);
    if($targel > $MINEL) {
	command("observe -s $targ6");
	command("tsys");
	command("integrate -s $ntarg -w");
    }

}

print,"Okay, all done...\n";
print "\n";
