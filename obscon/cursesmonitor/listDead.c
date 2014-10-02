#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "smainit_report.h"

#define N_COMPUTERS 17
#define N_NORMALLY_DEAD 2

static dsm_structure ds;
static int ds_has_init = 0;

listDead(int count)
{
  int status, i, j, page, currentLine;
  char rptNames[DSM_ARRAY_SIZE][DSM_NAME_LEN];
  unsigned char rptStatus[DSM_ARRAY_SIZE][4];
  time_t statusTimestamp, startTimesTimestamp;
  time_t curTime, stopTime;
  char *statusStrings[] = { STATUSLIST };
  char *whyStrings[] = { WHYLIST };
  char computer[10];
  char computers[N_COMPUTERS][10] = {"hal9000", "acc1", "acc2", "acc3",
				     "acc4", "acc5",
				     "acc6", "acc7", "acc8",
                                     "newdds", "crate1",
				     "crate2", "crate7",
				     "crate8", "corcon", "m5",
                                     "colossus"};
  char normallyDead[N_NORMALLY_DEAD][20] = {"int_control", "c1DCInit"};
  struct daemon_status dStatus[DSM_ARRAY_SIZE];

  if(!ds_has_init) {
    status = dsm_structure_init(&ds, "SMAINIT_REPORT_X");
    if(status != DSM_SUCCESS) {
       dsm_error_message(status, "Initializing SMAINIT_REPORT_X");
    }
  }
  if ((count % 15) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
#ifdef LINUX
    clear();
#endif
    move(0,0);
    refresh();
  }
  move(0,3);
  curTime = time((long *)0);
  printw("Programs managed by smainit which have died as of %s",
	 ctime(&curTime));
  move(2,0);
  printw("  Computer            Program Name  Signal        Dead since           Ran for");
  currentLine = 3;
  for (i = 0; i < N_COMPUTERS; i++) {
    int badComputer;

    badComputer = 0;
    if (currentLine > 22) {
      move(23,0);
      printw("There may be more!");
      refresh();
      return;
    }
    strcpy(computer, computers[i]);
#define TIMING 0
#if TIMING
{
static  struct timeval time1, time2;
static  struct timezone zone;
static  int dt;

  if(i == 0) gettimeofday(&time1, &zone);
#endif /* TIMING */
    if ((status = dsm_read(computer,
	  "SMAINIT_REPORT_X", &ds, &statusTimestamp)) != DSM_SUCCESS) {
      badComputer = 1;
    }
#if TIMING
  gettimeofday(&time2, &zone);
  dt = time2.tv_usec - time1.tv_usec + (time2.tv_sec - time1.tv_sec)*1000000;
  move(21+i/9, (i%9)*9);
  printw("%d  ", dt/1000);
}
#endif /* TIMING */
    if (!badComputer) {
      status = dsm_structure_get_element(&ds, "PRG_NAMES_V24_C24", rptNames);
      status |= dsm_structure_get_element(&ds, "PRG_STATUS_V24_V5_L", dStatus);
      if(status != DSM_SUCCESS) {
	badComputer = 1;
      }
    }
    if (!badComputer)
      if (statusTimestamp <= 0) {
	move(currentLine++, 0);
	printw("%10s has not reported program status since last reboot", computer);
	badComputer = 1;
      }
    if (!badComputer)
      for (j = 0; rptNames[j][0]; j++)
	if ((int)dStatus[j].status[STATUS_BYTE]) {
	  int lastRunTime, dd, hh, mm, ss, deadOK, p;
	  char dateString[50];
	  
	  move(currentLine++, 0);
	  if (currentLine > 22) {
	    move(23,0);
	    printw("There may be more!");
	    refresh();
	    return;
	  }
	  lastRunTime = dStatus[j].times[LAST_STOP_TIME] -
	    dStatus[j].times[LAST_START_TIME];
	  dd = lastRunTime / 86400;
	  hh = (lastRunTime - dd*86400) / 3600;
	  mm = (lastRunTime - dd*86400 - hh*3600) / 60;
	  ss = (lastRunTime - dd*86400 - hh*3600 - mm*60);
	  stopTime = (time_t)dStatus[j].times[LAST_STOP_TIME];
	  strcpy(dateString,  ctime(&stopTime));
	  dateString[24] = (char)0;
	  deadOK = FALSE;
	  for (p = 0; p < N_NORMALLY_DEAD; p++)
	    if (!strcmp(rptNames[j], normallyDead[p]))
	      deadOK = TRUE;
	  if (deadOK)
	    printw("%10s %23s*  0x%02x  %24s   ",
		   computer, rptNames[j],
		   dStatus[j].status[SIGNAL_RECEIVED_BYTE],
		   dateString);
	  else
	    printw("%10s %23s   0x%02x  %24s   ",
		   computer, rptNames[j],
		   dStatus[j].status[SIGNAL_RECEIVED_BYTE],
		   dateString);
	  if ((dd == 0) && (hh == 0) && (mm == 0))
	    printw(" < 1 min");
	  else if (dd == 0)
	    printw("%02d:%02d:%02d", hh, mm, ss);
	  else
	    printw("%2dd %02d:%02d", dd, hh, mm);
	}
  }
  move(23,0);
  printw("Program names followed by * normally aren't running and do not indicate errors.");
  move(0,0);
  refresh();
  return;
}
