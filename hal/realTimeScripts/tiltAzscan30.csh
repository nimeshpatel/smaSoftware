#!/bin/csh -vf
echo antenna list = $1 $2 
resume $1 $2
recordTilts $1 $2 -s azscan &
writeTilts  $1 $2 -c off
az  $1 $2 -d 270
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 300
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 330
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 0
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 30
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 60
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 90
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 120
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 150
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 180
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 210
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 240
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 270
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 300
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 330
antennaWait $1 $2
sleep 3
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
sleep 6
touch /global/killrecordTilts
