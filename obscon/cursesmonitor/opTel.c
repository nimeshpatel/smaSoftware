#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include <math.h>
#include <curses.h>
#include "commonLib.h"
#include "monitor.h"
#include "tune6status.h"
#include "optics.h"
#include "opticsMonitor.h"
#include "rscanpage.h"
#include "upspage.h"

#define WACKO_WIRE_GRID_MIN 0
#define WACKO_WIRE_GRID_MAX 150000
#define TUNE6_STALE_INTERVAL 10
#define C90_STALE_INTERVAL 60

extern char *toddtime(time_t *, char *str);
extern char *hsttime(time_t *, char *str);
void checkStatus(int status, char *string);
extern void noBoard(int ant);
char *getLoBoardTypeStringBrief(int type);
char *getCalibrationWheelPosition(int position);
char *getCalibrationWheelPosition3Char(int position);
int invalidLoBoardType(int type);
int invalidCalibrationWheelPosition(int position);

#define NUMBER_OF_RECEIVERS 8
#define NUMBER_OF_ANTENNAS 8

void opTel(int count, int *rm_list) 
{
  int rm_status;
  int ant, antennaNumber;
  float floatvalue;
  long rightnow, timevalue;
  time_t system_time;
  short maxPixel, minPixel;
  int timestamp;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  short milliseconds;
  short focusMotorPresent;
  FILE *fp;
  char line[200];
  static char dateString[NUMBER_OF_ANTENNAS+1][20];
  char filename[200];

  if ((count % 20) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
#ifdef LINUX
    clear();
#endif
    move(1,1);
    refresh();
    for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++){
      sprintf(filename,"/otherInstances/acc/%d/lib/mountModels/mount.model",
	      antennaNumber);
      fp = fopen(filename,"r");
      if (fp != NULL) {
	fgets(line,sizeof(line),fp);
	fgets(line,sizeof(line),fp);
	sscanf(line,"%*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %s",dateString[antennaNumber]);
	fclose(fp);
      }
    }
  }
  ant = 0;
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("OpTel Page   1        2        3        4        5        6        7        8");

  move(2,0);
  printw("C90Hum%%  ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber] || antennaNumber==7 || antennaNumber==8) {
      printw("  ----- ");
      if (antennaNumber != 8) {
	printw(" ");
      }
    } else {
#define WACKO_C90HUM_MIN -1
#define WACKO_C90HUM_MAX 102
      rm_status = rm_read(antennaNumber, "RM_CELESTRON_HUMIDITY_F", &floatvalue);
      checkStatus(rm_status,"rm_read(RM_CELESTRON_HUMIDITY_F)");
      rm_status = rm_read(antennaNumber, "RM_CELESTRON_TIMESTAMP_L", &timevalue);
      if (timevalue < rightnow-C90_STALE_INTERVAL) {
	printw("   stale ");
      } else {
	if (floatvalue < WACKO_C90HUM_MIN || floatvalue > WACKO_C90HUM_MAX) {
	  printw("  wacko  ");
	} else {
	  printw(" %6.2f  ", floatvalue);
	}
      }
    }
  }
  move(3,0);
  printw("C90 Temp  ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber] || antennaNumber==7||antennaNumber==8) {
      printw(" -----  ");
      if (antennaNumber != 8) {
	printw(" ");
      }
    } else {
      rm_status = rm_read(antennaNumber, "RM_CELESTRON_TIMESTAMP_L", &timevalue);
      if (timevalue < rightnow-C90_STALE_INTERVAL) {
	printw("   stale ");
      } else {
	rm_status = rm_read(antennaNumber, "RM_CELESTRON_TEMPERATURE_F", &floatvalue);
	checkStatus(rm_status,"rm_read(RM_CELESTRON_TEMPERATURE_F)");
#define WACKO_C90TEMP_MIN -50
#define WACKO_C90TEMP_MAX 100
	if (floatvalue < WACKO_C90TEMP_MIN || floatvalue > WACKO_C90TEMP_MAX) {
	  printw("  wacko  ");
	} else {
	  printw(" %5.2f   ",floatvalue);
	}
      }
    }
  }
  move(4,0);
  printw("C90Focus");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_CELESTRON_FOCUS_MOTOR_PRESENT_S",
        &focusMotorPresent);
      checkStatus(rm_status,"rm_read(RM_CELESTRON_FOCUS_MOTOR_PRESENT_S)");
      if (focusMotorPresent != 1 || antennaNumber==7 || antennaNumber==8) {
        printw("   none  ");
      } else {
#define WACKO_C90FOCUS_MIN 0.0001
#define WACKO_C90FOCUS_MAX 500
        rm_status = rm_read(antennaNumber, "RM_CELESTRON_FOCUS_MILS_F", &floatvalue);
        checkStatus(rm_status,"rm_read(RM_CELESTRON_FOCUS_MILS_F)");
        rm_status = rm_read(antennaNumber, "RM_CELESTRON_TIMESTAMP_L", &timevalue);
        if (timevalue < rightnow-C90_STALE_INTERVAL) {
          printw("   stale ");
        } else {
	  if (floatvalue < WACKO_C90FOCUS_MIN || floatvalue > WACKO_C90FOCUS_MAX) {
  	    printw("   wacko ");
	  } else {
  	    printw("  %6.1f ",floatvalue);
	  }
	}
      }
    }
  }
  move(6,0);
  printw("CCDMaxMin");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_CCD_MAX_PIXEL_VALUE_S",
        &maxPixel);
      rm_status = rm_read(antennaNumber, "RM_CCD_MIN_PIXEL_VALUE_S",
        &minPixel);
      if ((maxPixel == minPixel) && (maxPixel == -1)) {
	printw(" expose! ");
      } else {
#define MAX_PIXEL 255
	if (maxPixel < 0 || maxPixel > MAX_PIXEL) {
	  printw(" wac/");
	} else {
	  printw(" %3d/",maxPixel);
	}
	if (minPixel < 0 || minPixel > MAX_PIXEL) {
	  printw("wac ");
	} else {
	  printw("%3d ",minPixel);
	}
      }
    }
  }
  move(7,0);
  printw("Integrate");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_CCD_INTEGRATION_SEC_S",
        &milliseconds);
      if (milliseconds < 0 || milliseconds > 9999) {
	printw("  wacko  ");
      } else {
	if (milliseconds >= 1000) {
	  printw(" %4dmsec",milliseconds);
	} else {
	  printw(" %3dmsec ",milliseconds);
	}
      }
    }
  }
  move(8,0);
  printw("LastImage");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      /*
track updates the CCD_FITS_FILENAME value continuously, not just when 
an image is taken, so we cannot use it!
      */
      rm_status = rm_read(antennaNumber, "RM_CCD_TIMESTAMP_L", &timestamp);
      printw(" ");
      printAge(rightnow,timestamp);
      printw(" ");
    }
  }
  move(10,0);
  printw("OModelDate");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (antennaNumber<8) {
      printw(" ");
    }
    if (oldDate(dateString[antennaNumber],rightnow)) {
      standout();
    }
    printw("%s",dateString[antennaNumber]);
    standend();
    if (antennaNumber<8) {
      printw(" ");
    }
  }
  move(23,0);
  printw("                     Hit + or - to return to optics page");
  refresh();
}

