#!/bin/tcsh -vf
while (1)
/application/bin/observe -s 1037-295
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w
/application/bin/observe -s 3c279
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 8 -w
/application/bin/observe -s twhya -r 11:01:51.88 -d -34:42:17.12 -e 2000 -v 2.8
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 30 -w
end

