#!/bin/tcsh -vf

/application/bin/observe -s jupiter 
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1
/application/bin/observe -s callisto 
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1

/application/bin/observe -s jupiter 
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1
/application/bin/observe -s callisto 
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1
 
/application/bin/observe -s jupiter 
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1
/application/bin/observe -s callisto 
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1
 
/application/bin/observe -s jupiter
sleep 1
/application/bin/tsys
sleep 1
/application/bin/integrate -t 30 -s 20 -w
sleep 1

end
