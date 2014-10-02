#include <curses.h>
#include <stdio.h>
#include <termio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <rm.h>
#include "s_cmd2.h"
#include "monitor.h"
#include "commonLib.h"

#define SIS_CRIT_TEMP 5.7
#define LO_MOTOR_MIN -999999
#define LO_MOTOR_MAX  999999
#define TUNE6_STALE_INTERVAL 10
#define VARIAN_STALE_INTERVAL 20
#define BALZERS_STALE_INTERVAL 10

char *toddtime(time_t *, char *);
char *hsttime(time_t *t, char *str);
extern void checkStatus(int status, char *string);

void dewarpage(int count, int *rm_list) {
  int rm_status;
  int badStatus,i;
  int antennaNumber;
#define NUMBER_OF_RECEIVERS 8
  float floatvalue;
  float rebuildHours, hoursSinceRebuild;
  float runningHours;
  float floatarray2[RM_ARRAY_SIZE][16];
  long rightnow, longvalue;
  long timearray[RM_ARRAY_SIZE];
  int index[RM_ARRAY_SIZE];
  int channel = 5;
  char turboStatusString[20];
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int doWeCare[11];
  short emailAddresses;
  double glycolTemp = 0.0;
  double glycolFlow = 0.0;
  double glycolPressure = 0.0;

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
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  /*
  antsAvailable[8] = 0;
  */

  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("  Cryostat Status on %s UT = %s HST",timeString,timeString2);
  move(1,0);
  printw("Antenna      1        2        3        4        5        6        7        8");
  move(2,0);

  printw("77K Temp ");
  
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber]) {
      rm_status = rm_read(antennaNumber, "RM_DEWAR_TEMPS_V16_F", &floatarray2[antennaNumber]);
      checkStatus(rm_status,"rm_read(RM_DEWAR_TEMPS_V16_F)");
      rm_status = rm_read(antennaNumber,"RM_LAKESHORE_TIMESTAMP_L",&timearray[antennaNumber]);
    }
  }
#define LAKESHORE_STALE_INTERVAL 60
#define LAKESHORE_WACKO_MIN 2.0
#define LAKESHORE_WACKO_MAX 350.0
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    index[antennaNumber] = 0;
  }
  index[7] = 3;
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatarray2[antennaNumber][index[antennaNumber]] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][index[antennaNumber]] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
            printw(" %6.2f  ",floatarray2[antennaNumber][index[antennaNumber]]);
	}
      }
    }
  }
  move(3,0);

  printw("12K Temp ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    index[antennaNumber] = 1;
  }
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatarray2[antennaNumber][index[antennaNumber]] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][index[antennaNumber]] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
            printw(" %6.2f  ",floatarray2[antennaNumber][index[antennaNumber]]);
	}
      }
    }
  }
  move(4,0);

 printw("4K Temp  ");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	channel = DEWAR_TEMP_4K_STAGE;
	if (floatarray2[antennaNumber][channel] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][channel] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
	  if (floatarray2[antennaNumber][channel] > SIS_CRIT_TEMP) {
            standout();
	  }
          printw(" %6.2f  ",floatarray2[antennaNumber][channel]);
	  if (floatarray2[antennaNumber][channel] > SIS_CRIT_TEMP) {
            standend();
	  }
	}
      }
   }
 }
 move(5,0);
 printw("CH Hours ");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	channel = 2;
        rm_status = rm_read(antennaNumber, "RM_COLD_HEAD_REBUILD_HOURS_F", 
           &rebuildHours);
        rm_status = rm_read(antennaNumber, "RM_COLD_HEAD_RUNNING_HOURS_F", 
           &runningHours);
        hoursSinceRebuild = runningHours-rebuildHours;
	if (hoursSinceRebuild < 0 || hoursSinceRebuild > 9999999) {
	    printw("  wacko  ");
        } else {
#define CRYO_REBUILD_INTERVAL 20000
	  if (hoursSinceRebuild > CRYO_REBUILD_INTERVAL) {
	    standout();
	  }
          printw("%7.0f  ",hoursSinceRebuild);
	  if (hoursSinceRebuild > CRYO_REBUILD_INTERVAL) {
	    standend();
	  }
	}
      }
   }
 }



 move(6,0);
 printw("RxA1Temp ");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   index[antennaNumber] = 8;
 }
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatarray2[antennaNumber][index[antennaNumber]] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][index[antennaNumber]] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
	  if (floatarray2[antennaNumber][index[antennaNumber]] > SIS_CRIT_TEMP) {
	    standout();
	  }
          printw(" %6.2f  ",floatarray2[antennaNumber][index[antennaNumber]]);
	  if (floatarray2[antennaNumber][index[antennaNumber]] > SIS_CRIT_TEMP) {
	    standend();
	  }
	}
      }
   }
 }
 move(7,0);
 refresh();
 
//OLD RXA2TEMP LINE*****************************

 move(8,0);

 printw("RxB1Temp ");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   index[antennaNumber] = 10;
 }
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatarray2[antennaNumber][index[antennaNumber]] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][index[antennaNumber]] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
	  if (floatarray2[antennaNumber][index[antennaNumber]] > SIS_CRIT_TEMP) {
	    standout();
	  }
          printw(" %6.2f  ",floatarray2[antennaNumber][index[antennaNumber]]);
	  if (floatarray2[antennaNumber][index[antennaNumber]] > SIS_CRIT_TEMP) {
	    standend();
	  }
	}
      }
   }
 }
 move(9,0);

 //OLD RXB2 LINE ******************************

 move(10,0);

 printw("RxC Temp ");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
//     if (antennaNumber == 4 || antennaNumber == 5) {
       if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	 printw("  stale  ");
       } else {
	 channel = 14;
	 if (floatarray2[antennaNumber][channel] > LAKESHORE_WACKO_MAX ||
	     floatarray2[antennaNumber][channel] < LAKESHORE_WACKO_MIN) {
	   printw("  wacko  ");
	 } else {
	   printw(" %6.2f  ",floatarray2[antennaNumber][channel]);
	 }
       }
 //    } else {
 //      printw("  -----  "); /* receiver does not yet exist */
 //    }
   }
 }
 move(11,0);

 printw("RxX Temp ");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       channel = 13;
	  printw("  -----  "); /* receiver does not yet exist */
#if 0
       if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
       } else {
	if (floatarray2[antennaNumber][channel] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][channel] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
            printw(" %6.2f  ",floatarray2[antennaNumber][channel]);
	}
       }
#endif
     }
   }
 move(12,0);
 
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   index[antennaNumber] = 12; //I changed the line that BBC added CNS 22Aug11
//   index[antennaNumber] = 14;	//I added this line. BBC 10jun10
 }
// index[4] = 14;
// index[5] = 14;
 printw("RxE Temp ");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatarray2[antennaNumber][index[antennaNumber]] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][index[antennaNumber]] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
	  if (floatarray2[antennaNumber][index[antennaNumber]]>SIS_CRIT_TEMP) {
	    standout();
	  }
	  printw(" %6.2f  ",floatarray2[antennaNumber][index[antennaNumber]]);

	  if (floatarray2[antennaNumber][index[antennaNumber]]>SIS_CRIT_TEMP) {
	    standend();
	  }
	}
      }
     }
   }
 move(13,0);
 printw("RxF Temp ");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       rm_status = rm_read(antennaNumber, "RM_CRYO_MONITOR_EMAIL_STATUS_S", &emailAddresses);
       checkStatus(rm_status,"rm_read(RM_CRYO_MONITOR_EMAIL_STATUS_S)");
       if (emailAddresses < 1) {
	 standout();
       }
       if (abs(emailAddresses) > 99) {
	 printw(" wacko  ");
       } else if (emailAddresses < 10) {
	 printw(" email=%d ",emailAddresses);
       } else {
	 printw(" email=%d",emailAddresses);
       }
       standend();
       /* receiver does not yet exist */
       /*       printw("  -----  "); */
#if 0
      if (timearray[antennaNumber] < rightnow-LAKESHORE_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
       channel = 10;
	if (floatarray2[antennaNumber][channel] > LAKESHORE_WACKO_MAX ||
            floatarray2[antennaNumber][channel] < LAKESHORE_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
            printw(" %6.2f  ",floatarray2[antennaNumber][channel]);
	}
      }
#endif
     }
   }

#define BALZERS_WACKO_MAX 1000
#define BALZERS_WACKO_MIN 0

   move(14,0);
   printw("Pressure ");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rm_status = rm_read(antennaNumber, "RM_DEWAR_PRESSURE_F", &floatvalue);
      checkStatus(rm_status,"rm_read(RM_DEWAR_PRESSURE_F)");
      rm_status = rm_read(antennaNumber,"RM_BALZERS_TIMESTAMP_L",&timearray[antennaNumber]);
      checkStatus(rm_status,"rm_read(RM_BALZERS_TIMESTAMP_L)");
      if (timearray[antennaNumber] < rightnow-BALZERS_STALE_INTERVAL) {
        printw("  stale  ");
      } else {
	if (floatvalue > BALZERS_WACKO_MAX ||
            floatvalue < BALZERS_WACKO_MIN) {
	  if (floatvalue == -1) {
	    printw(" UndRang ");
	  } else {
	    printw("  wacko  ");
	  }
        } else {
	  if (floatvalue > 0.01) {
	    standout();
	  }
          printw(" %7.2g ",floatvalue);
	  if (floatvalue > 0.01) {
	    standend();
	  }
	}
      }
    }
  }

#if 0
  move(18,0);
  printw("ColdCath ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
    }
  }
#endif 
  move(15,0);
   printw("TurbokRPM");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
      rm_status = rm_read(antennaNumber, "RM_TURBOPUMP_RPM_F", &floatvalue);
      checkStatus(rm_status,"rm_read(RM_TURBOPUMP_RPM_F)");
      rm_status = rm_read(antennaNumber,"RM_TURBOPUMP_TIMESTAMP_L",&timearray[antennaNumber]);
      checkStatus(rm_status,"rm_read(RM_TURBOPUMP_TIMESTAMP_L)");
#if 1
#define TURBO_RPM_WACKO_MAX 100
#define TURBO_RPM_WACKO_MIN 0
      if (timearray[antennaNumber] < rightnow-VARIAN_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatvalue >= TURBO_RPM_WACKO_MAX ||
            floatvalue < TURBO_RPM_WACKO_MIN) {
	    printw("  wacko  ");
        } else {
            printw(" %6.1f  ",floatvalue);
	}
      }
#else
       printw("  -----  ");
#endif
     }
   }
  move(16,0);
   printw("ValvOpen@");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
      rm_status = rm_read(antennaNumber, "RM_TURBOPUMP_THRESHOLD_RPM_F", &floatvalue);
      checkStatus(rm_status,"rm_read(RM_TURBOPUMP_THRESHOLD_RPM_F)");
      rm_status = rm_read(antennaNumber,"RM_TURBOPUMP_TIMESTAMP_L",&timearray[antennaNumber]);
      checkStatus(rm_status,"rm_read(RM_TURBOPUMP_TIMESTAMP_L)");
      if (timearray[antennaNumber] < rightnow-VARIAN_STALE_INTERVAL) {
	printw("  stale  ");
      } else {
	if (floatvalue >= TURBO_RPM_WACKO_MAX ||
            floatvalue < TURBO_RPM_WACKO_MIN) {
	  printw("  wacko  ");
        } else {
          printw(" %2.0fkRPM  ",floatvalue);
	}
      }
     }
   }

 move(17,0);
   printw("TurboStat");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       rm_status = rm_read(antennaNumber, "RM_TURBOPUMP_STATUS_C20", 
           turboStatusString);
       checkStatus(rm_status,"rm_read(RM_TURBOPUMP_STATUS_C20)");
       turboStatusString[8] = (char)0;
       if (timearray[antennaNumber] < rightnow-VARIAN_STALE_INTERVAL) {
         printw("  stale  ");
       } else {
	 if (strlen(turboStatusString) > 8) {
	   printw(" wacko(1) ");
	 } else {
	   badStatus = 0;
	   for (i=0; i<7; i++) {
	     if (turboStatusString[i] < ' ' || turboStatusString[i] > '~') {
	       printw(" wacko(2) ");
	       badStatus = 1;
	       break;
	     }
	   }
	   if (badStatus == 0) {
	     printw(" %8s", turboStatusString);
	   }
	 }
       }
     }
   }
   move(18,0);
   printw("TurboVolt");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {

   
       rm_status = rm_read(antennaNumber, "RM_TURBOPUMP_VOLTAGE_F", &floatvalue);
       checkStatus(rm_status,"rm_read(RM_TURBOPUMP_VOLTAGE_F)");
       rm_status = rm_read(antennaNumber,"RM_TURBOPUMP_TIMESTAMP_L",&timearray[antennaNumber]);
       checkStatus(rm_status,"rm_read(RM_TURBOPUMP_TIMESTAMP_L)");
 #define TURBO_VOLTAGE_WACKO_MAX 300
 #define TURBO_VOLTAGE_WACKO_MIN 0
       if (timearray[antennaNumber] < rightnow-VARIAN_STALE_INTERVAL) {
 	printw("  stale  ");
       } 
       else
       {
 	if (floatvalue > TURBO_VOLTAGE_WACKO_MAX ||
             floatvalue < TURBO_VOLTAGE_WACKO_MIN) 
	 {
 	    printw("  wacko  ");
         } 
	else
	 {
             printw(" %6.2f  ",floatvalue);
 	}
       }}
    }

    move(19,0);
    printw("TurboTemp");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
      rm_status = rm_read(antennaNumber, "RM_TURBOPUMP_TEMPERATURE_F", &floatvalue);
       checkStatus(rm_status,"rm_read(RM_TURBOPUMP_TEMPERATURE_F)");
       rm_status = rm_read(antennaNumber,"RM_TURBOPUMP_TIMESTAMP_L",&timearray[antennaNumber]);
       checkStatus(rm_status,"rm_read(RM_TURBOPUMP_TIMESTAMP_L)");
 #define TURBO_TEMPERATURE_WACKO_MAX 120
 #define TURBO_TEMPERATURE_WACKO_MIN -40
       if (timearray[antennaNumber] < rightnow-VARIAN_STALE_INTERVAL) {
 	printw("  stale  ");
       } else {
 	if (floatvalue > TURBO_TEMPERATURE_WACKO_MAX ||
             floatvalue < TURBO_TEMPERATURE_WACKO_MIN) {
 	    printw("  wacko  ");
         } else {
             printw(" %4.0f C  ",floatvalue);
 	}
	}
     }}	

  /* insert the coolant flow fault here */
 move(20,0);
 printw("Coolant  ");
 for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
   if (!antsAvailable[antennaNumber]) {
     printw("  -----  ");
   } else {
      rm_status = rm_read(antennaNumber, "RM_SCB_FAULTWORD_L", &longvalue);
      checkStatus(rm_status,"rm_read(RM_SCB_FAULTWORD_L)");
      if (longvalue & (1<<COOLANT_FLOW_FAULT)) {
	if (doWeCare[antennaNumber]) {
          standout();
	} 
        printw("  fault  "); 
	if (doWeCare[antennaNumber]) {
          standend();
	} 
     } else {
       printw("  okay   ");
     }
   }
 }


   move(21,0);
   printw("GlycolTemp");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {

	rm_read(antennaNumber,"RM_GLYCOL_TEMP_D",&glycolTemp);
            printw("   %2.1f  ",glycolTemp);
	
      }
     }
   
  move(22,0);
   printw("GlycolFlow");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
	rm_read(antennaNumber,"RM_GLYCOL_FLOW_D",&glycolFlow);
            printw(" %6.2f  ",glycolFlow);
     }
   }
  move(23,0);
   printw("GlycolPres");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
	rm_read(antennaNumber,"RM_GLYCOL_PRESSURE_D",&glycolPressure);
            printw(" %6.2f  ",glycolPressure);
	}
      }
   

#if 0
 printw("JT-Valve ");
   for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
     if (!antsAvailable[antennaNumber]) {
       printw("  -----  ");
     } else {
       printw("  -----  ");
     }
   }
 move(21,0);
#endif

  refresh();
}

char *hsttime(time_t *t, char *str) {
  /* according to man ctime, the string length is always 26, with \n\0
   * as the final characters.  we want to chop out the \n
   */
  time_t modified_time;
  modified_time = *t-36000;
  return(toddtime(&modified_time,str));
}

char *toddtime(time_t *t, char *str) {
  /* according to man ctime, the string length is always 26, with \n\0
   * as the final characters.  we want to chop out the \n
   */
  strcpy(str,asctime(gmtime(t)));
  str[24] = str[25]; /* move the '0' back one char */
  if (str[8] == ' ') {
    str[8] = '0';
  }
  return(str);
}
