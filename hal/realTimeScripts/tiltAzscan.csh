#!/bin/csh -vf
echo antenna list = $1 $2
resume $1 $2
recordTilts $1 $2 -s azscan &
writeTilts  $1 $2 -c off
az  $1 $2 -d 270
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 280
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 290
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 300
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 310
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 330
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 340
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 350
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 0
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 10
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 20
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 30
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 40
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 50
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 60
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 70
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 80
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 90
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 100
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 110
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 120
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 130
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 140
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 150
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 160
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 170
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 180
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 190
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 200
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 210
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 220
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 230
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 240
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 250
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 260
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 270
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 280
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 290
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 300
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 310
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 320
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
az  $1 $2 -d 330
antennaWait $1 $2
sleep 10
writeTilts  $1 $2 -c on
sleep 6
writeTilts  $1 $2 -c off
antennaWait $1 $2
touch /global/killrecordTilts
