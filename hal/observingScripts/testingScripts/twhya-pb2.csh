#!/bin/tcsh -vf
while (1)
/application/bin/observe -s 3c279
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w
/application/bin/observe -s jupiter
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w
end
