#define DEBUG 1
#define A_LOW_FREQ  ((float)160e9)
#define A_HIGH_FREQ ((float)260e9)
#define B_LOW_FREQ  ((float)260e9)
#define B_HIGH_FREQ ((float)360e9)
#define INITIAL_LAST_VALUE -1
#include <curses.h>
#include <math.h>
#ifdef LINUX
#include <bits/nan.h>
#define _NAN NAN
#endif
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "rm.h"
#include "dsm.h"
#include "commonLib.h"
#include "monitor.h"
#include "upspage.h"
#include "optics.h"
#include "opticsMonitor.h"
#include "tune6status.h"
#include "receiverMonitor.h"

extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *, char *);
extern int translateBandNumberToGHz(int band);
short slaveToRxNumber(short slave);
void printGunnType(short type);
extern void noBoard(int); 
extern int lastDeiceWarn;
extern int deiceCheckCount;
extern dsm_structure mRGControl;
extern void checkStatus(int,char *);

static int readMixerCurrentTargets = 0;
int motorizedMultiplier[MAX_NUMBER_ANTENNAS+1][NUMBER_OF_RECEIVERS];

float sis0MixerBfieldMax[MAX_NUMBER_ANTENNAS+1][4];
float sis0MixerBfieldMin[MAX_NUMBER_ANTENNAS+1][4];
float sis1MixerBfieldMax[MAX_NUMBER_ANTENNAS+1][4];
float sis1MixerBfieldMin[MAX_NUMBER_ANTENNAS+1][4];

float sis0MixerCurrentMax[MAX_NUMBER_ANTENNAS+1][4];
float sis0MixerCurrentMin[MAX_NUMBER_ANTENNAS+1][4];
float sis1MixerCurrentMax[MAX_NUMBER_ANTENNAS+1][4];
float sis1MixerCurrentMin[MAX_NUMBER_ANTENNAS+1][4];

float sis0MixerVoltageMax[MAX_NUMBER_ANTENNAS+1][4];
float sis0MixerVoltageMin[MAX_NUMBER_ANTENNAS+1][4];
float sis1MixerVoltageMax[MAX_NUMBER_ANTENNAS+1][4];
float sis1MixerVoltageMin[MAX_NUMBER_ANTENNAS+1][4];

void receiverMonitor(int count, int *antlist, int pageMode, RECEIVER_FLAGS *flags) {
  int ant, rms, dummyInt, motors[MAX_NUMBER_ANTENNAS+1][8][9];
  char stringvalue[11];
  int use345at230[MAX_NUMBER_ANTENNAS+1];
  double restFrequency[2];
  double dummyDouble, lastGunnFreq[NUMBER_OF_RECEIVERS];
  float dummyFloat;
  short shortarray[MAX_NUMBER_ANTENNAS+1][NUMBER_OF_RECEIVERS];
  short dummyShort, actPLL[MAX_NUMBER_ANTENNAS+1], actRx[MAX_NUMBER_ANTENNAS+1], mixerBoardPresent[MAX_NUMBER_ANTENNAS+1];
  short actPLLrx[MAX_NUMBER_ANTENNAS+1], opticsBoardPresent[MAX_NUMBER_ANTENNAS+1];
  short lRx[MAX_NUMBER_ANTENNAS+1];
  char dummyString1[100];
  time_t timestamp, ivTime[MAX_NUMBER_ANTENNAS+1];
  float gunnBias[MAX_NUMBER_ANTENNAS+1];
  float mixerCurrent[MAX_NUMBER_ANTENNAS+1];
  int atten[MAX_NUMBER_ANTENNAS+1];
  static int lastatten[MAX_NUMBER_ANTENNAS+1] = {INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE};
  static float lastrfpower[MAX_NUMBER_ANTENNAS+1] = {INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE};
  static float lastBfield[MAX_NUMBER_ANTENNAS+1] = {INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE};
  static float lastVbias[MAX_NUMBER_ANTENNAS+1] = {INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE,
				 INITIAL_LAST_VALUE};
  static float lastgain[MAX_NUMBER_ANTENNAS+1] = {INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE,
			      INITIAL_LAST_VALUE};
  int doWeCare[MAX_NUMBER_ANTENNAS+1];
  long longvalue, timevalue;
  short mRGLocked[2];
  short network[MAX_NUMBER_ANTENNAS+1][15];
  int unixTime;
  int rx;
  float mincurrent, maxcurrent;
  float minbfield, maxbfield;
  int narg, i, band, motorizedAxes;
  long syncdet2timestamp;
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  short busy;
  short loboardPresent[MAX_NUMBER_ANTENNAS+1][NUMBER_OF_RECEIVERS];  
  short gunnType[NUMBER_OF_RECEIVERS];
  int rxnumber[MAX_NUMBER_ANTENNAS+1];
  int isReceiverInArray[3];
  int freqTooLow = 0;
  int freqTooHigh = 0;
  int line;
  char *ptr;
  char acc[10];
  char linebuf[200];
  short boardSubType[15];
  short shortvalue;
  short joystick;
  int bFieldLine;
  char filename[200];
  FILE *fp;
  static int linesRead = 0;

  if (readMixerCurrentTargets == 0) {
    readMixerCurrentTargets = 1;

/************** Added code for b-field targets ******************/       
    fp = fopen("/common/configFiles/mixerBfield.targets", "r");
    if (fp != NULL) {
      do {
	ptr = fgets(linebuf,sizeof(linebuf),fp);
	if (ptr != NULL) {
	  if (strstr(ptr,"#") == NULL) {
	    narg = sscanf(linebuf,"%d %d %f %f",&ant, &rx, &minbfield, &maxbfield);
	    if (narg == 4) {
	      if (ant >= 1 && ant <= 8) {
		if (rx >= 0 && rx < 4) {
		  sis0MixerBfieldMin[ant][rx] = minbfield;
		  sis0MixerBfieldMax[ant][rx] = maxbfield;
		} else if (rx >= 4 && rx < 8) {
		  sis1MixerBfieldMin[ant][rx-4] = minbfield;
		  sis1MixerBfieldMax[ant][rx-4] = maxbfield;
		}
	      }
	    }
	  }
	}
	linesRead++;
      } while (ptr != NULL);
      fclose(fp);    
    }        
/************** End of code for b-field targets ******************/    
    
    fp = fopen("/common/configFiles/mixerCurrent.targets", "r");
    if (fp != NULL) {
      do {
	ptr = fgets(linebuf,sizeof(linebuf),fp);
	if (ptr != NULL) {
	  if (strstr(ptr,"#") == NULL) {
	    narg = sscanf(linebuf,"%d %d %f %f",&ant, &rx, &mincurrent, &maxcurrent);
	    if (narg == 4) {
	      if (ant >= 1 && ant <= 8) {
		if (rx >= 0 && rx < 4) {
		  sis0MixerCurrentMin[ant][rx] = mincurrent;
		  sis0MixerCurrentMax[ant][rx] = maxcurrent;
		} else if (rx >= 4 && rx < 8) {
		  sis1MixerCurrentMin[ant][rx-4] = mincurrent;
		  sis1MixerCurrentMax[ant][rx-4] = maxcurrent;
		}
	      }
	    }
	  }
	}
	linesRead++;
      } while (ptr != NULL);
      fclose(fp);    
    }
    fp = fopen("/common/configFiles/mixerBias.targets", "r");
    if (fp != NULL) {
      do {
	ptr = fgets(linebuf,sizeof(linebuf),fp);
	if (ptr != NULL) {
	  if (strstr(ptr,"#") == NULL) {
	    narg = sscanf(linebuf,"%d %d %f %f",&ant, &rx, &mincurrent, &maxcurrent);
	    if (narg == 4) {
	      if (ant >= 1 && ant <= 8) {
		if (rx >= 0 && rx < 4) {
		  sis0MixerVoltageMin[ant][rx] = mincurrent;
		  sis0MixerVoltageMax[ant][rx] = maxcurrent;
		} else if (rx >= 4 && rx < 8) {
		  sis1MixerVoltageMin[ant][rx-4] = mincurrent;
		  sis1MixerVoltageMax[ant][rx-4] = maxcurrent;
		}
	      }
	    }
	  }
	}
	linesRead++;
      } while (ptr != NULL);
      fclose(fp);    
    }
    for (i=1; i<=8; i++) {
      /* assume all multipliers are motorized in the default fashion */
      /* i.e. 2 motors on the 230,345,690, and 0 on the 400 */  
      motorizedMultiplier[i][0] = 2;
      motorizedMultiplier[i][1] = 2;
      motorizedMultiplier[i][2] = 2;
      motorizedMultiplier[i][3] = 2;
      motorizedMultiplier[i][4] = 0;
      motorizedMultiplier[i][5] = 2;
      motorizedMultiplier[i][6] = 2;
      motorizedMultiplier[i][7] = 2;
      sprintf(filename,"/otherInstances/acc/%d/gunnTuningFiles/filelist",i);
      fp = fopen(filename, "r");
      if (fp != NULL) {
	do {
	  ptr = fgets(linebuf,sizeof(linebuf),fp);
	  if (ptr != NULL) {
	    if (strstr(ptr,"#") == NULL) {
	      narg = sscanf(linebuf,"%d %*s %d", &band, &motorizedAxes);
	      if (narg > 1 && band >=0 && band < NUMBER_OF_RECEIVERS) {
		motorizedMultiplier[i][band] = motorizedAxes;
	      }
	    }
	  }
	} while (ptr != NULL);
      }
      fclose(fp);    
    } /* end 'for' loop */
  }
  
  
  
  
  /* first determine if the mixer current is in a nominal range */
  getAntennaList(doWeCare);
  getReceiverList(isReceiverInArray);
  rms = call_dsm_read("hal9000", "DSM_AS_IFLO_REST_FR_V2_D", (char *)&restFrequency, &timestamp);
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms = rm_read(ant, "RM_UNIX_TIME_L", &unixTime);
    flags->flags[ant] = 0;
    flags->mixerBfield[ant] = 0;
    flags->mixerCurrent[ant] = 0;
    flags->mixerVoltage[ant] = 0;
    flags->mixerSelect[ant] = 0;
    flags->yigSelect[ant] = 0;
    flags->gridSelect[ant] = 0;
    flags->activePLL[ant] = 0;
    flags->ivsweepAge[ant] = 0;
    flags->ifpower[ant] = 0;
    if (antsAvailable[ant]) {
      rxnumber[ant] = findReceiverNumberLow(ant);
      if (rxnumber[ant]==0) {
	actRx[ant] = 0;
	if (restFrequency[0] < A_LOW_FREQ) {
	  freqTooLow = 1;
	  flags->mixerSelect[ant] = 1;
	}
	if (restFrequency[0] > A_HIGH_FREQ) {
	  freqTooHigh = 1;
	  flags->mixerSelect[ant] = 1;
	}
      } else if (rxnumber[ant]==1) {
	actRx[ant] = 1;
	if (restFrequency[0] < A_LOW_FREQ) {
	  freqTooLow = 1;
	  flags->mixerSelect[ant] = 1;
	}
	if (restFrequency[0] > A_HIGH_FREQ) {
	  freqTooHigh = 1;
	  flags->mixerSelect[ant] = 1;
	}
      } else if (rxnumber[ant]==2) {
	actRx[ant] = 2;
	if (restFrequency[0] < B_LOW_FREQ) {
	  if (use345at230[ant] == 0) {
	    freqTooLow = 1;
	    flags->mixerSelect[ant] = 1;
	  }
	}
	if (restFrequency[0] > B_HIGH_FREQ) {
	  if (use345at230[ant] == 0) {
	    freqTooHigh = 1;
	    flags->mixerSelect[ant] = 1;
	  }
	}
      } else if (rxnumber[ant]==3) {
	if (restFrequency[0] < B_LOW_FREQ) {
	  flags->mixerSelect[ant] = 1;
	  freqTooLow = 1;
	}
	if (restFrequency[0] > B_HIGH_FREQ) {
	  flags->mixerSelect[ant] = 1;
	  freqTooHigh = 1;
	}
	actRx[ant] = 3;
      } else if (rxnumber[ant]==4) {
	actRx[ant] = 4;
      } else if (rxnumber[ant]==5) {
	actRx[ant] = 5;
      } else if (rxnumber[ant]==6) {
	actRx[ant] = 6;
      } else if (rxnumber[ant]==7) {
	actRx[ant] = 7;
      } else {
	actRx[ant] = -1;
      }
      rms = rm_read(ant, "RM_MIXER_BOARD_PRESENT_S", &mixerBoardPresent[ant]);
      if (mixerBoardPresent[ant]) {
	
	rms = rm_read(ant, "RM_SIS_MIXER0_BFIELD_CALIB_F", &dummyFloat);
	if (fabs(dummyFloat) < sis0MixerBfieldMin[ant][actRx[ant]] || 
	    fabs(dummyFloat) > sis0MixerBfieldMax[ant][actRx[ant]]) {
	  flags->mixerBfield[ant] = 1;
	}
      
	rms = rm_read(ant, "RM_SIS_MIXER0_CURRENT_CALIB_F", &dummyFloat);
	if (fabs(dummyFloat) < sis0MixerCurrentMin[ant][actRx[ant]] || 
	    fabs(dummyFloat) > sis0MixerCurrentMax[ant][actRx[ant]]) {
	  flags->mixerCurrent[ant] = 1;
	}
	
        rms = rm_read(ant, "RM_SIS_MIXER0_VOLTAGE_CALIB_F", &dummyFloat);
        if (fabs(dummyFloat) < sis0MixerVoltageMin[ant][actRx[ant]] || 
            fabs(dummyFloat) > sis0MixerVoltageMax[ant][actRx[ant]]) {
	  flags->mixerVoltage[ant] = 1;
        }
      }
      rms = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", loboardPresent[ant]);
      if (loboardPresent[ant][rxnumber[ant]] < 0) {
	if (rxnumber[ant] == 2) {
	  use345at230[ant] = 1;
	}
      }
      rms = rm_read(ant, "RM_YIG_SWITCH_LOW_FREQ_S", &lRx[ant]);
      switch (lRx[ant]) {
      case 0:
	if (restFrequency[0] < A_LOW_FREQ || restFrequency[0] > A_HIGH_FREQ) {
	  flags->yigSelect[ant] = 1;
	}
	break;
      case 1:
	if (restFrequency[0] < A_LOW_FREQ || restFrequency[0] > A_HIGH_FREQ) {
	  flags->yigSelect[ant] = 1;
	}
	break;
      case 2:
	if (restFrequency[0] < B_LOW_FREQ || restFrequency[0] > B_HIGH_FREQ) {
	  if (use345at230[ant] == 0) {
	    flags->yigSelect[ant] = 1;
	  }
	}
	break;
      case 3:
	if (restFrequency[0] < B_LOW_FREQ || restFrequency[0] > B_HIGH_FREQ) {
	  if (use345at230[ant] == 0) {
	    flags->yigSelect[ant] = 1;
	  }
	}
	break;
      }
      rms = rm_read(ant, "RM_LOWFREQ_IVSWEEP_TIMESTAMP_L",&ivTime[ant]);
      if (unixTime-ivTime[ant] > IVSWEEP_HOURS_HIGHLIGHT*3600) {
	flags->ivsweepAge[ant] = 1;
      }
      rms = rm_read(ant, "RM_CONT1_DET1_POWER_MUWATT_F", &dummyFloat);
      if (dummyFloat < MIN_IFPOWER_MUWATT) {
	flags->ifpower[ant] = 1;
      }

      rms = rm_read(ant, "RM_SLAVE1_PLL_NO_S", &actPLL[ant]);
      rms = rm_read(ant,"RM_WIRE_GRID_RECEIVER_LOCATION_L", &longvalue);

      if (restFrequency[0] < A_HIGH_FREQ && longvalue!=0) {
	rms = rm_read(ant, "RM_ACTIVE_LOW_RECEIVER_C10", stringvalue);
	checkStatus(rms,"rm_read(RM_ACTIVE_LOW_RECEIVER_C10)");
	rxnumber[ant] = parseReceiverName(stringvalue);
	rms = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", 
		      loboardPresent[ant]);
	if (loboardPresent[ant][rxnumber[ant]] < 0) {
	  if (rxnumber[ant] == 2) {
	    use345at230[ant] = 1;
	  } else {
	    flags->gridSelect[ant] = 1;
	  }
	}
      }
      if (restFrequency[0] < B_HIGH_FREQ && restFrequency[0]> B_LOW_FREQ && 
	  longvalue!=2) {
	flags->gridSelect[ant] = 1;
      }
      if ((actPLL[ant]==2 && restFrequency[0]>A_HIGH_FREQ) ||
	  (actPLL[ant]==3 && (restFrequency[0]>B_HIGH_FREQ || restFrequency[0]<B_LOW_FREQ))) {
	rms = rm_read(ant, "RM_ACTIVE_LOW_RECEIVER_C10", stringvalue);
	checkStatus(rms,"rm_read(RM_ACTIVE_LOW_RECEIVER_C10)");
	rxnumber[ant] = parseReceiverName(stringvalue);
	rms = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", 
		      &loboardPresent[ant][0]);
	if (loboardPresent[ant][rxnumber[ant]] < 0) {
	  if (rxnumber[ant] == 2) {
	    use345at230[ant] = 1;
	  } else {
	    flags->activePLL[ant] = 1;
	  }
	}
      }
      /* do a logical 'or' on all the possible warning values */
      flags->flags[ant] |= flags->mixerBfield[ant];
      flags->flags[ant] |= flags->mixerCurrent[ant];
      flags->flags[ant] |= flags->mixerVoltage[ant];
      flags->flags[ant] |= flags->mixerSelect[ant];
      flags->flags[ant] |= flags->yigSelect[ant];
      flags->flags[ant] |= flags->ifpower[ant];
      flags->flags[ant] |= flags->activePLL[ant];
      flags->flags[ant] |= flags->gridSelect[ant];
      flags->flags[ant] |= flags->ivsweepAge[ant];
    }

    if (doWeCare[ant] == 0) {
      /* do not propagate errors to 'a' page if antenna is not in array */
      flags->flags[ant] = 0;
    }

  } /* loop over antennas */
  if (pageMode == RECEIVER_PAGE_CHECK_ONLY) {
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
  }
  ant = 0;
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  rms = call_dsm_read("m5",
		"MRG_CONTROL_X", 
		(char *)&mRGControl,
		&timestamp);
  rms = dsm_structure_get_element(&mRGControl,
				  "YIG_LOCKED_V2_S", 
				  (char *)&mRGLocked);
  if (mRGLocked[0]) {
    /*
    printw("Low-freq Receiver: %s UT = %s HST",timeString,timeString2);
    */
    printw("LowFreqRx   1");
  } else {
    /*    printw("LoFreqRx ");*/
    standout();
    printw("MRG Unlocked");
    standend();
    printw(" ");
    /*
    printw(" %s UT = %s HST",timeString,timeString2);
    */
  }
  /*
    printw(" %d\n",lastDeiceWarn);
  */
  if (DEBUG) refresh();
  /*
  move(1,0);
  printw("Antenna     1 ");
  */
  for (ant=2; ant<=numberAntennas; ant++) {
    switch (ant) {
    case CSO_ANTENNA_NUMBER:
      printw("      CSO");
      break;
    case JCMT_ANTENNA_NUMBER:
      printw("     JCMT");
      break;
    default:
      printw("        %d",ant);
    }
  }
  /*  printw(" %d\n",deiceCheckCount);*/
  if (DEBUG) refresh();
  line = 0;
  line++;
  move(line,0);
  printw("ActRx/PLL");
  standend();
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (flags->mixerSelect[ant] == 1 && doWeCare[ant]==1) {
	standout();
	/*
	if (freqTooLow) {
	  printw("freqTooLow");
	} else if (freqTooHigh) {
	  printw("freqTooHigh");
	}
	*/
      }
      rms = rm_read(ant, "RM_ACTIVE_LOW_RECEIVER_C10", &dummyString1);
      if (actRx[ant] == -1) {
	if (ant <= 8) {
	  printw("  wacko  ");
	} else {
	  rms = rm_read(ant, "RM_SLAVE1_PLL_NO_S",&shortvalue);
	  checkStatus(rms,"rm_read(RM_SLAVE1_PLL_NO_S)");
	  rms= rm_read(ant,"RM_RX_MICROCONTROLLER_SUBTYPE_V15_S",boardSubType);
	  checkStatus(rms,"rm_read(RM_RX_MICROCONTROLLER_SUBTYPE_V15_S)");
	  if (ant > 0)
	    shortvalue = 0;
	  actRx[ant] = boardSubType[shortvalue];
	  printw(" %s/ %1d ",getLoBoardTypeStringBrief(actRx[ant]),shortvalue);
	}
      } else {
	rms = rm_read(ant, "RM_SLAVE1_PLL_NO_S", &actPLL[ant]);
	rms = rm_read(ant, "RM_RX_GUNN_TYPE_V8_S", gunnType);
        printw(" %s/ ",dummyString1);
	if (ant == CSO_ANTENNA_NUMBER) {
	  actPLLrx[ant] = parseReceiverName(dummyString1);
	} else {
	  actPLLrx[ant] = slaveToRxNumber(actPLL[ant]);
	}
	if ((actPLL[ant] < 0) || (actPLL[ant] >= 8)) {
	  printw("?   ");
	} else {
	  if (flags->activePLL[ant] == 1 && doWeCare[ant]==1) {
	    standout();
	  }
	  printw("%1d", actPLL[ant]);
	  standend();
	  printGunnType(gunnType[actRx[ant]]);
#if 0
	  printw("%d",rxnumber[ant]);
#else
	  printw(" ");
#endif
	}
      }
      standend(); 
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("YIGSwitch ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("-----    ");
    } else {
      short lLocked;
      int delay;
      rms = rm_read(ant, "RM_YIG_SVC_TIMESTAMP_L", &dummyInt);
      delay = unixTime - dummyInt;
      rms = rm_read(ant, "RM_YIG1_LOCKED_S", &lLocked);
      if (flags->yigSelect[ant] == 1 && doWeCare[ant]==1) {
	standout();
      }
      if (ant > 8) {
	printw("  ");
      } else {
	switch (lRx[ant]) {
	case 0:
	  printw("A1");
	  break;
	case 1:
	  printw("A2");
	  break;
	case 2:
	  printw("B1");
	  break;
	case 3:
	  printw("B2");
	  break;
	default:
	  printw("??");
	  break;
	}
      }
      standend();
      if (delay < 30) {
	if (lLocked == 0) {
	  if (doWeCare[ant]==1) {
	    standout();
	  }
	  printw(" Unlk");
	  if (doWeCare[ant]==1) {
	    standend();
	  }
	} else if (lLocked == 1) {
	  printw(" Lock");
	} else {
	  if (doWeCare[ant]==1) {
	    standout();
	  }
	  printw("wacko");
	  if (doWeCare[ant]==1) {
	    standend();
	  }
	}
      } else {
        if (doWeCare[ant]==1) {
          standout();
	}
	printw("stale");
	/*	printw("%4d",lLocked);*/
	if (doWeCare[ant]==1) {
          standend();
	}
      }
      printw("  ");
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("WireGrid ");
  for (ant=1; ant<=numberAntennas; ant++) { 
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_OPTICS_BOARD_PRESENT_S",
		    &opticsBoardPresent[ant]);
      if (opticsBoardPresent[ant] == 0) {
	noBoard(ant);
	if (ant<7) {
	  printw(" ");
	}
	continue;
      }
      rms = rm_read(ant,"RM_WIRE_GRID_RECEIVER_LOCATION_L", &longvalue);
      checkStatus(rms,"rm_read(RM_WIRE_GRID_RECEIVER_LOCATION_L)");
      rms = rm_read(ant, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
      if (timevalue < unixTime-GRID_STALE_INTERVAL) {
	if (doWeCare[ant]==1) {
	  standout();
	}
	printw("   stale ");
	if (doWeCare[ant]==1) {
	  standend();
	}
      } else {
        rms = rm_read(ant, "RM_TUNE6_COMMAND_BUSY_S", &busy);
	switch (busy) {
        case TUNE6_BUSY_WITH_WIRE_GRID:
	  standout();
	  printw(" turning ");
	  standend();
	  break;
	default:
	  printw("  ");
	  /*	  if (doWeCare[ant]==1) { */
	    if (flags->gridSelect[ant] == 1) {
	      standout();
	    }
	    /*	  } */
          printw("%2s=%3d",getLoBoardTypeStringBrief(longvalue),
             translateBandNumberToGHz(longvalue));
	  standend();
	  printw(" ");
	  break;
	}
      }
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("LastFreq ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw(" -----   ");
    } else {
      rms=rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S",loboardPresent[ant]);
      rms = rm_read(ant, "RM_MOTORIZED_LO_CHAIN_V8_V9_L", motors[ant]);
      rms = rm_read(ant, "RM_LAST_GUNN_FREQ_V8_D", &lastGunnFreq);
      rms = rm_read(ant, "RM_ACTIVE_LOW_RECEIVER_C10", stringvalue);
      checkStatus(rms,"rm_read(RM_ACTIVE_LOW_RECEIVER_C10)");
      rxnumber[ant] = parseReceiverName(stringvalue);
      if (loboardPresent[ant][rxnumber[ant]] < 0) {
	if (rxnumber[ant] == 2) {
	  /* this might be the case where we are using the 345 at 230 */
	  use345at230[ant] = 1;
	  actRx[ant] = 0;
	}
      }
      if (!(motors[ant][actRx[ant]][8] == 1)) {
	printw("  manual ");
      } else if ((lastGunnFreq[actRx[ant]] > 50.0) &&
		 (lastGunnFreq[actRx[ant]] < 1000.0)) {
	printw(" %6.2f  ", lastGunnFreq[actRx[ant]]);
      } else {
	printw("  wacko  ");
      }
    }
  }
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", shortarray[ant-1]);
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("Gunn/PLL ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (DEBUG) refresh();
    if (antsAvailable[ant] == 0 && ant < 9) {
      printw(" -----   ");
    } else {
      rms = rm_read(ant, "RM_RX_MICROCONTROLLER_NETWORK_V15_S", network[ant]);
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7 || actPLL[ant] < 0 || actPLL[ant]>7) {
	printw(" wacko   ");
#define YIG_BOARD_TYPE 6
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
	printw("no board ");
      } else {
        rms = rm_read(ant, "RM_GUNN_ON_S", &dummyShort);
        switch (dummyShort) {
        case 0:
	  if (doWeCare[ant]==1) {
	    standout();
  	  }
	  printw("Off/");
  	  if (doWeCare[ant]==1) {
	    standend();
	  }
	  break;
        case 1:
	  printw(" On/");
  	  break;
        default:
  	  if (doWeCare[ant]==1) {
	    standout();
	  }
	  printw("---/");
	  if (doWeCare[ant]==1) {
	    standend();
	  }
        }
	if (DEBUG) refresh();
        rms = rm_read(ant, "RM_PLL_ON_S", &dummyShort);
        switch (dummyShort) {
        case -1:
	  if (doWeCare[ant]==1 && restFrequency[0] < HI_LO_FREQ_CUTOFF_HZ) {
	    standout();
	  }
	  printw("LSB");
	  standend();
	  printw("  ");
  	  break;
        case 0:
	  if (doWeCare[ant]==1 && restFrequency[0] < HI_LO_FREQ_CUTOFF_HZ) {
	    standout();
	  }
	  printw("off");
	  standend();
	  printw("  ");
	  break;
        case 1:
	  printw("USB  ");
	  break;
        default:
	  if (doWeCare[ant]==1 && restFrequency[0] < HI_LO_FREQ_CUTOFF_HZ) {
	    standout();
	  }
	  printw("---  ");
	  standend();
        }
      }
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("GunnLock");
  if (DEBUG) refresh();
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant] && ant != JCMT_ANTENNA_NUMBER) {
      printw("  -----  ");
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7 || actPLL[ant]<0 || actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        int delay;

        rms = rm_read(ant, "RM_GUNN1_LOCKED_S", &dummyShort);
        rms = rm_read(ant, "RM_GUNN1_LOCKED_TIMESTAMP_L", &dummyInt);
        rms = rm_read(ant, "RM_TUNE6_COMMAND_BUSY_S", &busy);
        delay = unixTime - dummyInt;
        if ((dummyShort != 0) && (dummyShort != 1)) {
	  dummyShort = 0;
        }
        if (dummyShort == 0 && doWeCare[ant]==1 && restFrequency[0] < HI_LO_FREQ_CUTOFF_HZ) {
	  standout();
        }
        if (delay <= 90) {
	  switch (busy) {
          case TUNE6_BUSY_WITH_TUNE_GUNN_LOW:
    	    printw(" tuning  ");
	    break;
          case TUNE6_BUSY_WITH_OPT_LO_POWER_LOW:
      	    printw(" LO adj  ");
	    break;
          case TUNE6_BUSY_WITH_WIRE_GRID:
	  default:
            if (delay > 5) {
  	      printw(" %1d stale ", dummyShort);
	    } else {
    	      printw("    %1d    ", dummyShort);
  	    }
	    break;
	  }
        } else {
          printw(" %1d stale ", dummyShort);
        }
	standend();
      }
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("Stress V");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7 || actPLL[ant]<0 || actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        float targetBias[NUMBER_OF_RECEIVERS]; /* one for each receiver in this antenna */
        float stress;
        rms = rm_read(ant, "RM_GUNN_TARGET_BIAS_V8_F", targetBias);
        rms = rm_read(ant, "RM_GUNN_BIAS_F", &gunnBias[ant]);
        if (actRx[ant] >= 0) {
          stress = gunnBias[ant]-targetBias[actRx[ant]];
#define GUNN_DROPOUT_IMMINENT 0.45
          if (doWeCare[ant]==1 && fabs(stress) > GUNN_DROPOUT_IMMINENT
	      && restFrequency[0] < HI_LO_FREQ_CUTOFF_HZ) {
	    rms = rm_read(ant, "RM_PLL_ON_S", &dummyShort);
	    if (dummyShort == 1) {
	      standout();
	    }
          }
	  if (fabs(stress) < 10) {
	    printw(" ");
	  }
          printw("%+7.4f ", stress);
	  standend();
        } else {
          printw("  wacko  ");
        }
      }
    }
  }


  if (DEBUG) refresh();
  move(++line,0);
  printw("Gunn V  ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant] && ant != JCMT_ANTENNA_NUMBER) {
      printw("  -----  ");
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7 || actPLL[ant]<0 || actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        if ((gunnBias[ant] > 1.0e2) || (gunnBias[ant] < -1.0e2)) {
	  gunnBias[ant] = _NAN;
        }
        printw(" %7.4f ", gunnBias[ant]);
      }
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("IF Power");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  "); 
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        float iFPower;
        float noisePower;
        rms = rm_read(ant, "RM_GUNN_PLL_IFPOWER_F", &iFPower);
        if ((iFPower > 1.0e6) || (iFPower < -1.0e6)) {
	  iFPower = _NAN;
        }
#define IF_NZ_ON_SEPARATE_LINES 1
#if IF_NZ_ON_SEPARATE_LINES 
	/*
	if (iFPower >= LO_BOARD_ADC_SATURATED) {
	  standout();
	}
	*/
        printw(" %7.4f ", iFPower);
	standend();
#else
        printw(" %3d/", (int)(iFPower*100));
        rms = rm_read(ant, "RM_GUNN_PLL_PHASENOISE_F", &noisePower);
        if ((noisePower > 10.0) || (noisePower < -10.0)) {
	  noisePower = _NAN;
        }
        printw("%3d ", (int)(noisePower*100));
#endif
      }
    }
  }
#if IF_NZ_ON_SEPARATE_LINES
  if (DEBUG) refresh();
  move(++line,0);
  printw("Nz Power");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        float noisePower;
        rms = rm_read(ant, "RM_GUNN_PLL_PHASENOISE_F", &noisePower);
        if ((noisePower > 10.0) || (noisePower < -10.0)) {
	  noisePower = _NAN;
        }
        printw(" %7.4f ", noisePower);
      }
    }
  }
#endif
  if (DEBUG) refresh();
  move(++line,0);
  printw("PLLRatio");
  for (ant=1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        float phaseNoise, iFPower, ratio;
        rms = rm_read(ant, "RM_GUNN_PLL_IFPOWER_F", &iFPower);
        rms = rm_read(ant, "RM_GUNN_PLL_PHASENOISE_F", &phaseNoise);
        if ((phaseNoise > 1.0e6) || (phaseNoise < -1.0e6)) {
	  phaseNoise = _NAN;
        }
        if ((iFPower > 1.0e6) || (iFPower < -1.0e6)) {
	  iFPower = _NAN;
        }
        if (phaseNoise != 0.0) {
	  ratio = iFPower/phaseNoise;
        } else {
	  ratio = _NAN;
        }
        if ((dummyShort != 0) && (dummyShort != 1)) {
	  dummyShort = 0;
        }
        if ((ratio < 100.0) && (ratio > -10.0)) {
	  printw(" %7.1f ", ratio);
        } else if (ratio >= 100000.0) {
	  printw("  wacko  ");
        } else if (ratio > 100.0) {
	  printw(" %7.0f ", ratio);
        } else {
	  printw(" %7.0f ", _NAN);
	}
      }
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("LoopGain");
  for (ant=1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (actPLLrx[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE
		 && ant < 9) {
        printw(" no board");
      } else {
        rm_read(ant, "RM_PLL1_GAIN_S", &dummyShort);
        if (dummyShort < 0 || dummyShort > 255) {
          printw("  wacko  ");
        } else {
	  if (lastgain[ant] != dummyShort && 
	      lastgain[ant] != INITIAL_LAST_VALUE) {
	    standout();
	  }
	  printw("   %3d   ",dummyShort);
	  standend();
	  lastgain[ant] = dummyShort;
	}
      }
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("RFReqAch ");
  for (ant=1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_TUNE6_COMMAND_BUSY_S", &busy);
      switch (busy) {
      case TUNE6_BUSY_WITH_IV_SWEEP:
	standout();
	printw(" IVcurve ");
	standend();
	break;
      case TUNE6_BUSY_WITH_PB_SWEEP:
	standout();
	printw(" PBcurve ");
	standend();
	break;
      default:
#define WACKO_YIG_POWER_MIN -150 /* dBm */
#define WACKO_YIG_POWER_MAX  25 /* dBm */
#define DOPPLER_TRACK_YIG_POWER  20 /* dBm */
       rm_read(ant, "RM_GUNN1_REQUESTED_RFPOWER_DBM_F", &dummyFloat);
       if (dummyFloat< WACKO_YIG_POWER_MIN || dummyFloat>WACKO_YIG_POWER_MAX) {
         printw("  wacko  ");
       } else {
#define RFPOWER_CLIENT_CREATE_FAILED -100
        if (dummyFloat >= DOPPLER_TRACK_YIG_POWER) {
          printw(" max/");
	} else if (dummyFloat <= RFPOWER_CLIENT_CREATE_FAILED) {
          printw("ClntFail ");
	  continue;
        } else {
	  if (lastrfpower[ant] != dummyFloat 
	      && lastrfpower[ant] != INITIAL_LAST_VALUE) {
	    standout();
	  } 
          printw("%4.1f",dummyFloat);
	  lastrfpower[ant] = dummyFloat;
          standend();
          printw("/");
	}
        rm_read(ant, "RM_GUNN1_ACHIEVED_RFPOWER_DBM_F", &dummyFloat);
        if (dummyFloat >= DOPPLER_TRACK_YIG_POWER) {
          printw("max");
	} else if (dummyFloat <= RFPOWER_CLIENT_CREATE_FAILED) {
          printw("ClF");
        } else {
	  if (dummyFloat != dummyFloat) {
	    printw("NaN");
	  } else if (dummyFloat >= 10) {
            printw("%2.0f.",dummyFloat);
	  } else if (dummyFloat < 0) {
            printw("%+1.0f.",dummyFloat);
	  } else {
            printw("%3.1f",dummyFloat);
	  }
	}
	if (ant != numberAntennas) {
          printw(" ");
	}
       }
      }
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("SIS-0 V ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_MIXER_BOARD_PRESENT_S", &mixerBoardPresent[ant]);
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER0_VOLTAGE_CALIB_F", &dummyFloat);
	if (fabs(lastVbias[ant] - dummyFloat) >= 0.03 
	    && lastVbias[ant] != INITIAL_LAST_VALUE) {
	  standout();
	} 
	if (doWeCare[ant]==1 && isReceiverInArray[1]==1 &&
            flags->mixerVoltage[ant] == 1) {
	  standout();
	}
#define MIXER_BOARD_WACKO_VOLTAGE 999.9
	if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
            dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
	} else {
    	  printw(" %7.2f ", dummyFloat);
	}
	lastVbias[ant] = dummyFloat;
        standend();
      } else {
	printw(" no board"); 
      }
    }



  if (DEBUG) refresh();
  move(++line,0);
  printw("SIS-0 I ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER0_CURRENT_CALIB_F", &dummyFloat);
	mixerCurrent[ant] = dummyFloat;
	if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
            dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
	} else {
          if (doWeCare[ant]==1 && isReceiverInArray[1]==1 &&
	      flags->mixerCurrent[ant] == 1) {
            standout();
          }
  	  printw(" %7.2f ", dummyFloat);
          standend();
	}
      } else {
        printw(" no board"); 
      }
#if 0
      /* this is for when there is no mixer control board, which will
	 hopefully never happen again */
      if (ant==7) {
        rms=rm_read(ant, "RM_TOTAL_POWER_VOLTS2_D", &dummyDouble);
        if (rms!= RM_SUCCESS) {
          rm_error_message(rms,"total_power_volts2");
        }
        dummyFloat=(float)dummyDouble;
        if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
          dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
        } else {
          printw(" %7.2f ", dummyFloat);
	}
      }
#endif
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("SIS-0 B ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER0_BFIELD_CALIB_F", &dummyFloat);
	if (fabs(lastBfield[ant] - dummyFloat ) > 0.2
	    && lastBfield[ant] != INITIAL_LAST_VALUE) {
	  standout();
	} 
    if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
          dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
        } else {
        	if (doWeCare[ant]==1 && isReceiverInArray[1]==1 && flags->mixerBfield[ant] == 1) {
            	standout();
 	  		}
  	  	printw(" %+7.1f ", dummyFloat);
	}
	lastBfield[ant] = dummyFloat;
	standend();
      } else {
	printw(" no board");
      }
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("SIS-0 P ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]){
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER0_POWER_F", &dummyFloat);
        if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
	    dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
        } else {
 	  printw(" %+7.3f ", dummyFloat);
	}
      } else {
	printw(" no board");
      }
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("Last I/V");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (antsAvailable[ant]) { 
      printw("  ");
      rms = rm_read(ant,"RM_TUNE_LOWFREQ_JOYSTICK_S", &joystick);
      if (joystick == 1) {
	standout();
	printw("  joy  ");
        standend();
      } else if (doWeCare[ant]==1) {
	printAgeStandoutN(unixTime,ivTime[ant],(int)4);
      } else {
	printAgeNoStandout(unixTime,ivTime[ant]);
      }
    } else {
      printw("  ----- ");
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("Atten   ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]){
      printw("  -----  ");
    } else {
      if ((motors[ant][actRx[ant]][8] == 1) &&
	  (motors[ant][actRx[ant]][2] >= 0)) {
	atten[ant] = motors[ant][actRx[ant]][2];
	if (lastatten[ant] != atten[ant] && lastatten[ant] != INITIAL_LAST_VALUE) {
	  standout();
	} 
	if (fabs(atten[ant]) > 999999) {
	  printw("  wacko  ");
	} else {
	  printw("  %6d ", atten[ant]);
	}
	lastatten[ant] = atten[ant];
	standend();
      } else {
	atten[ant] = 0;
	printw("  manual ");
      }
    }
  }

#define MIXER_PROTECTION_CURRENT 150
  bFieldLine = 16;
  /* overwrite the B-field value if mixer is protected */
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (antsAvailable[ant] && mixerBoardPresent[ant]) {
      if (mixerCurrent[ant] > MIXER_PROTECTION_CURRENT && 
          (gunnBias[ant]<7 || atten[ant]>20000)) {
        move(bFieldLine,(ant-1)*9+8);
	standout(); printw("Protected"); standend();
      }
    }
  }

#if 0
  if (DEBUG) refresh();
  move(19,0);
  if (DEBUG) {
    refresh();
  }
  /*  printw("IFLO_TP ");*/
  printw("PatchCh4");
  for (ant = 1; ant <= numberAntennas; ant++) {
    /*    rms = rm_read(ant, "RM_IF_LO_TOTAL_POWER_D", &dummyDouble);*/
    rms = rm_read(ant, "RM_TOTAL_POWER_VOLTS_D", &dummyDouble);
    if (!antsAvailable[ant]){
      printw("  -----  ");
    } else {
      if (fabs(dummyDouble) < 100.0) {
        if (dummyDouble > 11.0) {
	  standout();
	}
	if (fabs(dummyDouble) < 10) {
	  if (dummyDouble > 0) {
            printw(" ");
	  }
	}
	if (dummyDouble <= -10) {
	  printw("%7.1f ", dummyDouble*1000.0);
	} else {
	  printw("%7.2f ", dummyDouble*1000.0);
	}
        if (dummyDouble > 11.0) {
	  standend();
	}
      } else {
	printw("   Wacko ");
      }
    }
  }
#endif
  if (DEBUG) refresh();
  move(++line,0);
  printw("IFCnt1D1");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT1_DET1_F", &dummyFloat);
      if (fabs(dummyFloat) < 100.0) {
        if (dummyFloat > 11.0) {
	  standout();
	}
	if (fabs(dummyFloat) < 10) {
	  if (dummyFloat > 0) {
  	    printw(" ");
	  }
	}
	printw("%7.2f ", dummyFloat*1000.0);
        if (dummyFloat > 11.0) {
	  standend();
	}
      } else {
	printw("   Wacko ");
      }
    }
  }
  if (DEBUG) refresh();
  move(++line,0);
  printw("IFCnt1uW");
  for (ant = 1; ant <= numberAntennas; ant++) {
    short style;
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT1_DET1_POWER_MUWATT_F", &dummyFloat);
      if ((dummyFloat > 0.0) && (dummyFloat < 100000.0)) {
        if (dummyFloat < MIN_IFPOWER_MUWATT) {
	  standout();
	}
	printw(" %7.3f", dummyFloat);
	standend();
	rms = rm_read(ant, "RM_HOTLOAD_STYLE_S",&style);
	if (style == ORIGINAL_HOTLOAD_STYLE) {
	  (void)rm_read(ant,"RM_CALIBRATION_WHEEL_S",&dummyShort);
	  if (dummyShort == AMBIENT_IN) {
	    standout();
	    printw("A");
	    standend();
	  } else {
	    printw(" ");
	  }
	} else {
	  printLinearLoadStatusChar(ant);
	}
      } else {
	printw("   Wacko ");
      }
    }
  }
#if 0
  printw("TotPower");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_TOTAL_POWER_VOLTS_D", &dummyDouble);
      if ((dummyDouble > 0.0) && (dummyDouble < 100000.0)) {
        if (dummyDouble > 5.0) {
	  standout();
	}
	printw(" %7.2f ", dummyDouble*1000.0);
        if (dummyDouble > 5.0) {
	  standend();
	}
      } else {
	printw("   Wacko ");
      }
    }
  }
#endif

  move(++line,0);

  printw("Trx (K) ");
  for (i=1; i<=8; i++) {
    double trx;
    time_t timestamp;
    short style;
    rms = rm_read(i, "RM_HOTLOAD_STYLE_S",&style);
    if (style == NEW_HOTLOAD_STYLE) {
      rms = rm_read(i, "RM_TRX_D", &trx);
      rms = rm_read(i, "RM_HEATEDLOAD_TIMESTAMP_L", &timestamp);
#define WACKO_TRX 1000000
      if (abs(timestamp-unixTime) > HEATEDLOAD_STALE_HOURS*3600) {
	printw("  stale ");
      } else {
	if (fabs(trx) >= WACKO_TRX) {
	  printw("  wacko ");
	} else {
	  printw("%6.0f",trx);
	  if (i<8) {
	    printw("   ");
	  }
	}
      }
    } else {
      printw("   n/a   ");
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("Tsys (K)");

  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  ");
      printw("-----  ");
    } else {
      rms = rm_read(ant, "RM_OPTICS_BOARD_PRESENT_S",
		    &opticsBoardPresent[ant]);
      if (opticsBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SYNCDET2_TIMESTAMP_L",&syncdet2timestamp);
	if (syncdet2timestamp < (unixTime-300)) {
	  standout();
          printw("sync2st");
	  standend();
	  printw("   ");
	} else {
	  printw("  ");
	  rms = rm_read(ant, "RM_TSYS_D", &dummyDouble);
	  if ((dummyDouble >= 0.0) && (dummyDouble < 100000.0)) {
	    printw("%5.0f", dummyDouble);
	    printw("  ");
	  } else {
	    printw("Wacko  ");
	  }
	}
      } else {
	printw("no board  ");
      }
    }
  }
  move(24,0);
  refresh();
}

short slaveToRxNumber(short slave) {
  switch (slave) {
  case 0: return(1);
  case 1: return(3);
  case 2: return(0);
  case 3: return(2);
  case 4: return(4);
  case 5: return(5);
  case 6: return(6);
  case 7: return(7);
  default: return(-1);
  }
}

void printGunnType(short type) {
  switch (type) {
  case GUNN_TYPE_CARLSTROM:
    printw("  ");
    break;
  case GUNN_TYPE_RPG:
    printw(" Z");
    break;
  case GUNN_TYPE_WISEWAVE:
    printw(" W");
    break;
  default:
  case GUNN_TYPE_UNKNOWN:
    printw("  ");
    break;
  }
}

int findReceiverNumberLow(int ant) {
  char dummyString1[11];
  int rms;
  int actRx;
  rms = rm_read(ant, "RM_ACTIVE_LOW_RECEIVER_C10", &dummyString1);
  if (dummyString1[1] == (char)0) {
    dummyString1[1] = ' ';
  }
  dummyString1[2] = (char)0;
  actRx = parseReceiverName(dummyString1);
  return(actRx);
}
