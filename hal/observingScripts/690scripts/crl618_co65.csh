#!/bin/tcsh -vf
#
# Script for CO 6-5 observations of CRL 618
# 
# Contact: Taco
# Office:  617 495-7330
# Home:    617 547-6068
# 
# Source: CRL 618
# Coordinates: 04:42:53.67  36:06:53.2 (J2000), Vlsr -20 km/s
#    very low dec source, observable from 2 pm to 2 am(HST),  
#
# 690 GHz calibrator: Titan, Callisto
# 230 GHz calibrator: 3c111
#
# Passband: Callisto, Jupiter's limb
#
# 230GHz tunning: -t CO2-1 -s12
# 690GHz tunning: -t CO605 -s12
# observe -s crl618
# dopplerTrack -t co2-1 -s12 -R h -t CO6-5 -s12
#
# restartCorrelator -s128 -R h -s128
while (1)
  observe -s 3c111
  sleep 2
  tsys
  integrate -s 10 -w -t 30
#
  observe -s crl618
  sleep 2
  tsys 
  integrate -s 30 -w -t 30
#
  observe -s 3c111
  sleep 2
  tsys
  integrate -s 10 -w -t 30
#
  observe -s crl618
  sleep 2
  tsys 
  integrate -s 30 -w -t 30
#
  observe -s titan
  sleep 2
  tsys 
  integrate -s 10 -w -t 30
#
end
