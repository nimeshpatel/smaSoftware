#!/usr/local/bin/perl
# this script extracts lines from /rootfs/logs/SMAshLog
# between project and endProject commands (including these commands)
# and writes them to a file in the same area where the science data
# of the project gets written.
# This script is run by endProject but can be run by itself as well.

# first create a shorter smashlog file since the original file
# might be too huge. Just extracting last 2000 lines into a temp file
$response=`tail -2000 /rootfs/logs/SMAshLog > tSMAshLog`;
if($response ne "") { die "$response\n";}

# opening this tempfile and reading in all the lines
open(TLOG,"tSMAshLog");
@logLines=<TLOG>;
close(TLOG);

# getting the line numbers corresponding to project and endProject
# commands for extraction.
$endProjectLineNumber=0;
for($i=0;$i<=$#logLines;$i++) {
   if($logLines[$i]=~/endProject/) { 
	if($i>$endProjectLineNumber) {
	$endProjectLineNumber=$i;
	}
   }
}
$projectLineNumber=0;
for($i=0;$i<=$#logLines;$i++) {
   if($logLines[$i]=~/project/) { 
	if(!($logLines[$i]=~/\-r/)) {
	   if($i>$projectLineNumber) {
	   $projectLineNumber=$i;
	   }
	}
   }
}

#getting the science-data-file directory from DSM.
#and writing the extracted log file.
$directory=`readDSM -v DSM_AS_FILE_NAME_C80 -m m5 -l`;
chomp($directory);
$hour=(gmtime)[2];
$minute=(gmtime)[1];
$seconds=(gmtime)[0];
$day=(gmtime)[3];
$month=(gmtime)[4]+1;
$year=(gmtime)[5];
$year+=1900;
$timeStamp=sprintf("%4d%02d%02d_%02d%02d%02d",$year,$month,$day,$hour,$month,$seconds);
$logFileName="SMAshLog"."_".$timeStamp;

open(LOGFILE,">$directory/$logFileName")||die "Could not create  log file.";
for($i=$projectLineNumber;$i<=$endProjectLineNumber;$i++) {
$k=$i+1;
print LOGFILE  "$k $logLines[$i]";
}
close(LOGFILE);
