#!/bin/tcsh -vf
while (1)

/application/bin/observe -s 0433+053
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 12 -w
sleep 1
/application/bin/observe -s 0423-013
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 8 -w
sleep 1
/application/bin/observe -s nmltausio
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 40 -w
sleep 1

end

