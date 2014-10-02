#!/bin/tcsh -vf
#
while (1)
  observe -s 3c454_RL -r 22:53:57.748 -d 16:08:53.563 -e 2000 
  sleep 3
  tsys
  sleep 3
  rotateWaveplate -s L
  rotateWaveplate -a 2,3 -s R
  integrate -s 10 -w
  observe -s 3c454 -r 22:53:57.748 -d 16:08:53.563 -e 2000
  sleep 3
  tsys
  sleep 4
  rotateWaveplate -s L
  integrate -s 40 -w
  sleep4 
#
end


