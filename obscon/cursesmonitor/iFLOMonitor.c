#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "dDSCursesMonitor.h"
#include "blocks.h"

#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 1

extern int quit;
extern dsm_structure mRGControl;

void iFLODisplay(int count)
{
  CLIENT *blockscl;
  blksCommand dummy;
  blksInfo *fromBlocks1, *fromBlocks2;
  
  int s, i, rx, rxInUse[2], antennaInArray[11], rxInArray[3];
  short pLLHarm[2], gunnN[2], sideband[2];
  short vSystem;
  double restFrequency[2], vOffset[2], fOffset[2], catVelo, earthOrbit, 
    earthRot, mRGFreq[2], antYIGFreq[2], vRadial, rA, dec, residuals[2];
  char string1[100], string2[100], string3[100], string4[100];
  char lastDopplerTrack[100];
  char ant2YIG[2][11];
  time_t timestamp;
  time_t system_time;

  if ((count % 60) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
#ifdef LINUX
    clear();
#endif
    move(1,1);
    refresh();
  }
  getAntennaList(&antennaInArray[0]);
  getReceiverList(&rxInArray[0]);
  move(0,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_REST_FR_V2_D",
	       (char *)&restFrequency,
	       &timestamp);
  if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "dsm_read - DSM_AS_IFLO_REST_FR_V2_D");
      quit = TRUE;
  }
  strcpy(string1, asctime(gmtime(&timestamp)));
  string1[strlen(string1)-1] = (char)0;
  system_time = time(NULL);
  printw("                   IF/LO parameters at %s", asctime(gmtime(&system_time)));
  move(2,0);
  printw("DopplerTrack parameters (issued ");
  printAgeNoStandout(system_time,timestamp);
  printw("ago):");
  move(3,0);
  s = dsm_read("hal9000",
	       "DSM_LAST_DOPPLERTRACK_C100",
	       lastDopplerTrack,
	       &timestamp);
  if (s != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "dsm_read - DSM_LAST_DOPPLERTRACK_C100");
      quit = TRUE;
  }
  if (present(lastDopplerTrack,"doppler")) {
    printw("%s", strstr(lastDopplerTrack, "dopplerTrack")+13);
  } else {
    printw("command: wacko ");
  }
  move(4,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_SOUR_C34", 
	       (char *)&string3,
	       &timestamp);
  if(s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_AS_IFLO_SOUR_C34");
    quit = 1; return;
  }
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_RA_D", 
	       (char *)&rA,
	       &timestamp);
  if(s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_AS_IFLO_RA_D");
    quit = 1; return;
  }
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_DEC_D", 
	       (char *)&dec,
	       &timestamp);
  if(s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_AS_IFLO_DEC_D");
    quit = 1; return;
  }
  rad2HHMMSS(rA, 0, 2, string1);
  rad2DDMMSS(dec, 1, 1, 1, string2);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_VELO_D",
	       (char *)&catVelo,
	       &timestamp);
  printw("Tracking %s at  RA %s Dec %s  Vel ", string3, string1, string2);
  if (fabs(catVelo) < 300000000) {
    printw("%.2f", catVelo/1000.0);
  } else {
    standout();
    printw("wacko");
    standend();
  }
  move(5,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_VRADIAL_D",
	       (char *)&vRadial,
	       &timestamp);
  printw("Radial Vel:  %9.5f  ", vRadial*1.0e-3);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_EORBIT_D",
	       (char *)&earthOrbit,
	       &timestamp);
  printw("Earth Orbit:  ");
  if (fabs(earthOrbit) < 300000) {
    printw("%9.5f  ", earthOrbit*1.0e-3);
  } else {
    standout();
    printw("wacko");
    standend();
  }
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_EROTAT_D",
	       (char *)&earthRot,
	       &timestamp);
  strcpy(string1, asctime(gmtime(&timestamp)));
  string1[strlen(string1)-1] = (char)0;
  printw("Earth motion:  %9.5f  ",
	 earthRot*1.0e-3);
  move(6,0);
  printw("Updated at %s", string1);

  move(8,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_MRG_V2_D",
	       (char *)&antYIGFreq,
	       &timestamp);
  s = dsm_read("m5",
               "MRG_CONTROL_X",
               (char *)&mRGControl,
               &timestamp);
  s = dsm_structure_get_element(&mRGControl,
				"REQ_FREQ_V2_D",
				(char *)&mRGFreq);
  printw("MRG1 ");
  if (mRGFreq[0] != mRGFreq[0])
    printw("(Not in use)   YIG1 (Not in use)");
  else {
    if (mRGFreq[0] > 9.5e9 || mRGFreq[0] < 5.799999e9) {
      standout();
    }
    if (fabs(mRGFreq[0]) > 9e10) {
      printw("wacko");
    } else {
      printw("%12.9f", mRGFreq[0]*1.0e-9);
    }
    standend();
    printw("   YIG1 ");
    if (antYIGFreq[0] > 9.4e9 || antYIGFreq[0] < 6.0e9) {
      standout();
    }
    printw("%12.9f", antYIGFreq[0]*1.0e-9);
    standend();
  }
  printw("    MRG2 ");
  if (mRGFreq[1] != mRGFreq[1]) {
    printw("(Not in use)   YIG2 (Not in use)");
  } else {
    if (mRGFreq[1] > 9.5e9 || mRGFreq[1] < 5.79999e9) {
      standout();
    }
    if (fabs(mRGFreq[1]) > 8.0e12) {
      printw("wacko");
    } else {
      printw("%12.9f", mRGFreq[1]*1.0e-9);
    }
    standend();
    printw("   YIG2 ");
    if (antYIGFreq[1] > 9.4e9 || antYIGFreq[1] < 6.0e9) {
      standout();
    }
    printw("%12.9f", antYIGFreq[1]*1.0e-9);
    standend();
  }
  move(10,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_GUNN_N_V2_S",
	       (char *)&gunnN,
	       &timestamp);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_GPLL_N_V2_S",
	       (char *)&pLLHarm,
	       &timestamp);
  printw("Gunn 1 ");
  if (mRGFreq[0] != mRGFreq[0])
    printw("  (Not in use)  ");
  else {
    printw(" Mult %d      PLL Harm %d", gunnN[0], pLLHarm[0]);
  }
  printw("          Gunn 2 ");
  if (mRGFreq[1] != mRGFreq[1])
    printw("  (Not in use)  ");
  else {
    printw(" Mult %d      PLL Harm %d", gunnN[1], pLLHarm[1]);
  }
  move(9,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_RESIDUALS_V2_D",
	       (char *)&residuals,
	       &timestamp);
  printw("Res1 ");
  if (mRGFreq[0] != mRGFreq[0])
    printw("(Not in use)");
  else {
    if (fabs(residuals[0]) < 100) {
      printw("%12.9f ", residuals[0]);
    } else {
      standout();
      printw("wacko");
      standend();
      printw("        ");
    }
  }
  printw("                       ");
  printw("Res2 ");
  if (mRGFreq[1] != mRGFreq[1])
    printw("(Not in use)");
  else {
    if (fabs(residuals[1]) > 8e12) {
      printw("wacko ");
    } else {
      printw("%12.9f ", residuals[1]);
    }
  }
  move(11,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_ANT2YIG_V2_V11_B",
	       (char *)&ant2YIG,
	       &timestamp);
  printw("Antennas  ");
  if (mRGFreq[0] != mRGFreq[0])
    printw("  (Not in use)");
  else {
    int ii;

    for (ii = 1; ii < 11; ii++)
      if (ant2YIG[0][ii] && antennaInArray[ii])
	printw("%d ", ii);
  }
  move(11,41);
  printw("Antennas  ");
  if (mRGFreq[1] != mRGFreq[1])
    printw("  (Not in use)");
  else {
    int ii;

    for (ii = 1; ii < 11; ii++)
      if (ant2YIG[1][ii] && antennaInArray[ii])
	printw("%d ", ii);
  }

  move(13,0);
  printw("                Low Freq Rx                     High Freq Rx");
  move(14,0);
  printw("Rest Frequency  ");
  if ((restFrequency[0] == restFrequency[0]) && rxInArray[1]) {
    rxInUse[0] = 1;
    printw("%9.5f                       ", restFrequency[0]*1.0e-9);
  } else {
    rxInUse[0] = 0;
    printw("-----                           ");
  }
  if ((restFrequency[1] == restFrequency[1]) && rxInArray[2]) {
    rxInUse[1] = 1;
    printw("%9.5f", restFrequency[1]*1.0e-9);
  } else {
    rxInUse[1] = 0;
    printw("-----      ");
  }
  move(15,0);
  s = dsm_read("hal9000", "DSM_AS_IFLO_SIDEBAND_V2_S", 
	       (char *)&sideband, &timestamp);
  printw("Sideband        ");
  if (rxInUse[0] && rxInArray[1]) {
    switch (sideband[0]) {
      /* the following signs look wrong, but I believe they are correct
       * because somehwere else in the MRG code, the sign is used to compute
       * the LO frequency from the RF frequency, so if a line is in USB, you
       * have to subtract the IF from the RF,  etc. - Todd */
    case  1: printw("Lower"); break;
    case -1: printw("Upper"); break;
    default: printw("Wacko");
    }
  } else
    printw("-----");
  printw("                           ");
  if (rxInUse[1] && rxInArray[2]) {
    switch (sideband[1]) {
      /* the following signs look wrong, but I believe they are correct
       * because somehwere else in the MRG code, the sign is used to compute
       * the LO frequency from the RF frequency, so if a line is in USB, you
       * have to subtract the IF from the RF,  etc.  - Todd */
    case  1: printw("Lower"); break;
    case -1: printw("Upper"); break;
    default: printw("Wacko");
    }
  } else
    printw("-----");
  move(16,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_VOFFSET_V2_D",
	       (char *)&vOffset,
	       &timestamp);
  printw("Velo Offset     ");
  if (rxInUse[0] && rxInArray[1]) {
    if (fabs(vOffset[0]) < 300000000) {
      printw("%7.5f", vOffset[0]);
    } else {
      standout();
      printw(" wacko ");
      standend();
    }
  } else {
    printw("-----  ");
  }
  printw("                         ");    
  if (rxInUse[1] && rxInArray[2])
    printw("%7.5f", vOffset[1]);
  else
    printw("-----  ");
  move(17,0);
  s = dsm_read("hal9000",
	       "DSM_AS_IFLO_FOFFSET_V2_D",
	       (char *)&fOffset,
	       &timestamp);
  printw("Freq Offset    ");
  if (rxInUse[0] && rxInArray[1]) {
    if (fabs(fOffset[0]) < 1.0e11) {
      printw("%8.5f", fOffset[0]*1.0e-9);
    } else {
      standout();
      printw(" wacko  ");
      standend();
    }
  } else {
    printw(" -----  ");
  }
  printw("                        ");    
  if (rxInUse[1] && rxInArray[2])
    printw("%8.5f", fOffset[1]*1.0e-9);
  else
    printw(" -----  ");
  move(18,0);
  printw("Gunn Freq.      ");
  if (rxInUse[0] && rxInArray[1])
    printw("%12.8f", antYIGFreq[0]*1.0e-9*pLLHarm[0]+0.109);
  else
    printw("-----       ");
  printw("                    ");    
  if (rxInUse[1] && rxInArray[2])
    printw("%.8f", antYIGFreq[1]*1.0e-9*pLLHarm[1]+0.109);
  else
    printw("-----  ");

  if (!(blockscl = clnt_create("blocks1.rt.sma", BLKSPROG, BLKSVERS, "tcp"))) {
    fprintf(stderr, "Error connecting to blocks1 computer - will continue.\n");
    clnt_pcreateerror("blocks1");
  } else {
    fromBlocks1 = blksinquiry_1(&dummy, blockscl);
    clnt_destroy(blockscl);
  }
  move(20,0);
  if (fromBlocks1 != NULL) {
    printw("Block LOs\t1: %4.0f MHz\t2: %4.0f MHz\t3: %4.0f MHz",
	   fromBlocks1->lO1Frequency[0]/1.0e6,
	   fromBlocks1->lO1Frequency[1]/1.0e6,
	   fromBlocks1->lO1Frequency[2]/1.0e6);
    move(21,0);
    printw("\t\t4: %4.0f MHz\t5: %4.0f MHz\t6: %4.0f MHz",
	   fromBlocks1->lO1Frequency[3]/1.0e6,
	   fromBlocks1->lO1Frequency[4]/1.0e6,
	   fromBlocks1->lO1Frequency[5]/1.0e6);
#ifndef LINUX
    free(fromBlocks1);
#endif
  } else
    printw("**** BLOCKS1 COMPUTER NOT RESPONDING! ****");
  if (!(blockscl = clnt_create("blocks2.rt.sma", BLKSPROG, BLKSVERS, "tcp"))) {
    fprintf(stderr, "Error connecting to blocks2 computer - will continue.\n");
    clnt_pcreateerror("blocks2");
  } else {
    fromBlocks2 = blksinquiry_1(&dummy, blockscl);
    clnt_destroy(blockscl);
  }
  move (22,0);
  if (fromBlocks2 != NULL) {
    printw("Chunk LOs\t1: %4.0f MHz\t2: %4.0f MHz\t3: %4.0f MHz\t4: %4.0f MHz",
	   fromBlocks2->lO2Frequency[0]/1.0e6,
	   fromBlocks2->lO2Frequency[1]/1.0e6,
	   fromBlocks2->lO2Frequency[2]/1.0e6,
	   fromBlocks2->lO2Frequency[3]/1.0e6);
#ifndef LINUX
    free(fromBlocks2);
#endif
  } else
    printw("**** BLOCKS2 COMPUTER NOT RESPONDING! ****");
  move(23,0);
  refresh();
  return;
}
