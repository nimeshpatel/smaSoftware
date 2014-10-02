#!/usr/local/bin/perl

#####################################################
$filetype="vvmdat";
$delay=1;
#$timeout=2400; # (2 hours* 3600seconds/3 secondspersleep)
#####################################################
 
$timer=0;
$temp=0; 			# used for number of files.
$latest_mtime=0;		# used for file modification time.

$startedGnuPlot=0;

while(1)			# begin infinite loop.
{
	@a=<*.vvmdat>;	        # slurp in all the file names in this area.

	if($#a > $temp)
	{	# a new file has been added?

				# then get the file statistics and find the
				# latest file!

         for($i=0;$i<=$#a;$i++) {  
	 ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
		$atime,$mtime,$ctime,$blksize,$blocks)
		= stat $a[$i];
              	if($mtime > $latest_mtime){$index=$i; $latest_mtime=$mtime;}
	 }

			        # if this new file is a fits file, 		
				# wait while the file is still being written,
				# pausing every second.

if($startedGnuPlot==1) {
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
				
        if($a[$index] =~ m/$filetype$/) 
	{
print "OK";

#	do{
#	  $latest_mtime=$mtime;
#	  sleep 1;
#	  ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
#		$atime,$mtime,$ctime,$blksize,$blocks)
#		= stat $a[$index];
#	   } while($mtime > $latest_mtime);


if($startedGnuPlot==0) {

	open(GNUPLOT,">gnuplot.macro");
	print GNUPLOT "
T=1
loop=1
plot '$a[$index]' u (\$0):(\$9) with linespoints
pause T
if (loop==1) reread
";

	close(GNUPLOT);
}

	$timer=0;	# reset timer 

		}  # if loop for file-type check
	} # if loop for new file

if($startedGnuPlot==0) {
$gnuplotPid=open(F,"|gnuplot gnuplot.macro&");
$gnuplotPid++;
print "gnuplot pid = $gnuplotPid\n";
$startedGnuPlot=1;
}

	$temp=$#a;		# reset the number of files.
	sleep 1;		
	$timer++;

#	if($timer>$timeout) {die "sf2.pl going to bed.\n";}

} # while loop				
