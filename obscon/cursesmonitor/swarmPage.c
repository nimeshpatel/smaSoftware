#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "dDSCursesMonitor.h"

#define PRINT_DSM_ERRORS (TRUE)

#define WACKO_EARLY_TIME (1380585600)
#define WACKO_LATE_TIME  (1696118400)

extern dsm_structure roachStructure, dDSStatusStructure;
int roachDead[9];

void readNTime(int roach, char *varName, void *target)
{
  static int firstCall = TRUE;
  static char roachName[9][10];
  int dSMStatus;
  double timeBefore, timeAfter, dt;
  time_t timeStamp;
  struct timeval tvBefore, tvAfter;

  if (firstCall) {
    int i;

    for (i = 1; i <= 8; i++)
      sprintf(roachName[i], "roach2-%02d", i);
    firstCall = FALSE;
  }
  gettimeofday(&tvBefore, NULL);
  dSMStatus = dsm_read(roachName[roach], varName,
                       (char *)target, &timeStamp);
  gettimeofday(&tvAfter, NULL);
  if (dSMStatus != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(dSMStatus, varName);
    exit(-1);
  }
  timeBefore = (double)tvBefore.tv_sec + ((double)tvBefore.tv_usec)/1.0e6;
  timeAfter = (double)tvAfter.tv_sec + ((double)tvAfter.tv_usec)/1.0e6;
  dt = timeAfter - timeBefore;
  if (dt > 0.1)
    roachDead[roach] = TRUE;
}

void swarmPage(int count)
{
  short walshPattern[11][2];
  int dSMStatus, i, times[8];
  static int firstCall = TRUE;
  char string1[100];
  float temps[8];
  static dsm_structure sWARMMappingStructure, sWARMSourceGeomStructure;
  static dsm_structure sWARMSmaplerStats;
  time_t timestamp, systemTime;
  
  if(firstCall) {
    dSMStatus = dsm_structure_init(&sWARMMappingStructure, "SWARM_MAPPING_X");
    if (dSMStatus != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(dSMStatus, "init SWARM_MAPPING_X");
      exit(-1);
    }
    dSMStatus = dsm_structure_init(&sWARMSourceGeomStructure, "SWARM_SOURCE_GEOM_X");
    if (dSMStatus != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(dSMStatus, "init SWARM_SOURCE_GEOM_X");
      exit(-1);
    }
    firstCall = FALSE;
  }

  if (((count % 60) == 1)) {
    /* Initialize Curses Display */
    initscr();
    clear();
    move(1,1);
    refresh();
    for (i = 1; i <= 8; i++)
      roachDead[i] = FALSE;
#if 0
    dSMStatus = dsm_structure_init(&sWARMMappingStructure, "SWARM_MAPPING_X");
    if (dSMStatus != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(dSMStatus, "init SWARM_MAPPING_X");
      exit(-1);
    }
    dSMStatus = dsm_structure_init(&sWARMSourceGeomStructure, "SWARM_SOURCE_GEOM_X");
    if (dSMStatus != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(dSMStatus, "init SWARM_SOURCE_GEOM_X");
      exit(-1);
    }
    firstCall = FALSE;
#endif
  }
  
  move(0,0);
  strcpy(string1, asctime(gmtime(&timestamp)));
  string1[strlen(string1)-1] = (char)0;
  systemTime = time(NULL);
  printw("               SWARM Correlator Status at %s", asctime(gmtime(&systemTime)));
  move(1,0);
  printw("                          ROACH2-Related Temperatures");
  move(2,0);
  printw("ROACH2      1        2        3        4        5        6        7        8");
  dSMStatus = dsm_read("obscon", "ROACH2_TEMPS_X",
		       (char *)&roachStructure, &timestamp);
  if (dSMStatus != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(dSMStatus, "dsm_read - roachStructure");
    exit(-1);
  }
  dSMStatus = dsm_read("newdds", "DDS_TO_HAL_X",
		       (char *)&dDSStatusStructure, &timestamp);
  if (dSMStatus != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(dSMStatus, "dsm_read - dDSStatusStructure");
    exit(-1);
  }
  move(3,0);
  dSMStatus = dsm_structure_get_element(&roachStructure, "AMBIENT_TEMP_V8_F", &temps[0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "AMBIENT_TEMP_V8_F");
    exit(-1);
  }
  printw("Ambient  %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f ",
	 temps[0], temps[1], temps[2], temps[3], temps[4], temps[5], temps[6], temps[7]);
  move(4,0);
  dSMStatus = dsm_structure_get_element(&roachStructure, "INLET_TEMP_V8_F", &temps[0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "INLET_TEMP_V8_F");
    exit(-1);
  }
  printw("Inlet    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f ",
	 temps[0], temps[1], temps[2], temps[3], temps[4], temps[5], temps[6], temps[7]);
  move(5,0);
  dSMStatus = dsm_structure_get_element(&roachStructure, "OUTLET_TEMP_V8_F", &temps[0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "OUTLET_TEMP_V8_F");
    exit(-1);
  }
  printw("Outlet   %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f ",
	 temps[0], temps[1], temps[2], temps[3], temps[4], temps[5], temps[6], temps[7]);
  move(6,0);
  dSMStatus = dsm_structure_get_element(&roachStructure, "PPC_TEMP_V8_F", &temps[0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "PPC_TEMP_V8_F");
    exit(-1);
  }
  printw("PowerPC  %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f ",
	 temps[0], temps[1], temps[2], temps[3], temps[4], temps[5], temps[6], temps[7]);
  move(7,0);
  dSMStatus = dsm_structure_get_element(&roachStructure, "FPGA_TEMP_V8_F", &temps[0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "FPGA_TEMP_V8_F");
    exit(-1);
  }
  printw("FPGA     %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f    %5.1f ",
	 temps[0], temps[1], temps[2], temps[3], temps[4], temps[5], temps[6], temps[7]);
  move(8,0);
  dSMStatus = dsm_structure_get_element(&roachStructure, "TIMESTAMP_V8_L", &times[0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "TIMESTAMP_V8_L");
    exit(-1);
  }
  printw("Temp Age");
  for (i = 0; i < 8; i++) {
    int dt;
    float min, hours, days;

    dt = systemTime - times[i];
    if ((dt < 0) || (dt > 300000000)) {
      printw("  wacko  ");
    } else if (dt < 60)
      printw("%2d secs  ", dt);
    else if (dt < 3600) {
      min = ((float)dt)/60.0;
      if (min < 2.0)
	printw ("%4.1f min ", min);
      else
	printw("%2.0f mins  ", min);
    } else if (dt < 86400) {
      hours = ((float)dt)/3600.0;
      if (hours < 2.0)
	printw ("%4.1f hr  ", hours);
      else
	printw("%2.0f hrs   ", hours);
    } else if (dt < 31557600) {
      days = ((float)dt)/86400.0;
      if (days < 2.0)
	printw ("%4.1f day ", days);
      else
	printw("%2.0f days  ", days);
    }
  }
  move(9,0);
  printw("                        Stuff newdds sends the ROACH2s");
  move(10,0);
  for (i = 0; i <= 8; i++) {
    int iSyncUnixTime;
    double syncUnixTime;

    if (i == 0)
      printw("Next Scn");
    else if (roachDead[i]) {
      move(10, (i-1)*9+10);
      printw("dead?");
    } else {
      readNTime(i, "SYNC_UNIX_TIME_D", &syncUnixTime);
      iSyncUnixTime = round(syncUnixTime);
      if ((WACKO_EARLY_TIME < iSyncUnixTime) && (iSyncUnixTime < WACKO_LATE_TIME)) {
	int j;
	char justTime[10];
	char *timeString;

	timeString = ctime((const time_t *)&iSyncUnixTime);
	for (j = 0; j < 8; j++)
	  justTime[j] = timeString[j+11];
	justTime[8] = (char)0;
	move(10, (i-1)*9+8);
	printw("%s", justTime);
      } else {
	move(10, (i-1)*9+10);
	printw("wacko");
      }
    }
  }
  for (i = 0; i <= 8; i++) {
    if (i == 0) {
      move(11,0);
      printw("A:P:IF 0");
      move(12,0);
      printw("A:P:IF 1");
    } else if (roachDead[i]) {
      move(11, (i-1)*9+10);
      printw("dead?");
      move(12, (i-1)*9+10);
      printw("dead?");
    } else  {
      int j;
      char ant, pol, polString[5];
      float freqs[2];
      
      readNTime(i, "SWARM_MAPPING_X", &sWARMMappingStructure);
      for (j = 0; j < 2; j++) {
	char element[20];

	sprintf(element, "INPUT%d_ANT_B", j);
	dSMStatus = dsm_structure_get_element(&sWARMMappingStructure, element, &ant);
	if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	  dsm_error_message(dSMStatus, element);
	  exit(-1);
	}
	sprintf(element, "INPUT%d_POL_B", j);
	dSMStatus = dsm_structure_get_element(&sWARMMappingStructure, element, &pol);
	if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	  dsm_error_message(dSMStatus, element);
	  exit(-1);
	}
	sprintf(element, "INPUT%d_IFBW_V2_F", j);
	dSMStatus = dsm_structure_get_element(&sWARMMappingStructure, element, &freqs);
	if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	  dsm_error_message(dSMStatus, element);
	  exit(-1);
	}
	switch (pol) {
	case 0:
	  sprintf(polString, "H");
	  break;
	case 1:
	  sprintf(polString, "V");
	  break;
	case 2:
	  sprintf(polString, "L");
	  break;
	default:
	  sprintf(polString, "R");
	  break;
	}
	move(11+j, (i-1)*9+8);
	if ((4000.0 <= freqs[0]) && (freqs[0] <= 12000.0) && (4000.0 <= freqs[0]) && (freqs[0] <= 12000.0))
	  printw("%d:%s:%0.0f-%0.0f", ant, polString, freqs[0]/1000.0, freqs[1]/1000.0);
	else
	  printw("%d:%s:?-?", ant, polString, freqs[0]/1000.0, freqs[1]/1000.0);
      }
    }
  }
  for (i = 0; i <= 8; i++) {
    int j;

    if (i == 0) {
      move(13,0);
      printw("RA");
      move(14,0);
      printw("Del A 0");
      move(15,0);
      printw("Del B 0");
      move(16,0);
      printw("Del C 0");
      move(17,0);
      printw("Del A 1");
      move(18,0);
      printw("Del B 1");
      move(19,0);
      printw("Del C 1");
    } else if (roachDead[i]) {
      for (j = 0; j < 7; j++) {
	move(13+j, (i-1)*9+10);
	printw("dead?");
      }
    } else  {
      int hh, mm, ss;
      double rA, delayA[2], delayB[2], delayC[2];
      
      readNTime(i, "SWARM_SOURCE_GEOM_X", &sWARMSourceGeomStructure);
      dSMStatus = dsm_structure_get_element(&sWARMSourceGeomStructure, "SOURCE_RA_D", &rA);
      if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	dsm_error_message(dSMStatus, "SOURCE_RA_D");
	exit(-1);
      }
      dSMStatus = dsm_structure_get_element(&sWARMSourceGeomStructure, "GEOM_DELAY_A_V2_D", &delayA[0]);
      if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	dsm_error_message(dSMStatus, "GEOM_DELAY_A_V2_D");
	exit(-1);
      }
      dSMStatus = dsm_structure_get_element(&sWARMSourceGeomStructure, "GEOM_DELAY_B_V2_D", &delayB[0]);
      if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	dsm_error_message(dSMStatus, "GEOM_DELAY_B_V2_D");
	exit(-1);
      }
      dSMStatus = dsm_structure_get_element(&sWARMSourceGeomStructure, "GEOM_DELAY_C_V2_D", &delayC[0]);
      if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
	dsm_error_message(dSMStatus, "GEOM_DELAY_C_V2_D");
	exit(-1);
      }
      move(13, (i-1)*9+8);
      rA *= 12.0/M_PI;
      hh = (int)rA;
      mm = (int)((rA - (double)hh)*60.0);
      ss = round((rA - (double)hh - ((double)mm/60.0))*3600.0);
      printw("%02d:%02d:%02d", hh, mm, ss);
      for (j = 0; j < 2; j++) {
	move(14+3*j, (i-1)*9+8);
	printw("%8.2f", delayA[j]*1.0e9);
	move(15+3*j, (i-1)*9+8);
	printw("%8.2f", delayB[j]*1.0e9);
	move(16+3*j, (i-1)*9+8);
	printw("%8.2f", delayC[j]*1.0e9);
      }
    }
  }
#if 0
  move(20,0);
  printw("                             Antenna-Related Stuff");
#endif
  move(20,0);
  printw("Antenna     1        2        3        4        5        6        7        8");
  move(21,0);
  dSMStatus = dsm_structure_get_element(&dDSStatusStructure, "WALSH_MOD_V11_V2_S", &walshPattern[0][0]);
  if ((dSMStatus != DSM_SUCCESS) && PRINT_DSM_ERRORS) {
    dsm_error_message(dSMStatus, "AMBIENT_TEMP_V8_F");
    exit(-1);
  }
  printw("Walsh   ");
  for (i = 1; i <= 8; i++)
    printw("   %d/%d   ", walshPattern[i][0], walshPattern[i][1]);
  move(22,0);
  printw("LF-lf(dB)");
  move(23,0);
  printw("LF-hf(dB)");
  for (i = 1; i <= 8; i++) {
    static char rname[] = "roach2-01";
    float loadingFactor[2];

    rname[8] = '0' + i;
    dSMStatus = dsm_read(rname, "SWARM_LOADING_FACTOR_V2_F",
  		       (char *)loadingFactor, &timestamp);
    if (dSMStatus != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
        dsm_error_message(dSMStatus, "dsm_read - SWARM_LOADING_FACTOR_V2_F");
      exit(-1);
    }
    mvprintw(22, i*9, " %6.2f ", loadingFactor[0]);
    mvprintw(23, i*9, " %6.2f ", loadingFactor[1]);
  }
  refresh();
  return;
}
