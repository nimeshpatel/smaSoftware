#!/bin/tcsh -vf
while (1)

/application/bin/observe -s 0854+201
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 8 -w
sleep 1
/application/bin/observe -s 1058+015
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 6 -w
sleep 1
/application/bin/observe -s rleo
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 40 -w
sleep 1

end
