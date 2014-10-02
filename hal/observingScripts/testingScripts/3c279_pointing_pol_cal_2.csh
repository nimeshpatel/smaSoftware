#! /bin/csh -f
#
goto $1


1:
echo "Doing Position $1"
observe -s 3c279_1 -r 12:56:11.17 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

10:
echo "Doing Position 10"
observe -s 3c279_10 -r 12:56:11.84 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
observe -s 3c279_1 -r 12:56:11.17 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 8 -w -c 1

11:
echo "Doing Position 11"
observe -s 3c279_11 -r 12:56:11.17 -d -05:47:27.52 -e 2000 -v 0
antennaWait
tsys
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
observe -s 3c279_1 -r 12:56:11.17 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 8 -w -c 1

12:
echo "Doing Position 12"
observe -s 3c279_12 -r 12:56:11.34 -d -05:47:18.69 -e 2000 -v 0
antennaWait
tsys
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
observe -s 3c279_1 -r 12:56:11.17 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 8 -w -c 1

13:
echo "Doing Position 13"
observe -s 3c279_13 -r 12:56:10.79 -d -05:47:15.86 -e 2000 -v 0
antennaWait
tsys
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
observe -s 3c279_1 -r 12:56:11.17 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1

