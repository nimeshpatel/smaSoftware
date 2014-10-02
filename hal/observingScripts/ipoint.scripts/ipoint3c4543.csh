#!/bin/tcsh -vf
#
while (1)
  observe -s 3c454.3 
  sleep 2
  ipoint -i 40 -r 1 -Q -s
  sleep 2
#
end


