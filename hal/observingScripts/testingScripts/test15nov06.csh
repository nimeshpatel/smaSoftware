#!/bin/csh -f
echo "Turning off tilt correction."
tol -c off
sleep 1
echo "Observing $1."
observe -s $1
sleep 1
echo "Waiting for all antennas to reach the source."
antennaWait
echo "Pointing..."
point -D -i $2 -r 1 -L -l -t -k
sleep 2
tol -c on
sleep 2
echo "Turning on tilt correction."
echo "Pointing..."
point -D -i $2 -r 1 -L -l -t -k
sleep 2
echo "Done."
