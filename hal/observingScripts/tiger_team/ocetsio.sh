#!/bin/tcsh -vf
while (1)

/application/bin/observe -s 0132-169
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 14 -w
sleep 1
/application/bin/observe -s 0238+116
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 10 -w
sleep 1
/application/bin/observe -s ocetsio
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 40 -w
sleep 1

end
