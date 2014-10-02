#!/bin/tcsh -vf
while (1)
  /application/bin/observe -s 3c84
  sleep 5
  tsys
  /application/bin/integrate -s 10 -w
  /application/bin/observe -s 3c111
  sleep 5
  tsys
  /application/bin/integrate -s 6 -w
  observe -s IRAS4 -r 03:26:04.8 -d 31:03:14 -v 7.5 -e 1950
  sleep 5
  tsys
  /application/bin/integrate -s 50 -w
end

