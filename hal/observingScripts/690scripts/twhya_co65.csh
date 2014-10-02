#!/bin/tcsh -vf
#
# Script for CO 6-5 observations of TW Hya
# 
# Contact: Charlie Qi
# Office:  617 495-7087
# Cell:    617 519-7720
# 
# Source: TW Hya
# Coordinates: 11:01:51.88  -34:42:17.2 (J2000), Vlsr 2.8km/s
#    very low dec source, observable from 10:30pm to 4am(HST),  
#
# 690 GHz calibrator: callisto
# 230 GHz calibrator: 1037-295(0.7jy, 7.3deg), 1246-257(1.44, 24deg), 
#                     1058+015(1.88jy, 36deg)
#     note here: if 1037-295 is too weak to get fringe, please try 1246-257
#
# Passband and flux: at least 2 hours on Callisto
#
# 230GHz tunning: -r 216.1126 -l -s18
# 690GHz tunning: -r 691.4731 -u -s19
# observe -s twhya -r 11:01:51.88 -d -34:42:17.2 -e 2000 -v 2.8
# dopplerTrack -r 216.1126 -l -s18 -R h -r 691.4731 -u -s19
#
# restartCorrelator -s128 -R h -s128
while (1)
  observe -s 1037-295
  sleep 2
  tsys
  integrate -s 12 -w -t 30
#
  observe -s twhya -r 11:01:51.88 -d -34:42:17.2 -e 2000 -v 2.8
  sleep 2
  tsys 
  integrate -s 30 -w -t 30
#
  observe -s callisto
  sleep 2
  tsys 
  integrate -s 10 -w -t 30
#
end
