#!/usr/local/bin/perl

$throw_num=5;

@allfiles=<ant5.azscan.03Jun04_*.rpoint>;
@sortedfiles = sort {$a <=> $b} @allfiles;
foreach $file (@sortedfiles) {
push(@azfiles,$file);
}

@allfiles=<ant5.elscan.03Jun04_*.rpoint>;
@sortedfiles = sort {$a <=> $b} @allfiles;
foreach $file (@sortedfiles) {
push(@elfiles,$file);
}

$ifile=0;
foreach $file (@azfiles) {
$elfile=$elfiles[$ifile];
print "$file, throw_num: $throw_num\n";
flush;
$throw_num=dofile($file,"..\/rm_bad_gnuplot_az.macro",$throw_num);
#`cp tempplotfile2 $file.despiked690`;
`cp tempplotfile2 $file.despiked`;
#$answer=<stdin>;
#sleep(20);
wait;
print "$elfile, throw_num: $throw_num\n";
flush;
$throw_num=dofile($elfile,"..\/rm_bad_gnuplot_el.macro",$throw_num);
#print "$file.despiked690\n";
#`cp tempplotfile2 $elfile.despiked690`;
`cp tempplotfile2 $elfile.despiked`;
#$answer=<stdin>;
#sleep(20);
wait;
$ifile++;
}

sub dofile(){
$file1=@_[0];
$macro=@_[1];
$throw_num=@_[2];
print "$file1 $macro\n";
flush;
`cp $file1 original_file`;
$answer=1;
$throw_num=5;
while($answer != 0){
spikeFilter3($file1,tempplotfile,$throw_num);
# second pass below throws out 2 points 
spikeFilter3(tempplotfile, tempplotfile2,2);
 wait;
 system("gnuplot $macro");
 wait;
 print "new throw_num:";
flush;
 $answer=<stdin>;
 if ($answer >= 0) {$throw_num=$answer;}
 else {
#     `cp tempplotfile tmp.dat`;
      return ($throw_num);
 }
}
}

sub spikeFilter3(){
$infile=@_[0];
$outfile=@_[1];
$throw_num=@_[2];
print "$infile $outfile $throw_num\n";
flush;
` head -2 $infile > $outfile`;
`grep -v "#" $infile > temp_file`;
#`awk '(NR == 1) {old=\$7} (NR!=1) {printf "%s %f %f \\n", \$0,NR, (\$7-old); old=\$7}' temp_file | sort -n -k 13  > diffs_srt`;
`awk '(NR == 1) {old=\$6} (NR!=1) {printf "%s %f %f \\n", \$0,NR, (\$6-old); old=\$6}' temp_file | sort -n -k 13  > diffs_srt`;
flush;
open(TMPFILE, diffs_srt);
@all=<TMPFILE>;
close TMPFILE;
$stop=$#all-$throw_num;
open(OUTFILE, '>', temp_file2);
for($i=$throw_num;$i<=$stop;$i=$i+1){
        print OUTFILE $all[$i];
}
close INFILE;
close OUTFILE;
`sort -n -k 12 temp_file2 > temp_file3`;
`awk '{print \$1,\$2,\$3,\$4,\$5,\$6,\$7,\$8,\$9,\$10,\$11}' temp_file3 >> $outfile`;
`tail -1 $infile >> $outfile`;
}
