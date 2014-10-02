#define STUCK_MOTOR_VALUE -10
#define FILTER_RATIO_STANDOUT 9 
#define RECENTER_THRESHOLD_STANDOUT 0.4
#define DEBUG 0
#define NO_ASIAA_650 0
#include <curses.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include "commonLib.h"
#include "monitor.h"

#define LO_MOTOR_MIN -999999
#define LO_MOTOR_MAX  999999

#define TUNE6_STALE_INTERVAL 10
#define TUNE6_MOTOR_STALE_INTERVAL 100
enum {GUNN_TUNER=0, GUNN_BACKSHORT, ATTENUATOR, MULT_INPUT, MULT_OUTPUT, DIPLEXER};

extern int antsAvailable[RM_ARRAY_SIZE];

extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *, char *);
extern void checkStatus(int status, char *string);
extern int motorizedMultiplier[MAX_NUMBER_ANTENNAS+1][NUMBER_OF_RECEIVERS];
int invalidSlave(int sl);

void lomotorpage(int count, int *rm_list) {
  int use345at230[MAX_NUMBER_ANTENNAS+1];
  int showSecondReceiver;
  int rm_status;
  int ant, antennaNumber;
#define MOTOR_ARRAY_SIZE 9
  long tablearray[NUMBER_OF_RECEIVERS];
  long long2array[NUMBER_OF_RECEIVERS][MOTOR_ARRAY_SIZE];
  short shortarray[NUMBER_OF_RECEIVERS];
  short shortarray2[NUMBER_OF_RECEIVERS];
  short shortvalue = 0;
  float floatvalue;
  short actPLL[MOTOR_ARRAY_SIZE];
  short actPLLhigh[MOTOR_ARRAY_SIZE];
  short boardSubType[15];
  short loboardPresent[MAX_NUMBER_ANTENNAS+1][NUMBER_OF_RECEIVERS];
  long rightnow, timevalue;
#define INITIAL_LAST_MOTOR_VALUE -999
  static int lastGunnTuner[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE
  };
  static int lastGunnBackshort[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE
  };
  static int lastAtten[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE,
					    INITIAL_LAST_MOTOR_VALUE
  };
  static int lastMultInput[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE
  };
  static int lastMultOutput[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE,
						 INITIAL_LAST_MOTOR_VALUE
  };

  static int lastGunnTunerHigh[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE
  };
  static int lastGunnBackshortHigh[MOTOR_ARRAY_SIZE]={INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE,
						      INITIAL_LAST_MOTOR_VALUE
  };
  static int lastAttenHigh[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE,
						INITIAL_LAST_MOTOR_VALUE
  };
  static int lastMultInputHigh[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE,
						    INITIAL_LAST_MOTOR_VALUE
  };
  static int lastMultOutputHigh[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE
  };
  static int lastDiplexerHigh[MOTOR_ARRAY_SIZE] = {INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE,
						     INITIAL_LAST_MOTOR_VALUE
  };
  char stringvalue[11];
  int rxnumber[MOTOR_ARRAY_SIZE];
  int rxnumberHigh[MOTOR_ARRAY_SIZE];
  int doWeCare[11];
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */

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
  }
  getAntennaList(doWeCare);
  ant = 0;
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  /*  printf("now=%d\n",rightnow);*/
  /*
  antsAvailable[8] = 0;
  */

  move(0,0);
  if (DEBUG) refresh();
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("LO Motor page on %s UT = %s HST",timeString,timeString2);
  move(1,0);
  if (DEBUG) refresh();
  printw("Antenna      1        2        3        4        5        6        7        8");
  if (numberAntennas > 8) {
    printw("      CSO    JCMT");
  }
  move(2,0);
  if (DEBUG) refresh();
  printw("Rx/slave ");
  for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_SLAVE1_PLL_NO_S", &shortvalue);
      checkStatus(rm_status,"rm_read(RM_SLAVE1_PLL_NO_S)");
      rm_status = rm_read(antennaNumber, "RM_ACTIVE_LOW_RECEIVER_C10", stringvalue);
      checkStatus(rm_status,"rm_read(RM_ACTIVE_LOW_RECEIVER_C10)");
      rxnumber[antennaNumber] = parseReceiverName(stringvalue);
      actPLL[antennaNumber] = shortvalue;
      if (rxnumber[antennaNumber] == -1) {
	if (invalidSlave(shortvalue) == 1) {
          printw(" wac/wac ");
	} else {
	  if (antennaNumber < 9) {
	    printw(" wac/%1d   ",shortvalue);
	  } else {
	    rm_status = rm_read(antennaNumber, "RM_RX_MICROCONTROLLER_SUBTYPE_V15_S", boardSubType);
	    checkStatus(rm_status,"rm_read(RM_RX_MICROCONTROLLER_SUBTYPE_V15_S)");
	    rxnumber[antennaNumber] = boardSubType[shortvalue];
	    printw("  %2s/%1d   ",
		   getLoBoardTypeStringBrief(rxnumber[antennaNumber]),
		   shortvalue);
	  }
	}
      } else {
	if (invalidSlave(shortvalue) == 1) {
          printw("  %2s/wac ",stringvalue);
	} else {
          printw("  %2s/%1d   ",stringvalue,shortvalue);
	}
      }
    }
  }
  move(3,0);
  if (DEBUG) refresh();

  for (ant = 1; ant <= 8; ant++) {
    rm_status = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", loboardPresent[ant-1]);
    if (rxnumber[ant] < 0 || rxnumber[ant] > 7) {
    } else {
      if (loboardPresent[ant-1][rxnumber[ant]] < 0) {
	if (rxnumber[ant] == 2) {
	  /* this might be the case where we are using the 345 at 230 */
	  use345at230[ant] = 1;
	  rxnumber[ant] = 0;
	}
      }
    }
  }
  printw("PROM/RAM ");
  for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_LO_BOARD_EPROM_VERSION_V8_S", shortarray);
      checkStatus(rm_status,"rm_read(RM_LO_BOARD_EPROM_VERSION_V8_S)");
      rm_status = rm_read(antennaNumber, "RM_LO_BOARD_SRAM_VERSION_V8_S", shortarray2);
      checkStatus(rm_status,"rm_read(RM_LO_BOARD_SRAM_VERSION_V8_S)");
      if (shortarray[rxnumber[antennaNumber]] < 1 ||
	  shortarray[rxnumber[antennaNumber]] > 0xff) {
	printw(" wac/");
      } else {
	printw("0x%02x/",shortarray[rxnumber[antennaNumber]]);
      }
      if (shortarray2[rxnumber[antennaNumber]] < 1 ||
	  shortarray2[rxnumber[antennaNumber]] > 0xff) {
	printw("wac ");
      } else {
	printw("%02x  ",shortarray2[rxnumber[antennaNumber]]);
      }
    }
  }
  move(4,0);
  if (DEBUG) refresh();
  printw("GunnTable");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
       if (rxnumber[antennaNumber] >= 0 && rxnumber[antennaNumber] < NUMBER_OF_RECEIVERS) {
	 rm_status = rm_read(antennaNumber, "RM_LAST_GUNN_TUNER_INTERPOLATION_V8_L", tablearray);
	 checkStatus(rm_status,"rm_read(RM_LAST_GUNN_TUNER_INTERPOLATION_V8_L)");
	 rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L", long2array);
	 checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	 if (tablearray[rxnumber[antennaNumber]] < LO_MOTOR_MIN || 
	     tablearray[rxnumber[antennaNumber]] > LO_MOTOR_MAX) {
	   printw("  wacko  ");
	 } else {
	   printw(" ");
#define GUNN_TABLE_VALUE_MISMATCH 500
	   if (abs(long2array[rxnumber[antennaNumber]][GUNN_TUNER]-tablearray[rxnumber[antennaNumber]]) 
	       > GUNN_TABLE_VALUE_MISMATCH) {
	     standout();
	   }
	   printw("%6d",tablearray[rxnumber[antennaNumber]]);
	   standend();
	   printw("  ");
	 }
       } else {
	 printw(" wackoRx ");
       }
    }
  }

  move(5,0);
  if (DEBUG) refresh();
  printw("GunnTuner");
  
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (DEBUG) refresh();
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
	printw("no board ");
      }
      if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] >= 0) {
       rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L", long2array);
       checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
       rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
       if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	 printw("  stale  ");
       } else {
	 if (long2array[rxnumber[antennaNumber]][8] == 0) {
	   printw(" manual  ");
	 } else {
	   if (long2array[rxnumber[antennaNumber]][GUNN_TUNER]<LO_MOTOR_MIN || 
	       long2array[rxnumber[antennaNumber]][GUNN_TUNER]>LO_MOTOR_MAX) {
	     printw("  wacko  ");
	   } else {
	     if (lastGunnTuner[antennaNumber] != 
		 long2array[rxnumber[antennaNumber]][GUNN_TUNER] && 
		 lastGunnTuner[antennaNumber] != INITIAL_LAST_MOTOR_VALUE) {
	       standout();
	     }
	     printw(" %6d  ",long2array[rxnumber[antennaNumber]][GUNN_TUNER]);
	     standend();
	     lastGunnTuner[antennaNumber] = long2array[rxnumber[antennaNumber]][GUNN_TUNER];
	     /*
	       printw(" %d/%6d  ",long2array[rxnumber[antennaNumber]][8],long2array[rxnumber[antennaNumber]][GUNN_TUNER]);
	     */
	   }
	 }
       }
     }
    }
  }
  move(6,0);
  if (DEBUG) refresh();

  printw("Backshort");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (DEBUG) refresh();
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
      rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L", long2array);
      checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
      rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
      if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	printw("  stale  ");
      } else {
	if (long2array[rxnumber[antennaNumber]][8] == 0) {
	  printw(" manual  ");
	} else {
	  if (long2array[rxnumber[antennaNumber]][GUNN_BACKSHORT] < LO_MOTOR_MIN || 
	      long2array[rxnumber[antennaNumber]][GUNN_BACKSHORT] > LO_MOTOR_MAX) {
	    printw("  wacko  ");
	  } else {
	    if (lastGunnBackshort[antennaNumber] != 
		long2array[rxnumber[antennaNumber]][GUNN_BACKSHORT] &&
		lastGunnBackshort[antennaNumber] != INITIAL_LAST_MOTOR_VALUE) {
	      standout();
	    }
	    if (long2array[rxnumber[antennaNumber]][GUNN_BACKSHORT] < STUCK_MOTOR_VALUE) {
	      standout();
	    }
	    printw(" %6d  ",long2array[rxnumber[antennaNumber]][GUNN_BACKSHORT]);
	    standend();
	    lastGunnBackshort[antennaNumber] = long2array[rxnumber[antennaNumber]][GUNN_BACKSHORT];
	  }
	}
      }
     }
    }
  }
  move(7,0);
  if (DEBUG) refresh();

 printw("Attenuatr");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (DEBUG) refresh();
   if (!antsAvailable[antennaNumber]) {
     printw(" -----  ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
       rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L", long2array);
       checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
       rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
       if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
         printw("  stale ");
       } else {
         if (long2array[rxnumber[antennaNumber]][8] == 0) {
           printw(" manual  ");
         } else {
	   if (long2array[rxnumber[antennaNumber]][ATTENUATOR] < LO_MOTOR_MIN || 
  	       long2array[rxnumber[antennaNumber]][ATTENUATOR] > LO_MOTOR_MAX) {
	     printw(" wacko  ");
  	   } else {
	     if (lastAtten[antennaNumber] != 
		 long2array[rxnumber[antennaNumber]][ATTENUATOR] &&
		 lastAtten[antennaNumber] != INITIAL_LAST_MOTOR_VALUE) {
	       standout();
	     }
	     if (long2array[rxnumber[antennaNumber]][ATTENUATOR] < STUCK_MOTOR_VALUE) {
	       standout();
	     }
	     printw(" %6d  ",long2array[rxnumber[antennaNumber]][ATTENUATOR]);
	     lastAtten[antennaNumber] = long2array[rxnumber[antennaNumber]][ATTENUATOR];
	     standend();
	   }
	 }
       }
     }
   }
 }
 move(8,0);
  if (DEBUG) refresh();
 
 printw("MultInput");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
       rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
       checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
       rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
       if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
         printw("  stale  ");
       } else {
         if (long2array[rxnumber[antennaNumber]][8] == 0) {
	   printw(" manual  ");
         } else {
	   if (rxnumber[antennaNumber] == 4) {
	     printw("  auto   ");
	   } else {
	     if (motorizedMultiplier[antennaNumber][rxnumber[antennaNumber]] < 1) {
	       printw("  auto   ");
	     } else {
	       if (long2array[rxnumber[antennaNumber]][MULT_INPUT] < LO_MOTOR_MIN || 
		   long2array[rxnumber[antennaNumber]][MULT_INPUT] > LO_MOTOR_MAX) {
		 printw("  wacko  ");
	       } else {
		 if (lastMultInput[antennaNumber] !=
		     long2array[rxnumber[antennaNumber]][MULT_INPUT] &&
		     lastMultInput[antennaNumber] != INITIAL_LAST_MOTOR_VALUE) {
		   standout();
		 }
		 if (long2array[rxnumber[antennaNumber]][MULT_INPUT] < STUCK_MOTOR_VALUE) {
		   standout();
		 }
		 printw(" %6d  ",long2array[rxnumber[antennaNumber]][MULT_INPUT]);
		 lastMultInput[antennaNumber] =
		   long2array[rxnumber[antennaNumber]][MULT_INPUT];
		 standend();
	       }
	     }
	   }
  	 }
       }
     }
   }
 }
 move(9,0);
  if (DEBUG) refresh();
 
 printw("MultOutpt");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
       rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
       checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
       rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
       if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
         printw("  stale  ");
       } else {
         if (long2array[rxnumber[antennaNumber]][8] == 0) {
	   printw(" manual  ");
         } else {
	   if (rxnumber[antennaNumber] == 4) {
	     printw("  auto   ");
	   } else {
	     if (motorizedMultiplier[antennaNumber][rxnumber[antennaNumber]] < 2) {
	       printw("  auto   ");
	     } else {
	       if (long2array[rxnumber[antennaNumber]][MULT_OUTPUT] < LO_MOTOR_MIN || 
		   long2array[rxnumber[antennaNumber]][MULT_OUTPUT] > LO_MOTOR_MAX) {
		 printw("  wacko  ");
	       } else {
		 if (lastMultOutput[antennaNumber] !=
		     long2array[rxnumber[antennaNumber]][MULT_OUTPUT] &&
		     lastMultOutput[antennaNumber] != INITIAL_LAST_MOTOR_VALUE) {
		   standout();
		 }
		 if (long2array[rxnumber[antennaNumber]][MULT_OUTPUT] < STUCK_MOTOR_VALUE) {
		   standout();
		 }
		 printw(" %6d  ",long2array[rxnumber[antennaNumber]][MULT_OUTPUT]);
		 lastMultOutput[antennaNumber] = long2array[rxnumber[antennaNumber]][MULT_OUTPUT];
		 standend();
	       }
	     }
	   }
	 }
       }
     }
   }
 }
 move(10,0);
  if (DEBUG) refresh();

 printw("PLL Temp ");
 for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
       rm_status = rm_read(antennaNumber, "RM_GUNN_LO_PLATE_TEMPERATURE_F", &floatvalue);
       checkStatus(rm_status,"rm_read(RM_GUNN_LO_PLATE_TEMPERATURE_F)");
       if (floatvalue < -40 || floatvalue >= 300) {
         printw("  wacko  " );
       } else {
         if (floatvalue > 100) {
           printw("  %6.1f  ",floatvalue);
         } else {
           printw("  %5.2f  ",floatvalue);
	 }
       }
     }
   }
 }
#define PLL_SUPPLY_MIN 17.2
#define PLL_SUPPLY_WACKO_MAX 30
#define PLL_SUPPLY_WACKO_MIN -30
#if 0
 move(10,0);
  if (DEBUG) refresh();

 printw("18VSupply");
 for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
       rm_status = rm_read(antennaNumber, "RM_PLL_POWER_SUPPLY_F",&floatvalue);
       checkStatus(rm_status,"rm_read(RM_PLL_POWER_SUPPLY_F)");
       if (floatvalue<PLL_SUPPLY_WACKO_MIN || floatvalue>PLL_SUPPLY_WACKO_MAX) {
         printw("  wacko  " );
       } else {
         if (doWeCare[ant]==1 && floatvalue<PLL_SUPPLY_MIN) {
           standout();
         }
         printw("  %5.2f  ",floatvalue);
         if (doWeCare[ant]==1 && floatvalue<PLL_SUPPLY_MIN) {
           standend();
	 }
       }
     }
   }
 }
#endif
 move(11,0);
  if (DEBUG) refresh();

 printw("18V/10V  ");
 for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       if (rxnumber[antennaNumber] != 2) {
	 printw("no board ");
       } else {
	 /* this might be the case where we are using the 345 at 230 */
	 printw("345at230 ");
       }
     } else {
       rm_status = rm_read(antennaNumber, "RM_PLL_POWER_SUPPLY_F",&floatvalue);
       checkStatus(rm_status,"rm_read(RM_PLL_POWER_SUPPLY_F)");

       if (floatvalue<PLL_SUPPLY_WACKO_MIN || floatvalue>PLL_SUPPLY_WACKO_MAX) {
         printw(" wac" );
       } else {
         if (doWeCare[ant]==1 && floatvalue<PLL_SUPPLY_MIN) {
           standout();
         }
         printw(" %3d",(int)(floatvalue*10));
         if (doWeCare[ant]==1 && floatvalue<PLL_SUPPLY_MIN) {
           standend();
	 }
       }
       printw("/");
       rm_status= rm_read(antennaNumber,"RM_LOGIC_POWER_SUPPLY_F",&floatvalue);
       checkStatus(rm_status,"rm_read(RM_LOGIC_POWER_SUPPLY_F)");
#define LOGIC_SUPPLY_MIN 9.2
       if (floatvalue<PLL_SUPPLY_WACKO_MIN || floatvalue>PLL_SUPPLY_WACKO_MAX) {
         printw("wac " );
       } else {
         if (doWeCare[ant]==1 && floatvalue<LOGIC_SUPPLY_MIN) {
           standout();
         }
         printw("%3d ",(int)(floatvalue*10));
         if (doWeCare[ant]==1 && floatvalue<LOGIC_SUPPLY_MIN) {
           standend();
	 }
       }
     }
   }
 }

 move(12,0);
  if (DEBUG) refresh();
 printw("Thrs/recV");
 for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw(" -- ---- ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumber[antennaNumber]] < 0) {
       printw(" no ");
     } else {
       rm_status = rm_read(antennaNumber, "RM_GUNN1_PLL_FILTER_RATIO_THRESHOLD_S",&shortvalue);
       checkStatus(rm_status,"rm_read(RM_GUNN1_PLL_FILTER_RATIO_THRESHOLD_S)");
       if (shortvalue<1 || shortvalue>999) {
         printw(" -- " );
       } else {
	 printw(" ");
	 if (shortvalue < FILTER_RATIO_STANDOUT) {
	   standout();
	 }
         printw("%2d",shortvalue);
	 standend();
	 printw(",");
       }
     }
     rm_status = rm_read(antennaNumber, "RM_GUNN1_PLL_RECENTER_THRESHOLD_F",&floatvalue);
     checkStatus(rm_status,"rm_read(RM_GUNN1_PLL_RECENTER_THRESHOLD_F)");
     if (floatvalue<0.0 || floatvalue>=1.0) {
       printw("wack" );
     } else {
       if (floatvalue > RECENTER_THRESHOLD_STANDOUT) {
	 standout();
       }
       printw("%.2f",floatvalue);
       standend();
       printw(" ");
     }
   }
 }

 move(13,0);
  if (DEBUG) refresh();

 /* second receiver */

 printw("Rx/slave ");
 showSecondReceiver = 1;

  for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_SLAVE2_PLL_NO_S", &shortvalue);
      actPLLhigh[antennaNumber] = shortvalue;
      checkStatus(rm_status,"rm_read(RM_SLAVE2_PLL_NO_S)");
      rm_status = rm_read(antennaNumber, "RM_ACTIVE_HIGH_RECEIVER_C10", stringvalue);
      checkStatus(rm_status,"rm_read(RM_ACTIVE_HIGH_RECEIVER_C10)");
      rxnumberHigh[antennaNumber] = parseReceiverName(stringvalue);
      if (rxnumberHigh[antennaNumber] == -1) {
	if (invalidSlave(shortvalue) == 1) {
          printw(" wac/wac  ");
	} else {
          printw(" wac/%1d   ",shortvalue);
	}
      } else {
	if (invalidSlave(shortvalue) == 1) {
          printw("  %2s/wac ",stringvalue);
	} else {
          printw("  %2s/%1d   ",stringvalue,shortvalue);
	}
      }
    }
  }

 move(14,0);
  if (DEBUG) refresh();
 
 printw("PROM/RAM ");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       rm_status = rm_read(antennaNumber, "RM_LO_BOARD_EPROM_VERSION_V8_S", shortarray);
       checkStatus(rm_status,"rm_read(RM_LO_BOARD_EPROM_VERSION_V8_S)");
       rm_status = rm_read(antennaNumber, "RM_LO_BOARD_SRAM_VERSION_V8_S", shortarray2);
       checkStatus(rm_status,"rm_read(RM_LO_BOARD_SRAM_VERSION_V8_S)");
       rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
       if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	 printw("  stale  ");
       } else {
         if (shortarray[rxnumberHigh[antennaNumber]] < 1 ||
	     shortarray[rxnumberHigh[antennaNumber]] > 0xff) {
           printw(" wac/");
         } else {
	   printw("0x%02x/",shortarray[rxnumberHigh[antennaNumber]]);
         }
         if (shortarray2[rxnumberHigh[antennaNumber]] < 1 ||
	     shortarray2[rxnumberHigh[antennaNumber]] > 0xff) {
           printw("wac ");
         } else {
	   printw("%02x  ",shortarray2[rxnumberHigh[antennaNumber]]);
         }
       }
     }
   }
 }
 move(15,0);
 if (DEBUG) refresh();
 printw("GunnTable");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
       if (rxnumberHigh[antennaNumber] >= 0 && rxnumberHigh[antennaNumber] < NUMBER_OF_RECEIVERS) {
	 rm_status = rm_read(antennaNumber, "RM_LAST_GUNN_TUNER_INTERPOLATION_V8_L", tablearray);
	 checkStatus(rm_status,"rm_read(RM_LAST_GUNN_TUNER_INTERPOLATION_V8_L)");
	 rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
	 checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	 if (tablearray[rxnumberHigh[antennaNumber]] < LO_MOTOR_MIN || 
	     tablearray[rxnumberHigh[antennaNumber]] > LO_MOTOR_MAX) {
	   printw("  wacko  ");
	 } else {
	   printw(" ");
	   if (abs(long2array[rxnumberHigh[antennaNumber]][GUNN_TUNER]-tablearray[rxnumberHigh[antennaNumber]]) 
	       > GUNN_TABLE_VALUE_MISMATCH) {
	     standout();
	   }
	   printw("%6d",tablearray[rxnumberHigh[antennaNumber]]);
	   standend();
	   printw("  ");
	 }
       } else {
	 printw(" wackoRx ");
       }
    }
  }

 move(16,0);
 if (DEBUG) refresh();
 printw("GunnTuner");

 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (DEBUG) refresh();
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
      if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
        printw("no board ");
      } else {
       rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
       checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
       rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
       if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	 printw("  stale  ");
       } else {
	 if (long2array[rxnumberHigh[antennaNumber]][8] == 0) {
	   printw(" manual  ");
	 } else {
	   if (long2array[rxnumberHigh[antennaNumber]][GUNN_TUNER] < LO_MOTOR_MIN || 
	       long2array[rxnumberHigh[antennaNumber]][GUNN_TUNER] > LO_MOTOR_MAX) {
	     printw("  wacko  ");
	   } else {
	     if (lastGunnTunerHigh[antennaNumber] != 
		 long2array[rxnumberHigh[antennaNumber]][GUNN_TUNER] &&
		 lastGunnTunerHigh[antennaNumber] !=INITIAL_LAST_MOTOR_VALUE) {
	       standout();
	     }
	     printw(" %6d  ",long2array[rxnumberHigh[antennaNumber]][GUNN_TUNER]);
	     standend();
             lastGunnTunerHigh[antennaNumber] = 
	       long2array[rxnumberHigh[antennaNumber]][GUNN_TUNER];
	   }
	 }
       }
      }
     }
   }
 }
 move(17,0);
  if (DEBUG) refresh();
 
 printw("Backshort");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (DEBUG) refresh();
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_S", &shortvalue);
	 checkStatus(rm_status,"rm_read(RM_CALIBRATION_WHEEL_S)");
	 rm_status = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
	 if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	   printw("  stale  ");
	 } else {
	   rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
	   checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	   rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
	   if (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL) {
	     printw("  stale  ");
	   } else {
	     if (long2array[rxnumberHigh[antennaNumber]][8] == 0) {
	       printw(" manual  ");
	     } else {
	       if (long2array[rxnumberHigh[antennaNumber]][GUNN_BACKSHORT] < LO_MOTOR_MIN || 
		   long2array[rxnumberHigh[antennaNumber]][GUNN_BACKSHORT] > LO_MOTOR_MAX) {
		 printw("  wacko  ");
	       } else {
		 if (lastGunnBackshortHigh[antennaNumber] != 
		     long2array[rxnumberHigh[antennaNumber]][GUNN_BACKSHORT] &&
		     lastGunnBackshortHigh[antennaNumber] !=
		     INITIAL_LAST_MOTOR_VALUE) {
		   standout();
		 }
		 if (long2array[rxnumberHigh[antennaNumber]][GUNN_BACKSHORT] < STUCK_MOTOR_VALUE) {
		   standout();
		 }
		 printw(" %6d  ",long2array[rxnumberHigh[antennaNumber]][GUNN_BACKSHORT]);
		 standend();
		 lastGunnBackshortHigh[antennaNumber] = long2array[rxnumberHigh[antennaNumber]][GUNN_BACKSHORT];
	       }
	     }
	   }
	 }
       }
     }
   }
 }
 move(18,0);
  if (DEBUG) refresh();

 printw("Attenuat ");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
	 checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	 rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
	 if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	   printw("  stale  ");
	 } else {
	   if (long2array[rxnumberHigh[antennaNumber]][8] == 0) {
	     printw(" manual  ");
	   } else {
	     if (long2array[rxnumberHigh[antennaNumber]][ATTENUATOR] < LO_MOTOR_MIN || 
		 long2array[rxnumberHigh[antennaNumber]][ATTENUATOR] > LO_MOTOR_MAX) {
	       printw("  wacko  ");
	     } else {
	       if (lastAttenHigh[antennaNumber] != 
		   long2array[rxnumberHigh[antennaNumber]][ATTENUATOR] &&
		   lastAttenHigh[antennaNumber] !=INITIAL_LAST_MOTOR_VALUE) {
		 standout();
	       }
	       if (long2array[rxnumberHigh[antennaNumber]][ATTENUATOR] < STUCK_MOTOR_VALUE) {
		 standout();
	       }
	       printw(" %6d  ",long2array[rxnumberHigh[antennaNumber]][ATTENUATOR]);
	       standend();
	       lastAttenHigh[antennaNumber] = 
		 long2array[rxnumberHigh[antennaNumber]][ATTENUATOR];
	     }
	   }
	 }
       }
     }
   }
 }
 move(19,0);
  if (DEBUG) refresh();

 printw("MultInput");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
	 checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	 rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
	 if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	   printw("  stale  ");
	 } else {
	   if (long2array[rxnumberHigh[antennaNumber]][8] == 0) {
	     printw(" manual  ");
	   } else {
	     if (rxnumberHigh[antennaNumber] == 4) {
	       printw("  auto   ");
	     } else if (antennaNumber==7) {
	       printw("  manual ");
	     } else
	       if (long2array[rxnumberHigh[antennaNumber]][MULT_INPUT] < LO_MOTOR_MIN || 
		   long2array[rxnumberHigh[antennaNumber]][MULT_INPUT] > LO_MOTOR_MAX) {
		 printw("  wacko  ");
	       } else {
		 if (lastMultInputHigh[antennaNumber] != 
		     long2array[rxnumberHigh[antennaNumber]][MULT_INPUT] &&
		     lastMultInputHigh[antennaNumber] !=INITIAL_LAST_MOTOR_VALUE) {
		   standout();
		 }
		 if (long2array[rxnumberHigh[antennaNumber]][MULT_INPUT] < STUCK_MOTOR_VALUE) {
		   standout();
		 }
		 printw(" %6d  ",long2array[rxnumberHigh[antennaNumber]][MULT_INPUT]);
		 standend();
		 lastMultInputHigh[antennaNumber] = 
		   long2array[rxnumberHigh[antennaNumber]][MULT_INPUT];
	       }
	   }
	 }
       }
     }
   }
 }
 move(20,0);
 if (DEBUG) refresh();
 
 printw("MultOutpt");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
	 checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	 rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
	 if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	   printw("  stale  ");
	 } else {
	   if (long2array[rxnumberHigh[antennaNumber]][8] == 0) {
	     printw(" manual  ");
	   } else {
	     if (rxnumberHigh[antennaNumber] == 4) {
	       printw("  auto   ");
	     } else if (antennaNumber==7) {
	       printw("  manual ");
	     } else      
	       if (long2array[rxnumberHigh[antennaNumber]][MULT_OUTPUT] < LO_MOTOR_MIN || 
		   long2array[rxnumberHigh[antennaNumber]][MULT_OUTPUT] > LO_MOTOR_MAX) {
		 printw("  wacko  ");
	       } else {
		 if (lastDiplexerHigh[antennaNumber] != 
		     long2array[rxnumberHigh[antennaNumber]][MULT_OUTPUT] &&
		     lastDiplexerHigh[antennaNumber]!=INITIAL_LAST_MOTOR_VALUE) {
		   standout();
		 }
		 if (long2array[rxnumberHigh[antennaNumber]][MULT_OUTPUT] < STUCK_MOTOR_VALUE) {
		   standout();
		 }
		 printw(" %6d  ",long2array[rxnumberHigh[antennaNumber]][MULT_OUTPUT]);
		 standend();
		 lastDiplexerHigh[antennaNumber] = 
		   long2array[rxnumberHigh[antennaNumber]][MULT_OUTPUT];
	       }
	   }
	 }
       }
     }
   }
 }
 move(21,0);
  if (DEBUG) refresh();

#if 1
 printw("Diplexer ");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber, "RM_MOTORIZED_LO_CHAIN_V8_V9_L",long2array);
	 checkStatus(rm_status,"rm_read(RM_MOTORIZED_LO_CHAIN_V8_V9_L)");
	 rm_status = rm_read(antennaNumber, "RM_PHASE_LOCK_TIMESTAMP_L",&timevalue);
	 if (0 && (timevalue < rightnow-TUNE6_MOTOR_STALE_INTERVAL)) {
	   printw("  stale  ");
	 } else {
	   if (long2array[rxnumberHigh[antennaNumber]][8] == 0) {
	     printw(" manual  ");
	   } else {
	     if (rxnumberHigh[antennaNumber] == 4) {
	       printw("  none   ");
	     } else if (antennaNumber==7) {
	       printw("  manual ");
	     } else if (long2array[rxnumberHigh[antennaNumber]][DIPLEXER] < LO_MOTOR_MIN || 
			long2array[rxnumberHigh[antennaNumber]][DIPLEXER] > LO_MOTOR_MAX) {
	       printw("  wacko  ");
	     } else {
	       if (lastMultOutputHigh[antennaNumber] != 
		   long2array[rxnumberHigh[antennaNumber]][DIPLEXER] &&
		   lastMultOutputHigh[antennaNumber]!=INITIAL_LAST_MOTOR_VALUE) {
		 standout();
	       }
	       if (long2array[rxnumberHigh[antennaNumber]][DIPLEXER] < STUCK_MOTOR_VALUE) {
		 standout();
	       }
	       printw(" %6d  ",long2array[rxnumberHigh[antennaNumber]][DIPLEXER]);
	       standend();
	       lastMultOutputHigh[antennaNumber] = 
		 long2array[rxnumberHigh[antennaNumber]][DIPLEXER];
	     }
	   }
	 }
       }
     }
   }
 }
#else
 printw("PLL Temp ");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber, "RM_GUNN2_LO_PLATE_TEMPERATURE_F", 
			     &floatvalue);
	 checkStatus(rm_status,"rm_read(RM_GUNN2_LO_PLATE_TEMPERATURE_F)");
	 if (floatvalue < -40 || floatvalue >= 300) {
	   printw("  wacko  " );
	 } else {
	   if (floatvalue > 100) {
	     printw("  %6.1f  ",floatvalue);
	   } else {
	     printw("  %5.2f  ",floatvalue);
	   }
	 }
       }
     }
   }
 }
#endif

#if 0
 move(21,0);
  if (DEBUG) refresh();
 
 printw("18VSupply");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber,"RM_PLL2_POWER_SUPPLY_F",&floatvalue);
	 checkStatus(rm_status,"rm_read(RM_PLL2_POWER_SUPPLY_F)");
	 if (floatvalue<PLL_SUPPLY_WACKO_MIN || floatvalue>PLL_SUPPLY_WACKO_MAX){
	   printw("  wacko  " );
	 } else {
	   printw("  %.2f  ",floatvalue);
	 }
       }
     }
   }
 }
#endif
 move(22,0);
  if (DEBUG) refresh();

 printw("18V/10V  ");
 if (showSecondReceiver) {
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
	 printw("no board ");
       } else {
	 rm_status = rm_read(antennaNumber,"RM_PLL2_POWER_SUPPLY_F",&floatvalue);
	 checkStatus(rm_status,"rm_read(RM_PLL2_POWER_SUPPLY_F)");

	 if (floatvalue<PLL_SUPPLY_WACKO_MIN || floatvalue>PLL_SUPPLY_WACKO_MAX) {
	   printw(" wac" );
	 } else {
	   printw(" ");
	   if (doWeCare[ant]==1 && floatvalue<PLL_SUPPLY_MIN) {
	     standout();
	   }
	   printw("%3d",(int)(floatvalue*10));
	   standend();
	 }
	 printw("/");

	 rm_status = rm_read(antennaNumber,"RM_LOGIC2_POWER_SUPPLY_F",&floatvalue);
	 checkStatus(rm_status,"rm_read(RM_LOGIC2_POWER_SUPPLY_F)");
	 if (floatvalue<PLL_SUPPLY_WACKO_MIN || floatvalue>PLL_SUPPLY_WACKO_MAX) {
	   printw("wac " );
	 } else {
	   if (doWeCare[ant]==1 && floatvalue<LOGIC_SUPPLY_MIN) {
	     standout();
	   }
	   printw("%3d",(int)(floatvalue*10));
	   standend();
	   printw(" ");
	 }
       }
     }
   }
 }
 move(23,0);
  if (DEBUG) refresh();
 printw("Thrs/recV");
 /* printw("         ");*/
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
#if NO_ASIAA_650
     if (antennaNumber==7 || antennaNumber==8) {
       printw("  no Rx  ");
       continue;
     }
#endif
   if (!antsAvailable[antennaNumber]) {
     printw(" -- ---- ");
   } else {
     if (loboardPresent[antennaNumber-1][rxnumberHigh[antennaNumber]] < 0) {
       printw(" no ");
     } else {
       rm_status = rm_read(antennaNumber, "RM_GUNN2_PLL_FILTER_RATIO_THRESHOLD_S",&shortvalue);
       checkStatus(rm_status,"rm_read(RM_GUNN2_PLL_FILTER_RATIO_THRESHOLD_S)");
       if (shortvalue<1 || shortvalue>999) {
         printw(" -- " );
       } else {
	 printw(" ");
	 if (shortvalue < FILTER_RATIO_STANDOUT) {
	   standout();
	 }
	 printw("%2d",shortvalue);
	 standend();
	 printw(",");
       }
     }
     rm_status = rm_read(antennaNumber, "RM_GUNN2_PLL_RECENTER_THRESHOLD_F",&floatvalue);
     checkStatus(rm_status,"rm_read(RM_GUNN2_PLL_RECENTER_THRESHOLD_F)");
     if (floatvalue<0.0 || floatvalue>=1.0) {
       printw("wack" );
     } else {
       if (floatvalue > RECENTER_THRESHOLD_STANDOUT) {
	 standout();
       }
       printw("%.2f",floatvalue);
       standend();
       printw(" ");
     }
   }
 }

  refresh();
}

int invalidSlave(int sl) {
  if (sl < 0 || sl > 15) {
    return(1);
  } else {
    return(0);
  }
}
