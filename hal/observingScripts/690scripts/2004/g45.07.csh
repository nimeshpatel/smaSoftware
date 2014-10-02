#!/bin/tcsh -vf
# Target source: g45.07
#  Test for 690 GHz with six antennas and full correlator
#
# Correlator: default SMA correlator configuration
#             for low spectral resolution (CO6-5) 128 chan/chunk 
# Frequency: 691.47308 GHz (USB) S14 center 
#
# Main loop after pointing scans:
while(1)
#  /application/bin/observe -s neptune
  /application/bin/observe -s uranus
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s 20 -w -t 30
  /application/bin/observe -s g45.07
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s  20 -w -t 30
end
