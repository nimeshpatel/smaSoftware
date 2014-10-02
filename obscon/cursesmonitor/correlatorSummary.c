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

void correlatorSummary(int count, int rx)
{
  short mode, iRIGStatus, rBMapping[13][9], del[9][2], pmtr[9][2],
    cb_status[9], bsln[9][2];
  int s, i, j, scanNumber, setScanNo, plotErrors, idlErrors, cntr_errors[9],
    stat_errors[9], crate, nActiveCrate, crateList[13], attn;
  int lowestActiveCrate, activeCrate[14];
  double scanTime, scanLength, pPCTemp, c2DCAttn[13][5][9][2], hourAngle,
    c2dc_rates[3][9][2], rates[9];
  char string1[100], plotServer[40], iDLServer[40], string4[100],
    crateName[10], dataFileName[81];
  short chanPerChunk[13][4][2];
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
  crate = crateList[0];
  move(0,0);
  printw("Crate      1     2     3     4     5     6     7     8     9    10    11    12");
  move(1,0);
  printw("T PPC  ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_read(crateName,
		   "CRATE_TO_HAL_X", 
		   &crateStatusStructure[i],
		   &timestamp);
      if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_read - CRATE_TO_HAL_X");
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "PPC_TEMP_D", 
				    (char *)&pPCTemp);
      if ((s != DSM_SUCCESS) && (PRINT_DSM_ERRORS))
	dsm_error_message(s, "dsm_read - PPC_TEMP_D");
      if (fabs(pPCTemp)<200) {
	printw(" %5.1f", pPCTemp);
      } else {
	printw(" wacko");
      }
    } else
      printw("  ----");
  move(2,0);
  printw("IRIG   ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "IRIG_STATUS_S", 
				    (char *)&iRIGStatus);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - IRIG_STATUS_S");
      }
      if ((iRIGStatus & 0x101) == 0x001)
	printw("    OK");
      else if ((iRIGStatus & 0x101) == 0x100)
	printw("  R Er");
      else if ((iRIGStatus & 0x101) == 0x101)
	printw("   U/R");
      else if (iRIGStatus == 0)
	printw("  ????");
      else
	/* printw(" Unlck"); */
	printw("0x%04x", iRIGStatus);
    } else
      printw("  ----");
  move(3,0);
  printw("Block  ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      int block;

      block = i % 6;
      if (block == 0)
	block = 6;
      printw("    %2d", block);
    } else
      printw("  ----");
  move(4,0);
  printw("I Time ");
  for (i = 1; i < 13; i++) {
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "SCAN_TIME_D", 
				    (char *)&scanTime);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - SCAN_TIME_D");
      }
      if (fabs(scanTime) >= 10000) {
	printw(" wacko");
      } else if (fabs(scanTime) >= 1000) {
	printw(" %5.0f", scanTime);
      } else if (fabs(scanTime) >= 100) {
	printw(" %5.1f", scanTime);
      } else {
	printw(" %5.2f", scanTime);
      }
    } else {
      printw("  ----");
    }
  }
  move(5,0);
  printw("S Len  ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "SCAN_LENGTH_D", 
				    (char *)&scanLength);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - SCAN_LENGTH_D");
      }
      if (fabs(scanLength) >= 10000) {
	printw(" wacko");
      } else if (fabs(scanLength) >= 1000) {
	printw(" %5.0f", scanLength);
      } else if (fabs(scanLength) >= 100) {
	printw(" %5.1f", scanLength);
      } else {
	printw(" %5.2f", scanLength);
      }
    } else
      printw("  ----");
  move(6,0);
  printw("Scan # ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "SCAN_NO_L", 
				    (char *)&scanNumber);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - SCAN_NUMBER_L");
      }
      if (fabs(scanNumber) < 100000) {
	printw(" %5d", scanNumber);
      } else {
	printw(" wacko");
      }
    } else
      printw("  ----");
  move(7,0);
  printw("P Errs ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "PLOT_ERRORS_L", 
				    (char *)&plotErrors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - ERRORS_L");
      }
      if (plotErrors == -1)
	printw(" Plots");
      else if (fabs(plotErrors) < 100000) {
	printw(" %5d", plotErrors);
      } else {
	printw(" wacko");
      }
    } else
      printw("  ----");
  move(8,0);
  printw("P Serv ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      if (plotErrors == -1)
	printw(" Disab");
      else {
	sprintf(crateName, "crate%d", i);
	s = dsm_structure_get_element(&crateStatusStructure[i],
				      "PLOT_SERVER_C40", 
				      plotServer);
	if (s != DSM_SUCCESS) {
	  if (PRINT_DSM_ERRORS)
	    dsm_error_message(s, "dsm_read - PLOT_SERVER_C40");
	}
	plotServer[5] = (char)0;
	printw("%6s", plotServer);
      }
    } else
      printw("  ----");
  move(9,0);
  printw("S Errs ");
  for (i = 1; i < 13; i++) {
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "IDL_ERRORS_L", 
				    (char *)&plotErrors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - IDL_ERRORS_L");
      }
      if (plotErrors == -1)
	printw(" Store");
      else if (fabs(plotErrors) < 100000) {
	printw(" %5d", plotErrors);
      } else {
	printw(" wacko");
      }
    } else {
      printw("  ----");
    }
  }
  move(10,0);
  printw("S Serv ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      if (plotErrors == -1)
	printw(" Disab");
      else {
	sprintf(crateName, "crate%d", i);
	s = dsm_structure_get_element(&crateStatusStructure[i],
				      "IDL_SERVER_C40", 
				      iDLServer);
	if (s != DSM_SUCCESS) {
	  if (PRINT_DSM_ERRORS)
	    dsm_error_message(s, "dsm_read - IDL_SERVER_C40");
	}
	iDLServer[5] = (char)0;
	printw("%6s", iDLServer);
      }
    } else
      printw("  ----");
  move(11,0);
  printw("Cnt Er ");
  for (i = 1; i < 13; i++) {
    int total;
    
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "RB_MAPPING_V9_S", 
				    (char *)&rBMapping[i]);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - RB_MAPPING_V9_S");
      }
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "STAT_ERRORS_V9_L", 
				    (char *)&stat_errors);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - STAT_ERRORS_V9_L");
      }
      total = 0;
      for (j = 0; j < 9; j++) {
	if ((rBMapping[i][j] > 0) && (rBMapping[i][j] < 9))
	  total += stat_errors[j];
      }
      if (total >= 100000 || total <= -10000) {
	printw(" wacko");
      } else {
	printw(" %5d", total);
      }
    } else
      printw("  ----");
  }
  move(12,0);
  printw("DC Err ");
  for (i = 1; i < 13; i++) {
    int total, dCData;

    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "DC_DATA_ERRORS_L", 
				    (char *)&dCData);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - DC_DATA_ERROR_S");
      }
      if (fabs(dCData) < 100000) {
	printw(" %5d", dCData);
      } else {
	printw(" wacko");
      }
    } else
      printw("  ----");
  }
  move(13,0);
  if (rx == 0)
    printw("Res (L)");
  else
    printw("Res (H)");
  for (i = 1; i < 13; i++) {
    if (activeCrate[i]) {
      int ii;
      
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "CHIPSPERCHUNK_V4_V2_S", 
				    (char *)&chanPerChunk[i]);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - CHIPSPERCHUNK_V4_V2_S");
      }
      printw("  ");
      for (ii = 0; ii < 4; ii++) {
	int logNChannels;
	
	logNChannels = (int)(log((double)chanPerChunk[i][ii][rx])/log(2.0) + 0.5);
	if (logNChannels == 10)
	  printw("A");
	else if (logNChannels == 11)
	  printw("B");
	else if (logNChannels == 12)
	  printw("C");
	else
	  printw("%c", (char)((int)'0' + logNChannels));
      }
    } else {
      printw("  ----");
    }
  }
  for (attn = 1; attn < 9; attn++) {
    move(13+attn,0);
    if (rx == 0)
      printw("dBLRB%d ", attn);
    else
      printw("dBHRB%d ", attn);
    for (i = 1; i < 13; i++)
      if (activeCrate[i]) {
	if (attn == 1) {
	  sprintf(crateName, "crate%d", i);
	  s = dsm_structure_get_element(&crateStatusStructure[i],
					"C2DC_ATTN_V5_V9_V2_D", 
					(char *)&c2DCAttn[i]);
	  if (s != DSM_SUCCESS) {
	    if (PRINT_DSM_ERRORS)
	      dsm_error_message(s, "dsm_read - C2DC_ATTN_V5_V9_V2_D");
	  }
	  {
	    int ii, jj, kk;
	    
	    for (ii = 0; ii < 5; ii++)
	      for (jj = 0; jj < 9; jj++)
		for (kk = 0; kk < 2; kk++)
		  if ((c2DCAttn[i][ii][jj][kk] < 0.0) ||
		      (c2DCAttn[i][ii][jj][kk] > 100.0))
		    c2DCAttn[i][ii][jj][kk] = -3.0;
	  }
	}
	printw("  ");
	if ((rBMapping[i][attn] < 1) || (rBMapping[i][attn] > 8))
	  printw("----", rBMapping[i][attn]);
	else {
	  int offset;
	  
	  for (j = 1; j < 5; j++) {
	    offset = (int)(c2DCAttn[i][j][attn][rx] + 0.5);
	    if ((offset < 34) && (chanPerChunk[i][j-1][rx] > 0)) {
	      if (offset < 10)
		offset += (int)'0';
	      else
		offset += (int)'A' - 10;
	      printw("%c", (char)offset);
	    } else
	      printw("-");
	  }
	}
      } else
	printw("  ----");
  }
  move(22,0);
  printw("Mode   ");
  for (i = 1; i < 13; i++)
    if (activeCrate[i]) {
      sprintf(crateName, "crate%d", i);
      s = dsm_structure_get_element(&crateStatusStructure[i],
				    "MODE_S", 
				    (char *)&mode);
      if (s != DSM_SUCCESS) {
	if (PRINT_DSM_ERRORS)
	  dsm_error_message(s, "dsm_read - MODE_S");
      }
      printw("    %2d", mode);
    } else
      printw("  ----");
  printw(" ");
  move(23,0);
  s = dsm_read(iDLServer,
	       "DSM_AS_FILE_NAME_C80", 
	       dataFileName,
	       &timestamp);
  if (s != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(s, "dsm_read - DSM_AS_FILE_NAME_C80");
  }
  printw("The data will be stored in %s", dataFileName);
  move(23,79);
  refresh();
  return;
}

