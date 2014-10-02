#!/bin/tcsh -vf

/application/bin/integrate -t 30 

while (1)

    /application/bin/observe -s titan
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 14 -w
    sleep 1
    /application/bin/observe -s 0851+202
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 14 -w
    sleep 1
    /application/bin/observe -s IRC+10216
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 14 -w
    sleep 1
end
