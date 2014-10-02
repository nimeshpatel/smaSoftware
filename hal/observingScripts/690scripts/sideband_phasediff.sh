#!/bin/tcsh -vf
while(1)
   observe -s 3c111 
   sleep 2
   tsys
   integrate -s 10 -t 60 -w
#   observe -s 0854+201
#   sleep 2
#   integrate -s 10 -t 60 -w
#   observe -s 0234+285
#   sleep 2 
#   tsys
#   integrate -s 10 -t 60 -w
#   observe -s 3c84
#   tsys
#   sleep 2
#   tsys
#   integrate -s 10 -t 60 -w
   observe -s 0420-014
   tsys
   sleep 2
   integrate -s 10 -t 60 -w
   observe -s 3c273
   sleep 2
   integrate -s 10 -t 60 -w
   observe -s 1055+018
   sleep 2
   tsys
   integrate -s 10 -t 60 -w
   observe -s 3c279
   sleep 2
   integrate -s 10 -t 60 -w
end
