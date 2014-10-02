#define DEBUG 1
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include <math.h>
#include <curses.h>
#include <dsm.h>
#include "commonLib.h"
#include "upsd.h"
#include "upspage.h"
#include "monitor.h"

extern dsm_structure upsStructure[MAX_NUMBER_ANTENNAS+1];
extern dsm_structure upsStructurem5[0];
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *t, char *str);
void checkDSMStatus(int status, char *string);
int printAge(long rightnow, long longvalue);
int printAgeNoStandout(long rightnow, long longvalue);
int printAgeStandout2(long rightnow, long longvalue);
void printUPSMode(short shortvalue);
void printUPSMode2(short shortvalue);
void printUPSBeeper(short shortvalue);

void upspage(int count, int *rm_list) {
  int rm_status, dsm_status;
  int ant, antennaNumber;
  int lastBrownOut[9];
  time_t system_time;
  char acc[5];
  char m5[7]; //this is going to be obscon, BBC
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int doWeCare[11];
  static int avoid = 0;
  float floatvalue, floatvalue2;
  short shortvalue,shortvalue2;
  long longvalue;
  float floatarray[11];
  float seconds;
  struct timespec tp;
  long rightnow,timestamp[11],m5timestamp[0];

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
  printw("Antenna ");
  for (ant=1; ant<9; ant++) {
    printw("      ");
    if (ant==avoid) {
      standout();
    }
    printw("%d",ant);
    if (ant==avoid) {
      standend();
    }
  }
  /*  printw("    AR12   AR13");*/
//  printw("  AR12 obs  AR13");
  if (DEBUG) { refresh(); }
  move(2,0);
    printw("ACVoltInpt");
    if (1) printw(" ");
    if (DEBUG) { refresh(); }
    strcpy(m5,"obscon");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      sprintf(acc,"acc%d",antennaNumber);
      dsm_status = call_dsm_read(acc, "UPS_DATA_X", 
				 (char *)&upsStructure[antennaNumber], 
				 &timestamp[antennaNumber]);
      if (antennaNumber == avoid) {
	printw(" avoid ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
			       "AC_VOLTAGE_IN_F", (char *)&floatvalue);
	
#define STALE_INTERVAL 180
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" stale ");
	} else {
	  if (floatvalue > 300 || floatvalue < 0) {
	    printw(" wacko ");
	  } else {
	    if (floatvalue < AC_VOLTS_IN_THRESHOLD) {
	      standout();
	    }
	    printw(" %5.1f ",floatvalue);
	    standend();
	  }
	}
      }
    } /* end of 'for' loop over antennas */

    
    if (DEBUG) { refresh(); }
    move(3,0);
    
    printw("ACFreqInpt");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "AC_FREQUENCY_IN_F", &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" stale ");
	} else {
	  if (floatvalue > 300 || floatvalue < 0) {
	    printw(" wacko ");
	  } else {
	    printw(" %5.2f ",floatvalue);
	  }
	}
      }
    }
    if (DEBUG) { refresh(); }
    move(4,0);
    
    printw("AC WattsIn");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "AC_WATTS_IN_F",
					       &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 9999 || floatvalue < 0) {
	    printw(" wacko ");
	  } else if (floatvalue >= 1000.) {
	    printw(" %5.0f ",floatvalue);
	  } else {
	    printw(" %5.1f ",floatvalue);
	  }
	}
      }
    }

    if (DEBUG) { refresh(); }
    move(5,0);   
    printw("ACVoltsOut");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "AC_VOLTAGE_OUT_F",
					       &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 9999 || floatvalue < 0) {
	    printw(" wacko ");
	  } else if (floatvalue >= 1000.) {
	    printw(" %5.0f ",floatvalue);
	  } else {
	    printw(" %5.1f ",floatvalue);
	  }
	}
      }
    }
 
    if (DEBUG) { refresh(); }
    move(6,0);
    
    printw("ACCurrnOut");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "AC_CURRENT_OUT_F", &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 300 || floatvalue < 0) {
	    printw(" wacko ");
	  } else {
	    printw(" %5.1f ",floatvalue);
	  }
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(7,0);
    
    printw("ACVAOutput");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "AC_VA_OUT_F", 
					       &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {  
	  printw(" ----- ");
	} else {
	  if (floatvalue > 9999 || floatvalue < 0) {
	    printw(" wacko ");
	  } else if (floatvalue >= 1000) {
	    printw(" %5.0f ",floatvalue);
	  } else {
	    printw(" %5.1f ",floatvalue);
	  }
	}
      }
    }
    if (DEBUG) { refresh(); }
    move(8,0);
    
    printw("BattryVolt");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "BATTERY_VOLTAGE_F", &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 300 || floatvalue < 0) {
	    printw(" wacko ");
	  } else {
	    printw(" %5.1f ",floatvalue);
	  }
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(9,0);
    
    printw("BattryCurr");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "BATTERY_CURRENT_F", &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 300 || floatvalue < 0) {
	    printw(" wacko ");
	  } else {
	    printw(" %5.1f ",floatvalue);
	  }
	}
      }
    }
    if (DEBUG) { refresh(); }
    move(10,0);
    
    printw("Load%%/RunT");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "LOAD_PERCENTAGE_F", &floatvalue);
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "RUN_TIME_AVAILABLE_F", &floatvalue2);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 150 || floatvalue < 0) {
	    printw("wac/");
	  } else {
	    if (floatvalue >= 100) {
	      printw(" %3.0f%",floatvalue);
	    } else {
	      printw(" %2.0f%/",floatvalue);
	    }
	  }
	  /* print the running time available */
	  if (floatvalue2 > 999 || floatvalue2 < 0) {
	    printw("wac");
	  } else if (floatvalue2 > 100) {
	    printw("%3.0f",floatvalue2);
	  } else {
	    printw("%2.0f",floatvalue2);
	  }
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(11,0);
    
    printw("AmbT/BrwnV");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw("--/---");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "AMBIENT_TEMPERATURE_F", 
					       &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw("--/");
	} else {
	  if (floatvalue > 300 || floatvalue < 0) {
	    printw("wa/");
	  } else {
	    printw("%2.0f/",floatvalue);
	  }
	}
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "BROWNOUT_THRESHOLD_F", &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw("--- ");
	} else {
	  if (floatvalue > 999 || floatvalue < 0) {
	    printw("wac ");
	  } else {
	    printw("%3.0f ",floatvalue);
	  }
	}
      }
    }

    if (DEBUG) { refresh(); }
    move(12,0);
    
    printw("HSnk/XfrmT");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "HEAT_SINK_TEMPERATURE_F", &floatvalue);
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
			       "TRANSFORMER_TEMPERATURE_F", &floatvalue2);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (floatvalue > 100 || floatvalue < 0) {
	    printw("wa/");
	  } else {
	    printw("%2d/",(int)floatvalue);
	  }
	  if (floatvalue2 > 100 || floatvalue2 < 0) {
	    printw("wac ");
	  } else {
	    printw("%2dC ",(int)floatvalue2);
	  }
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(13,0);
    
    printw("LastLinFlt");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "MOST_RECENT_LINE_FAULT_L",&longvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  printAgeStandoutN(rightnow,longvalue,6);
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(14,0);
    
    printw("LinFltDura");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "LINE_FAULT_DURATION_L",&longvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  /* a standout value of 2 means to highlight if less than 1 day old */
	  printInterval(longvalue,2);
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(15,0);
    
    printw("LstBrwnOut");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber],
				       "MOST_RECENT_BROWNOUT_L",&longvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  lastBrownOut[antennaNumber] = printAgeStandout2(rightnow,longvalue);
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(16,0);
    
    printw("LstBrwnDur");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
				       "BROWNOUT_DURATION_L",&longvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (lastBrownOut[antennaNumber] == 2) {
	    printw(" never ");
	  } else {
	    /* a standout value of 2 means to highlight if less than 1 day old */
	    printInterval(longvalue,2);
	  }
	}
      }
    }
    
    move (17,0);
    printw("OverL/PwOut");
    /*  if (1) printw(" ");*/
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "NUMBER_OF_OVERLOADS_S", 
					       &shortvalue);
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
			       "NUMBER_OF_POWER_OUTAGES_S", &shortvalue2);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (shortvalue < 0) {
	    printw("wac/");
	  } else {
	    printw("%3d/",shortvalue);
	  }
	  if (shortvalue2 < 0) {
	    printw("wac");
	  } else {
	    if (shortvalue2 < 100) {
	      printw("%2d ",shortvalue2);
	    } else {
	      printw("%3d",shortvalue2);
	    }
	  }
	}
      }
    }
    
    
    if (DEBUG) { refresh(); }
    move(18,0);
    
    printw("SysMod/Inv");
    /*  if (1) printw(" ");*/
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ---/---");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ---/---");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "SYSTEM_MODE_S", 
					       &shortvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" --/");
	} else {
	  printUPSMode(shortvalue);
	}
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "INVERTER_STATE_S", 
					       &shortvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw("---");
	} else {
	  if (shortvalue == 1) {
	    printw(" on");
	  } else {
	    printw("off");
	  }
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(19,0);
    
    printw("ChargrStat");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "CHARGER_STATE_S", 
					       &shortvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  printUPSMode2(shortvalue);
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(20,0);
    
    printw("BeeprStat ");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "BEEPER_STATE_S",
					       &shortvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  printUPSBeeper(shortvalue);
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(21,0);
    
    printw("ActvAlarms");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber],
					       "ACTIVE_ALARMS_L",
					       &longvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  printActiveAlarm(longvalue);
	}
      }
    }
    
    if (DEBUG) { refresh(); }
    move(22,0);
    
//		SystmHours
// This variable was not used, so corrected values, VAoutTweak (based on power meter measurements ) were substituted.
//
//	printw("SystmHours");
//     if (1) printw(" ");
//     for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
//       if (antennaNumber == avoid) {
// 	printw(" ----- ");
// 	continue;
//       }
//       if (antsAvailable[antennaNumber] == 0) {
// 	printw(" ----- ");
//       } else {
// 	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber],
// 					       "SYSTEM_HOURS_L", 
// 					       &longvalue);
// 	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
// 	  printw(" ----- ");
// 	} else {
// 	  if (shortvalue < 0) {
// 	    printw(" wacko ");
// 	  } else {
// 	    printw(" %5d ",longvalue);
// 	  }
// 	}
//       }
//     }
//     dsm_status = dsm_structure_get_element(&upsStructurem5[0], "SYSTEM_HOURS_L", 
// 					   &longvalue);
//     if (rightnow-m5timestamp[0] > STALE_INTERVAL) {
//       printw(" ----- ");
//     } else {
//       if (shortvalue < 0) {
// 	printw(" wacko ");
//       } else {
// 	printw(" %5d ",longvalue);
//       }
//     }
//     
//     dsm_status = dsm_structure_get_element(&upsStructurem5[1],"SYSTEM_HOURS_L", 
// 					   &longvalue);
//     if (rightnow-m5timestamp[1] > STALE_INTERVAL) {
//       printw(" ----- ");
//     } else {
//       if (shortvalue < 0) {
// 	printw(" wacko ");
//       } else {
// 	printw(" %5d ",longvalue);
//       }
//     }
//     
printw("VAoutTweak");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber], 
					       "AC_VA_OUT_F", 
					       &floatvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {  
	  printw(" ----- ");
	} else {
	  if (floatvalue > 9999 || (1.0579*floatvalue - 102.26) < 0) {
	    printw(" wacko ");
	  } else if ((1.0579*floatvalue - 102.26) >= 1000) {
	    printw(" %5.0f ",(1.0579*floatvalue - 102.26));
	  } else {
	    printw(" %5.1f ",(1.0579*floatvalue - 102.26));
	  }
	}
      }
    }

    if (DEBUG) { refresh(); }
    move(23,0);
    
    printw("SoftwareV.");
    if (1) printw(" ");
    for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
      if (antennaNumber == avoid) {
	printw(" ----- ");
	continue;
      }
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	dsm_status = dsm_structure_get_element(&upsStructure[antennaNumber],
					       "SOFTWARE_VERSION_S", 
					       &shortvalue);
	if (rightnow-timestamp[antennaNumber] > STALE_INTERVAL) {
	  printw(" ----- ");
	} else {
	  if (longvalue < 0) {
	    printw(" wacko ");
	  } else {
	    printw("  %.2f ",(float)(shortvalue)/100.);
	  }
	}
      }
    }
  move(24,0);
  refresh();
}

void checkDSMStatus(int status, char *string) {
    if (status != DSM_SUCCESS) {
      dsm_error_message(status,string);
      exit(1);
    }
}

int printAge(long rightnow, long longvalue) {
  long age;
  int standoutValue = 1;
  age = rightnow-longvalue;
  return(printInterval(age,standoutValue));
}

int printAgeStandout2(long rightnow, long longvalue) {
  long age;
  int standoutValue = 2;
  age = rightnow-longvalue;
  return(printInterval(age,standoutValue));
}

int printAgeStandoutN(long rightnow, long longvalue, int standoutValue) {
  long age;
  age = rightnow-longvalue;
  return(printInterval(age,standoutValue));
}

int printAgeNoStandout(long rightnow, long longvalue) {
  long age;
  int standoutValue = 0;
  age = rightnow-longvalue;
  return(printInterval(age, standoutValue));
}

int printAgeNoStandoutNoTrailingSpace(long rightnow, long longvalue) {
  long age;
  int standoutValue = 0;
  age = rightnow-longvalue;
  return(printIntervalNoTrailingSpace(age, standoutValue));
}

int printInterval(long age, int standoutValue) {
  int statusvalue = printIntervalNoTrailingSpace(age,standoutValue);
  printw(" ");
  return(statusvalue);
}

int printIntervalNoTrailingSpace(long age, int standoutValue) {
 /* a standout value of 6 means to highlight if less than 24hours old */
 /* a standout value of 5 means to highlight if less than 1hour old */
 /* a standout value of 4 means to highlight if more than 24 hours old */
 /* a standout value of 3 means to highlight if more than 48 hours old */
 /* a standout value of 2 means to highlight if between 1-99 hours old */
 /* a standout value of 1 means to highlight if more than 1 hour old */
 /* a standout value of 0 means to never highlight */

  /* returns 1 if value is 1 hour or more old */
  /* returns 2 if value is more than 15 years old */
  int days,hours,minutes,seconds;
  int value = 1;

  if (standoutValue == 1) {
    standout();
  }
  if (age<-86400) {  /* seconds in 15 years */
    printw(" wacko");
  } else if (age < 0) {
    /* take care of case when current time is 00:04 and update was 23:58 */
    age = 86400-fabs(age);
    hours = age/3600;
    minutes = (age % 3600)/60;
    printw("%02dh%02dm",hours,minutes);
  } else if (age>4.73e8) {  /* seconds in 15 years */
    standend();
    printw(" never");
    value = 2;
  } else if (age>99*86400) { 
    /* 00000d */
    days = age / 86400;
    if (standoutValue == 3 || standoutValue == 4) {
      standout();
    }
    printw("%05dd",days);
  } else if (age>99*3600) {
    /* 00d00h */
    days = age / 86400;
    hours = (age % 86400)/3600;
    if (standoutValue == 3 || standoutValue == 4) {
      standout();
    }
    printw("%02dd%02dh",days,hours);
  } else if (age>48*3600) {
    /* 00d00h */
    if (standoutValue == 3 || standoutValue == 2 || standoutValue == 4) {
      standout();
    }
    hours = age/3600;
    minutes = (age % 3600)/60;
    printw("%02dh%02dm",hours,minutes);
  } else if (age>24*3600) {
    /* 00h00m */
    if (standoutValue == 2 || standoutValue == 4) {
      standout();
    }
    hours = age/3600;
    minutes = (age % 3600)/60;
    printw("%02dh%02dm",hours,minutes);
  } else if (age>3600) {
    /* 00h00m */
    if (standoutValue == 2 || standoutValue == 6) {
      standout();
    }
    hours = age/3600;
    minutes = (age % 3600)/60;
    printw("%02dh%02dm",hours,minutes);
  } else { /* age is less than 1 hour */
    /* 00m00s */
    value = 0;
    if (standoutValue == 5 || standoutValue == 6) {
      standout();
    } else {
      standend();
    }
    minutes = age/60;
    seconds = age % 60;
    printw("%02dm%02ds",minutes,seconds);
  }
  standend();
  return(value);
}

void printUPSMode(short shortvalue) {
        switch (shortvalue) {
        case UPS_MODE_OFF:
          printw("off/");
	  break;
        case UPS_MODE_AUTO:
          printw(" au/");
	  break;
        case UPS_MODE_LINECONDITION:
          printw(" lc/");
	  break;
        case UPS_MODE_INVERTER:
          printw("inv/");
	  break;
        default:
          printw("wac/");
	}
}

void printUPSMode2(short shortvalue) {
        switch (shortvalue) {
        case UPS_CHARGER_MODE_AUTO:
          printw("  auto ");
	  break;
        case UPS_CHARGER_MODE_DISABLED:
          printw("  off  ");
	  break;
        case UPS_CHARGER_MODE_ENABLED:
          printw("   on  ");
	  break;
        default:
          printw(" wacko ");
	}
}

void printUPSBeeper(short shortvalue) {
        switch (shortvalue) {
        case 1:
          printw(" ready ");
	  break;
        case 0:
          printw("  off  ");
	  break;
        default:
          printw(" wacko ");
	}
}

void printActiveAlarm(long longvalue) {
  if (1 == 0) {
    /*  if (1 == 1) {
     */
    printw(" %6d",longvalue);
  } else if (longvalue & ((long)1<<UPS_CHECK_CHARGER)) { /* L */
    standout();
    printw(" ChkChr");
    standend();
  } else if (longvalue & ((long)1<<UPS_CHECK_BATTERY)) { /* M */
    standout();
    printw(" ChkBat");
    standend();
  } else if (longvalue & ((long)1<<UPS_CHECK_INVERTER)) { /* N */
    standout();
    printw(" ChkInv");
    standend();
  } else if (longvalue & ((long)1<<UPS_CHECK_MEMORY)) { /* O */
    standout();
    printw(" ChkMem");
    standend();
  } else if (longvalue & ((long)1<<UPS_LOW_AC_OUTPUT)) { /* E */
    standout();
    printw(" LowAC ");
    standend();
  } else if (longvalue & ((long)1<<UPS_HIGH_AC_OUTPUT)) { /* F */
    standout();
    printw(" HiACou");
    standend();
  } else if (longvalue & ((long)1<<UPS_LOW_BATTERY)) { /* A */
    standout();
    printw(" LoBat ");
    standend();
  } else if (longvalue & ((long)1<<UPS_NEAR_LOW_BATTERY)) { /* B */
    standout();
    printw(" NLBat ");
    standend();
  } else if (longvalue & ((long)1<<UPS_LOW_RUNTIME_LEFT)) { /* D */
    standout();
    printw(" LowRT ");
    standend();
  } else if (longvalue & ((long)1<<UPS_HIGH_AC_INPUT)) { /* S */
    standout();
    printw(" HiACin");
    standend();
  } else if (longvalue & ((long)1<<UPS_HIGH_BATTERY)) { /* C */
    standout();
    printw(" HiBat ");
    standend();
  } else if (longvalue & ((long)1<<UPS_HIGH_AMBIENT_TEMPERATURE)) { /* H */
    standout();
    printw(" HAmbT ");
    standend();
  } else if (longvalue & ((long)1<<UPS_HIGH_HEATSINK_TEMPERATURE)) { /* I */
    standout();
    printw(" HSnkT ");
    standend();
  } else if (longvalue & ((long)1<<UPS_USER_TEST_ALARM)) { /* J */
    standout();
    printw(" UsrTA ");
    standend();
  } else if (longvalue & ((long)1<<UPS_HIGH_TRANSFORMER_TEMPERATURE)) { /* K */
    standout();
    printw(" HXfrT ");
    standend();
  } else if (longvalue & ((long)1<<UPS_EMERGENCY_POWER_OFF)) { /* P */
    standout();
    printw(" EmerOf");
    standend();
  } else if (longvalue & ((long)1<<UPS_HI_PFM_RES_TEMP)) { /* Q */
    standout();
    printw(" HiPFM ");
    standend();
  } else if (longvalue & ((long)1<<UPS_PROBE_MISSING)) { /* R */
    standout();
    printw(" PrbMis");
    standend();
  } else if (longvalue & ((long)1<<UPS_CALL_SERVICE)) { /* T */
    standout();
    printw(" CSrvc ");
    standend();
  } else if (longvalue & ((long)1<<UPS_OUTPUT_OVERLOAD)) { /* G */
    standout();
    printw(" OverLd");
    standend();
  } else if (longvalue & ((long)1<<UPS_CIRCUIT_BREAKER_TRIPPED)) { /* U */
    standout();
    printw(" CBrkrT");
    standend();
  } else {
    printw("  none ");
  }
}
