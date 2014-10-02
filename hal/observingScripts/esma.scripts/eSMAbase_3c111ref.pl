#!/usr/bin/perl -w
#
# Experiment Code: eSMAbase
# Experiment Title: Baseline script for eSMA
# PI: Glen, Taco, Remo
# Contact Person: Glen  
# Email  : gpetitpa@cfa.harvard.edu  
# Office : 961 2936  
# Home   : 982 5743   
# Array  : compact   
####################  PRIMING  ####################  
#
# restartCorrelator -d -s 32
# observe -s  -r  -d  -e 2000 -v 0
# dopplerTrack -r 230.538 -u -s12 
# setFeedOffset -f 230
#
###################   POINTING  ###################  
#
# Pointing: None requested
# Syntax Example: point -i 60 -r 3 -L -l -t -Q
#
############### SOURCE, CALIBRATOR and LIMITS ############## 
#
# Edit $cal45 (currently 3c84) to change the reference
# quasar. It does NOT loop automatically. Feel free to add
# a loop wrapper if restarting every few hours becomes a
# nuicance.
#
# Reference quasar:
#
$cal45="3c111"; $ncal45="14";
#
$cal0="bllac"; $ncal0="14";
$cal1="3c446"; $ncal1="14";
$cal2="0019+734"; $ncal2="14";
$cal3="0059+581"; $ncal3="14";
$cal4="0133+476"; $ncal4="14";
$cal5="3c454.3"; $ncal5="14";
$cal6="2230+114"; $ncal6="14";
$cal7="2255-282"; $ncal7="14";
$cal8="2345-167"; $ncal8="14";
$cal9="0235+164"; $ncal9="14";
$cal10="3c120"; $ncal10="14";
$cal11="0420-014"; $ncal11="14";
$cal12="0454-234"; $ncal12="14";
$cal13="0507+179"; $ncal13="14";
$cal14="0521-365"; $ncal14="14";
$cal15="0727-115"; $ncal15="14";
$cal16="0528+134"; $ncal16="14";
$cal17="3c84"; $ncal17="14";
$cal18="0355+508"; $ncal18="14";
$cal19="0234+285"; $ncal19="14";
$cal20="0823+033"; $ncal20="14";
$cal21="0851+202"; $ncal21="14";
$cal22="1034-293"; $ncal22="14";
$cal23="1055+018"; $ncal23="14";
$cal24="0923+392"; $ncal24="14";
$cal25="0954+658"; $ncal25="14";
$cal26="1156+295"; $ncal26="14";
$cal27="3c273"; $ncal27="14";
$cal28="1308+326"; $ncal28="14";
$cal29="1334-127"; $ncal29="14";
$cal30="1514-241"; $ncal30="14";
$cal31="1546+027"; $ncal31="14";
$cal32="1418+546"; $ncal32="14";
$cal33="1611+343"; $ncal33="14";
$cal34="1655+077"; $ncal34="14";
$cal35="nrao530"; $ncal35="14";
$cal36="1741-038"; $ncal36="14";
$cal37="1749+096"; $ncal37="14";
$cal38="1633+382"; $ncal38="14";
$cal39="3c345"; $ncal39="14";
$cal40="1751+288"; $ncal40="14";
$cal41="1803+784"; $ncal41="14";
$cal42="1800+440"; $ncal42="14";
$cal43="1849+670"; $ncal43="14";
$cal44="1928+738"; $ncal44="14";
#
$inttime="30"; 
$MINEL_GAIN = 25; $MAXEL_GAIN = 85;
#$LST_start=0; $LST_end=24;
######################################################### 

do 'sma.pl';
checkANT();
command("radio");
command("integrate -t $inttime");
#$myPID=$$;
print "----- initialization done, starting script -----\n";

LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45; 
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal0);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal0;
     command("observe -s $cal0");
#     command("tsys");
     command("integrate -s $ncal0 -w");
}
LST(); $targel=checkEl($cal1);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal1;
     command("observe -s $cal1");
#     command("tsys");
     command("integrate -s $ncal1 -w");
}
LST(); $targel=checkEl($cal2);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal2;
     command("observe -s $cal2");
#     command("tsys");
     command("integrate -s $ncal2 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal3);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal3;
     command("observe -s $cal3");
#     command("tsys");
     command("integrate -s $ncal3 -w");
}
LST(); $targel=checkEl($cal4);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal4;
     command("observe -s $cal4");
#     command("tsys");
     command("integrate -s $ncal4 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal5);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal5;
     command("observe -s $cal5");
#     command("tsys");
     command("integrate -s $ncal5 -w");
}
LST(); $targel=checkEl($cal6);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal6;
     command("observe -s $cal6");
#     command("tsys");
     command("integrate -s $ncal6 -w");
}
LST(); $targel=checkEl($cal7);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal7;
     command("observe -s $cal7");
#     command("tsys");
     command("integrate -s $ncal7 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal8);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal8;
     command("observe -s $cal8");
#     command("tsys");
     command("integrate -s $ncal8 -w");
}
LST(); $targel=checkEl($cal9);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal9;
     command("observe -s $cal9");
#     command("tsys");
     command("integrate -s $ncal9 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal10);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal10;
     command("observe -s $cal10");
#     command("tsys");
     command("integrate -s $ncal10 -w");
}
LST(); $targel=checkEl($cal11);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal11;
     command("observe -s $cal11");
#     command("tsys");
     command("integrate -s $ncal11 -w");
}
LST(); $targel=checkEl($cal12);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal12;
     command("observe -s $cal12");
#     command("tsys");
     command("integrate -s $ncal12 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal13);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal13;
     command("observe -s $cal13");
#     command("tsys");
     command("integrate -s $ncal13 -w");
}
LST(); $targel=checkEl($cal14);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal14;
     command("observe -s $cal14");
#     command("tsys");
     command("integrate -s $ncal14 -w");
}
LST(); $targel=checkEl($cal15);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal15;
     command("observe -s $cal15");
#     command("tsys");
     command("integrate -s $ncal15 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal16);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal16;
     command("observe -s $cal16");
#     command("tsys");
     command("integrate -s $ncal16 -w");
}
LST(); $targel=checkEl($cal17);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal17;
     command("observe -s $cal17");
#     command("tsys");
     command("integrate -s $ncal17 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal18);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal18;
     command("observe -s $cal18");
#     command("tsys");
     command("integrate -s $ncal18 -w");
}
LST(); $targel=checkEl($cal19);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal19;
     command("observe -s $cal19");
#     command("tsys");
     command("integrate -s $ncal19 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal20);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal20;
     command("observe -s $cal20");
#     command("tsys");
     command("integrate -s $ncal20 -w");
}
LST(); $targel=checkEl($cal21);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal21;
     command("observe -s $cal21");
#     command("tsys");
     command("integrate -s $ncal21 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal22);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal22;
     command("observe -s $cal22");
#     command("tsys");
     command("integrate -s $ncal22 -w");
}
LST(); $targel=checkEl($cal23);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal23;
     command("observe -s $cal23");
#     command("tsys");
     command("integrate -s $ncal23 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal24);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal24;
     command("observe -s $cal24");
#     command("tsys");
     command("integrate -s $ncal24 -w");
}
LST(); $targel=checkEl($cal25);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal25;
     command("observe -s $cal25");
#     command("tsys");
     command("integrate -s $ncal25 -w");
}
LST(); $targel=checkEl($cal26);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal26;
     command("observe -s $cal26");
#     command("tsys");
     command("integrate -s $ncal26 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal27);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal27;
     command("observe -s $cal27");
#     command("tsys");
     command("integrate -s $ncal27 -w");
}
LST(); $targel=checkEl($cal28);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal28;
     command("observe -s $cal28");
#     command("tsys");
     command("integrate -s $ncal28 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal29);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal29;
     command("observe -s $cal29");
#     command("tsys");
     command("integrate -s $ncal29 -w");
}
LST(); $targel=checkEl($cal30);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal30;
     command("observe -s $cal30");
#     command("tsys");
     command("integrate -s $ncal30 -w");
}
LST(); $targel=checkEl($cal31);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal31;
     command("observe -s $cal31");
#     command("tsys");
     command("integrate -s $ncal31 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal32);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal32;
     command("observe -s $cal32");
#     command("tsys");
     command("integrate -s $ncal32 -w");
}
LST(); $targel=checkEl($cal33);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal33;
     command("observe -s $cal33");
#     command("tsys");
     command("integrate -s $ncal33 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal34);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal34;
     command("observe -s $cal34");
#     command("tsys");
     command("integrate -s $ncal34 -w");
}
LST(); $targel=checkEl($cal35);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal35;
     command("observe -s $cal35");
#     command("tsys");
     command("integrate -s $ncal35 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal36);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal36;
     command("observe -s $cal36");
#     command("tsys");
     command("integrate -s $ncal36 -w");
}
LST(); $targel=checkEl($cal37);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal37;
     command("observe -s $cal37");
#     command("tsys");
     command("integrate -s $ncal37 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal38);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal38;
     command("observe -s $cal38");
#     command("tsys");
     command("integrate -s $ncal38 -w");
}
LST(); $targel=checkEl($cal39);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal39;
     command("observe -s $cal39");
#     command("tsys");
     command("integrate -s $ncal39 -w");
}
LST(); $targel=checkEl($cal40);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal40;
     command("observe -s $cal40");
#     command("tsys");
     command("integrate -s $ncal40 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal41);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal41;
     command("observe -s $cal41");
#     command("tsys");
     command("integrate -s $ncal41 -w");
}
LST(); $targel=checkEl($cal42);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal42;
     command("observe -s $cal42");
#     command("tsys");
     command("integrate -s $ncal42 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
LST(); $targel=checkEl($cal43);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal43;
     command("observe -s $cal43");
#     command("tsys");
     command("integrate -s $ncal43 -w");
}
LST(); $targel=checkEl($cal44);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal44;
     command("observe -s $cal44");
#     command("tsys");
     command("integrate -s $ncal44 -w");
}
LST(); $targel=checkEl($cal45);
if($targel < $MAXEL_GAIN and $targel > $MINEL_GAIN){
    $current_source = $cal45;
     command("observe -s $cal45");
#     command("tsys");
     command("integrate -s $ncal45 -w");
}
