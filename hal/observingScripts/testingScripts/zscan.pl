#!/usr/bin/perl
# usage:zscan antenna_number
# TK, 02 Oct 99; on hal9000 13 March 00.
	if($#ARGV!=3) {print "usage:zscan.pl ant_number start stop step\n";exit(1);}
        $com=`date`;
        ($n1,$n2,$n3,$n4,$n5)=split(' ',$com);
        ($hour,$min)=split(':',$n4);
        $filename="/data/engineering/holo/ant".$ARGV[0]."_".$n2.$n3."_".$hour.$min.".zscn";
        if(-e "$filename") {unlink($filename);}
        $filenameo=">".$filename;
	print "filename is $filename\n";
        open(OUT,$filenameo)|| die "can't open the file. Sorry\n";
        chmod(0666,$filename);
	$start = $ARGV[1]/1.0;
	$stop  = $ARGV[2]/1.0;
	$step  = $ARGV[3]/1.0;
	$com="value -a  ". $ARGV[0] ." -v chopper_z_counts";
	$chop_z=`$com`;
	$z_now=$chop_z;
#	sleep(1);
	$com="chopperZ -a".$ARGV[0]. " -c $start";
	print "$com\n";
 	`$com`;
#	sleep(1);
	$com="value -a  ". $ARGV[0] ." -v chopper_z_counts";
	print "$com\n";
	$chop_z=`$com`;
	chop($chop_z);
	while(abs(1.0*$chop_z - $start) >= 5.0){
	$com="value -a ". $ARGV[0] ." -v chopper_z_counts";
	print "$com\n";
	$chop_z=`$com`;
	print "$start $chop_z";
	chop($chop_z);
	select(undef,undef,undef,0.25);
#	sleep(2);
	}
	$now=time;
	$com="value -a " .$ARGV[0]. " -v actual_az_deg";
	print "$com\n";
	$az=`$com`;
	chop($az);
	$com="value -a " .$ARGV[0]. " -v actual_el_deg";
	print "$com\n";
	$el=`$com`;
	chop($el);
	$com="value -a " .$ARGV[0]. " -v chopper_x_counts";
	print "$com\n";
	$chop_x=`$com`;
	chop($chop_x);
#	sleep(1);
	$com="value -a " .$ARGV[0]. " -v chopper_y_counts";
	print "$com\n";
	$chop_y=`$com`;
	chop($chop_y);
#	sleep(1);
	$com="value -a " .$ARGV[0]. " -v chopper_tilt_counts";
	print "$com\n";
	$chop_t=`$com`;
	chop($chop_t);
#	sleep(1);
	print OUT "# $now zscan $az $el $chop_x $chop_y $chop_z $chop_t\n";
	print "# $now zscan $az $el $chop_x $chop_y $chop_z $chop_t\n";
	$chop_go=$start;
	while(abs(1.0*$chop_z - $stop) >= 5.0){
	$com="value -a ". $ARGV[0] ." -v chopper_z_counts";
	print "$com\n";
	$chop_z=`$com`;
	print $chop_z;
	chop($chop_z);
#	sleep(1);
#	$com="value -a ". $ARGV[0] ." -v total_power_volts";
	$com="/application/holo/src/chop_vvm tmp 2 1 1 1";
	print "$com\n";
	$tot_pow=`$com`;
#	chop($tot_pow);
	print OUT "$chop_z $tot_pow \n ";
	print "$chop_z $tot_pow \n ";
	$chop_go=$chop_go+$step;
	$com="chopperZ -a ".$ARGV[0]. " -c $chop_go";
	print "$com\n";
	`$com`;
	$com="value -a ". $ARGV[0] ." -v chopper_z_counts";
	$chop_z=`$com`;
	while(abs(1.0*$chop_z - 1.0*$chop_go) >= 5.0){
	print "$com\n";
	$chop_z=`$com`;
	print "$chop_go $chop_z";
	chop($chop_z);
	select(undef,undef,undef,0.25);
#	sleep(2);
	}
#	sleep(5);
	}
	close (OUT);
#	Following lines commented out because we are now writing on data on 
#       the cross mounted disk /application/data on hal9000. 13 March 00, TK.
#        $com= "scp $filename sma1:/user1/software/sma/files/ant2_99/focus";
#        system($com)==0 or die " system $com failed:$?\n"; 
#        $com= "\\rm $filename";
#        system($com)==0 or die " system $com failed:$?\n"; 
	$com="chopperZ -a ".$ARGV[0]." -c $z_now";
	system($com);
