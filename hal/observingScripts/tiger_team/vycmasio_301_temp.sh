#!/bin/tcsh -vf
while (1)
/application/bin/observe -s 0739+016
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 8 -w
sleep 1
/application/bin/observe -s vycmasio
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 40 -w
sleep 1

end
