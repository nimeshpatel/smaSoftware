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

2:
echo "Doing Position $1"
observe -s 3c279_2 -r 12:56:11.30 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

3:
echo "Doing Position $1"
observe -s 3c279_3 -r 12:56:10.98 -d -05:47:24.35 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

4:
echo "Doing Position $1"
observe -s 3c279_4 -r 12:56:11.17 -d -05:47:15.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

5:
echo "Doing Position $1"
observe -s 3c279_5 -r 12:56:11.55 -d -05:47:27.18 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

6:
echo "Doing Position $1"
observe -s 3c279_6 -r 12:56:10.50 -d -05:47:21.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

7:
echo "Doing Position $1"
observe -s 3c279_7 -r 12:56:11.74 -d -05:47:13.03 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

8:
echo "Doing Position $1"
observe -s 3c279_8 -r 12:56:11.17 -d -05:47:35.52 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

9:
echo "Doing Position $1"
observe -s 3c279_9 -r 12:56:10.41 -d -05:47:10.21 -e 2000 -v 0
antennaWait
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1
polarPattern -p 7 -w -c 1

