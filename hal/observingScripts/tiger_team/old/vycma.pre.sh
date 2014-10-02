#!/bin/tcsh -vf
while (1)
  /application/bin/observe -s callisto
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s 15 -w -t 60
  /application/bin/observe -s vycma
  sleep 1
  /application/bin/tsys
  /application/bin/integrate -s 20 -w -t 60
end

