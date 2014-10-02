#!/bin/tcsh -vf
#
# Script for continuum & C18O(6-5) observations toward IRAS 07427-2400 
# (G240.31+0.07)  (borrow orion_co6-5.csh style)
#
# Contact: Vivien Chen 
# Email  hchen@asiaa.sinica.edu.tw
# Office +886-2-3365-2200 x 752
# Home   +886-2-2301-7422
#
# Source : G240
# Coordinates : (07:44:52.0,-24:07:42.6) (J2000) v= 67.5 km/s 
#
# 690 GHz calibrator: H2O masers in vycma, titan 
# 230 GHz calibrator: vycma, titan
# Passband and flux: At least two hours on callisto           
#
# The source is observable from LST 4-11
#
# How is the dopplerTrack command for two receivers working?
# 
# 690 GHz: 658.553273 GHz, USB, chunk 13 for C18O(6-5) at G240
# 230 GHz: 215.595950 GHz, USB, chunk 13 for SiO masers of vycma
#
# This setting puts H2O masers of vycma at 658 GHz in chunk 9 and
# SiO masers at 216 GHz in chunk 13 if vlsr=22 km/s is assumed.
# 
# Tuning syntax is probably like this:
#       dopplerTrack -r 215.595950 -u -s13 -R h -r 658.553273 -u -s13
#
# Correlator setup standard with 128 channels and 0.825MHz channel resolution
#       restartCorrelator -s128 -R h -s128
#
# Observation strategy:
# Regarding the source loop, we would like to try 5 mins on vycma 
# and 15 mins on G240.  Besides, we would like to observe titan for
# 10 mins every hour.  If the observers think the integration for each 
# target is inappropriate, please feel free to edit the script. 

while (1)
#  @ i = 1
#  while ( $i < 4 )
#
    observe -s titan
    sleep 1
    tsys
    integrate -s 10 -t 30 -w
#
    observe -s 0736+017
    sleep 1
    tsys
    integrate -s 8 -t 30 -w
#
    observe -s vycma 
    sleep 1
    tsys
    integrate -s 16 -t 30 -w
#
    observe -s g240
    sleep 1
    tsys
    integrate -s 16 -t 30 -w
#
#    @ i = $i + 1
#  end
#
end


