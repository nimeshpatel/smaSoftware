#!/bin/tcsh -vf
#
# Script for CO(6-5) observations toward Orion-KL
#
# Contact: Henrik Beuther, hbeuther@cfa.harvard.edu
# Office 617 496 7647
# Home   781 646 0421
#
# Source : Orion-KL
# Coordinates : 05:35:14.505 -05:22:30.45 (J2000), Vlsr 8.0
#
# 690 GHz calibrator: titan
# 230 GHz calibrator: 0607-157 (1.14Jy@1mm, 13 degree from Orion), one could
#                     also use 0528+134
# Passband and flux: At least two hours on callisto           
#
# The source is observable from 1.2-10 LST 
#
# How is the dopplerTrack command for two receivers working?
# For the 690 observations we like to set the receiver to 691.45 GHz in the
# USB and chunk 21. For the 230 GHz receiver, we like to use the standard CO
# setup at 230.55 GHz in the USB and chunk 16. If I understand the 
# dopplerTrack syntax correctly, it should be:
#                  dopplerTrack -r 230.55 -u -s16 -R h -r 691.45 -u -s 21
#
# Correlator setup standard with 128 channels and 0.825MHz channel resolution
#                  restartCorrelator -s128 -R h -s128
#
# Regarding the source loop, 5 minutes on titan and the quasar appears ok,
# but I am not exactly sure whether 20 minutes on the source are too long.
# If the observers think that the loop should be shorter, please edit the 
# minutes of the source
while (1)
  observe -s titan
  sleep 1
  tsys
  integrate -s 12 -w -t 30
#
  observe -s 0607-157
  sleep 1
  tsys
  integrate -s 10 -w -t 30
#
  observe -s orion-kl -r 05:35:14.505 -d -05:22:30.45 -e 2000 -v 8.0
  sleep 1
  tsys
  integrate -s 32 -w -t 30
#
end

# Please make sure to end with a final gain calibration on Titan and 0607!




















