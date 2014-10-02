#define DEBUG 0
#include <curses.h>
#ifndef LINUX
/* Compensate for incomplete LynxOS curses.h file */
extern int mvprintw    _AP((int, int, const char *fmt, ...));
#endif
#include <math.h>
#include <termio.h>
#include <sys/types.h>
#ifndef LINUX
#include <resource.h>
#endif
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include "rm.h"
#include "dsm.h"
#include "deiced.h"
#include "monitor.h"

int line;

#define VALID_CMD_TIME (14*24*3600)	/* Limit old values to 2 weeks */
#define VALID_STATUS_TIME (600)		/* Limit status to 10 min */

extern dsm_structure Ant5Temps;

/* Reflective memory variables */
int rm_open_status;

/* Number of SMA antennas with deice */
#define NUMANTENNAS 8
int antennas[RM_ARRAY_SIZE];
short powerCmd[RM_ARRAY_SIZE][5];
unsigned int cmdTimestamp[RM_ARRAY_SIZE];
float current[RM_ARRAY_SIZE][3];
unsigned int status[RM_ARRAY_SIZE];
unsigned int phase[RM_ARRAY_SIZE];
unsigned int statusTimestamp[RM_ARRAY_SIZE];
float chopperTemperature[RM_ARRAY_SIZE];

struct rm_var {
	void *a;
	char *name;
} rv[] = {
	{powerCmd, "RM_DEICE_POWER_CMD_V5_S"},
	{cmdTimestamp, "RM_DEICE_CMD_TIMESTAMP_L"},
	{current, "RM_DEICE_CURRENT_V3_F"},
	{status, "RM_DEICE_STATUS_BITS_L"},
	{phase, "RM_DEICE_PHASE_L"},
	{statusTimestamp, "RM_DEICE_TIMESTAMP_L"},
	{chopperTemperature, "RM_CHOPPER_TEMPERATURE_F"},
#if 0
	{antennaThermometers, "ANTENNA_THERMOMETERS_V20_F"},
	{,"TIME_STAMP_V9_L"},
#endif
};
#define NUMVARS (sizeof(rv) / sizeof(rv[0]))

time_t system_time;
char timeString[27];	/* according to 'man ctime', string length = 26 */
char timeString2[27];	/* according to 'man ctime', string length = 26 */

/* deicemon.c */
static void SigHndlr(int signo);
void OpenRM(void);
void ReadRM(void);
extern int numberAntennas;

void deicemon(int count, int *antlist) {
  char ch;
  int rm_rtn, *ip;
  int ant, stat, i;
  char statStr[] = "                                 ";
  unsigned int unixTime;
  int unixTimeAntenna;
//  short int ant5Temps[24];
  float ant5Temps[24];
  float antennaThermometers[20];
  int ant5TempsTimestamp;
  int laptopTimestamp, timestamp[9];
  int rm_status, dsm_status;
  long Ant5Timestamp;

  for (i=0; i<numberAntennas; i++) {
    antennas[i] = 0;
  }
  if ((count % 60) == 0) {
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
  move(0,2);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("Deice System Status: %s UT, %s HT",timeString,timeString2);

  mvaddstr(2,0,"Ant   Power Level   Sys        State of Zones           "
	   "  Phase Currents Chopper\n");
  mvaddstr(3,0,"Num Sur Cho Sub Opt Pha Op Su Ch Q2 Q1 D6 D5 D4 D3 D2 D1"
	   "  A     B     C  Temp(C)\n");

    if (DEBUG) {
      refresh();
    }
  for (ip = antlist; *ip != RM_ANT_LIST_END; ip++) {
    if (DEBUG) {
      printw("ip=%d ",ip);
      refresh();
    }
    antennas[*ip - 1] = 1;
  }
  if (DEBUG) {
    printw("finished for loop\n");
    refresh();
  }
  unixTimeAntenna = antlist[0];
  if (DEBUG) {
    printw("unixTimeAntenna = %d\n",unixTimeAntenna);
    refresh();
  }
  for (i = 0; i < NUMVARS; i++) {
    rm_rtn = rm_read(RM_ANT_ALL, rv[i].name, rv[i].a);
    if (rm_rtn != RM_SUCCESS) {
      fprintf(stderr, "rm_read of ");
      if (DEBUG) {
	refresh();
      }
      rm_error_message(rm_rtn, rv[i].name);
    } else {
      if (DEBUG) {
	printw("read rm_var %d ",i);
	refresh();
      }
    }
  }
  if (DEBUG) {
    refresh();
  }

  rm_rtn=rm_read(unixTimeAntenna, "RM_UNIX_TIME_L", &unixTime);
  if (rm_rtn != RM_SUCCESS) {
    fprintf(stderr, "rm_read of ");
    rm_error_message(rm_rtn, "RM_UNIX_TIME_L");
  }
  for (ant = 1; ant <= NUMANTENNAS; ant++) {
    if (antennas[ant - 1] == 0) {
      continue;
    }
    i = unixTime - cmdTimestamp[ant];
    if (i > VALID_CMD_TIME || i < -100) { /* Commands are invalid */
      mvprintw(3 + ant, 0, " %1d   ?   ?   ?   ?   ", ant);
    } else {
      mvprintw(3 + ant, 0, " %1d  %2d  %2d  %2d  %2d   ", ant,
	       powerCmd[ant][MAIN_P], powerCmd[ant][CHOP_P],
	       powerCmd[ant][SUBR_P], powerCmd[ant][OPTEL_P] );
    }
    if (DEBUG) {
      refresh();
    }
    i = unixTime - statusTimestamp[ant];
    stat = status[ant];
    if (abs(unixTime - statusTimestamp[ant]) > 300) { /* Status is invalid */
      mvprintw(3 + ant, 21, " Status is stale\n");
      continue;
    } else if(stat & (DEICED_FAULT)) {
      mvprintw(3 + ant, 21,
	      " Deiced can not contact deice system modules\n");
      continue;
    } else {
      for (i = 0; i < ZONES; i++) {
	statStr[i*3] = (stat & (1 << (11 - i)))? '1': '0';
      }
      mvprintw(3 + ant, 21, "%d  %s%4.1f  %4.1f  %4.1f ",
	       phase[ant], statStr, current[ant][0],
	       current[ant][1], current[ant][2]);
#define CHOPPER_TEMP_WACKO_HIGH 200
#define CHOPPER_TEMP_HIGHLIGHT_HIGH 30
#define CHOPPER_TEMP_WACKO_LOW -50
      if (chopperTemperature[ant] < CHOPPER_TEMP_WACKO_LOW) {
	printw("wacko-");
      } else if (chopperTemperature[ant] > CHOPPER_TEMP_WACKO_HIGH) {
	printw("wacko+");
      } else {
	if (chopperTemperature[ant] > CHOPPER_TEMP_HIGHLIGHT_HIGH) {
	  standout();
	}
	printw("%6.1f\n", chopperTemperature[ant]);
	standend();
      }
    }
    if (DEBUG) {
      refresh();
    }
  }
/********************** added for DSM temperature data ************************/
  rm_status = dsm_read("obscon","DSM_ANTENNA5_TEMPS_X", &Ant5Temps, &Ant5Timestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(DSM_ANTENNA5_TEMPS_X)");
  }  
   	
  rm_status = dsm_structure_get_element(&Ant5Temps,"ANTENNA_THERMOMETERS_V20_F", &antennaThermometers[0]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(ANTENNA_THERMOMETERS_V20_F)");
  }
  rm_status = dsm_structure_get_element(&Ant5Temps,"TIME_STAMP_V9_L", &timestamp[0]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(TIME_STAMP_V9_L)");
  }
  laptopTimestamp = timestamp[0];
/*****************************************************************************/

  toddtime(&laptopTimestamp,timeString);
  move(15, 5);
  printw("Antenna 5 structural temps reported %s UT", timeString);
#define STALE_LAPTOP_SECONDS 600
  if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS) {
    printw(" ");
    standout();
    printw("stale %ds\n", abs(laptopTimestamp-unixTime));
    standend();
  } else {
		printw("\n");
    }  
  mvaddstr(16, 0, "BUS Quadrants at Tube 5   Cabin Steel   "
	"Quadrupod Primary Str B1");
//  "Quadrupod Primary Str B1   Weldment"); Eliminated until I can access the Cabin Weldment sensors.

  mvaddstr(17, 0, 
	"   Q1    Q2    Q3    Q4    Left Right     "
	"QP1   QP2   QP3   QP4");
//	"QP1   QP2   QP3   QP4    Left Right"); Eliminated until I can access the Cabin Weldment sensors.

  move(18, 0);
#define WACKO_TEMP 100
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw(" stale ");
  standend();  
  } else if (fabs(antennaThermometers[0]) >= WACKO_TEMP){
    printw("wacko ");
  }  else {
    printw("%5.1f ",	 antennaThermometers[0]	 );
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[1]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f ",	 antennaThermometers[1]	 );
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[2]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f ",	 antennaThermometers[2]	 );
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[3]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f   ",	 antennaThermometers[3]	 );
  }  
	if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale  ");
  standend();  
  } else if (fabs(antennaThermometers[4]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f ",	 antennaThermometers[4]	 );
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale   ");
  standend();  
  } else if (fabs(antennaThermometers[5]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f   ",	 antennaThermometers[5]	 );
  }  
  if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[6]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f ", 	 antennaThermometers[6]);
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[7]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f ", 	 antennaThermometers[7]);
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[8]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f ", 	 antennaThermometers[8]);
  }  
if (abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS){
	standout();
  printw("stale ");
  standend();  
  } else if (fabs(antennaThermometers[9]) >= WACKO_TEMP) {
    printw("wacko ");
  } else {
    printw("%5.1f   ",	 antennaThermometers[9]	 );
  }
// Eliminated until I can access the Cabin Weldment sensors.
// (Bug in EDS software limits access to 26 sensors, 52 for two passes.)
//   if (fabs(antennaThermometers[10]) >= WACKO_TEMP) {
//     printw("wacko ");
//   } else {
//     printw("%5.1f ",	 antennaThermometers[10]	 );
//   }
//   if (fabs(antennaThermometers[11]) >= WACKO_TEMP) {
//     printw("wacko ");
//   } else {
//     printw("%5.1f\n",	 antennaThermometers[11]	 );
//   }


/********************** added for DSM temperature data ************************/  	
  rm_status = dsm_structure_get_element(&Ant5Temps,"ANTENNA_SURFACE_MINMAX_TEMPS_V24_F", &ant5Temps[0]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(ANTENNA_SURFACE_MINMAX_TEMPS_V24_F)");
  }
  ant5TempsTimestamp = timestamp[0];
/*****************************************************************************/

//mvaddstr(21,0,"Ant5    Ch    Q2b   Q2a   Q1b   Q1a   D6    D5    D4  " Removed. Chopper temps never implemented.
  mvaddstr(21,0,"Ant5    Q2b   Q2a   Q1b   Q1a   D6    D5    D4  " 
	"  D3    D2    D1\n");
  if (DEBUG) {
    refresh();
  }

 if(abs(laptopTimestamp-unixTime) > STALE_LAPTOP_SECONDS) {
	standout();
  move(22,0);
  printw("MAX T STALE\n");
  printw("MIN T STALE\n");
	standend();
 	} else {
  mvaddstr(22,0,"MAX T");
// for(i = 21; i >= 11; i--) Removed. Chopper temps never implemented.
   for(i = 20; i >= 11; i--) {
      if (ant5Temps[i] > WACKO_TEMP) {
	printw(" wacko");
      } else {
	printw(" %5.1f", ant5Temps[i]);
      }
    }
    addstr("\n");
    mvaddstr(23,0,"MIN T");
// for(i = 10; i >= 0; i--)  Removed. Chopper temps never implemented.
   for(i = 9; i >= 0; i--) {
      if (ant5Temps[i] > WACKO_TEMP) {
	printw(" wacko");
      } else {
	printw(" %5.1f", ant5Temps[i]);
      }
    }
    addstr("\n");
 }
  refresh();
  /*
    sleep(1);
    while(read(0, &ch, 1)) {
    if (ch == 'q')
    goto DONE;
    }
  */

  /*
    DONE:
    endwin();
  */
  return;
}

