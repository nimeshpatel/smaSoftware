#!/bin/tcsh -vf
#
# Script for CO 6-5 observations of IRAS16293-2422
# 
# Contact: Tyler Bourke or Shige
# Office:  617 496-7619
# Cell:    617 642-0682
# 
# Source: 1629A
# Coordinates: 16:32:22.910 -24:28:35.520 (J2000) VLSR 4.6
#  Visible 30 deg from 4am until end of observations (HST)
#
# 690 GHz calibrator: ceres 10 Jy, 0.5"  (15h12, -08d)
# 230 GHz calibrator: 1743-038 (2.2 Jy, 27deg)
#                     ceres is expected to be 3 Jy unresolved at 230 GHz
#
# Passband and flux: 1 hour on Ganymede
#		     Jupiter's limb (as for Taco's project)
#
# 230GHz tunning: -t CO2-1 -s16
# 690GHz tunning: -t CO6-5 -s12
# observe -s i1629a -r 16 32 22.91 -d -24 28 35.52 -e 2000 -v 4.6 
# dopplerTrack -t CO2-1 -s14 -R h -t CO6-5 -s12
#
# restartCorrelator -s128 -R h -s128 - not needed, same as Taco
#
# *******************************************************************
#
# Please start with 60 min on Ganymede
# Then do Jupiter's limb, making sure for baselines where 
# there is little or no flux from Ganymede there is some flux
# from Jupiter's limb.  use limbOffset command, and try different
# parts of the limb.  
#
# *******************************************************************
#
while (1)
  observe -s ceres
  sleep 2
  tsys
  integrate -s 10 -w -t 30
#
  observe -s i1629a -r 16:32:22.91 -d -24:28:35.52 -e 2000 -v 4.6 
  sleep 2
  tsys 
  integrate -s 30 -w -t 30
#
  observe -s 1743-038
  sleep 2
  tsys 
  integrate -s 10 -w -t 30
#
end
#
# Please end with a final gain cal on 1743-038
#
