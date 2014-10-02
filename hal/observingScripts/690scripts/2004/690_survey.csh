#!/bin/tcsh -vf
# Target source: 690 survey
#  Test for 690 GHz with six antennas and full correlator
#
# Correlator: default SMA correlator configuration
#             for low spectral resolution (CO6-5) 128 chan/chunk 
# Frequency: 691.47308 GHz (USB) S14 center 
#
# Early Phase Calibrator: ???? 
# PASSBAND Calibrator:   Venus (10,000Jy) 
# Flux scale calibrator: Callisto, Ganymede 
#                        
# Amplitude reference: 
# 
# Main loop after pointing scans:
while(1)
  /application/bin/observe -s 1629a
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s 12 -w -t 30
  /application/bin/observe -s g5.89
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s 12 -w -t 30
  /application/bin/observe -s g45.07
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s  12 -w -t 30
end

