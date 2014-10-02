#!/bin/tcsh -vf
#
while (1)
observe -s 0607-085
sleep 5
tsys
integrate -s 10 -w
observe -s 0423-013
sleep 5
tsys
integrate -s 10 -w
observe -s bnkl1
sleep 5
tsys
integrate -s 40 -w
end

