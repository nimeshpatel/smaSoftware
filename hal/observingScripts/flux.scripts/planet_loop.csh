#!/bin/tcsh -vf
#
while (1)
  observe -s titan
  tsys
  integrate -s 12 -w
  observe -s iapetus
  tsys
  integrate -s 12 -w 
#
end


