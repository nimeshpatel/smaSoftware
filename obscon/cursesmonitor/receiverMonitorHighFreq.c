#define DEBUG 0
#define MIXER_BOARD_WACKO_VOLTAGE 999.9
#define INITIAL_LAST_VALUE -1
#include <curses.h>
#include <math.h>
#ifdef LINUX
#include <bits/nan.h>
#define _NAN NAN
#endif
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "rm.h"
#include "dsm.h"
#include "commonLib.h"
#include "tune6status.h"
#include "monitor.h"
#include "upspage.h"
#include "optics.h"
#include "opticsMonitor.h"
#include "receiverMonitor.h"
#include "receiverMonitorHighFreq.h"
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *, char *);
extern void noBoard(int); 
extern void checkStatus(int,char *);
extern void printGunnType(short type);
extern dsm_structure mRGControl;
extern int findReceiverNumberHigh(int ant, char *dummyString1);

#define MIXER_PROTECTION_CURRENT 400
#define MIXER_PROTECTION_CURRENT_ASIAA 250

float mixerProtectionCurrent[MAX_NUMBER_ANTENNAS+1] = {
  0.0,
  MIXER_PROTECTION_CURRENT, 
  MIXER_PROTECTION_CURRENT, 
  MIXER_PROTECTION_CURRENT, 
  MIXER_PROTECTION_CURRENT, 
  MIXER_PROTECTION_CURRENT, 
  MIXER_PROTECTION_CURRENT, 
  MIXER_PROTECTION_CURRENT_ASIAA, 
  MIXER_PROTECTION_CURRENT_ASIAA,
  0.0,0.0
};

void receiverMonitorHighFreq(int count, int *antlist, int pageMode, RECEIVER_FLAGS *flags) {
  int ant, rms, dummyInt, motors[MAX_NUMBER_ANTENNAS+1][8][9], antennaNumber;
  double restFrequency[2];
  double dummyDouble, lastGunnFreq[8];
  float dummyFloat;
  short shortarray[MAX_NUMBER_ANTENNAS+1][8];
  short dummyShort, actPLL[MAX_NUMBER_ANTENNAS+1];
  short actRx[MAX_NUMBER_ANTENNAS+1], mixerBoardPresent[MAX_NUMBER_ANTENNAS+1];
  short opticsBoardPresent[MAX_NUMBER_ANTENNAS+1];
  char dummyString1[100];
  time_t timestamp, ivTime[MAX_NUMBER_ANTENNAS+1];
  int line;
  short mRGLocked[2];
  float gunnBias[MAX_NUMBER_ANTENNAS+1];
  int atten[MAX_NUMBER_ANTENNAS+1];
  static int lastatten[MAX_NUMBER_ANTENNAS+1] = {INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
			     INITIAL_LAST_VALUE,
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
						  INITIAL_LAST_VALUE,
						  INITIAL_LAST_VALUE,
						  INITIAL_LAST_VALUE};
  short gunnType[8]; /* 8 is for the number of receivers per dewar */
  float mixerCurrent[MAX_NUMBER_ANTENNAS+1];
  long longvalue, timevalue;
  int unixTime;
  long syncdet2timestamp;
  int doWeCare[11], i;
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  short combinerMirrorMotorized[MAX_NUMBER_ANTENNAS+1];
  short busy;
  int isReceiverInArray[3];
  short joystick;

  /* first determine if the mixer current is in a nominal range */
  getAntennaList(doWeCare);
  getReceiverList(isReceiverInArray);
  for (ant = 1; ant <= numberAntennas; ant++) {
    flags->ifpower[ant] = 0;
    flags->ivsweepAge[ant] = 0;
    actRx[ant] = findReceiverNumberHigh(ant,dummyString1);
    if (antsAvailable[ant]) {
      rms = rm_read(ant, "RM_MIXER_BOARD_PRESENT_S", &mixerBoardPresent[ant]);
      if (mixerBoardPresent[ant]==1) {
	rms = rm_read(ant, "RM_SIS_MIXER1_CURRENT_CALIB_F", &dummyFloat);
	if (dummyFloat < sis1MixerCurrentMin[ant][actRx[ant]-4] || 
	    dummyFloat > sis1MixerCurrentMax[ant][actRx[ant]-4]) {
	  flags->mixerCurrent[ant] = 1;
	} else {
	  flags->mixerCurrent[ant] = 0;
	}
        rms = rm_read(ant, "RM_SIS_MIXER1_VOLTAGE_CALIB_F", &dummyFloat);
	if (dummyFloat < sis1MixerVoltageMin[ant][actRx[ant]-4] || 
	    dummyFloat > sis1MixerVoltageMax[ant][actRx[ant]-4]) {
	  flags->mixerVoltage[ant] = 1;
        } else {
	  flags->mixerVoltage[ant] = 0;
        }
	rms = rm_read(ant, "RM_CONT2_DET1_POWER_MUWATT_F", &dummyFloat);
	if (dummyFloat < MIN_IFPOWER_MUWATT) {
	  flags->ifpower[ant] = 1;
	}
	rms = rm_read(ant, "RM_HIGHFREQ_IVSWEEP_TIMESTAMP_L",&ivTime[ant]);
	if (unixTime-ivTime[ant] > IVSWEEP_HOURS_HIGHLIGHT*3600) {
	  flags->ivsweepAge[ant] = 1;
	}
        /* do a logical 'or' on all the possible warning values */
        flags->flags[ant] = flags->mixerCurrent[ant];
	flags->flags[ant] |= flags->mixerVoltage[ant];
	flags->flags[ant] |= flags->ivsweepAge[ant];
	flags->flags[ant] |= flags->ifpower[ant];
        if (doWeCare[ant] == 0) {
	  flags->flags[ant] = 0;
	}
        standend();
      }/* endif mixer board present */
    } /* endif antenna available */
  } /* end of 'for' loop */

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
  if (mRGLocked[1]) {
    /*
      printw("High-freq receiver: %s UT = %s HST",timeString,timeString2);
    */
    printw("HighFreqRx  1");
  } else {
    printw("High-freq ");
    standout();
    printw("MRG Unlocked");
    standend();
    printw(" %s UT =%s HST",timeString,timeString2);
  }
  if (DEBUG) refresh();
  /*
  move(1,0);
  printw("Antenna     1");
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
  if (DEBUG) refresh();
  line = 0;
  line++;
  move(line,0);
  printw("ActRx/PLL");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      actRx[ant] = findReceiverNumberHigh(ant,dummyString1);
      if (actRx[ant] == -1) {
        printw("  wacko  ");
      } else {
        printw(" %s", dummyString1);
	if (actRx[ant] > 3) {
	  printw(" ");
	}
	printw("/ ");
	rms = rm_read(ant, "RM_SLAVE2_PLL_NO_S", &actPLL[ant]);
	rms = rm_read(ant, "RM_RX_GUNN_TYPE_V8_S", gunnType);
	if ((actPLL[ant] < 0) || (actPLL[ant] >= 8)) {
	  printw("?   ");
	} else {
	  printw("%1d", actPLL[ant]);
#if 0
	  printGunnType(gunnType[actRx[ant]]);
#else
	  printw("  ");
#endif
	  printw(" ");
	}
      }
    }
  }
  if (DEBUG) {
    refresh();
  }
  move(++line,0);
  printw("YIGSwitch");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw(" -----   ");
    } else {
      short hLocked, hRx;
      int delay;

      rms = rm_read(ant, "RM_UNIX_TIME_L", &unixTime);
      rms = rm_read(ant, "RM_YIG_SVC_TIMESTAMP_L", &dummyInt);
      delay = unixTime - dummyInt;
      rms = rm_read(ant, "RM_YIG2_LOCKED_S", &hLocked);
      rms = rm_read(ant, "RM_YIG_SWITCH_HIGH_FREQ_S", &hRx);
      printw(" ");
      switch (hRx) {
      case 4:
	printw("C ");
	break;
      case 5:
	printw("D ");
	break;
      case 6:
	printw("E ");
	break;
      case 7:
	printw("F ");
	break;
      default:
	printw("??");
      }
      if (delay < 30) {
	if (hLocked == 0) {
          if (actRx[ant] != -1 && doWeCare[ant] == 1) {
	    standout();
	  }
	  printw(" Unlk");
	  standend();
	} else if (hLocked == 1) {
	  printw(" Lock");
	} else {
          if (actRx[ant] != -1 && doWeCare[ant]==1) {
  	    standout();
	  }
	  printw("wacko");
	  standend();
	}
      } else {
        if (actRx[ant] != -1 && doWeCare[ant]==1) {
          standout();
	}
	printw("stale");
	standend();
      }
      printw(" ");
    }
  }
  if (DEBUG) {
    refresh();
  }
  move(++line,0);
  printw("MirrorPos");
  for (antennaNumber=1; antennaNumber<=numberAntennas; antennaNumber++) { 
    rms = rm_read(antennaNumber, "RM_OPTICS_BOARD_PRESENT_S",
		    &opticsBoardPresent[antennaNumber]);
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      if (opticsBoardPresent[antennaNumber] == 0) {
	noBoard(antennaNumber);
	if (antennaNumber < 7) {
	  printw(" ");
	}
	continue;
      }
      rms = rm_read(antennaNumber, "RM_COMBINER_MIRROR_MOTORIZED_S", 
		    &combinerMirrorMotorized[antennaNumber]);
      checkStatus(rms,"rm_read(RM_COMBINER_MIRROR_MOTORIZED_S)");
      if (combinerMirrorMotorized[antennaNumber]==0) {
	printw("  E=%3d  ",translateBandNumberToGHz(6));
      } else {
	rms = rm_read(antennaNumber,"RM_COMBINER_MIRROR_RECEIVER_LOCATION_L", &longvalue);
	checkStatus(rms,"rm_read(RM_COMBINER_MIRROR_RECEIVER_LOCATION_L)");
	rms = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
	if (timevalue < unixTime-GRID_STALE_INTERVAL) {
	  if (doWeCare[antennaNumber]==1) {
	    standout();
	  }
	  printw("   stale ");
	  if (doWeCare[antennaNumber]==1) {
	    standend();
	  }
	} else {
	  rms = rm_read(antennaNumber, "RM_TUNE6_COMMAND_BUSY_S", &busy);
	  switch (busy) {
	  case TUNE6_BUSY_WITH_COMBINER_MIRROR:
	    standout();
	    printw(" turning ");
	    standend();
	    break;
	  default:
	    /* display the high-freq receiver illuminated by the grid */
	    printw("  %2s=%3d ",getLoBoardTypeStringBrief(longvalue),
		   translateBandNumberToGHz(longvalue));
	    break;
	  }
	}
      }
    }
  }

  if (DEBUG) {
    refresh();
  }
  move(++line,0);
  printw("LastFreq ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw(" -----  ");
    } else {
      rms = rm_read(ant, "RM_MOTORIZED_LO_CHAIN_V8_V9_L", motors[ant]);
      rms = rm_read(ant, "RM_LAST_GUNN_FREQ_V8_D", lastGunnFreq);
      if (!(motors[ant][actRx[ant]][8] == 1)) {
	printw(" manual  ");
      } else if ((lastGunnFreq[actRx[ant]] > 50.0) &&
		 (lastGunnFreq[actRx[ant]] < 1000.0)) {
	printw("%7.2f  ", lastGunnFreq[actRx[ant]]);
      } else {
	printw("  wacko  ");
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", shortarray[ant-1]);
  }
  rms = call_dsm_read("m5", "DSM_AS_IFLO_REST_FR_V2_D", (char *)&restFrequency, &timestamp);
  printw("Gunn/PLL ");
  if (DEBUG) {
    refresh();
  }
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw(" -----   ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
	printw("no board ");
      } else {
        rms = rm_read(ant, "RM_GUNN2_ON_S", &dummyShort);
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
        rms = rm_read(ant, "RM_PLL2_ON_S", &dummyShort);
        switch (dummyShort) {
        case -1:
	  if (doWeCare[ant]==1 && restFrequency[1] > HI_LO_FREQ_CUTOFF_HZ) {
	    standout();
	  }
	  printw("LSB");
	  standend();
	  printw("  ");
          break;
        case 0:
	  if (doWeCare[ant]==1 && restFrequency[1] > HI_LO_FREQ_CUTOFF_HZ) {
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
	  if (doWeCare[ant]==1 && restFrequency[1] > HI_LO_FREQ_CUTOFF_HZ) {
	    standout();
	  }
	  printw("---  ");
	  standend();
	}
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("GunnLock");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
      } else {
        int delay;

        rms = rm_read(ant, "RM_GUNN2_LOCKED_S", &dummyShort);
        rms = rm_read(ant, "RM_UNIX_TIME_L", &unixTime);
        rms = rm_read(ant, "RM_GUNN2_LOCKED_TIMESTAMP_L", &dummyInt);
        rms = rm_read(ant, "RM_TUNE6_COMMAND_BUSY_S", &busy);
        delay = unixTime - dummyInt;
        if ((dummyShort != 0) && (dummyShort != 1)) {
	  dummyShort = 0;
        }
        if (dummyShort == 0 && doWeCare[ant]==1 && restFrequency[1] > HI_LO_FREQ_CUTOFF_HZ) {
	  standout();
        }
        if (delay <= 90) {
	  switch (busy) {
          case TUNE6_BUSY_WITH_TUNE_GUNN_HIGH:
    	    printw(" tuning  ");
	    break;
          case TUNE6_BUSY_WITH_OPT_LO_POWER_HIGH:
    	    printw(" LO adj ");
	    break;
  	  default:
            if (delay > 5) {
	      printw(" %1d stale ", dummyShort);
  	    } else {
    	      printw("    %1d    ", dummyShort);
	    }
	    break;
	  } /* end switch */
        } else {
          printw(" %1d stale ", dummyShort);
        }
	standend();
      }
    } /* end if antennaAvailable */
  } /* end for */
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("Stress V");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
      } else {
        float targetBias[NUMBER_OF_RECEIVERS]; /* one for each receiver in this antenna */
        float stress;
        rms = rm_read(ant, "RM_GUNN_TARGET_BIAS_V8_F", targetBias);
        rms = rm_read(ant, "RM_GUNN2_BIAS_F", &gunnBias[ant]);
        if (actRx[ant] >= 0) {
          stress = gunnBias[ant]-targetBias[actRx[ant]];
#define GUNN_DROPOUT_IMMINENT 0.45
          if (doWeCare[ant]==1 && fabs(stress) > GUNN_DROPOUT_IMMINENT
	      && restFrequency[1] > HI_LO_FREQ_CUTOFF_HZ) {
	    rms = rm_read(ant, "RM_PLL2_ON_S", &dummyShort);
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
          printw(" wacko  ");
        }
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("Gunn V  ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
      } else {
        if ((gunnBias[ant] > 1.0e2) || (gunnBias[ant] < -1.0e2)) {
	  gunnBias[ant] = _NAN;
        }
        printw(" %7.4f ", gunnBias[ant]);
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("IF Power");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
       } else {
        float iFPower;
        rms = rm_read(ant, "RM_GUNN2_PLL_IFPOWER_F", &iFPower);
        if ((iFPower > 1.0e6) || (iFPower < -1.0e6)) {
	  iFPower = _NAN;
	}
	/*
	if (iFPower >= LO_BOARD_ADC_SATURATED) {
	  standout();
	}
	*/
        printw(" %7.4f ", iFPower);
	standend();
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("Nz Power");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
      } else {
        float noisePower;
        rms = rm_read(ant, "RM_GUNN2_PLL_PHASENOISE_F", &noisePower);
        if ((noisePower > 10.0) || (noisePower < -10.0)) {
	  noisePower = _NAN;
	}
        printw(" %7.4f ", noisePower);
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("PLLRatio");
  for (ant=1; ant <= numberAntennas; ant++) {

    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else     if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
      } else {
        float phaseNoise, iFPower, ratio;
        rms = rm_read(ant, "RM_GUNN2_PLL_IFPOWER_F", &iFPower);
        rms = rm_read(ant, "RM_GUNN2_PLL_PHASENOISE_F", &phaseNoise);
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
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("LoopGain");
  for (ant=1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (actPLL[ant] < 0) {
        printw(" rm off  ");
      } else if (actPLL[ant]>7) {
	printw(" wacko   ");
      } else if (shortarray[ant-1][actPLL[ant]] < 0) {
        printw(" no board");
      } else {
        rm_read(ant, "RM_PLL2_GAIN_S", &dummyShort);
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

  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("RFReqAch ");
  for (ant=1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_TUNE6_COMMAND_BUSY_S", &busy);
      switch (busy) {
      case TUNE6_BUSY_WITH_IV_SWEEP_HIGH_FREQ:
	standout();
	printw(" IVcurve ");
	standend();
	break;
      case TUNE6_BUSY_WITH_PB_SWEEP_HIGH_FREQ:
	standout();
	printw(" PBcurve ");
	standend();
	break;
      default:
#define WACKO_YIG_POWER_MIN -150 /* dBm */
#define WACKO_YIG_POWER_MAX  25 /* dBm */
#define DOPPLER_TRACK_YIG_POWER  20 /* dBm */
       rm_read(ant, "RM_GUNN2_REQUESTED_RFPOWER_DBM_F", &dummyFloat);
       if (dummyFloat< WACKO_YIG_POWER_MIN || dummyFloat>WACKO_YIG_POWER_MAX) {
         printw("  wacko  ");
       } else {
        if (dummyFloat >= DOPPLER_TRACK_YIG_POWER) {
          printw(" max/");
#define RFPOWER_CLIENT_CREATE_FAILED -100
	} else if (dummyFloat <= RFPOWER_CLIENT_CREATE_FAILED) {
          printw("ClntFail");
	  continue;
        } else {
	  if (lastrfpower[ant] != dummyFloat &&
	      lastrfpower[ant] != INITIAL_LAST_VALUE) {
	    standout();
	  } 
          printw("%4.1f/",dummyFloat);
	  standend();
	  lastrfpower[ant] = dummyFloat;
	}
        rm_read(ant, "RM_GUNN2_ACHIEVED_RFPOWER_DBM_F", &dummyFloat);
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

  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("SIS-1 V ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms = rm_read(ant, "RM_MIXER_BOARD_PRESENT_S", &mixerBoardPresent[ant]);
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER1_VOLTAGE_CALIB_F", &dummyFloat);
	if (doWeCare[ant]==1 && isReceiverInArray[1]==1 
            && flags->mixerVoltage[ant] == 1
           ) {
	  standout();
	}
	if (fabs(lastVbias[ant] - dummyFloat) >= 0.01 
	    && lastVbias[ant] != INITIAL_LAST_VALUE) {
	  standout();
	} 
	lastVbias[ant] = dummyFloat;
        if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
            dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
        } else {
	  printw(" %7.2f ", dummyFloat);
	}
        standend();
      } else {
	printw(" no board");
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("SIS-1 I ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
        rms = rm_read(ant, "RM_SIS_MIXER1_CURRENT_CALIB_F", &dummyFloat);
        mixerCurrent[ant] = dummyFloat;
        if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
            dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw("   wacko ");
        } else {
 	  if (doWeCare[ant]==1 && isReceiverInArray[1]==1
 	      && flags->mixerCurrent[ant] == 1
              ) {
	    standout();
	  }
	  printw(" %7.2f ", dummyFloat);
          standend();
	}
      } else {
	printw(" no board");
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("SIS-1 B ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER1_BFIELD_CALIB_F", &dummyFloat);
	if (fabs(lastBfield[ant] - dummyFloat ) > 0.2
	    && lastBfield[ant] != INITIAL_LAST_VALUE) {
	  standout();
	} 
        if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
          dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
        } else {
	  printw(" %+7.1f ", dummyFloat);
	}
	lastBfield[ant] = dummyFloat;
	standend();
      } else {
	printw(" no board");
      }
    }
  }

  /* overwrite the magnetic field bias */
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms = rm_read(ant, "RM_MIXER_BOARD_PRESENT_S", &mixerBoardPresent[ant]);
    if (antsAvailable[ant] && mixerBoardPresent[ant]) {
      if (mixerCurrent[ant] > mixerProtectionCurrent[ant] && 
          (gunnBias[ant]<7 || atten[ant]>20000)) {
        move(line,(ant-1)*9+8);
	standout(); printw("Protected"); standend();
      }
    }
  }

  if (DEBUG) refresh();
  move(++line,0);
  printw("SIS-1 P ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]){
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER1_POWER_F", &dummyFloat);
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
      rms = rm_read(ant,"RM_TUNE_HIGHFREQ_JOYSTICK_S", &joystick);
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

  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("Atten   ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]){
      printw("  -----  ");
    } else {
      if ((motors[ant][actRx[ant]][8] == 1)
	  /*
	  &&
	  (motors[ant][actRx[ant]][2] >= 0)
	  */
	  ) {
	if (lastatten[ant] != motors[ant][actRx[ant]][2] &&
	    lastatten[ant] != INITIAL_LAST_VALUE) {
	  standout();
	} 
	if (fabs(motors[ant][actRx[ant]][2]) > 999999) {
	  printw("  wacko  "); 
	} else {
	  printw("  %6d ", motors[ant][actRx[ant]][2]);
	}
	lastatten[ant] = motors[ant][actRx[ant]][2];
	standend();
      } else {
	printw("  manual ");
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("IFCnt2D1");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT2_DET1_F", &dummyFloat);
      dummyFloat *= 1000;
      if ((dummyFloat > 0.0) && (dummyFloat < 100000.0)) {
        if (dummyFloat < 0.01 || dummyFloat > 9990.0) {
	  standout();
	}
	printw(" %7.2f", dummyFloat);
	standend();
	if (ant < numberAntennas) {
	  printw(" ");
	}
      } else {
	printw("   Wacko ");
      }
    }
  }
  move(++line,0);
  if (DEBUG) {
    refresh();
  }
  printw("IFCnt2uW");
  for (ant = 1; ant <= numberAntennas; ant++) {
    short style;
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT2_DET1_MUWATT_F", &dummyFloat);
      if (fabs(dummyFloat) < 100.0) {
        if (dummyFloat > 11.0 || dummyFloat < MIN_IFPOWER_MUWATT) {
	  standout();
	}
	if (fabs(dummyFloat) < 10) {
	  if (dummyFloat > 0) {
            printw(" ");
	  }
	}
	printw("%7.2f", dummyFloat);
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
  move(++line,0);

  printw("Trx (K) ");
  for (i=1; i<=8; i++) {
    double trx;
    time_t timestamp;
    short style;
    rms = rm_read(i, "RM_HOTLOAD_STYLE_S",&style);
    if (style == NEW_HOTLOAD_STYLE) {
      rms = rm_read(i, "RM_HEATEDLOAD_TIMESTAMP_L", &timestamp);
      rms = rm_read(i, "RM_TRX2_D", &trx);
#define WACKO_TRX 1000000
      if (abs(timestamp-unixTime) > HEATEDLOAD_STALE_HOURS*3600) {
	printw("  stale ");
      } else {
	if (fabs(trx) >= WACKO_TRX) {
	  printw(" wacko   ");
	} else {
	  printw(" %6.0f  ",trx);
	}
      }
    } else {
      printw("   n/a   ");
    }
  }

  move(++line,0);
  if (DEBUG) {
    refresh();
  }
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
	  printw("  ");
	  printw(" ");
	} else {
	  printw("  ");
	  rms = rm_read(ant, "RM_TSYS2_D", &dummyDouble);
	  if ((dummyDouble >= 0.0) && (dummyDouble < 100000.0)) {
	    printw("%5.0f  ", dummyDouble);
	  } else {
	    printw("Wacko  ");
	  }
	}
      } else {
	printw("no board");
	printw("  ");
      }
    }
  }
  move(24,0);
  refresh();
}

int findReceiverNumberHigh(int ant, char *dummyString1) {
  int rms;
  int actRx;

  rms = rm_read(ant, "RM_ACTIVE_HIGH_RECEIVER_C10", dummyString1);
  /*
  if (dummyString1[1] == (char)0) {
    dummyString1[1] = ' ';
  }
  */
  dummyString1[2] = (char)0;
  actRx = parseReceiverName(dummyString1);
  return(actRx);
}

