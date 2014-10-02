#!/bin/tcsh -vf
#
while (1)
observe -s 0449+113
sleep 5
tsys
integrate -s 10 -w
observe -s 0409+122 -r 04:09:22.0 -d +12:17:39.847 -e 2000 -v 0
sleep 5
tsys
integrate -s 10 -w
observe -s 0510+180
sleep 5
tsys
integrate -s 10 -w
observe -s 3c120
sleep 5
tsys
integrate -s 10 -w
observe -s  0530+135  
sleep 5
tsys
integrate -s 10 -w

end

