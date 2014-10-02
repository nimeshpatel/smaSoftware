#define DEBUG 0
#include <curses.h>
#include <math.h>
#ifdef LINUX
#include <bits/nan.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "dsm.h"
#include "monitor.h"
#include "astrophys.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0
#include "dDSCursesMonitor.h"

double chunkBases[24] = {4.008e9, 4.090e9, 4.166e9, 4.248e9,
			 4.336e9, 4.418e9, 4.494e9, 4.576e9,
			 4.664e9, 4.746e9, 4.822e9, 4.904e9,
			 4.992e9, 5.074e9, 5.150e9, 5.232e9,
			 5.320e9, 5.402e9, 5.478e9, 5.560e9,
			 5.648e9, 5.730e9, 5.806e9, 5.880e9};
extern dsm_structure dDSStatusStructure;

void yIGFrequencies(int count, int rx)
{
  short gunn[N_RECEIVERS+1], pLLHarm[2];
  int s, quit;
  int line, i, j, offset;
  int doubleBandwidth;
  double trackingFrequency[N_RECEIVERS+1], yIGFreq, antYIGFreq[2];
  double iF2Low, iF2High, lastDelay[N_ANTENNAS+1], delaySum;
  time_t timestamp, unixTime;

  if (DEBUG) {
    /*    printw("rx=%d\n",rx);*/
    refresh();
  }
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
  doubleBandwidth = isDoubleBandwidth();
  s = dsm_read("newdds",
	       "DDS_TO_HAL_X", 
	       &dDSStatusStructure,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS || 1)
      dsm_error_message(s, "dsm_read - DDS_TO_HAL_X");
    quit = TRUE;
    return;
  }
  s = dsm_structure_get_element(&dDSStatusStructure,
                                "DEL_V11_D",
                                (char *)&lastDelay);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      dsm_error_message(s, "dsm_structure_get_element - DEL_V11_D");
    }
    quit = 1;
    return;
  }
  delaySum = 0.0;
  for (i = 1; i <= 8; i++)
    delaySum += lastDelay[i];
  unixTime = time(NULL);
  move(0,0);
  clear();
  if ((unixTime - timestamp > 3) || (delaySum == 0.0)) {
    move(12,1);
    printw(
	   "This page doesn't work unless you have an active project with the DDS running.");
    move(23,79);
    refresh();
    return;
  }
  if (rx == 0) {
    printw(
	   "YIG Frequency Choice Page - Low Frequency Receiver          ( +/- to change Rx )");
  } else if (rx == 1) {
    printw(
	   "YIG Frequency Choice Page - High Frequency Receiver         ( +/- to change Rx )");
  } else {
    printw("        strange receiver number = %d ",rx);
  }
  move(2,0);
  s = dsm_structure_get_element(&dDSStatusStructure,
                                "FREQ_V3_D",
                                (char *)&trackingFrequency);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - DSM_DDS_FREQ_V3_D");
    quit = 1; return;
  }
  printw("LO Frequency %f GHz", trackingFrequency[rx+1]*1.0e-9);
  move(3,0);
  s = dsm_structure_get_element(&dDSStatusStructure,
				"GUNN_V3_S",
				(char *)&gunn);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_structure_get_element - GUNN_V3_S");
    quit = 1; return;
  }  
  printw("Multiplier following the gunn: x%d", gunn[rx+1]);

  s = dsm_read("hal9000",
               "DSM_AS_IFLO_GPLL_N_V2_S",
               (char *)&pLLHarm,
               &timestamp);
  move(4,0);
  if (doubleBandwidth) {
    s = dsm_read("hal9000",
		 "DSM_AS_IFLO_MRG_V2_D",
		 (char *)&antYIGFreq,
		 &timestamp);
    iF2Low = 6.0e9 + 2.0*(antYIGFreq[1] - 6.0e9);
    iF2High = iF2Low + 2.0e9;
    printw("Double Bandwidth Mode, IF2 covers %4.2f to %4.2f GHz", iF2Low*1.0e-9, iF2High*1.0e-9);
  } else {
    printw("Not Double Bandwidth Mode");
  }
  move(6,10);
  printw("Possible YIG Freq.    PLL Harm     Warnings");
  line = 0;
  for (i = 6; i <= 22; i += 2) {
    yIGFreq = ((trackingFrequency[rx+1]/((double)gunn[rx+1])) - 0.109e9)/((double)i);
    if ((yIGFreq >= 4.0e9) && (yIGFreq <= 9.5e9)) {
      if (i == pLLHarm[rx]) {
	move(8+line, 0);
	printw("Current -->");
      }
      move(8+line, 15);
      printw("%f", yIGFreq*1.0e-9);
      if (line == 0) {
	move(8+line, 24);
	printw("GHz");
      }
      move(8+line, 36);
      printw("%d", i);
      if (yIGFreq < 6.0e9) {
	move(8+line, 45);
	printw("In ASIC IF1");
	offset = 0;
	for (j = 0; j < 24; j++) {
	  if ((chunkBases[j] < yIGFreq) && (chunkBases[j]+0.104e9 > yIGFreq)) {
	    move(8+line, 58+5*offset);
	    printw("s%02d", j+1);
	      offset++;
	  }
	}
	if (fabs(yIGFreq-antYIGFreq[1]) < 500.0e6) {
	  move(8+line, 57+5*offset);
	  printw("close to YIG2");
	}
      }
      if (doubleBandwidth && (iF2Low <= yIGFreq) && (iF2High >= yIGFreq)) {
	move(8+line, 45);
	printw("In ASIC IF2");
	offset = 0;
	for (j = 0; j < 24; j++) {
	  if ((iF2Low+chunkBases[j]-4.0e9 < yIGFreq) && (iF2Low+chunkBases[j]+0.104e9-4.0e9 > yIGFreq)) {
	    move(8+line, 58+5*offset);
	    printw("s%02d", j+25);
	      offset++;
	  }
	}
	if (fabs(yIGFreq-antYIGFreq[1]) < 500.0e6) {
	  move(8+line, 57+5*offset);
	  printw("close to YIG2");
	}
      }
      line++;
    }
  }
  return;
}
