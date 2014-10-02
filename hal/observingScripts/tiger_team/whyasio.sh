#!/bin/tcsh -vf
while (1)

/application/bin/observe -s 1246-257
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 10 -w
sleep 1
/application/bin/observe -s 1337-129
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 8 -w
sleep 1
/application/bin/observe -s whyasio
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 30 -w
sleep 1

end
