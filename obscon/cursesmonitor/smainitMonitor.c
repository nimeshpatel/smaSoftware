#define USE_DSM_STRUCT 1

#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "dsm.h"
#include "smainit_report.h"
#if USE_DSM_STRUCT
static dsm_structure ds;
static int ds_has_init = 0;
#endif /* USE_DSM_STRUCT */


smainitMonitor(int count, int page)
{
  int status, i;
  char rptNames[DSM_ARRAY_SIZE][DSM_NAME_LEN];
  time_t namesTimestamp, statusTimestamp, startTimesTimestamp;
  time_t curTime, seconds;
  char *statusStrings[] = { STATUSLIST };
  char *whyStrings[] = { WHYLIST };
  char computer[40];
  long daemonStatus[24][5];
  struct daemon_status dStatus[DSM_ARRAY_SIZE];

#if USE_DSM_STRUCT
  if(!ds_has_init) {
    status = dsm_structure_init(&ds, "SMAINIT_REPORT_X");
    if(status != DSM_SUCCESS) {
       dsm_error_message(status, "Initializing SMAINIT_REPORT_X");
    }
  }
#endif /* USE_DSM_STRUCT */
  if ((count % 60) == 1) {
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
  move(0,12);
  curTime = time((long *)0);
  printw("smainit monitor for ");
  switch (page) {
  case 0:
    strcpy(computer, "hal9000");
    break;
  case 1:
    strcpy(computer, "acc1");
    break;
  case 2:
    strcpy(computer, "acc2");
    break;
  case 3:
    strcpy(computer, "acc3");
    break;
  case 4:
    strcpy(computer, "acc4");
    break;
  case 5:
    strcpy(computer, "acc5");
    break;
  case 6:
    strcpy(computer, "acc6");
    break;
  case 7:
    strcpy(computer, "acc7");
    break;
  case 8:
    strcpy(computer, "acc8");
    break;
  case 9:
    strcpy(computer, "colossus");
    break;
  case 10:
    strcpy(computer, "newdds");
    break;
  case 11:
    strcpy(computer, "corcon");
    break;
  case 12:
    strcpy(computer, "m5");
    break;
  case 25:
    strcpy(computer, "acc9");
    break;
  case 26:
    strcpy(computer, "acc10");
    break;
  case 27:
    strcpy(computer, "obscon");
    break;
  case 28:
    strcpy(computer, "phasemon");
    break;
  case 29:
    strcpy(computer, "hcn");
    break;
  default:
    sprintf(computer, "crate%d", page-12);
  }
  printw("%7s at %s", computer, asctime(gmtime((&curTime))));

  if ((status = dsm_read(computer,
	  "SMAINIT_REPORT_X", &ds, &statusTimestamp)) != DSM_SUCCESS) {
    dsm_error_message(status, "Reading SMAINIT_REPORT_X");
    return;
  }
  if (statusTimestamp <= 0) {
    move(11, 10);
    printw("Program status on %s has not been written since startup",
	   computer);
    move(23,0);
    refresh();
    return;
  }
  seconds = curTime - statusTimestamp;
  if (abs(seconds) > 70) {
    move(2, 0);
    printw("\n\n\t\tProgram status on %s has been stale for %d seconds.\n\n\n",
	   computer,seconds);
    move(23,0);
    refresh();
    return;
  }
  status = dsm_structure_get_element(&ds, "PRG_NAMES_V24_C24", rptNames);
#ifdef LINUX
  status |= dsm_structure_get_element(&ds, "PRG_STATUS_V24_V5_L", daemonStatus);
  if ((page < 27) || 1) {
    for (i = 0; i < 24; i++) {
      int j, k;
      
      if (page < 27)
	for (j = 0; j < 2; j++)
	  for (k = 0; k < 4; k++)
	    dStatus[i].status[(4*j)+k] = ((char *)&daemonStatus[i][0])[(4*j)+3-k];
      for (j = 0; j < TIME_SIZE; j++)
	dStatus[i].times[j] = daemonStatus[i][2+j];
    }
  }
#else
  status |= dsm_structure_get_element(&ds, "PRG_STATUS_V24_V5_L", dStatus);
#endif
  if(status != DSM_SUCCESS) {
    dsm_error_message(status, "Getting elements");
    return;
  }
  move(2,0);
  printw("        Process Name             Status   Run time   Started by Restarts Signal");
  for(i = 0; rptNames[i][0] && (i < 20); i++) {
    rptNames[i][DSM_NAME_LEN-1] = (char)0;
    move(3+i,0);
    if (!((int)dStatus[i].status[STATUS_BYTE]))
      if ((((int)dStatus[i].status[STATUS_BYTE]) < 6)  &&
	  ((int)dStatus[i].status[START_BYTE] < 5)) {
	int elapsedSeconds;
	
	elapsedSeconds = (int)(curTime - dStatus[i].times[LAST_START_TIME]);
	if (elapsedSeconds < 60)
	  printw("%23s %15s   %7ds %12s %8d  0x%02x",
		 rptNames[i],
		 statusStrings[(int)dStatus[i].status[STATUS_BYTE]],
		 elapsedSeconds,
		 whyStrings[(int)dStatus[i].status[START_BYTE]],
		 dStatus[i].status[RESTART_BYTE],
		 dStatus[i].status[SIGNAL_RECEIVED_BYTE]
		 );
	else if (elapsedSeconds < 86400) {
	  int hours, minutes, seconds;
	  
	  hours = elapsedSeconds / 3600;
	  minutes = (elapsedSeconds - (hours * 3600)) / 60;
	  seconds = elapsedSeconds - (hours * 3600) - (minutes * 60);
	  printw("%23s %15s   %02d:%02d:%02d %12s %8d  0x%02x",
		 rptNames[i],
		 statusStrings[(int)dStatus[i].status[STATUS_BYTE]],
		 hours, minutes, seconds,
		 whyStrings[(int)dStatus[i].status[START_BYTE]],
		 dStatus[i].status[RESTART_BYTE],
		 dStatus[i].status[SIGNAL_RECEIVED_BYTE]
		 );
	}
	else {
	  int days, hours, minutes;
	  
	  days = elapsedSeconds / 86400;
	  hours = (elapsedSeconds - (days * 86400)) / 3600;
	  minutes = (elapsedSeconds - (days * 86400) - (hours * 3600)) / 60;
	  printw("%23s %15s %3dd %02d:%02d %12s %8d  0x%02x",
		 rptNames[i],
		 statusStrings[(int)dStatus[i].status[STATUS_BYTE]],
		 days, hours, minutes,
		 whyStrings[(int)dStatus[i].status[START_BYTE]],
		 dStatus[i].status[RESTART_BYTE],
		 dStatus[i].status[SIGNAL_RECEIVED_BYTE]
		 );
	}
      } else
	printw("Garbage value seen - smainit related DSM variables corrupted");
    else if ((int)dStatus[i].status[START_BYTE] < 5)
      printw("%23s %15s   -------- %12s %8d  0x%02x",
	     rptNames[i],
	     statusStrings[(int)dStatus[i].status[STATUS_BYTE]],
	     whyStrings[(int)dStatus[i].status[START_BYTE]],
	     dStatus[i].status[RESTART_BYTE],
	     dStatus[i].status[SIGNAL_RECEIVED_BYTE]
	     );
    else
      printw("Garbage value seen - smainit related DSM variables corrupted");
  }
  move(23,0);
  printw("1->8 acc, c(+) crate, C col, d dds, ~ corc, 0 hal, m m5, O obsc, p phase, h hcn");
  refresh();
}
