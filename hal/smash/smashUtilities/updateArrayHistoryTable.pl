#!/usr/local/bin/perl
open(FILE,"/global/arrayHistory.html")||die "could not open array History table file to read.\n";
@origLines=<FILE>;
close(FILE);
for($i=1;$i<=8;$i++) {
$padid=`value -a $i -v pad_id`;
chomp($padid);
print "$padid ";
$pad[$i]=$padid;
}
print "\n";
print "Enter date of reconfiguration (DD Month YYYY e.g. 12 Oct 2005): ";
$date=<STDIN>;
chomp($date);
($d,$month,$year)=split(' ',$date);
open(NEWFILE,">/global/temp.html")||die "could not write to temp.html\n";
for($i=0;$i<$#origLines;$i++) {
print NEWFILE "$origLines[$i]";
}
print NEWFILE "<tr height=13>\n";
print NEWFILE "  <td height=13 class=xl27 width=27>$d</td>\n";
print NEWFILE "  <td class=xl28 width=28>$month</td>\n";
print NEWFILE "  <td class=xl27 width=35>$year</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[1]</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[2]</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[3]</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[4]</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[5]</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[6]</td>\n";
print NEWFILE "  <td class=xl28 align=right width=75>$pad[7]</td>\n";
print NEWFILE "  <td class=xl29 align=right width=75>$pad[8]</td>\n";
print NEWFILE " </tr>\n";
print NEWFILE " </table>\n";
close(NEWFILE);
$ans=`cp /global/temp.html /global/arrayHistory.html`;
if($ans ne "") {print $ans;}
