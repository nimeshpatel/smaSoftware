#!/bin/tcsh -vf
while (1)
/application/bin/observe -s uranus 
sleep 3
tsys
/application/bin/integrate -s 30 -w
/application/bin/observe -s 3c454.3
sleep 3
tsys
/application/bin/integrate -s 30 -w
end
