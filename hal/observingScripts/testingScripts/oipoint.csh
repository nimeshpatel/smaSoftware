#! /bin/csh -f


set ipointSource=$1
set opointSource=$2 

if($#argv<2) then
echo "Incorrect usage: Needs source list"
echo "oipoint.csh 3c454.3 hip114939"
goto terminate
endif

echo "ipoint: $ipointSource, opoint: $opointSource"

while (1)

  radio
  sleep 2  
  observe -s $ipointSource
  echo  "Slewing to ipoint source: $ipointSource"
   antennaWait 
  ipoint -r 1 -Q -L
  antennaWait
 
  optical
  sleep 2
  observe -s $opointSource
  echo  "Slewing to opoint source: $opointSource"

  antennaWait 
  snapshot -s 400
  sleep 5 
  antennaWait
end

terminate: 
