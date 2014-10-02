#!/bin/tcsh
echo starting the csh lookup test
set lookup = (`lookup -s test3.4 -r 08:40:00 -d 45:29:00 -e 2000`)
echo the current elevation of test3.4 is $lookup[2]
