#define HAL "colossus"
#define OBSCON "obscon"
#define DEBUG 1
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "rm.h"
#include <math.h>
#include <curses.h>
#include <dsm.h>
#include "commonLib.h"
#include "upsd.h"
#include "upspage.h"
#include "monitor.h"

extern void printBatteryActiveAlarm(long longvalue);
extern dsm_structure upsStructureObscon[NUMBER_OF_UPSs_ON_OBSCON];
extern dsm_structure upsStructureObsconX;

extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *t, char *str);
void pickUPS(int upsNumber, char *var);
extern int printAge(long rightnow, long longvalue);
extern int printAgeNoStandout(long rightnow, long longvalue);
extern int printAgeStandout2(long rightnow, long longvalue);
extern int printInterval(long age, int standout);
extern void printUPSMode(short shortvalue);
extern void printUPSMode2(short shortvalue);
extern void printUPSBeeper(short shortvalue);


void upspage2(int count, int *rm_list) {
  int rm_status, dsm_status;
  int upsNumber;
  int lastBrownOut[9];
  time_t system_time;
  char accX[DSM_NAME_LENGTH];
  char acc2X[DSM_NAME_LENGTH];

  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int doWeCare[11];
  float floatvalueX[NUMBER_OF_UPSs_ON_OBSCON];
  short shortvalueX[NUMBER_OF_UPSs_ON_OBSCON], shortvalue2X[NUMBER_OF_UPSs_ON_OBSCON];
  long longvalueX[NUMBER_OF_UPSs_ON_OBSCON]; 
  long rightnow,timestampObscon[NUMBER_OF_UPSs_ON_OBSCON];

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

  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("  UPS Status on %s UT = %s HST",timeString,timeString2);
  if (DEBUG) { refresh(); }
  move(1,0);
  /*  printw("           ");*/
  printw(" obscon  : ");
  printw(" AR12  ");
  printw(" AR13 ");
  printw("  DR7  ");
  printw("  DR8a ");
  printw("  DR8b ");
  printw("  MAR ");
  printw("   Alpha "); 
  printw(" Beta  ");
  printw(" CtlRmA ");
  printw("CtlRmB ");
  printw("\n");



  if (DEBUG) { refresh(); }
  move(2,0);
     dsm_status = call_dsm_read("obscon","UPS_ENET_DATA_X",(char *)&upsStructureObsconX, 
 			       &timestampObscon[0]);
  if (DEBUG) { refresh(); }

    printw("ACVoltInpt");
    if (1) printw(" ");
    strcpy(accX,"AC_VOLTAGE_IN_V24_F");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, accX, &floatvalueX[0]);
     if(floatvalueX[upsNumber] < 1){
       timestampObscon[upsNumber] = 0; 
      	} else {
        timestampObscon[upsNumber] = timestampObscon[0];
				}							     	  
#define STALE_INTERVAL 180 //was 180
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
 			printw(" stale ");
       } else {
	if (floatvalueX[upsNumber] > 300 || floatvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	printw(" %5.1f ",floatvalueX[upsNumber]);
// 	  printw(" %d: %ld ",upsNumber, timestampObscon[upsNumber]);

	}
       }
    }
    printw("\n");
    if (DEBUG) { refresh(); }
    move(3,0);
    
    printw("ACFreqInpt");
    if (1) printw(" ");
    strcpy(accX,"AC_FREQUENCY_IN_V24_F");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &floatvalueX[0]);
      /*    fprintf(stderr,"var = %s\n",acc);*/
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (floatvalueX[upsNumber] > 300 || floatvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	  printw(" %5.2f ",floatvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(4,0);
    
    printw("AC WattsIn");
    if (1) printw(" ");
    strcpy(accX,"AC_WATTS_IN_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (shortvalueX[upsNumber] > 9999 || shortvalueX[upsNumber] < 0) {
	  printw(" wacko ");
//	} else if (shortvalueX[upsNumber] >= 1000.) {
//	  printw(" %5d ",shortvalueX[upsNumber]);
	} else {
	  printw(" %5d ",shortvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(5,0);
    
    printw("ACVoltsOut");
    if (1) printw(" ");
    strcpy(accX,"AC_VOLTAGE_OUT_V24_F");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &floatvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (floatvalueX[upsNumber] > 300 || floatvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	  printw(" %5.1f ",floatvalueX[upsNumber]);
	}
     }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(6,0);
    
    printw("ACCurrnOut");
    if (1) printw(" ");
    strcpy(accX,"AC_CURRENT_OUT_V24_F");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &floatvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (floatvalueX[upsNumber] > 300 || floatvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	  printw(" %5.1f ",floatvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(7,0);
    
    printw("ACVAOutput");
    if (1) printw(" ");
    strcpy(accX,"AC_VA_OUT_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {  
	printw(" ----- ");
      } else {
	if (shortvalueX[upsNumber] > 9999 || shortvalueX[upsNumber] < 0) {
	  printw(" wacko ");
//	} else if (shortvalueX[upsNumber] >= 1000) {
//	  printw(" %4d ",shortvalueX[upsNumber]);
	} else {
	  printw(" %5d ",shortvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(8,0);
    printw("BattryVolt");
    if (1) printw(" ");
    strcpy(accX,"BATTERY_VOLTAGE_V24_F");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &floatvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (floatvalueX[upsNumber] > 300 || floatvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	  printw(" %5.1f ",floatvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(9,0);
    strcpy(accX,"BATTERY_CURRENT_V24_F");
    printw("BattryCurr");
    if (1) printw(" ");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &floatvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (floatvalueX[upsNumber] > 300 || floatvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	  printw(" %5.1f ",floatvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(10,0);
    printw("Load%%/RunT");
    strcpy(accX,"LOAD_PERCENTAGE_V24_S");
    strcpy(acc2X,"RUN_TIME_AVAILABLE_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX,
					     accX, &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (shortvalueX[upsNumber] > 150 || shortvalueX[upsNumber] < 0) {
	  printw("wac/");
	} else {
	  if (shortvalueX[upsNumber] >= 100) {
	    printw(" %3d%",shortvalueX[upsNumber]);
	  } else {
	    printw(" %2d%/",shortvalueX[upsNumber]);
	  }
	}
    dsm_status = dsm_structure_get_element(&upsStructureObsconX,
					     acc2X, &shortvalue2X[0]);
	/* print the running time available */
	if (shortvalue2X[upsNumber] > 999 || shortvalue2X[upsNumber] < 0) {
	  printw("wac");
	} else if (shortvalue2X[upsNumber] > 100) {
	  printw("%3d",shortvalue2X[upsNumber]);
	} else {
	  printw("%2d",shortvalue2X[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(11,0);
    printw("AmbT/BrwnV");
    if (1) printw(" ");
    strcpy(accX,"AMBIENT_TEMPERATURE_V24_S");
    strcpy(acc2X,"BROWNOUT_THRESHOLD_V24_F");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw("--/");
      } else {
	if (shortvalueX[upsNumber] > 300 || shortvalueX[upsNumber] < 0) {
	  printw("wa/");
	} else {
	  printw("%2d/",shortvalueX[upsNumber]);
	}
      }
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     acc2X, &floatvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw("--- ");
      } else {
	if (floatvalueX[upsNumber] > 999 || floatvalueX[upsNumber] < 0) {
	  printw("wac ");
	} else {
	  printw("%3.0f ",floatvalueX[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(12,0);
    printw("HSnk/XfrmT");
    if (1) printw(" ");
    strcpy(accX,"HEAT_SINK_TEMPERATURE_V24_S");
    strcpy(acc2X,"TRANSFORMER_TEMPERATURE_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (shortvalueX[upsNumber] > 100 || shortvalueX[upsNumber] < 0) {
	  printw("wa/");
	} else {
	  printw("%2d/",shortvalueX[upsNumber]);
	}
    dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     acc2X, &shortvalue2X[0]);
	if (shortvalue2X[upsNumber] > 100 || shortvalue2X[upsNumber] < 0) {
	  printw("wac ");
	} else {
	  printw("%2dC ",shortvalue2X[upsNumber]);
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(13,0);
    printw("LastLinFlt");
    if (1) printw(" ");
    strcpy(accX,"MOST_RECENT_LINE_FAULT_V24_L");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,&longvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	printAgeStandoutN(rightnow,longvalueX[upsNumber],6);
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(14,0);
    printw("LinFltDura");
    if (1) printw(" ");
    strcpy(accX,"LINE_FAULT_DURATION_V24_L");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,&longvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	/* a standout value of 2 means to highlight if less than 1 day old */
	printInterval(longvalueX[upsNumber],2);
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(15,0);
    printw("LstBrwnOut");
    if (1) printw(" ");
    strcpy(accX,"MOST_RECENT_BROWNOUT_V24_L");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,&longvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	lastBrownOut[upsNumber] = printAgeStandout2(rightnow,longvalueX[upsNumber]);
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(16,0);
    printw("LstBrwnDur");
    if (1) printw(" ");
    strcpy(accX,"BROWNOUT_DURATION_V24_L");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,&longvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (lastBrownOut[upsNumber] == 2) {
	  printw(" never ");
	} else {
	  /* a standout value of 2 means to highlight if less than 1 day old */
	  printInterval(longvalueX[upsNumber],2);
	}
      }
    }
    printw("\n");

    move (17,0);
    printw("OverL/PwOut");
 //   if (1) printw(" ");
    strcpy(accX,"NUMBER_OF_OVERLOADS_V24_S");
    strcpy(acc2X,"NUMBER_OF_POWER_OUTAGES_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,      &shortvalueX[0]);
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     acc2X,      &shortvalue2X[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (shortvalueX[upsNumber] < 0) {
	  printw("wac/");
	} else {
	  printw("%2d/",shortvalueX[upsNumber]);
	}
	if (shortvalue2X[upsNumber] < 0) {
	  printw("wac");
	} else {
	  if (shortvalue2X[upsNumber] < 100) {
	    printw("%2d ",shortvalue2X[upsNumber]);
	  } else {
	  if (shortvalue2X[upsNumber] < 1000) {
	    printw("%3d ",shortvalue2X[upsNumber]);
	} else {
	  printw("%4d ",shortvalue2X[upsNumber]);
		}	  
    }
    }
    }
    }
    printw("\n");
    if (DEBUG) { refresh(); }
    move(18,0);
    
    printw("SysMod/Inv");
    /*  if (1) printw(" ");*/
    strcpy(accX,"SYSTEM_MODE_V24_S");
    strcpy(acc2X,"INVERTER_STATE_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,     &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" --/");
      } else {
	printUPSMode(shortvalueX[upsNumber]);
      }
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     acc2X,      &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw("---");
      } else {
	if (shortvalueX[upsNumber] == 1) {
	  printw(" on");
	} else {
	  printw("off");
	}
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(19,0);
    
    printw("ChargrStat");
    if (1) printw(" ");
    strcpy(accX,"CHARGER_STATE_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX, &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	printUPSMode2(shortvalueX[upsNumber]);
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(20,0);
    
    printw("BeeprStat ");
    if (1) printw(" ");
    strcpy(accX,"BEEPER_STATE_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,          &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	printUPSBeeper(shortvalueX[upsNumber]);
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(21,0);
    
    printw("ActvAlarms");
    if (1) printw(" ");
    strcpy(accX,"ACTIVE_ALARMS_V24_L");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,          &longvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	printActiveAlarm(longvalueX[upsNumber]);
      }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
    move(22,0);
    
 printw("VAoutTweak");
    if (1) printw(" ");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     "AC_VA_OUT_V24_S", &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {  
	printw(" ----- ");
      } else {
      if ((1.0579*shortvalueX[upsNumber] - 102.26) > 9999 || (1.0579*shortvalueX[upsNumber] - 102.26) < 0) {
	printw(" wacko ");
      } else if ((1.0579*(float)shortvalueX[upsNumber] - 102.26) >= 1000) {
	printw(" %5.0f ",(1.0579*(float)shortvalueX[upsNumber] - 102.26));
      } else {
	printw(" %5.1f ",(1.0579*(float)shortvalueX[upsNumber] - 102.26));
      }
    }
    }
    
    printw("\n");
    if (DEBUG) { refresh(); }
  
// printw("VAout");
//     if (1) printw(" ");
//     dsm_status = dsm_structure_get_element(&upsStructureColossus[upsNumber], "AC_VA_OUT_F", 
// 					   &floatvalue);
//     if (rightnow-timestampColossus[upsNumber] > STALE_INTERVAL)  {  
//       printw(" ----- ");
//     } else {
//       if ((1.0579*floatvalue - 102.26) > 9999 || (1.0579*floatvalue - 102.26) < 0) {
// 	printw(" wacko ");
//       } else if ((1.0579*floatvalue - 102.26) >= 1000) {
// 	printw(" %5.0f ",(1.0579*floatvalue - 102.26));
//       } else {
// 	printw(" %5.1f ",(1.0579*floatvalue - 102.26));
//       }
//     }
//     dsm_status = dsm_structure_get_element(&upsStructureObsconX, "AC_VA_OUT_F", 
// 					   &floatvalue);
//    if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {  
//       printw(" ----- ");
//     } else {
//       if ((1.0579*floatvalue - 102.26) > 9999 || (1.0579*floatvalue - 102.26) < 0) {
// 	printw(" wacko ");
//       } else if ((1.0579*floatvalue - 102.26) >= 1000) {
// 	printw(" %5.0f ",(1.0579*floatvalue - 102.26));
//       } else {
// 	printw(" %5.1f ",(1.0579*floatvalue - 102.26));
//       }
//     }
//     if (DEBUG) { refresh(); }
//     move(23,0);
    
    
    
    
	move(23,0);
    printw("SoftwareV.");
    if (1) printw(" ");
    strcpy(accX,"SOFTWARE_VERSION_V24_S");
    for (upsNumber=0; upsNumber<NUMBER_OF_UPSs_ON_OBSCON; upsNumber++) { 
      dsm_status = dsm_structure_get_element(&upsStructureObsconX, 
					     accX,       &shortvalueX[0]);
      if (rightnow-timestampObscon[upsNumber] > STALE_INTERVAL) {
	printw(" ----- ");
      } else {
	if (shortvalueX[upsNumber] < 0) {
	  printw(" wacko ");
	} else {
	  printw("  %.2f ",(float)(shortvalueX[upsNumber])/100.);
	}
      }
    }
    printw("\n");

  move(24,0);
  refresh();
}

void pickUPS(int upsNumber, char *var) {
  char localvar[DSM_NAME_LENGTH];
  switch (upsNumber) {
  case 1:
    break;
  default:
    sprintf(localvar,"DSM_UPS%d%s",upsNumber,&var[7]);
    strcpy(var,localvar);
  }
}
