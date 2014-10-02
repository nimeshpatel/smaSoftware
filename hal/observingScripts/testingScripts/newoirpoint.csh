#! /bin/csh -f

set cpointSource=$1
set ipointSource=$2
set opointSource=$3 

if($#argv<3) then
echo "Incorrect usage: Needs source list"
echo "newoirpoint.csh uranus 3c454.3 hip114939"
goto terminate
endif

echo "cpoint: $cpointSource, ipoint: $ipointSource, opoint: $opointSource"

while (1)

  integrate -t 30
  radio
  azoff -s 0
  eloff -s 0
  sleep 2 
  observe -s $cpointSource
  echo  "Slewing to cpoint source: $cpointSource"
  antennaWait 
  cpoint -s $cpointSource -c -u l
  antennaWait  
 
  azoff -s 0
  eloff -s 0
  sleep 2  
  observe -s $ipointSource
  echo  "Slewing to ipoint source: $ipointSource"
   antennaWait 
  ipoint -r 1 -Q -L
  antennaWait
 
  optical
  sleep 1 
  azoff -s 0
  eloff -s 0
  sleep 2
  observe -s $opointSource
  echo  "Slewing to opoint source: $opointSource"

  antennaWait 
  snapshot -s 200
  antennaWait
end

terminate: 
