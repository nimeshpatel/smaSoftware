#!/bin/tcsh -vf
while (1)
observe -a 1,8 -s saturn 
wait
sleep 15
radio -a 1,8 
sleep 5
cpoint -a 1,8 -s saturn -c -u l
wait
optical -a 1,8
sleep 5
observe -a 1,8 -s hip51585
wait
sleep 10
snapshot -a 1,8  -s 200
wait
sleep 10
end
