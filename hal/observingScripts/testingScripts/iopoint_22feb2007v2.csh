#!/bin/tcsh -vf
while (1)
radio
sleep 3
observe -s 3c273
sleep 15
ipoint -r 1 -Q -s
optical
sleep 3
observe -s hip60172
sleep 15
snapshot -a 2..8 -s 400
sleep 3
snapshot -a 2..8 -s 400
sleep 3
snapshot -a 2..8 -s 400
sleep 3
snapshot -a 2..8 -s 400
sleep 3
snapshot -a 2..8 -s 400
sleep 3
end
