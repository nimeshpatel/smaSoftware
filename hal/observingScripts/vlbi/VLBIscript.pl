#!/usr/bin/perl

use Time::Local;
use Getopt::Long;
$Getopt::Long::autoabbrev=1;

GetOptions('filename=s'=>\$filename);

open(DATA,"$filename");

#$time = currenttime();
#print "$time\n";
#$now = gmtime($time);
#print "$now\n";
$header = 0;
$part1 = 0;
while(<DATA>)
{
    $line = $_;
    if($header == 0)
    {
	if($line =~ /\sDay/)
	{
	    #print "$line";
	    @temp = split(" ",$line);
	    $day = $temp[4];
	    $month = $temp[5];
	    $year = $temp[6];
	    if ($month eq "Jan")  {
		$month = "0";
	    }
	    if ($month eq "Feb")  {
		$month = "1";
	    }
	    if ($month eq "Mar")  {
		$month = "2";
	    }
	    if ($month eq "Apr")  {
		$month = "3";
	    }
	    if ($month eq "May")  {
		$month = "4";
	    }
	    if ($month eq "Jun")  {
		$month = "5";
	    }
	    if ($month eq "Jul")  {
		$month = "6";
	    }
	    if ($month eq "Aug")  {
		$month = "7";
	    }
	    if ($month eq "Sep")  {
		$month = "8";
	    }
	    if ($month eq "Oct")  {
		$month = "9";
	    }
	    if ($month eq "Nov")  {
		$month = "10";
	    }
	    if ($month eq "Dec")  {
		$month = "11";
	    }
	    #print "$temp[4] $temp[5] $temp[6]\n";
	}
	if($line =~ /^SCAN/)
	{
	    $header = 1;
	}
    }
    else
    {
	if($line =~ /\s\d/ && $line !~ /Day/)
	{
	    if($part1 == 0)
	    {
		@temp = split(" ",$line);
		$scan = $temp[0];
		print "The current scan is scan number $scan.\n";
		mytime();
		if($seconds > currenttime())
		{
		    $source = $temp[3];
		    if($source =~ /M87/)
		    {
			print "M87 is known as 3c274 in the SMA catalog, switching the source name.\n";
			$source = "3c274";
		    }
		    if($source =~ /SGRA/)
		    {
			print "SGRA is known as sgrastar in the SMA catalog, switching the source name.\n";
			$source = "sgrastar";
		    }
		    print "observe -s $source\n";
		    `observe -s $source`;
		    $oldseconds = $seconds;
		    $part1 = 1;
		}
		else
		{
		    print "Skipping scan number $scan, because it already occured.\n";
		}
	    }
	    else
	    {
		@temp = split(" ",$line);
		mytime();
		$wait = $seconds - currenttime() + 1;
		print "Wating $wait seconds for the scan to finish at $temp[2].\n";
		sleep($wait);
		while($seconds > currenttime())
		{
		    print "The scan hasn't finsihed, waiting 5 seconds before checking again.\n";
		    sleep(5);
		}
		$part1 = 0;
	    }
	}
    }
}
close(DATA);

sub mytime
{
    unless($temp[2] =~ /:/)
    {
	$temp[2] = $temp[1];
    }
    @temp2 = split(":",$temp[2]);
    $seconds = timegm($temp2[2],$temp2[1],$temp2[0],$day,$month,$year);
    #print "In mytime\n";
    #print "s m h d m y: $temp2[2] $temp2[1] $temp2[0] $day $month $year\n";
    #print "$temp2[2] $temp2[1] $temp2[0] $day $month $year\n";
    #$time = currenttime();
    #print "The current time:  $time\n";
    #print "The time to check: $seconds\n";
}

#this replaces the call to time, because time uses the MB clock which is different then the system clock which is synced over the network.
sub currenttime
{
    $date = `date -u`;
    #print "Here\'s the date\: $date\n";
    @temp3 = split(" ",$date);
    $cday = $temp3[2];
    $cyear = $temp3[5];
    $cmonth = $temp3[1];
    if ($cmonth eq "Jan")  
    {
	$cmonth = "0";
    }
    if ($cmonth eq "Feb")  
    {
	$cmonth = "1";
    }
    if ($cmonth eq "Mar")  
    {
	$cmonth = "2";
    }
    if ($cmonth eq "Apr")  
    {
	$cmonth = "3";
    }
    if ($cmonth eq "May")  
    {
	$cmonth = "4";
    }
    if ($cmonth eq "Jun")  
    {
	$cmonth = "5";
    }
    if ($cmonth eq "Jul")  
    {
	$cmonth = "6";
    }
    if ($cmonth eq "Aug")  
    {
	$cmonth = "7";
    }
    if ($cmonth eq "Sep")  
    {
	$cmonth = "8";
    }
    if ($cmonth eq "Oct")  
    {
	$cmonth = "9";
    }
    if ($cmonth eq "Nov")  
    {
	$cmonth = "10";
    }
    if ($cmonth eq "Dec")  
    {
	$cmonth = "11";
    }
    @temp4 = split(":",$temp3[3]);
    $h = $temp4[0];
    $m = $temp4[1];
    $s = $temp4[2];
    #print "s m h d m y: $s $m $h $cday $cmonth $cyear\n";
    $cseconds = timegm($s,$m,$h,$cday,$cmonth,$cyear);
    #print "The current number of seconds is: $cseconds\n";
    return $cseconds;
}
