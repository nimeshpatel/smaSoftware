#!/bin/tcsh -vf
#
while (1)
  observe -s "0530+135" 
  sleep 2
  ipoint -i 60 -r 1 -Q -s
  sleep 2
#
end


