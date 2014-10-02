#!/usr/bin/perl -w
#  
# Script for Southern Hemisphere Flux Survey
# This script is the full list that contains all strong (>0.8 Jy@230) candidates.
#
# PI: Sherry & Naomi @ ASIAA
#
# Contact Infomation
#
#   Sherry Yeh
#     Office phone:  +886-2-3365-2200 ext.817
#     Evening phone: 
#
#
#   Naomi Hirano
#     Office phone:  +886-2-3365-2200 ext.745
#     Evening phone: 
#
#
# Script made by Satoki Matsushita @ ASIAA
#        revised by Sherry Yeh, Dec 30, 2005.
############################################################
# 
$inttime=30;
$nflux=30;
$scan=10;
# Planets and Jovian Moons
#$calib[0]="uranus";
$calib[0]="callisto";
#$calib[2]="ganimede";
#$calib[3]="neptune";
# Gain Calibrators
#$calqso[0]="0423-013";
#$calqso[1]="0522-364";
#$calqso[2]="1058+015";
$calqso[0]="3c279";
$calqso[1]="1337-129";
#$calqso[5]="1743-038";
#$calqso[6]="sgra";
#$calqso[7]="1921-293";
#$calqso[7]="2145+067";
#$calqso[8]="2232+117";

# Southern hemisphere QSOs
#$qso[0]="0002-478 -r 00:04:35.640 -d -47:36:19.73 -e 2000 -v 0";
#$qso[1]="0010-401 -r 00:12:59.90986 -d -39:54:26.0553 -e 2000 -v 0";
#$qso[2]="j0026-3512 -r 00:26:16.388 -d -35:12:48.45 -e 2000 -v 0";
$qso[0]="0104-408 -r 01:06:45.107966 -d -40:34:19.960360 -e 2000 -v 0";
#$qso[4]="0112-017 -r 01:15:17.099972 -d -01:27:04.577370 -e 2000 -v 0";
#$qso[5]="0118-272 -r 01:20:31.663334 -d -27:01:24.652570 -e 2000 -v 0";
#$qso[6]="0122-003 -r 01:25:28.843788 -d -00:05:55.931800 -e 2000 -v 0";
#$qso[7]="0130-171 -r 01:32:43.487454 -d -16:54:48.521370 -e 2000 -v 0";
$qso[1]="0135-247 -r 01:37:38.34634  -d -24:30:53.882000 -e 2000 -v 0";
#$qso[9]="0202-172 -r 02:04:57.674350 -d -17:01:19.840180 -e 2000 -v 0";
#$qso[10]="0220-349 -r 02:22:56.401625 -d -34:41:28.730110 -e 2000 -v 0";
#$qso[11]="0237-233 -r 02:40:08.174510 -d -23:09:15.730850 -e 2000 -v 0";
#$qso[12]="0327-241 -r 03:29:54.075537 -d -23:57:08.773300 -e 2000 -v 0";
#$qso[13]="0332-403 -r 03:34:13.654494 -d -40:08:25.397980 -e 2000 -v 0";
#$qso[14]="0338-214 -r 03:40:35.607864 -d -21:19:31.172230 -e 2000 -v 0";
#$qso[15]="0346-279 -r 03:48:38.144561 -d -27:49:13.565300 -e 2000 -v 0";
#$qso[16]="0402-362 -r 04:03:53.749908 -d -36:05:01.913200 -e 2000 -v 0";
#$qso[17]="0405-385 -r 04:06:59.035347 -d -38:26:28.042060 -e 2000 -v 0";
#$qso[18]="0422-380 -r 04:24:42.243727 -d -37:56:20.784230 -e 2000 -v 0";
$qso[2]="0426-380 -r 04:28:40.434100 -d -37:56:19.508000 -e 2000 -v 0";
#$qso[20]="0438-436 -r 04:40:17.179992 -d -43:33:08.604060 -e 2000 -v 0";
#$qso[21]="0451-282 -r 04:53:14.646807 -d -28:07:37.327600 -e 2000 -v 0";
$qso[3]="0454-463 -r 04:55:50.772498 -d -46:15:58.68233 -e 2000 -v 0";
$qso[4]="0537-441 -r 05:38:50.361551 -d -44:05:08.939080 -e 2000 -v 0";
#$qso[24]="0537-286 -r 05:39:54.281429 -d -28:39:55.947450 -e 2000 -v 0";
#$qso[25]="0627-199 -r 06:29:23.761851 -d -19:59:19.723450 -e 2000 -v 0";
#$qso[26]="0646-306 -r 06:48:14.096441 -d -30:44:19.659400 -e 2000 -v 0";
$qso[5]="0648-165 -r 06:50:24.581852 -d -16:37:39.725000 -e 2000 -v 0";
#$qso[28]="0745-330 -r 07:47:19.68 -d -33:10:47.20 -e 2000 -v 0";
$qso[6]="0723-008 -r 07:25:50.639953 -d -00:54:56.544380 -e 2000 -v 0";
#$qso[30]="0805-077 -r 08:08:15.536036 -d -07:51:09.886340 -e 2000 -v 0";
#$qso[31]="0823-223 -r 08:26:01.572909 -d -22:30:27.202110 -e 2000 -v 0";
#$qso[32]="0826-373 -r 08:28:04.780268 -d -37:31:06.280640 -e 2000 -v 0";
$qso[7]="0834-201 -r 08:36:39.216940 -d -20:16:59.469000 -e 2000 -v 0";
#$qso[34]="0859-140 -r 09:02:16.830907 -d -14:15:30.875380 -e 2000 -v 0";
#$qso[35]="0919-260 -r 09:21:29.353874 -d -26:18:43.386040 -e 2000 -v 0";
#$qso[36]="0925-203 -r 09:27:51.824323 -d -20:34:51.232660 -e 2000 -v 0";
#$qso[37]="1032-199 -r 10:35:02.155274 -d -20:11:34.359750 -e 2000 -v 0";
$qso[8]="1034-293 -r 10:37:16.079740 -d -29:34:02.812000 -e 2000 -v 0";
#$qso[39]="1104-445 -r 11:07:08.694135 -d -44:49:07.618440 -e 2000 -v 0";
$qso[9]="1144-379 -r 11:47:01.370704 -d -38:12:11.023530 -e 2000 -v 0";
#$qso[41]="1203-262 -r 12:05:33.212335 -d -26:34:04.464570 -e 2000 -v 0";
#$qso[42]="1206-238 -r 12:09:02.445128 -d -24:06:20.759560 -e 2000 -v 0";
#$qso[43]="1243-072 -r 12:46:04.232113 -d -07:30:46.574500 -e 2000 -v 0";
$qso[10]="1244-255 -r 12:46:46.802100 -d -25:47:49.288000 -e 2000 -v 0";
#$qso[45]="1255-316 -r 12:57:59.061 -d -31:55:16.85 -e 2000 -v 0";
#$qso[46]="1256-220 -r 12:58:54.478780 -d -22:19:31.125450 -e 2000 -v 0";
#$qso[47]="1302-102 -r 13:05:33.015026 -d -10:33:19.428210 -e 2000 -v 0";
$qso[11]="1313-333 -r 13:16:07.986020 -d -33:38:59.173000 -e 2000 -v 0";
#$qso[49]="1322-427 -r 13:25:27.615544 -d -43:01:08.80365 -e 2000 -v 0";
#$qso[50]="j1354-1041 -r 13:54:46.5186 -d -10:41:02.656 -e 2000 -v 0";
#$qso[51]="1354-152 -r 13:57:11.244974 -d -15:27:28.786530 -e 2000 -v 0";
#$qso[52]="1406-076 -r 14:08:56.481200 -d -07:52:26.666330 -e 2000 -v 0";
#$qso[53]="1421-490 -r 14:24:32.28 -d -49:13:49.30 -e 2000 -v 0";
$qso[12]="1424-418 -r 14:27:56.297560 -d -42:06:19.437620 -e 2000 -v 0";
#$qso[55]="j1427-3306 -r 14:27:41.549 -d -33:05:32.01 -e 2000 -v 0";
$qso[13]="1451-375 -r 14:54:27.409764 -d -37:47:33.144830 -e 2000 -v 0";
#$qso[57]="1451-400 -r 14:54:32.912371 -d -40:12:32.514390 -e 2000 -v 0";
#$qso[58]="1511-100 -r 15:13:44.893444 -d -10:12:00.264350 -e 2000 -v 0";
$qso[14]="1514-241 -r 15:17:41.814110 -d -24:22:19.441000 -e 2000 -v 0";
#$qso[60]="1519-273 -r 15:22:37.675991 -d -27:30:10.785430 -e 2000 -v 0";
#$qso[61]="1555+001 -r 15:57:51.433973 -d -00:01:50.413710 -e 2000 -v 0";
#$qso[62]="1600-445 -r 16:04:31.024731 -d -44:41:31.909238 -e 2000 -v 0";
$qso[15]="1622-297 -r 16:26:06.019320 -d -29:51:26.999000 -e 2000 -v 0";
#$qso[64]="1657-261 -r 17:00:53.154090 -d -26:10:51.724000 -e 2000 -v 0";
#$qso[65]="1742-289 -r 17:45:40.0383 -d -29:00:28.069 -e 2000 -v 0";
$qso[16]="1759-396 -r 18:02:42.669 -d -39:40:07.97 -e 2000 -v 0";
$qso[17]="1908-201 -r 19:11:09.652920 -d -20:06:55.109000 -e 2000 -v 0";
$qso[18]="1920-211 -r 19:23:32.189815 -d -21:04:33.33308 -e 2000 -v 0";
#$qso[69]="1933-400 -r 19:37:16.217362 -d -39:58:01.553060 -e 2000 -v 0";
$qso[19]="1954-388 -r 19:57:59.819271 -d -38:45:06.356260 -e 2000 -v 0";
#$qso[71]="2005-489 -r 20:09:25.391 -d -48:49:53.72 -e 2000 -v 0";
$qso[20]="2052-474 -r 20:56:16.40 -d -47:14:47.80 -e 2000 -v 0";
#$qso[73]="2106-413 -r 21:09:33.188582 -d -41:10:20.605300 -e 2000 -v 0";
#$qso[74]="2203-188 -r 22:06:10.417082 -d -18:35:38.746520 -e 2000 -v 0";
#$qso[75]="2232-488 -r 22:35:13.237 -d -48:35:58.79 -e 2000 -v 0";
#$qso[76]="2240-260 -r 22:43:26.408781 -d -25:44:30.687300 -e 2000 -v 0";
#$qso[77]="2245-328 -r 22:48:38.686 -d -32:35:52.19 -e 2000 -v 0";
$qso[21]="2255-282 -r 22:58:05.962900 -d -27:58:21.259000 -e 2000 -v 0";
#$qso[79]="2326-477 -r 23:29:17.704 -d -47:30:19.11 -e 2000 -v 0";
#$qso[80]="2329-162 -r 23:31:38.652436 -d -15:56:57.009520 -e 2000 -v 0";
#$qso[81]="2335-027 -r 23:37:57.339089 -d -02:30:57.629370 -e 2000 -v 0";
#$qso[82]="2355-106 -r 23:58:10.882409 -d -10:20:08.611260 -e 2000 -v 0";
#
#########################################################################
# initialization

# load the sma library of functions used in this observing script.
do 'sma.pl';

# check participating antennas.
checkANT();

# just in case antennas are left in optical mode from opointing.
command("radio");

# set default integration time for the correlator.
command("integrate -t $inttime");

print "----- initialization done, starting script -----\n";

$calibnum=@calib; # number of listed quasars
$calqsonum=@calqso; # number of listed quasars
$qsonum=@qso; # number of listed quasars
$ELLLIMIT=20.0;  # Lower Elevation Limit of the SMA
$ELULIMIT=80.0;  # Upper Elevation Limit of the SMA, Be careful!!!!!


#####################################
# Start with the flux calibration   #
#####################################
print "Start with Planets and/or Jovian Moons, if it is available .....\n";
for ($i=0;$i<$calibnum;$i++) {
  printPID();
  LST();
  $sourceCoordinates=`lookup -s $calib[$i]`;
  chomp($sourceCoordinates);
  ($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
  if ($ELLLIMIT<$sourceEl && $sourceEl<$ELULIMIT && $sunDistance>30.0) {
    command("observe -s $calib[$i]");
    command("tsys");
    command("integrate -s $nflux -t $inttime -w");
  }
  else {
    print "$calib[$i] is not available now.\n";
  }
}

#########################
# First Gain Calibrator #
#########################
print "Starting main loop...................\n";
for ($i=0;$i<$calqsonum;$i++) {
  printPID();
  LST();
  $calel = checkEl($calqso[$i]);
  if ($calel > 20 and $calel < 80) { 
    command("observe -s $calqso[$i]");  
    command("tsys");
    command("integrate -s $scan -w -b");
  }
  else {
    print "$calqso[$i] is not in 20deg < EL < 80deg.\n"
  }
}


##################
# Observing Loop #
##################
$nloopend = 1;
$iqso = 1;
for($nloop=0;$nloop<$nloopend;$nloop++){
  for ($i=0;$i<$qsonum;$i++) {
    if ($iqso % 5 == 0) { # Every 5 sources, observe gain calibrators.
      for ($j=0;$j<$calqsonum;$j++) {
        printPID();
        LST();
        $calel = checkEl($calqso[$j]);
        if ($calel > 20 and $calel < 80) { 
          command("observe -s $calqso[$j]");  
          command("tsys");
          command("integrate -s $scan -w -b");
        }
        else {
          print "$calqso[$j] is not in 20deg < EL < 80deg.\n"
        }
      }
    }
    printPID();
    LST();
    $calel = checkEl($qso[$i]);
    if ($calel > 20 and $calel < 80) { 
      command("observe -s $qso[$i]");  
      command("tsys");
      command("integrate -s $scan -w -b");
      $iqso++;
    }
    else {
      chomp($qso[$i]);
      ($qsoname) = split(' ',$qso[$i]);
      print "$qsoname is not in 20deg < EL < 80deg.\n";
      if ($iqso % 5 == 0) {
        $iqso++;
      }
    }
  }
}

#########################
# Final Gain Calibrator #
#########################
for ($i=0;$i<$calqsonum;$i++) {
  printPID();
  LST();
  $calel = checkEl($calqso[$i]);
  if ($calel > 20 and $calel < 82) { 
    command("observe -s $calqso[$i]");  
    command("tsys");
    command("integrate -s $scan -w -b");
  }
  else {
    print "$calqso[$i] is not in 20deg < EL < 80deg.\n"
  }
}


##########################
# Final flux calibration #
##########################
print "Final calibration with Planets and/or Jovian Moons, if it is available .....\n";

for ($i=0;$i<$calibnum;$i++) {
  printPID();
  LST();
  $sourceCoordinates=`lookup -s $calib[$i]`;
  chomp($sourceCoordinates);
  ($sourceAz,$sourceEl,$sunDistance)=split(' ',$sourceCoordinates);
  if ($ELLLIMIT<$sourceEl && $sourceEl<$ELULIMIT && $sunDistance>30.0) {
    command("observe -s $calib[$i]");
    command("tsys");
    command("integrate -s $nflux -t $inttime -w");
  }
  else {
    print "$calib[$i] is not available now.\n";
  }
}


# bye-bye message
print "----- Congratulations! This script is done. -----\n";

