#!/bin/tcsh -vf
#
while (1)
  observe -s 2232+117
  sleep 1
  tsys
  point -i 60  -l -t -s  -r 1
  sleep 2
#
end


