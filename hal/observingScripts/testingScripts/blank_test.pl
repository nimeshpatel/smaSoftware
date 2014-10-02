#!/usr/local/bin/perl
$ant = "1";
print "Starting the blank image test.\n";
`optical -a $ant`;
$lookup = `lookup -s test1 -r 01:00:00 -d 35:36:00 -e 2000`;
@el = split(" ", $lookup);
if($el[1] gt 20)
{
    print "Source test1 is high enough.  Taking five test blanks.\n";
    `observe -a $ant -s test1 -r 01:00:00 -d 35:36:00 -e 2000`;
    `antennaWait -a $ant -e 4`;
    `snapshot -a $ant -s 600`;
    `observe -a $ant -s test1.1 -r 01:01:00 -d 35:36:00 -e 2000`;
    `antennaWait -a $ant -e 4`;
    `snapshot -a $ant -s 600`;
    `observe -a $ant -s test1.2 -r 00:59:00 -d 35:36:00 -e 2000`;
    `antennaWait -a $ant -e 4`;
    `snapshot -a $ant -s 600`;
    `observe -a $ant -s test1.3 -r 01:00:00 -d 35:37:00 -e 2000`;
    `antennaWait -a $ant -e 4`;
    `snapshot -a $ant -s 600`;
    `observe -a $ant -s test1.4 -r 01:00:00 -d 35:35:00 -e 2000`;
    `antennaWait -a $ant -e 4`;
    `snapshot -a $ant -s 600`;
   
}
else
{
    print "Source test1 is not high enough.\n";
}
#$lookup = `lookup -s test2 -r 04:00:00 -d 29:45:00 -e 2000`;
#@el = split(" ", $lookup);
#if($el[1] gt 33)
#{
#    print "Source test2 is high enough. Taking five test blanks.\n";
#    `observe -a $ant -s test2 -r 04:00:00 -d 29:45:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test2.1 -r 04:01:00 -d 29:45:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test2.2 -r 03:59:00 -d 29:45:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test2.3 -r 04:00:00 -d 29:46:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test2.4 -r 04:00:00 -d 29:44:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;

#}
#else
#{
#    print "Source test2 is not high enough.\n";
#}
#$lookup = `lookup -s test3 -r 08:40:00 -d 45:30:00 -e 2000`;
#@el = split(" ", $lookup);
#if($el[1] gt 34)
#{
#    print "Source test3 is high enough. Taking five test blanks.\n";
#    `observe -a $ant -s test3 -r 08:40:00 -d 45:30:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test3.1 -r 08:41:00 -d 45:30:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test3.2 -r 08:39:00 -d 45:30:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test3.3 -r 08:40:00 -d 45:31:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test3.4 -r 08:40:00 -d 45:29:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;

#}
#else
#{
#    print "Source test3 is not high enough.\n";
#}
#$lookup = `lookup -s test4 -r 12:00:00 -d 37:00:00 -e 2000`;
#@el = split(" ", $lookup);
#if($el[1] gt 34)
#{
#    print "Source test4 is high enough. Taking five test blanks.\n";
#    `observe -a $ant -s test4 -r 12:00:00 -d 37:00:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test4.1 -r 12:01:00 -d 37:00:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test4.2 -r 11:59:00 -d 37:00:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test4.3 -r 12:00:00 -d 37:01:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test4.4 -r 12:00:00 -d 36:59:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;

#}
#else
#{
#    print "Source test4 is not high enough.\n";
#}
#$lookup = `lookup -s test5 -r 16:40:00 -d 44:00:00 -e 2000`;
#@el = split(" ", $lookup);
#if($el[1] gt 20)
#{
#    print "Source test5 is high enough. Taking five test blanks.\n";
#    `observe -a $ant -s test5 -r 16:40:00 -d 44:00:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test5.1 -r 16:41:00 -d 44:00:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test5.2 -r 16:39:00 -d 44:00:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test5.3 -r 16:40:00 -d 44:01:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test5.4 -r 16:40:00 -d 43:59:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;

#}
#else
#{
#    print "Source test5 is not high enough.\n";
#}
#$lookup = `lookup -s test6 -r 21:40:00 -d 31:45:00 -e 2000`;
#@el = split(" ", $lookup);
#if($el[1] gt 20)
#{
#    print "Source test6 is high enough. Taking five test blanks.\n";
#    `observe -a $ant -s test6 -r 21:40:00 -d 31:45:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test6.1 -r 21:41:00 -d 31:45:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test6.2 -r 21:39:00 -d 31:45:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test6.3 -r 21:40:00 -d 31:46:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;
#    `observe -a $ant -s test6.4 -r 21:40:00 -d 31:44:00 -e 2000`;
#    `antennaWait -a $ant -e 4`;
#    `snapshot -a $ant -s 600`;

#}
#else
#{
#    print "Source test6 is not high enough.\n";
#}
`radio -a $ant`;
