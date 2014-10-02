#!/bin/tcsh -vf
# Target source: Sgr A*
# Science goal:  SgrA*  at 690 GHz
#
# Correlator: default SMA correlator configuration
#             for low spectral resolution (Continuum)
# Frequency: 690.0 GHz (USB) avoiding strong spectral lines.
#
# 
# Main loop after pointing scans:
while(1)
  /application/bin/observe -s sgrb2 
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s 10 -w -t 60
  /application/bin/observe -s sgra-star
#  sleep 1
#  /application/bin/tsys
#  /application/bin/integrate -s  10 -w -t 60
#  /application/bin/observe -s 1629a 
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s  10 -w -t 60
  /application/bin/observe -s neptune 
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s  10 -w -t 60
end

