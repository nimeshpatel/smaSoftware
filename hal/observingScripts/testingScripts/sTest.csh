#!/bin/tcsh -vf
integrate -t 15
while (1)
  set el1 = `lookup -s 1733-130 | gawk '{print $2}'`
  set el3 = `lookup -s 1751+096 | gawk '{print $2}'`
  set t1 = `fcomp $el1 20.0`
  set t3 = `fcomp $el3 20.0`
  if (($t1 > 0) && ($t3 > 0)) then
    /application/bin/observe -s 1733-130
    sleep 5
    tsys
    /application/bin/integrate -s 7 -w
    /application/bin/observe -s 1751+096
    sleep 5
    tsys
    /application/bin/integrate -s 7 -w
  endif
end

