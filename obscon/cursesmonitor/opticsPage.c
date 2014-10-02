#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
#define COMBINER_WACKO 1000000

extern char *toddtime(time_t *, char *str);
extern char *hsttime(time_t *, char *str);
void checkStatus(int status, char *string);
extern void noBoard(int ant);
char *getLoBoardTypeStringBrief(int type);
char *getCalibrationWheelPosition(int position);
char *getCalibrationWheelPosition3Char(int position);
int invalidLoBoardType(int type);
int invalidCalibrationWheelPosition(int position);

void opticsPage(int count, int *rm_list, int pageMode, OPTICS_FLAGS *opticsFlags) 
{
  int rm_status;
  int isReceiverInArray[3];
  int actRxLow[MAX_NUMBER_ANTENNAS+1];
  int actRxHigh[MAX_NUMBER_ANTENNAS+1];
  char dummyString1[20];
  long feed;
  int ant, antennaNumber,rms;
  /*
  int rm_list[RM_ARRAY_SIZE];
  */
  long longvalue, otherBand;
#define NUMBER_OF_RECEIVERS 8
#define NUMBER_OF_ANTENNAS 8
  long gridtable[NUMBER_OF_ANTENNAS+1][NUMBER_OF_RECEIVERS];
  long combinertable[NUMBER_OF_ANTENNAS+1][NUMBER_OF_RECEIVERS];
  long anotherlongvalue;
  int mismatch;
  int band;
  short shortvalue, shortvalue2;
  short opticsBoardPresent[9];
  short style;
  double nst;
  float floatvalue;
  long rightnow, timevalue;
  time_t system_time;
  int doWeCare[11];
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  short busy;
  short combinerMirrorMotorized[NUMBER_OF_ANTENNAS+1];
  FILE *fp;
  char line[200];
  static char dateString[NUMBER_OF_ANTENNAS+1][20];
  char filename[200];
  int isAntennaInArray[20];
  short opticalBreaks[8];

  getAntennaList(doWeCare);
  getAntennaList(isAntennaInArray);
  /* first determine if the grid is in a proper place */
  for (ant = 1; ant <= 8; ant++) {
    opticsFlags->flags[ant] = opticsFlags->gridRx[ant] =  opticsFlags->wavePlate[ant] =
      opticsFlags->combinerRx[ant] = opticsFlags->feedOffsetMismatch[ant] = 0;
    rm_status = rm_read(ant, "RM_OPTICS_BOARD_PRESENT_S",
		    &opticsBoardPresent[ant]);
    if (antsAvailable[ant] && opticsBoardPresent[ant]) {
      rm_status = rm_read(ant,"RM_WIRE_GRID_RECEIVER_LOCATION_L", &longvalue);
      checkStatus(rm_status,"rm_read(RM_WIRE_GRID_RECEIVER_LOCATION_L)");
      if (invalidLoBoardType(longvalue) && doWeCare[ant]==1) {
	opticsFlags->gridRx[ant] = 1; 
      } else {
	opticsFlags->gridRx[ant] = 0; 
      }
    } else {
      opticsFlags->gridRx[ant] = 0; 
    }
    rm_status = rm_read(ant, "RM_WAVEPLATE_STATUS_S", &shortvalue);
    if (shortvalue == LINEAR_LOAD_OUT_BREAK)
      opticsFlags->wavePlate[ant] = 0;
    else
      opticsFlags->wavePlate[ant] = 1;
    if (antsAvailable[ant]) {
      rms = rm_read(ant, "RM_ACTIVE_LOW_RECEIVER_C10", &dummyString1);
      if (dummyString1[1] == (char)0) {
	dummyString1[1] = ' ';
      }
      dummyString1[2] = (char)0;
      actRxLow[ant] = parseReceiverName(dummyString1);
      rms = rm_read(ant, "RM_ACTIVE_HIGH_RECEIVER_C10", &dummyString1);
      if (dummyString1[1] == (char)0) {
	dummyString1[1] = ' ';
      }
      dummyString1[2] = (char)0;
      actRxHigh[ant] = parseReceiverName(dummyString1);
      rm_status = rm_read(ant,"RM_FEED_L",&feed);
      getReceiverList(isReceiverInArray);
      mismatch = 0;
      if (isReceiverInArray[2] == 0 && isAntennaInArray[ant] == 1 &&
	  ((actRxLow[ant] == 0 && feed != 230) ||
	   (actRxLow[ant] == 2 && feed != 345))) {
	mismatch = 1;
      }
      if (isReceiverInArray[2]!=0 && 
	  ((actRxLow[ant]==0  && feed == 345) ||
	   (actRxLow[ant]==2  && feed == 230) ||
	   (actRxHigh[ant]==4 && feed == 690) ||
	   (actRxHigh[ant]==6 && feed == 400))) {
	mismatch = 1;
      }
      if (mismatch) {
	opticsFlags->feedOffsetMismatch[ant] = mismatch; 
      } else {
	opticsFlags->feedOffsetMismatch[ant] = 0; 
      }
    }
    if (antsAvailable[ant] && opticsBoardPresent[ant]) {
      rm_status= rm_read(ant, "RM_COMBINER_MIRROR_MOTORIZED_S", 
			 &combinerMirrorMotorized[ant]);
      checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_MOTORIZED_S)");
      if (combinerMirrorMotorized[ant]) {
	rm_status = rm_read(ant,"RM_COMBINER_MIRROR_RECEIVER_LOCATION_L", 
			    &longvalue);
	checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_RECEIVER_LOCATION_L)");
	if (invalidLoBoardType(longvalue) && doWeCare[ant]==1 && 
	    isReceiverInArray[2] == 1) {
	  opticsFlags->combinerRx[ant] = 1; 
	} else {
	  opticsFlags->combinerRx[ant] = 0; 
	}
      }
    } else {
      opticsFlags->combinerRx[ant] = 0; 
    }
    opticsFlags->flags[ant] = opticsFlags->gridRx[ant] | 
      opticsFlags->combinerRx[ant] | opticsFlags->feedOffsetMismatch[ant] |
      opticsFlags->wavePlate[ant];
  }
  if (pageMode == OPTICS_PAGE_CHECK_ONLY) {
    return;
  }
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
#if 0
  printw("   Optics status on %s UT = %s HST",timeString,timeString2);
  antsAvailable[8] = 0;
  move(1,0);
#else
  printw("OpticsPage   1        2        3        4        5        6        7        8");
#endif
  move(1,9);
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber,"RM_WIRE_GRID_RECEIVER_LOCATION_L", &longvalue);
      checkStatus(rm_status,"rm_read(RM_WIRE_GRID_RECEIVER_LOCATION_L)");
      rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
      if (combinerMirrorMotorized[antennaNumber]) {
	rm_status = rm_read(antennaNumber,"RM_COMBINER_MIRROR_RECEIVER_LOCATION_L", 
			    &otherBand);
	checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_RECEIVER_LOCATION_L)");
      } else {
	otherBand = 6;
      }
      if (timevalue < rightnow-GRID_STALE_INTERVAL) {
	if (doWeCare[antennaNumber]==1) {
	  standout();
	}
	printw("   stale ");
	if (doWeCare[antennaNumber]==1) {
	  standend();
	}
      } else {
	if (opticsFlags->gridRx[antennaNumber]==1) {
	  standout();
	}
        rm_status = rm_read(antennaNumber, "RM_TUNE6_COMMAND_BUSY_S", &busy);
	switch (busy) {
        case TUNE6_BUSY_WITH_WIRE_GRID:
	  printw("turn/");
	  band = translateBandNumberToPort(otherBand);
	  if (band == 0) {
	    standout();
	  }
	  printw("%2d ",band);
	  standend();
	  printw(" ");
	  break;
        case TUNE6_BUSY_WITH_COMBINER_MIRROR:
	  band = translateBandNumberToPort(longvalue);
	  printw(" ");
	  if (band == 0) {
	    standout();
	  }
          printw("%2d ",band);
	  standend();
	  printw("/turn");
	  break;
	default:
	  printw(" ");
	  band = translateBandNumberToPort(longvalue);
	  if (band == 0) {
	    standout();
	  }
          printw("%2d ",band);
	  standend();
	  printw("/");
	  band = translateBandNumberToPort(otherBand);
	  if (band == 0) {
	    standout();
	  }
	  printw("%2d ",band);
	  standend();
	  printw(" ");
	  break;
	}
	if (opticsFlags->gridRx[antennaNumber]==1) {
	  standend();
	}
      }
    }
  }
  move(1,0);
  printw("PortNumber");


  move(2,0);
  printw("Grid/Mirr");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    rm_status = rm_read(antennaNumber, "RM_OPTICAL_BREAKS_V8_S", opticalBreaks);
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_HOTLOAD_STYLE_S",&style);
      rm_status = rm_read(antennaNumber,"RM_WIRE_GRID_RECEIVER_LOCATION_L", &longvalue);
      checkStatus(rm_status,"rm_read(RM_WIRE_GRID_RECEIVER_LOCATION_L)");
      rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
      if (combinerMirrorMotorized[antennaNumber]) {
	rm_status = rm_read(antennaNumber,"RM_COMBINER_MIRROR_RECEIVER_LOCATION_L", 
			    &otherBand);
	checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_RECEIVER_LOCATION_L)");
      } else {
	otherBand = 6;
      }
      if (timevalue < rightnow-GRID_STALE_INTERVAL) {
	if (doWeCare[antennaNumber]==1) {
	  standout();
	}
	printw("   stale ");
	if (doWeCare[antennaNumber]==1) {
	  standend();
	}
      } else {
	if (opticsFlags->gridRx[antennaNumber]==1) {
	  standout();
	}
        rm_status = rm_read(antennaNumber, "RM_TUNE6_COMMAND_BUSY_S", &busy);
	switch (busy) {
        case TUNE6_BUSY_WITH_WIRE_GRID:
	  printw("turn/");
	  band = translateBandNumberToGHz(otherBand);
	  if (band == 0) {
	    standout();
	  }
	  printw("%3d",band);
	  standend();
	  printw(" ");
	  break;
        case TUNE6_BUSY_WITH_COMBINER_MIRROR:
	  band = translateBandNumberToGHz(longvalue);
	  printw(" ");
	  if (band == 0) {
	    standout();
	  }
          printw("%3d",band);
	  standend();
	  printw("/turn");
	  break;
	default:
	  band = translateBandNumberToGHz(longvalue);
	  if (band == 0) {
	    standout();
	  }
	  if (opticalBreaks[OPTICAL_BREAK_WIRE_GRID] == 0) {
	    printw("home");
	    standend();
	  } else {
	    printw(" ");
	    printw("%3d",band);
	    standend();
	  }
	  printw("/");
	  band = translateBandNumberToGHz(otherBand);
	  if (band == 0) {
	    standout();
	  }
	  if (opticalBreaks[OPTICAL_BREAK_COMBINER_MIRROR] == 0 &&
	      style != NEW_HOTLOAD_STYLE) {
	    printw("home");
	    standend();
	  } else {
	    printw("%3d",band);
	    standend();
	    printw(" ");
	  }
	  break;
	}
	if (opticsFlags->gridRx[antennaNumber]==1) {
	  standend();
	}
      }
    }
  }
  move(4,0);
  printw("Table230 ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_WIRE_GRID_TABLE_V8_L",gridtable[antennaNumber]);
      checkStatus(rm_status,"rm_read(RM_WIRE_GRID_TABLE_V8_L)");
      if (gridtable[antennaNumber][0] > WACKO_WIRE_GRID_MAX || 
	  gridtable[antennaNumber][0] < WACKO_WIRE_GRID_MIN) {
	if (doWeCare[antennaNumber]==1) {
	  standout();
	}
	printw("   wacko ");
	if (doWeCare[antennaNumber]==1) {
	  standend();
	}
      } else {
        printw("  %6d ",gridtable[antennaNumber][0]);
      }
    }
  }
  move(5,0);
  printw("Table345 ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) {
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (gridtable[antennaNumber][2] > WACKO_WIRE_GRID_MAX || 
	  gridtable[antennaNumber][2] < WACKO_WIRE_GRID_MIN) {
	if (doWeCare[antennaNumber]==1) {
	  standout();
	}
	printw("   wacko ");
	if (doWeCare[antennaNumber]==1) {
	  standend();
	}
      } else {
	printw("  %6d",gridtable[antennaNumber][2]);
	if (antennaNumber < NUMANTS) {
          addstr(" ");
	}
      }
    }
  }

  move(6,0);
  printw("Grid Enc ");
  move(7,0);
  printw("Grid Tbl ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    int col;
    int count, table;
    int wgTimestamp;

    (void)rm_read(antennaNumber, "RM_WIRE_GRID_TIMESTAMP_L", &wgTimestamp);
    col = antennaNumber*9;
    if(abs(rightnow - wgTimestamp) < 10) {
      move(6, col);
      rm_status = rm_read(antennaNumber, "RM_WIRE_GRID_POSITION_L", &count);
      rm_status = rm_read(antennaNumber, "RM_WIRE_GRID_NOMINAL_POSITION_L",
        &table);
      if(abs(count-table) > 15) standout();
      printw("  %6d",count);
      standend();
      move(7, col);
      printw("  %6d",table);
    } else {
      move(6, col);
      addstr(" -Stale-");
      move(7, col);
      addstr(" -Stale-");
    }
  }

  move(11,0);
  printw("Comb Enc ");
  move(12,0);
  printw("Comb Tbl ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    int col;
    int count, table;
    int cmTimestamp;

    (void)rm_read(antennaNumber, "RM_COMBINER_MIRROR_TIMESTAMP_L", &cmTimestamp);
    col = antennaNumber*9;
    if(abs(rightnow - cmTimestamp) < 10) {
      move(11, col);
      rm_status = rm_read(antennaNumber, "RM_COMBINER_MIRROR_POSITION_L", &count);
      rm_status = rm_read(antennaNumber, "RM_COMBINER_MIRROR_NOMINAL_POSITION_L",
			  &table);
      if(abs(count-table) > 15) standout();
      if (abs(count) >= COMBINER_WACKO)
	printw("   Wacko");
      else
	printw("  %6d",count);
      standend();
      move(12, col);
      if (abs(table) >= COMBINER_WACKO)
	printw("   Wacko");
      else
	printw("  %6d",table);
    } else {
      move(11, col);
      addstr(" -Stale-");
      move(12, col);
      addstr(" -Stale-");
    }
  }

  move(3,0);
  printw("Grid Pos ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
#define GRID_ENCODER_TOLERANCE 100 /* this is set to 300 in tune6's motor.c */
      rm_status = rm_read(antennaNumber, "RM_TUNE6_COMMAND_BUSY_S", &busy);
      switch (busy) {
      case TUNE6_BUSY_WITH_WIRE_GRID:
	standout();
	printw(" turning ");
	standend();
	break;
      default:
	rm_status = rm_read(antennaNumber, "RM_WIRE_GRID_ENCODER_POSITION_L", &longvalue);
	checkStatus(rm_status,"rm_read(RM_WIRE_GRID_ENCODER_POSITION_L)");
	if (longvalue > WACKO_WIRE_GRID_MAX || longvalue < WACKO_WIRE_GRID_MIN) {
	  if (doWeCare[antennaNumber]==1) {
	    standout();
	  }
	  printw("   wacko "); 
	  if (doWeCare[antennaNumber]==1) {
	    standend();
	  }
	} else {
	  if ((abs(longvalue-gridtable[antennaNumber][0]) > GRID_ENCODER_TOLERANCE) &&
	      (abs(longvalue-gridtable[antennaNumber][2]) > GRID_ENCODER_TOLERANCE)) {
	    standout();
	  }
	  printw("  %6d",longvalue);
	  standend();
	  if (antennaNumber < 8) printw(" ");
	  break;
	}
      }
    }
  }

  move(8,0);
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsFlags->feedOffsetMismatch[antennaNumber] == 1) {
    /* if any antenna shows a mismatch, then print the status of all of them */
      printw("FeedOffset");
      for (ant=1; ant<=NUMBER_OF_ANTENNAS; ant++) { 
	if (opticsFlags->feedOffsetMismatch[ant] == 1) {
	  standout();
	  printw("mismatch");
	  standend();
	  printw(" ");
	} else {
	  printw("   okay  ");
	}
      }
      break;
    } else if (antennaNumber==NUMBER_OF_ANTENNAS) {
      printw("\n");
    }
  }
  move(9,0);
  printw("Table400 ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (combinerMirrorMotorized[antennaNumber] != 1) {
	if (antennaNumber == 7 || antennaNumber ==8) {
          printw("    none ");
	} else {
          printw("  manual ");
	}
      } else {
        rm_status = rm_read(antennaNumber, "RM_COMBINER_MIRROR_TABLE_V8_L", 
			    combinertable[antennaNumber]);
        checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_TABLE_V8_L)");
        printw("  %6d ",combinertable[antennaNumber][4]);
      }
    }
  }
  move(10,0);
  printw("Table690 ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (combinerMirrorMotorized[antennaNumber] != 1) {
	if (antennaNumber == 7 || antennaNumber ==8) {
          printw("    none ");
	} else {
          printw("  manual ");
	}
      } else {
        printw("  %6d",combinertable[antennaNumber][6]);
	if (antennaNumber < NUMANTS) {
          addstr(" ");
	}
      }
    }
  }
  move(8,0);
  printw("MirrorPos");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) {
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----");
      if (antennaNumber < NUMBER_OF_ANTENNAS) {
	printw("  ");
      }
    } else {
      rm_status= rm_read(antennaNumber, "RM_COMBINER_MIRROR_MOTORIZED_S", &short\
value);
      checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_MOTORIZED_S))");
      if (shortvalue != 1) {
	printw("  manual");
	if (antennaNumber < 8) printw(" ");
      } else {
	rm_status = rm_read(antennaNumber, "RM_TUNE6_COMMAND_BUSY_S", &busy);
	switch (busy) {
	case TUNE6_BUSY_WITH_COMBINER_MIRROR:
	  standout();
	  printw(" turning ");
	  standend();
	  break;
	default:
	  rm_status = rm_read(antennaNumber, "RM_COMBINER_MIRROR_ENCODER_POSITION_L", &longvalue);
	  checkStatus(rm_status,"rm_read(RM_COMBINER_MIRROR_ENCODER_POSITION_L))");
#define COMBINER_ENCODER_TOLERANCE 100 /*this is set = 300 in tune6's motor.c*/
	  if ((abs(longvalue-combinertable[antennaNumber][4]) > 
	       COMBINER_ENCODER_TOLERANCE) &&
	      (abs(longvalue-combinertable[antennaNumber][6]) > 
	       COMBINER_ENCODER_TOLERANCE) &&
	      (abs(longvalue-combinertable[antennaNumber][7]) > 
	       COMBINER_ENCODER_TOLERANCE)) {
	    standout();
	  }
	  printw("  %6d",longvalue);
          if (antennaNumber < NUMANTS) {
            addstr(" ");
          }
	  standend();
	  break;
	}
      }
    }
  }
#define AMBIENT_TEMP_WACKO_MIN -10
#define AMBIENT_TEMP_WACKO_MAX 100
  move(14,0);
  printw("WaveplPos");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
      switch (shortvalue) {
      case NEW_HOTLOAD_STYLE:
	rm_status = rm_read(antennaNumber, "RM_OPTICAL_BREAKS_V8_S", opticalBreaks);
	rm_status = rm_read(antennaNumber, "RM_WAVEPLATE_STATUS_S", &shortvalue);
	rm_status = rm_read(antennaNumber, "RM_WAVEPLATE_POSITION_L", &longvalue);
	printw("%5d/",longvalue);
#if 1
	  printLinearLoadStatus(shortvalue);
#else
	  if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_IN]==0) {
	    if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_SKY]==0) {
	      printw("wac");
	    } else {
	      printw("in ");
	    }
	  } else if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_SKY]==0) {
	    if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_IN]==0) {
	      printw("wac");
	    } else {
	      printw("out");
	    }
	  } else {
	    printw(" ? ");
	  }
#endif
	break;
      case ORIGINAL_HOTLOAD_STYLE:
	printw("         ");
	/*
	printw("    n/a  ");
	*/
	break;
      default:
	printw("  wacko  ");
	break;
      } /* end switch */
    }
  }
  move(15,0);
  printw("HotVnPos ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
      switch (shortvalue) {
      case NEW_HOTLOAD_STYLE:
	rm_status = rm_read(antennaNumber, "RM_OPTICAL_BREAKS_V8_S", opticalBreaks);
	rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_STATUS_S", &shortvalue);
	rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_POSITION_L", &longvalue);
	if (abs(longvalue) >= 1000000) {
	  printw(" wacko  ");
	} else {
	  printw("%5d/",longvalue);
#if 1
	  printLinearLoadStatus(shortvalue);
#else
	  if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_IN]==0) {
	    if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_SKY]==0) {
	      printw("wac");
	    } else {
	      printw("in ");
	    }
	  } else if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_SKY]==0) {
	    if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_IN]==0) {
	      printw("wac");
	    } else {
	      printw("out");
	    }
	  } else {
	    printw(" ? ");
	  }
#endif
	}
	break;
      case ORIGINAL_HOTLOAD_STYLE:
	printw("         ");
	/*
	printw("    n/a  ");
	*/
	break;
      default:
	printw("  wacko  ");
	break;
      } /* end switch */
    }
  }
  move(16,0);
  printw("HotVnTemp");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
      if (shortvalue == ORIGINAL_HOTLOAD_STYLE) {
	printw("         ");
      } else {
	rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_TEMPERATURE_F", &floatvalue);
	if (floatvalue < 0 || floatvalue > 200) {
	  printw("  wacko  ");
	} else {
	  printw("  %6.2f ",floatvalue);
	}
      }
    }
  }

  move(17,0);
  printw("HotServoT ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
      switch (shortvalue) {
      case NEW_HOTLOAD_STYLE:
	rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_SERVO_TEMP_F", &floatvalue);
	checkStatus(rm_status,"rm_read(RM_HEATEDLOAD_SERVO_TEMP_F)");
	rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_TIMESTAMP_L", &timevalue);
	if (timevalue < rightnow-TUNE6_STALE_INTERVAL) {
	  printw("   stale ");
	} else {
	  if ((floatvalue < 0) ||
	      (floatvalue >= 200)) {
	    printw(" wacko   ");
	  } else {
	    printw("%7.1f  ",floatvalue);
#if 0
	    rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_SET_POINT_F", &floatvalue);
	    printw("%2.0f ", floatvalue);
#endif
	  }
	}
	break;
      case ORIGINAL_HOTLOAD_STYLE:
	printw("         ");
	/*
	printw("    n/a  ");
	*/
	break;
      default:
	printw("  wacko  ");
	break;
      } /* end switch */
    }
  }

  move(18,0);
  printw("AmbVnTemp");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
      if (shortvalue == ORIGINAL_HOTLOAD_STYLE) {
	rm_status = rm_read(antennaNumber, "RM_AMBIENTLOAD_TEMPERATURE_F", &floatvalue);
	checkStatus(rm_status,"rm_read(RM_AMBIENTLOAD_TEMPERATURE_F)");
	rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
	if (timevalue < rightnow-TUNE6_STALE_INTERVAL) {
	  printw("   stale ");
	} else {
	  if ((floatvalue < AMBIENT_TEMP_WACKO_MIN) ||
	      (floatvalue >= AMBIENT_TEMP_WACKO_MAX)) {
	    printw("   wacko ");
	  } else {
	    printw("  %6.1f ",floatvalue);
	  }
	}
      } else {
	/* new  hotload */
	rm_status = rm_read(antennaNumber, "RM_UNHEATEDLOAD_TEMPERATURE_F",
		&floatvalue);
	checkStatus(rm_status,"rm_read(RM_UNHEATEDLOAD_TEMPERATURE_F)");

	rm_status = rm_read(antennaNumber, "RM_HEATEDLOAD_TIMESTAMP_L", &timevalue);
	if (timevalue < rightnow-TUNE6_STALE_INTERVAL) {
	  printw("   stale ");
	} else {
	  if ((floatvalue < AMBIENT_TEMP_WACKO_MIN) ||
	      (floatvalue >= AMBIENT_TEMP_WACKO_MAX)) {
	    standout();
	    printw("   wacko ");
	  } else {
	    /* check to see if the unheated load thermometer is alive or not */
	    /* because it is written to the following RM variable by */
	    /* calvaneservo, even if the value is wacko */
	    rm_status = rm_read(antennaNumber, "RM_NOISE_SOURCE_TEMPERATURE_D",
		&nst);
	    if (nst < AMBIENT_TEMP_WACKO_MIN ||
		(nst >= AMBIENT_TEMP_WACKO_MAX)) {
	      standout();
	    }
	    printw("  %6.1f ",floatvalue);
	  }
	  standend();
	}
      }
    }
  }
  move(19,0);
  printw("AmbVnPos ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
      switch (shortvalue) {
      case NEW_HOTLOAD_STYLE:
	rm_status = rm_read(antennaNumber, "RM_OPTICAL_BREAKS_V8_S", opticalBreaks);
	rm_status = rm_read(antennaNumber, "RM_UNHEATEDLOAD_STATUS_S", &shortvalue);
	rm_status = rm_read(antennaNumber, "RM_UNHEATEDLOAD_POSITION_L", &longvalue);
	if (abs(longvalue) >= 1000000) {
	  printw(" wacko  ");
	} else {
	  printw("%5d/",longvalue);
#if 1
	  printLinearLoadStatus(shortvalue);
#else
	  if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_IN]==0) {
	    if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_SKY]==0) {
	      printw("wac");
	    } else {
	      printw("in ");
	    }
	  } else if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_SKY]==0) {
	    if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_IN]==0) {
	      printw("wac");
	    } else {
	      printw("out");
	    }
	  } else {
	    printw(" ? ");
	  }
#endif
	}
	break;
      case ORIGINAL_HOTLOAD_STYLE:
	rm_status = rm_read(antennaNumber, "RM_AMBIENTLOAD_POSITION_L", &longvalue);
	checkStatus(rm_status,"rm_read(RM_AMBIENTLOAD_POSITION_L)");
	rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
	rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_S", &shortvalue);
	checkStatus(rm_status,"rm_read(RM_CALIBRATION_WHEEL_S)");
	rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
	if (timevalue < rightnow-TUNE6_STALE_INTERVAL) {
	  printw("   stale ");
	} else {
	  if (abs(longvalue) > 9999) {
	    printw("wack/",longvalue);
	  } else {
	    printw("%4d/",longvalue);
	  }
	  if (invalidCalibrationWheelPosition(shortvalue) && doWeCare[antennaNumber]==1) {
	    standout();
	  }
	  printw("%3s ",getCalibrationWheelPosition3Char(shortvalue));
	  if (invalidCalibrationWheelPosition(shortvalue) && doWeCare[antennaNumber]==1) {
	    standend();
	  }
	}
      }
    }
  }

  move(20,0);
  printw("VnIn/Out ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    rm_status = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S", &shortvalue);
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else if (shortvalue == NEW_HOTLOAD_STYLE) {
      rm_status = rm_read(antennaNumber,"RM_HEATEDLOAD_CMDVOLTAGE_F",&floatvalue);
      if (fabs(floatvalue) > 1000) {
	printw("  wacko ");
      } else {
	printw("  %6.3f ",floatvalue);
      }
    } else {
      rm_status = rm_read(antennaNumber, "RM_AMBIENTLOAD_INPOS_L", &longvalue);
      checkStatus(rm_status,"rm_read(RM_AMBIENTLOAD_INPOS_L)");
      rm_status = rm_read(antennaNumber, "RM_AMBIENTLOAD_OUTPOS_L", &anotherlongvalue);
      checkStatus(rm_status,"rm_read(RM_AMBIENTLOAD_OUTPOS_L)");
      rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
      if (timevalue < rightnow-TUNE6_STALE_INTERVAL) {
	printw("   stale ");
      } else {
#define WACKO_VANE_MAX 1024
#define WACKO_VANE_MIN 0
	if (longvalue > WACKO_VANE_MAX || longvalue < WACKO_VANE_MIN) {
	  printw(" wac/");
	} else {
	  printw(" %3d/",longvalue);
	}
	if (anotherlongvalue>WACKO_VANE_MAX || anotherlongvalue<WACKO_VANE_MIN) {
	  printw("wac ");
	} else {
	  printw("%3d ", anotherlongvalue);
	}
      }
    }
  }

  move(13,0);
  printw("PROM/RAM ");
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (opticsBoardPresent[antennaNumber] == 0) {
      noBoard(antennaNumber);
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_OPTICS_BOARD_EPROM_VERSION_S", &shortvalue);
      checkStatus(rm_status,"rm_read(RM_OPTICS_BOARD_EPROM_VERSION_S)");
      rm_status = rm_read(antennaNumber, "RM_OPTICS_BOARD_SRAM_VERSION_S", &shortvalue2);
      checkStatus(rm_status,"rm_read(RM_OPTICS_BOARD_SRAM_VERSION_S)");
      rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
      if (timevalue < rightnow-TUNE6_STALE_INTERVAL) {
	printw("   stale");
	if (antennaNumber < 8) {
	  printw(" ");
	}
      } else {
	if (shortvalue < 1) {
	  printw("  wac/");
	} else {
	  printw(" 0x%02x/",shortvalue);
	}
	if (shortvalue2 < 1) {
	  printw("wac");
	} else {
	  printw("%02x",shortvalue2);
	}
	if (antennaNumber < 8) {
	  printw(" ");
	}
      }
    }
  }
  move(23,0);
  printw("                          Hit + or - to for opTel page");
  refresh();
}

void checkStatus(int status, char *string) {
    if (status != RM_SUCCESS) {
      rm_error_message(status,string);
      exit(1);
    }
}

int invalidLoBoardType(int type) {
  if (type < LO_BOARD_A1_TYPE || type > LO_BOARD_F_TYPE) {
    return(1);
  } else {
    return(0);
  }
}

int invalidCalibrationWheelPosition(int position) {
  switch (position) {
          case AMBIENT_IN:
          case SKY_IN:
          case HOTLOAD_IN:
          case WAVEPLATE_IN:
            return(0);
          default:
	    return(1);
  }
}

char *getCalibrationWheelPosition(int position) {
          switch (position) {
          case AMBIENT_IN:
            return("ambient");
          case SKY_IN:
            return("sky");
          case HOTLOAD_IN:
            return("hotload");
          case WAVEPLATE_IN:
            return("waveplate");
          default:
	    return("unknown");
          }
}

char *getCalibrationWheelPosition3Char(int position) {
          switch (position) {
          case AMBIENT_IN:
            return("amb");
          case SKY_IN:
            return("sky");
          case HOTLOAD_IN:
            return("hot");
          case WAVEPLATE_IN:
            return("wav");
          default:
	    return("unk");
          }
}

#if 0
void noBoard(int ant) {
  printw("No Board");
  if (ant < NUMANTS) {
    addstr(" ");
  }
}
#endif

int translateBandNumberToPort(int band) {
  switch (band) {
  case 0: return(3);
  case 1: return(4);
  case 2: return(7);
  case 3: return(8);
  case 4: return(5);
  case 5: return(6);
  case 6: return(1);
  case 7: return(2);
  default: return(0);
  }
}

void printLinearLoadStatus(short shortvalue) {
  switch (shortvalue) {
  case LINEAR_LOAD_IN:
    standout();
    printw("in");
    standend();
    printw(" ");
    break;
  case LINEAR_LOAD_OUT:
    printw("out");
    break;
  case LINEAR_LOAD_IN_BREAK:
    standout();
    printw("In");
    standend();
    printw(" ");
    break;
  case LINEAR_LOAD_OUT_BREAK:
    printw("Out");
    break;
  case LINEAR_LOAD_UNCERTAIN:
    printw(" ? ");
    break;
  case LINEAR_LOAD_WACKO:
  default:
    printw("wac");
  }
}

int printLinearLoadStatusWordStandout(int antenna) {
  standout();
  
  return(printLinearLoadStatusWord(antenna));
}

int printLinearLoadStatusWordSky(int antenna) {
  int retval = printLinearLoadStatusWord(antenna);

  switch (retval) {
  case -LINEAR_LOAD_OUT_BREAK:
    printw("  Sky  ");
    break;
  case -LINEAR_LOAD_OUT:
    printw("  sky  ");
    break;
  }
  return(retval);
}

int printLinearLoadStatusWord(int antenna) {
  short dummyShort;
  int retval = 0;
  int rm_status;

  short busy;
  rm_status = rm_read(antenna, "RM_TUNE6_COMMAND_BUSY_S", &busy);
  if (TUNE6_BUSY_WITH_LINEAR_LOAD == busy) {
    printw("moving");
    standend();
    printw(" ");
    return(retval);
  }

  /* must check the unheated load first, as it is in front of the heated load */
  (void)rm_read(antenna,"RM_UNHEATEDLOAD_STATUS_S",&dummyShort);
  switch (dummyShort) {
  case LINEAR_LOAD_IN:
    printw("ambient");
    break;
  case LINEAR_LOAD_IN_BREAK:
    printw("Ambient");
    break;
  case LINEAR_LOAD_OUT:
  case LINEAR_LOAD_OUT_BREAK:
  case LINEAR_LOAD_UNCERTAIN:
  case LINEAR_LOAD_WACKO:
  default:
    (void)rm_read(antenna,"RM_HEATEDLOAD_STATUS_S",&dummyShort);
    switch (dummyShort) {
    case LINEAR_LOAD_IN:
      printw(" heated");
      break;
    case LINEAR_LOAD_IN_BREAK:
      printw(" Heated");
      break;
    case LINEAR_LOAD_OUT:
      retval = -LINEAR_LOAD_OUT;
      break;
    case LINEAR_LOAD_OUT_BREAK:
      retval = -LINEAR_LOAD_OUT_BREAK;
      break;
    case LINEAR_LOAD_UNCERTAIN:
    case LINEAR_LOAD_WACKO:
    default:
      retval = -1;
      standend();
      printw(" ");
      break;
    }
  }
  standend();
  return(retval);
}

void printLinearLoadStatusChar(int antenna) {
  short dummyShort;
  /* must check the unheated load first, as it is in front of the heated load */
  (void)rm_read(antenna,"RM_UNHEATEDLOAD_STATUS_S",&dummyShort);
  standout();
  switch (dummyShort) {
  case LINEAR_LOAD_IN:
    printw("a");
    break;
  case LINEAR_LOAD_IN_BREAK:
    printw("A");
    break;
  case LINEAR_LOAD_OUT:
  case LINEAR_LOAD_OUT_BREAK:
  case LINEAR_LOAD_UNCERTAIN:
  case LINEAR_LOAD_WACKO:
  default:
    (void)rm_read(antenna,"RM_HEATEDLOAD_STATUS_S",&dummyShort);
    switch (dummyShort) {
    case LINEAR_LOAD_IN:
      printw("h");
      break;
    case LINEAR_LOAD_IN_BREAK:
      printw("H");
      break;
    case LINEAR_LOAD_OUT:
    case LINEAR_LOAD_OUT_BREAK:
    case LINEAR_LOAD_UNCERTAIN:
    case LINEAR_LOAD_WACKO:
    default:
      standend();
      printw(" ");
      break;
    }
  }
  standend();
}
