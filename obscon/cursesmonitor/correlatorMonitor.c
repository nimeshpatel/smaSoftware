#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "dDSCursesMonitor.h"
#define N_ANTENNAS 10
#define N_RECEIVERS 2
#define PRINT_DSM_ERRORS 0

#define DHADT  (2.0 * M_PI / 86400.00)

extern dsm_structure crateStatusStructure[13];

void correlatorDisplay(int count, int *crateOffset)
{
  short mode, iRIGStatus, rBMapping[9], del[9][2], pmtr[9][2],
    cb_status[9], bsln[9][2];
  int s, i, scanNumber, setScanNo, plotErrors, idlErrors, cntr_errors[9],
    stat_errors[9], crate, nActiveCrate, crateList[13];
  int lowestActiveCrate, activeCrate[14];
  float c2DCPwr[5][9][2];
  double scanTime, scanLength, pPCTemp, c2DCAttn[5][9][2], hourAngle,
    c2dc_rates[3][9][2], rates[9];
  char string1[100], plotServer[40], idlServer[40], string4[100],
    crateName[10];
  time_t timestamp;
  
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
  for (i = 1; i < 13; i++)
    if (activeCrate[i])
      crateList[nActiveCrate++] = i;
  if (nActiveCrate > 0) {
    *crateOffset %= nActiveCrate;
    crate = crateList[*crateOffset];
    sprintf(crateName, "crate%d", crate);
    if ((lowestActiveCrate > 0) && (lowestActiveCrate < 13)) {
      s = dsm_read(crateName,
		   "CRATE_TO_HAL_X", 
		   &crateStatusStructure[crate],
		   &timestamp);
      if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_read - CRATE_TO_HAL_X");
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "SCAN_NO_L", 
				    (char *)&scanNumber);
      if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_read - SCAN_NO_L");
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "MODE_S", 
				    (char *)&mode);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - MODE_S");
      }
      if (!timestamp) {
	move(12,2);
	printw(
	       "Correlator status has not been written - correlator software is not running.");
	move(23,79);
	refresh();
	return;
      }
      strcpy(string1, asctime(gmtime(&timestamp)));
      string1[strlen(string1)-1] = (char)0;
      move(0,0);
      printw(
	     "Correlator Crate %d Status on %s   ", crate, string1);
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
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "SCAN_TIME_D", 
				    (char *)&scanTime);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - SCAN_TIME_D");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "SCAN_LENGTH_D", 
				    (char *)&scanLength);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - SCAN_LENGTH_D");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "SET_SCAN_NO_L", 
				    (char *)&setScanNo);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - SET_SCAN_NO_L");
      }
      printw("Scan #%d, time so far %5.1f/%5.1f, ",
	     scanNumber, scanTime, scanLength);
      if (setScanNo == 1)
	printw("1 more scan remains for this source ");
      else if (setScanNo == -1)
	printw("scans will be taken indefinitely    ");
      else if (setScanNo > 1)
	printw("%d more scans remain for this source",
	       setScanNo);
      else
	printw("Scans are not currently being stored.");
      move(2,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "PLOT_SERVER_C40", 
				    plotServer);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - PLOT_SERVER_C40");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "PLOT_ERRORS_L", 
				    (char *)&plotErrors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - ERRORS_L");
      }
      printw("Servers: Plot %s", plotServer);
      if (plotErrors < 0)
	printw(" (inactive)");
      else {
	printw(" (active)  ");
	if (plotErrors == 1)
	  printw(" %d error", plotErrors);
	if (plotErrors > 1)
	  printw(" %d errors", plotErrors);
      }
      move(2,45);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "IDL_SERVER_C40", 
				    idlServer);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - IDL_SERVER_C40");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "IDL_ERRORS_L", 
				    (char *)&idlErrors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - ERRORS_L");
      }
      printw("Data %s", idlServer);
      if (idlErrors < 0)
	printw(" (inactive)");
      else {
	printw(" (active)  ");
	if (idlErrors == 1)
	  printw(" %d error", idlErrors);
	if (idlErrors > 1)
	  printw(" %d errors", idlErrors);
      }
      move(3,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "PPC_TEMP_D", 
				    (char *)&pPCTemp);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - PPC_TEMP_D");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "IRIG_STATUS_S", 
				    (char *)&iRIGStatus);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - IRIG_STATUS_S");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "HOUR_ANGLE_D", 
				    (char *)&hourAngle);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - HOUR_ANGLE_D");
      }
      printw("PPC Temperature %3.0fC   IRIG Status ", pPCTemp);
      if (iRIGStatus & 1)
	printw("  locked,");
      else
	printw("UNLOCKED,");
      if (!(iRIGStatus & 0x100))
	printw("reference ok   ");
      else
	printw("REFERENCE ERROR");
      rad2HHMMSS(hourAngle, 1, 2, string1);
      printw(" HA: %s", string1);
      move(4,0);
      printw("--------------------------------------------------------------------------------");
      move(5,0);
      printw("RB        1        2        3        4        5        6        7        8");
      move(6,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "RB_MAPPING_V9_S", 
				    (char *)&rBMapping);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - RB_MAPPING_V9_S");
      }
      printw("Ant     ");
      for (i = 1; i < 9; i++)
	if (rBMapping[i] < 1)
	  printw(" NONE    ");
	else if (rBMapping[i] > 8)
	  printw(" GARBAGE ");
	else
	  printw("  %1d      ", rBMapping[i]);
      move(7,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "DEL_V9_V2_S", 
				    (char *)&del);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - DEL_V9_V2_S");
      }
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "PMTR_V9_V2_S", 
				    (char *)&pmtr);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - PMTR_V9_V2_S");
      }
      printw("Dly/Pm");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1) || (rBMapping[i] > 8))
	  printw("  ------ ");
	else
	  printw(" %3d/%1d   ", del[i][1], pmtr[i][1]);
      move(8,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "C2DC_ATTN_V5_V9_V2_D", 
				    (char *)&c2DCAttn);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	dsm_error_message(s, "dsm_read - C2DC_ATTN_V5_V9_V2_D");
      }
      {
	int ii, jj, kk;
	
	for (ii = 0; ii < 5; ii++)
	  for (jj = 0; jj < 9; jj++)
	    for (kk = 0; kk < 2; kk++)
	      if ((c2DCAttn[ii][jj][kk] < 0.0) || (c2DCAttn[ii][jj][kk] > 100.0))
		c2DCAttn[ii][jj][kk] = -1.0;
      }
      printw("AtL 1/2");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1) || (rBMapping[i] > 8))
	  printw(" ------  ");
	else
	  printw(" %2.0f/%2.0f   ", c2DCAttn[1][i][0], c2DCAttn[2][i][0]);
      move(9,0);
      printw("AtL 3/4");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1)  || (rBMapping[i] > 8))
	  printw(" ------  ");
	else
	  printw(" %2.0f/%2.0f   ", c2DCAttn[3][i][0], c2DCAttn[4][i][0]);
      move(10,0);
      printw("AtH 1/2");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1) || (rBMapping[i] > 8))
	  printw(" ------  ");
	else
	  printw(" %2.0f/%2.0f   ", c2DCAttn[1][i][1], c2DCAttn[2][i][1]);
      move(11,0);
      printw("AtH 3/4");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1)  || (rBMapping[i] > 8))
	  printw(" ------  ");
	else
	  printw(" %2.0f/%2.0f   ", c2DCAttn[3][i][1], c2DCAttn[4][i][1]);
      move(12,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "STAT_ERRORS_V9_L", 
				    (char *)&stat_errors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - STAT_ERRORS_V9_L");
      }
      printw("Cnt Err");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1) || (rBMapping[i] > 8))
	  printw(" ------  ");
	else {
	  if (stat_errors[i] > 99999999) {
	    printw("  wacko  ");
	  } else {
	    printw("  %5d  ", stat_errors[i]);
	  }
	}
      move(13,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "C2DC_RATES_V3_V9_V2_D", 
				    (char *)&c2dc_rates);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - C2DC_RATES_V3_V9_V2_D");
      }
      printw("Fr Rate");
      for (i = 1; i < 9; i++)
	if ((rBMapping[i] < 1) || (rBMapping[i] > 8))
	  printw(" ------  ");
	else {
	  printw(" %8.5f", (c2dc_rates[0][rBMapping[i]][1]*cos(hourAngle) +
			    c2dc_rates[1][rBMapping[i]][1]*sin(hourAngle)) * DHADT);
	}
      move(14,0);
      printw("--------------------------------------------------------------------------------");
      move(15,0);
      printw("CB        1        2        3        4        5        6        7        8");
      move(16,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "CB_STATUS_V9_S", 
				    (char *)&cb_status);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - CB_STATUS_V9_S");
      }
      printw("Status ");
      for (i = 1; i < 9; i++)
	if (cb_status[i] == 0)
	  printw(" Unused  ");
	else
	  printw(" In Use  ");
      move(17,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "BSLN_V9_V2_S", 
				    (char *)&bsln);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - BSLN_V9_V2_S");
      }
      printw("Bslns   ");
      for (i = 1; i < 9; i++) {
	if (cb_status[i] == 0)
	  printw("------   ");
	else {
	  if (bsln[i][0] == 0)
	    printw("{Q} ");
	  else
	    printw("%1d-%1d ", bsln[i][0] / 0x100, bsln[i][0] & 0xf);
	  if (bsln[i][1] == 0)
	    printw("{Z}  ");
	  else
	    printw("%1d-%1d ", bsln[i][1] / 0x100, bsln[i][1] & 0xf);
	  if ((bsln[i][0] & 0xf) < 10)
	    printw(" ");
	}
      }
      move(18,0);
      s = dsm_structure_get_element(&crateStatusStructure[crate],
				    "CNTR_ERR_V9_L", 
				    (char *)&cntr_errors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - CNTR_ERR_V9_L");
      }
      printw("Cnt Err");
      for (i = 1; i < 9; i++)
	if (cb_status[i] == 0) {
	  printw(" ------  ");
	} else {
	  if (cntr_errors[i] > 99999999) {
	    printw("  wacko  ");
	  } else {
	    printw("  %5d  ", cntr_errors[i]);
	  }
	}
    }
  }
  move(23,0);
  printw("Hit + to cycle through active crates, R for counter statistics & powers");
  move(23,79);
  refresh();
}
