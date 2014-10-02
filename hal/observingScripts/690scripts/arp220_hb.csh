#!/bin/tcsh -vf

/application/bin/integrate -t 30 

while (1)

#    /application/bin/observe -s callisto
    observe -s ceres 
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 12 -w
    sleep 1
    /application/bin/observe -s 3c345
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 10 -w
    sleep 1
    /application/bin/observe -s arp220 -r 15:34:57.19 -d 23:30:11.3 -v 0 -e 2000
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 28 -w 
    sleep 1
end
