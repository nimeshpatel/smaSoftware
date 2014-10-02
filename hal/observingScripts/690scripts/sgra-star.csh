#!/bin/tcsh -vf
#
# Script for 690 observations of Sgr A*
# 
# Contact: Dan Marrone
# Office:  617 495-4142
# Cell:    617 792-9427
# Home:    617 666-2337
# 
# Source:  Sgr A*  
#           Transit on 2/16 UT: 820am HST
#           Rise above 20 deg: 455am HST
#           Rise above 30 deg: 600am HST
# Coordinates:  17:45:40.041 -29:00:28.118 (J2000.0)
#                     NOTE: SIMBAD COORDINATES ARE WRONG!
# 
# Passband/flux: Callisto (or Ceres)
# 
# 230 GHz calibrators:
#       Primary       Sgr A* (0deg away, 2-4 Jy)
#       Seondary      nrao530 (16deg away, 1.5Jy)
#                     1924-292 (22deg away, 5Jy)  <-- ~2h behind Sgr A*
#       Tertiary      Sgr B2(N)
# 690 GHz calibrators: 
#       Primary       Callisto 69deg away
#                     Ceres 41deg away
#       Secondary     Sgr A*
#       Tertiary      Sgr B2(N)
#                       
# Frequency setup:
#    Looking for line-free regions of the spectrum. 
#    Todd has indicated that the 230 receivers will be at the SiO(5-4) 
#       line, which should be fine.
#    At 690, my proposal recommended: 
#                dopplerTrack -r 689 -u -s13
#       based on the mixer current plot and the atmospheric transmission. 
#       LO 684 is labeled as Arp220(LSB) on the plot, so perhaps this is a 
#       convenient choice.
# 
# Correlator setup:  low-resolution for continuum
#    perhaps:   restartCorrelator -s16 -R h -s16
# 
# 
# Of course, I'm not tied to any of these times or sequences. Please make
# corrections based on your experience in the last month at 690.
# 


/application/bin/integrate -t 30 

while (1)
    ######################################################################
    # Ceres is closer, but didn't look as good in the Arp220 track as Callisto
    # Perhaps with better weather Ceres will be good enough.
#    /application/bin/observe -s ganymede
#    /application/bin/observe -s ceres
#    sleep 1
#    /application/bin/tsys
#    sleep 1
#    /application/bin/integrate -s 8 -w
#    sleep 1
    ######################################################################
    # Source - not to be commented out
    /application/bin/observe -s sgrastar
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 10 -w 
    sleep 1
    ######################################################################
    # nrao530 is closer, but much weaker    
    # 1924-292 is much later, but may require shorter cycles when it's up
#    /application/bin/observe -s nrao530
    /application/bin/observe -s 1924-292
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 6 -w
    sleep 1
    ######################################################################
    # Experimental calibrator
    /application/bin/observe -s sgrb2n
    sleep 1
    /application/bin/tsys
    sleep 1
    /application/bin/integrate -s 5 -w 
    sleep 1
end
