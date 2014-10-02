#!/bin/tcsh -vf
while (1)

/application/bin/observe -s 0359+509
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 8 -w
sleep 1
/application/bin/observe -s 0721+713
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 8 -w
sleep 1
/application/bin/observe -s txcam
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 30 -w
sleep 1

end
