#!/bin/tcsh -vf
#
# 690 GHz observations of Orion-KL
#
# Source : Orion-KL
# Coordinates : 05:35:14.505 -05:22:30.63 (J2000), Vlsr 5
#
# Primary calibrator  : Titan
# Secondary Cal.      : Ceres
# Potential third Cal.: 0420-014
# Passband            : Mars or others ??? 
# flux                : Uranus or others ???
#
# The source is observable from 1-10 LST 
# 0420-014 can be used for pointing
#
while (1)
  observe -s titan
  tsys
  integrate -s 5 -w -t 60 

  observe -s ceres
  tsys
  integrate -s 5 -w -t 60

# observe -s 0420-014
# tsys
# integrate -s 5 -w -t 60

  observe -s orion-kl
  tsys
  integrate -s 15 -w -t 60
end


