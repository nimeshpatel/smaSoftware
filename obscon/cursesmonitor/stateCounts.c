#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "monitor.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define N_RXBOARDS 8
#define PRINT_DSM_ERRORS 0
#define N_CRATES 12

#define DHADT  (2.0 * M_PI / 86400.00)
#define POWER_HI_THRESHOLD 75.0
#define POWER_LO_THRESHOLD 55.0

extern dsm_structure crateStatusStructure[13];

void stateCounts(int count, int rx, int *crateOffset)
{
  short mode, rBMapping[9], statsOK[3][5][9];
  int doWeCare[11];
  int rm_status;
  int i, j, k, s, scanNumber, crate, nActiveCrate, crateList[N_CRATES+1];
  int lowestActiveCrate, activeCrate[N_CRATES+2];
  float counts[4][5][9][2], c2DCPower[5][9][2];
  char string1[100], crateName[10];
  time_t timestamp;
  FILE *fp;

  getAntennaList(doWeCare);
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
  lowestActiveCrate = getCrateList(&activeCrate[0]);
  nActiveCrate = 0;
  for (i = 1; i <= N_CRATES; i++) {
    if (activeCrate[i])
      crateList[nActiveCrate++] = i;
  }
  *crateOffset %= nActiveCrate;
  crate = crateList[*crateOffset];
  if ((crate < 1) || (crate > 12))
    return;
  sprintf(crateName, "crate%d", crate);
  if ((lowestActiveCrate > 0) && (lowestActiveCrate <= N_CRATES)) {
    s = dsm_read(crateName,
		 "CRATE_TO_HAL_X", 
		 &crateStatusStructure[crate],
		 &timestamp);
    if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
      dsm_error_message(s, "dsm_read - CRATE_TO_HAL_X");
    s = dsm_structure_get_element(&crateStatusStructure[crate],
				  "MODE_S", 
				  (char *)&mode);
    if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_read - MODE_S");
    if (!timestamp) {
      move(12,2);
      printw(
	     "Correlator status has not been written - correlator software is not running.");
      move(23,79);
      refresh();
      return;
    }
    s = dsm_structure_get_element(&crateStatusStructure[crate],
				  "RB_MAPPING_V9_S", 
				  (char *)&rBMapping);
    if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_structure_get_element - RB_MAPPING_V9_S");
    s = dsm_structure_get_element(&crateStatusStructure[crate],
				  "C2DC_CNTS_V4_V5_V9_V2_F", 
				  (char *)&counts);
    if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_structure_get_element - C2DC_CNTS_V4_V5_V9_V2_F");
    s = dsm_structure_get_element(&crateStatusStructure[crate],
				  "STATS_OK_V3_V5_V9_S", 
				  (char *)&statsOK);
    if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_structure_get_element - C2DC_CNTS_V4_V5_V9_V2_F");
    s = dsm_structure_get_element(&crateStatusStructure[crate],
				  "C2DC_PWR_V5_V9_V2_F", 
				  (char *)&c2DCPower);
    if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_structure_get_element - C2DC_PWR_V5_V9_V2_F");
#ifdef LINUX
    ctime_r(&timestamp, string1);
#else
    ctime_r(&timestamp, string1, 100);
#endif
    /*
    printf("\n\n\n%s\n", string1);exit(0);
    */
    move(0,0);
    printw(
	   "Crate %d State Counters on %s   ", crate, string1);
    switch (mode) {
    case -1:
      printw("Correlator Paused    ");
      break;
    case 0:
      printw("AUTOCORRELATION MODE ");
      break;
    case 1:
      printw("Crosscorrelation mode");
      break;
    default:
      printw("UNKNOWN MODE!");
    }
    move(1,0);
    if (rx == 2)
      printw("High Freq. Rx.");
    else
      printw("Low Freq. Rx. ");
    printw("                 Receiver Board");
    move(2,0);
    printw("Chunk     1        2        3        4        5        6        7        8");
    
    move(3,0);
    printw("1 -N ");
    for (i = 1; i <= N_RXBOARDS; i++) {
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][1][i])
	  standout();
	printw("%4.1f", counts[0][1][i][2-rx]);
	standend();
	printw(":");
	if ((c2DCPower[1][i][rx-1] > POWER_HI_THRESHOLD) || (c2DCPower[1][i][rx-1] < POWER_LO_THRESHOLD))
	  standout();
        printw("%3.0f", c2DCPower[1][i][rx-1]);
	standend();
      }
    }
    move(4,0);
    printw("1 -1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][1][i])
	  standout();
	printw("%4.1f", counts[1][1][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(5,0);
    printw("1 +1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][1][i])
	  standout();
	printw("%4.1f", counts[2][1][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
        printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(6,0);
    printw("1 +N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][1][i])
	  standout();
	printw("%4.1f", counts[3][1][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(8,0);
    printw("2 -N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][2][i])
	  standout();
	printw("%4.1f", counts[0][2][i][2-rx]);
	standend();
	printw(":");
	if ((c2DCPower[2][i][rx-1] > POWER_HI_THRESHOLD) || (c2DCPower[2][i][rx-1] < POWER_LO_THRESHOLD))
	  standout();
	printw("%3.0f", c2DCPower[2][i][rx-1]);
	standend();
      }
    move(9,0);
    printw("2 -1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][2][i])
	  standout();
	printw("%4.1f", counts[1][2][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(10,0);
    printw("2 +1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][2][i])
	  standout();
	printw("%4.1f", counts[2][2][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(11,0);
    printw("2 +N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][2][i])
	  standout();
	printw("%4.1f", counts[3][2][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(13,0);
    printw("3 -N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][3][i])
	  standout();
	printw("%4.1f", counts[0][3][i][2-rx]);
	standend();
	printw(":");
	if ((c2DCPower[3][i][rx-1] > POWER_HI_THRESHOLD) || (c2DCPower[3][i][rx-1] < POWER_LO_THRESHOLD))
	  standout();
	printw("%3.0f", c2DCPower[3][i][rx-1]);
	standend();
      }
    move(14,0);
    printw("3 -1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][3][i])
	  standout();
	printw("%4.1f", counts[1][3][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(15,0);
    printw("3 +1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][3][i])
	  standout();
	printw("%4.1f", counts[2][3][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(16,0);
    printw("3 +N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][3][i])
	  standout();
	printw("%4.1f", counts[3][3][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }

    move(18,0);
    printw("4 -N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][4][i])
	  standout();
	printw("%4.1f", counts[0][4][i][2-rx]);
	standend();
	printw(":");
	if ((c2DCPower[4][i][rx-1] > POWER_HI_THRESHOLD) || (c2DCPower[4][i][rx-1] < POWER_LO_THRESHOLD))
	  standout();
	printw("%3.0f", c2DCPower[4][i][rx-1]);
	standend();
      }
    move(19,0);
    printw("4 -1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][4][i])
	  standout();
	printw("%4.1f", counts[1][4][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(20,0);
    printw("4 +1 ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][4][i])
	  standout();
	printw("%4.1f", counts[2][4][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
    move(21,0);
    printw("4 +N ");
    for (i = 1; i <= N_RXBOARDS; i++)
      if ((rBMapping[i] < 1) || (rBMapping[i] > N_ANTENNAS))
	printw("  ------ ");
      else {
	printw(" ");
	if (!statsOK[rx][4][i])
	  standout();
	printw("%4.1f", counts[3][4][i][2-rx]);
	standend();
	printw("    ");
	/*
	printw(":");
	if (rms[0][1][i][2-rx] > rmsThreshold) {
	  standout();
	}
	printw("%3.1f", rms[0][1][i][2-rx]);
	standend();
	*/
      }
  }
  move(23,0);
  printw("+: cycle through crates, C: return to Crate mode, R: toggle Rx");
  move(23,79);
  refresh();
}

