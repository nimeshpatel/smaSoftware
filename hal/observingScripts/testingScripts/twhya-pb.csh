#!/bin/tcsh -vf
while (1)
/application/bin/observe -s callisto
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w
/application/bin/observe -s ganymede
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w
/application/bin/observe -s europa
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w
/application/bin/observe -s io
sleep 5
tsys
tune -a 8 -c 'hotload out'
/application/bin/integrate -s 10 -w

end

