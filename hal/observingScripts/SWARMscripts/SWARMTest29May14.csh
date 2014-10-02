#!/bin/tcsh -vf
while (1)
/application/bin/observe -s 1625-254
sleep 3
tsys
/application/bin/integrate -s 15 -w
/application/bin/observe -s i1629a
sleep 3
tsys
/application/bin/integrate -s 35 -w
/application/bin/observe -s 1512-090
sleep 3
tsys
/application/bin/integrate -s 10 -w
end

