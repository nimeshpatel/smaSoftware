#!/bin/tcsh -vf
while (1)
/application/bin/observe -s nrao530 
sleep 3
tsys
/application/bin/integrate -s 15 -w
/application/bin/observe -s sgrb2n 
sleep 3
tsys
/application/bin/integrate -s 35 -w
end
