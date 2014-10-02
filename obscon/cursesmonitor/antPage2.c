#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "s_cmd2.h"
#include "tsshm.h"
#include "rm.h"
#include "monitor.h"

#define CHECKRMERRORS 0

extern char *toddtime(time_t *, char *); /* located in dewarpage.c */
extern char *hsttime(time_t *, char *); /* located in dewarpage.c */
#ifndef LINUX
extern int mvprintw    _AP((int, int, const char *fmt, ...));
#endif
void parseFaults(unsigned long fault, int antNo, float encAz, float limAz, float encEl, float limEl);

struct FAULTS {
	int bit;
	char *msg;
} faults[] = {
    {1 << ELEVATION_UPPER_PRELIM_FAULT, "Elevation upper prelimit"},
    {1 << ELEVATION_LOWER_PRELIM_FAULT, "Elevation lower prelimit"},
    {1 << ELEVATION_UPPER_LIMIT_FAULT, "Elevation upper limit switch"},
    {1 << ELEVATION_LOWER_LIMIT_FAULT, "Elevation lower limit switch"},
    {1 << ELEVATION_ENCODER_FAULT, "Elevation low-res encoder out-of-bounds"},
    {1 << ELEVATION_TEMPERATURE_FAULT, "Elevation motor overtemperature fault"},
    {1 << ELEVATION_CURRENT_FAULT, "Elevation motor overcurrent fault"},
    {1 << ELEVATION_GLENTEK_FAULT, "Elevation Glentek fault"},
    {1 << ELEVATION_OVERFLOW_FAULT, "Elevation overflow fault"},
    {1 << AZIMUTH_CLOCKWISE_PRELIM_FAULT, "Azimuth clockwise prelimit"},
    {1 << AZIMUTH_COUNTERCLOCKWISE_PRELIM_FAULT, "Azimuth counterclockwise prelimit"},
    {1 << AZIMUTH_CLOCKWISE_LIMIT_FAULT, "Azimuth clockwise limit switch"},
    {1 << AZIMUTH_COUNTERCLOCKWISE_LIMIT_FAULT, "Azimuth counterclockwise limit switch"},
    {1 << AZIMUTH_ENCODER_FAULT, "Azimuth low-res encoder out-of-bounds"},
    {1 << AZIMUTH1_TEMPERATURE_FAULT, "Azimuth motor #1 overtemperature"},
    {1 << AZIMUTH2_TEMPERATURE_FAULT, "Azimuth motor #2 overtemperature"},
    {1 << AZIMUTH1_CURRENT_FAULT, "Azimuth motor #1 overcurrent"},
    {1 << AZIMUTH2_CURRENT_FAULT, "Azimuth motor #2 overcurrent"},
    {1 << AZIMUTH1_GLENTEK_FAULT, "Azimuth motor #1 Glentek fault"},
    {1 << AZIMUTH2_GLENTEK_FAULT, "Azimuth motor #2 Glentek fault"},
    {1 << AZIMUTH_OVERFLOW_FAULT, "Azimuth overflow fault"},
    {1 << EMERGENCY_STOP_FAULT, "Emergency stop pressed"},
    {1 << QUADADC_RESET_FAULT, "Quad ADC reset fault"},
    {1 << COOLANT_FLOW_FAULT, "Coolant flow fault"},
    {1 << HANDPADDLE_BYTE_FRAMING_FAULT, "Hand paddle byte framing fault"},
    {1 << PALMPILOT_SYNTAX_FAULT, "Hand paddle syntax error fault"},
    {1 << PALMPILOT_NO_RESPONSE_FAULT, "Hand paddle no response fault"},
    {1 << ANTENNA_COMPUTER_TIMEOUT_FAULT, "Acc velocity packet timeout fault"},
    {1 << ELEVATION_COLLISION_LIMIT_FAULT, "el too low on a collision pad"},
    {1 << AZIMUTH_ROCKER_FAULT, "Azimuth rocker deployment fault"},
    {1 << AIR_PRESSURE_SWITCH_FAULT, "Air pressure failed on azimuth brake"},
    {1 << ENC_TACH_DIFF_FAULT, "Integrated Tach not following limit encoder"}
};
#define NFAULTS (sizeof(faults) / sizeof(faults[0]))

int lastLine = 0;
int line;
short disableDrivesFlag;
unsigned short bypass;
int msecPrev = 0, msecSameCnt = 0;

char drvStateStrings[][6] = DRVSTATE_STRINGS ;
char m3StateStrings[][8] = M3STATE_STRINGS ;
char rockerStateStrings[][10] = {"Neutral", "CCW", "CW", "Transient"};

char *StateStrings(int state);

void antPage2(int count, int *antlist, int antNo, int antName)
{
  #include "antennaServoRmVars.h"
  RV rv[] = {
#include "antennaServoRvList.h"
  };
#define NUMVARS (sizeof(rv) / sizeof(rv[0]))
  char padIDIsFake;
  int ant, i, rms;
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  char *inLimitString, *controlString;
/*	int tracktimestamp;
  time_t timeStamp; */
  char dummyByte;
  int padid;
  float dummyFloat;
  double glycolFlow = 0.0;
  double  glycolPressure = 0.0;
  double  glycolTemp = 0.0;
  long errorCount;
  extern double radian; /* for sunDistance calculation */
  double sunD;
  unsigned char dummyChar,stowRequest;

#if CHECKRMERRORS
  int rmErrors;
#endif /* CHECKRMERRORS */

  if ((count % 20) == 0) {
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
	  rms=rm_read(antNo,"RM_PAD_ID_B",&dummyByte);
	  if(rms!= RM_SUCCESS) {
	    rm_error_message(rms,"padid");
	    exit(1);
	  }
	  padid = (short)dummyByte;
  if (deadAntennas[antNo]) {
    move(11, 13);
    printw("Antenna %d ",antNo);
    if (antNo == CSO_ANTENNA_NUMBER) {
      printw("(CSO) ");
    }
    if (antNo == JCMT_ANTENNA_NUMBER) {
      printw("(JCMT) ");
    }
    printw("is out of service - no data are available");
    if (padid == HANGAR_PAD_ID) {
      move(13,16);
      printw("(it has the hangar ID connector attached)");
    }
    move(0,0);
    refresh();
    return;
  }


#if CHECKRMERRORS
  rmErrors = 0;
#endif /* CHECKRMERRORS */
  for (i = 0; i < NUMVARS; i++) {
    rms = rm_read(antNo, rv[i].name, rv[i].a );
#if CHECKRMERRORS
    if(rms != RM_SUCCESS)
	rmErrors++;
#endif /* CHECKRMERRORS */
  }

/* For testing the in limit display */
#if 0
  switch((count >> 3) & 3) {
  case 0:
      cmdAz = cwLimit + 1;
      break;
  case 1:
      cmdAz = ccwLimit - 1;
      break;
  case 2:
      cmdEl = upperLimit + 1;
      break;
  case 3:
      cmdEl = lowerLimit - 1;
      break;
  }
#endif

  move(0,0);
  rms=rm_read(antNo,"RM_SCB_STOW_REQUEST_B",&stowRequest);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("Antenna %d Drive Status: %s UT, %s HT",antName,timeString,timeString2);
  move(1,8);
/*	rm_read(antNo,"RM_UNIX_TIME_L",&timeStamp);
	rm_read(antNo,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
*/
        if(abs(tracktimestamp-unixTime)>3L) {
	  standout();
          addstr("Track not running");
	  standend();
          addstr("  |    servo");
	} else {
          addstr("Track              |    servo");
	}
#if 0
	  rms=rm_read(antNo,"RM_SUN_AZ_DEG_F",&dummyFloat);
	  sunaz=(double)dummyFloat;
	  rms=rm_read(antNo,"RM_SUN_EL_DEG_F",&dummyFloat);
	  sunel=(double)dummyFloat;
	  sunaz=sunaz*radian;
	  sunel=sunel*radian;
  sunD=sunDistance(az*radian,el*radian,sunaz,sunel);
#endif
  sunD=sunDistance(az*radian,el*radian,(double)sunAz*radian,
	sunEl*(double)radian);
  if (sunD < SUN_DISTANCE_LIMIT && sunEl > -3) {
    addstr("   ");
    standout();
    addstr("SUN!");
    standend();
  }
  move(1, 50);
  /* Decide if servo is running. */
#if 0
  i = msec/1000 - unixTime % 86400;
  if(i > 86000) {
    i -= 86400;
  } else if(i < -86000) {
    i += 86400;
  }
  if(abs(i) < 19) { /* this used to be 10, but value seems always 11-17 */
    addstr((fault== LOCKOUT)? " Palm in Control\n":((fault == ONE_MOTOR_FAULT)?
		"PowerPC in control one motor": "PowerPC in control\n"));
#else
  /* msec is updated each second by servo, so we should never see the same
   * value more than twice if servo is running independent of the timing
   * of the two programs.
   */
  if(msec == msecPrev) {
    msecSameCnt++;
  } else {
    msecSameCnt = 0;
    msecPrev = msec;
  }
  if(msecSameCnt < 3) {
      switch(fault) {
      case LOCKOUT:
	if (stowRequest==1 && padID>=MIN_ANTENNA_PAD && padID<=MAX_ANTENNA_PAD) {
	  controlString = "Telephone in Control\n";
	} else {
	  controlString = "Palm in Control\n";
	}
	break;
      case ONE_MOTOR_FAULT:
	if (stowRequest==1 && padID>=MIN_ANTENNA_PAD && padID<=MAX_ANTENNA_PAD) {
	  controlString = "Telephone in Control one motor\n";
	} else {
	  controlString = "PowerPC in control one motor\n";
	}
	break;
      case ONE_TACH_FAULT:
	if (stowRequest==1 && padID>=MIN_ANTENNA_PAD && padID<=MAX_ANTENNA_PAD) {
	  controlString = "Telephone in Control one tach\n";
	} else {
	  controlString = "PowerPC in control one tach\n";
	}
	break;
      default:
	if (stowRequest==1 && padID>=MIN_ANTENNA_PAD && padID<=MAX_ANTENNA_PAD) {
	  controlString = "Telephone in Control\n";
	} else {
	  controlString = "PowerPC in control\n";
	}
	break;
      }
      addstr(controlString);
#endif
  } else {
    standout();
    printw("Servo is not running");
    standend();
    msecSameCnt = 3;
  }
  move(2,3);
  addstr("state     pos     vel   | state  commanded  encoder  limenc Following Error");
  move(3,0);
  addstr("Az");
  move(4,0);
  addstr("El");
#if 0
  mvprintw(3, 4, "%4s %9.4f %7.4f | %5s %9.4f %9.4f %8.3f %10.5f\n",
	   (azCmd)? "on ": "off", az, azVel, StateStrings(azDrvState),
	   cmdAz, encAz, limAz, azTrErrorArcSec / 3600.);  
#endif
  mvprintw(3, 4, "%4s ",(azCmd)? "on ": "off");
  if (fabs(az) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%9.4f ",az);
  }
  if (fabs(azVel) >= 100) {
    printw(" wacko  | %5s",StateStrings(azDrvState));
  } else { 
    printw("%7.4f | %5s ",azVel, StateStrings(azDrvState));
  }
  if (fabs(cmdAz) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%9.4f ",cmdAz);
  }
  if (fabs(encAz) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%9.4f ",encAz);
  }
  if (fabs(limAz) >= 1000) {
    printw("  wacko  ");
  } else {
    printw("%8.3f ",limAz);
  }
  if (fabs(azTrErrorArcSec / 3600.) >= 100000) {
    printw("  wacko   \n");
  } else {
    printw("%10.5f\n",azTrErrorArcSec / 3600.);  
  }
  if (az >= cwLimit || az <= ccwLimit) {
    standout();
    move(3, 75);
    addstr("LIMIT");
    standend();
  }
#if 0
  mvprintw(4, 4, "%4s %9.4f %7.4f | %5s %9.4f %9.4f %8.3f %10.5f\n",
	   (elCmd)? "on ": "off", el, elVel, StateStrings(elDrvState), cmdEl,
	   encEl, limEl, elTrErrorArcSec / 3600.);
#endif
  mvprintw(4, 4, "%4s ",(elCmd)? "on ": "off");
  if (fabs(el) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%9.4f ",el);
  }
  if (fabs(elVel) >= 1000) {
    printw(" wacko  | %5s ",elVel, StateStrings(elDrvState));
  } else {
    printw("%7.4f | %5s ",elVel, StateStrings(elDrvState));
  }
  if (fabs(cmdEl) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%9.4f ",cmdEl);
  }
  if (fabs(encEl) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%9.4f ",encEl);
  }
  if (fabs(limEl) >= 1000) {
    printw("  wacko   ");
  } else {
    printw("%8.3f ",limEl);
  }
  if (fabs(elTrErrorArcSec / 3600.) >= 100000) {
    printw("  wacko \n");
  } else {
    printw("%10.5f\n",elTrErrorArcSec / 3600.);
  }
	   
	   
  if(el >= upperLimit || el <= lowerLimit) {
    /* do not show a false LIMIT condition when az=el=0 which often happens
     * after an RM reallocation */
    if (el != 0 || az != 0) {
      standout();
      move(4, 75);
      addstr("LIMIT");
      standend();
    }
  }
  rms = rm_read(antNo,"RM_DRIVES_TIMEOUT_FLAG_S",&disableDrivesFlag);
  if (disableDrivesFlag==1) {
    move(5,0);
    standout();
    printw("Drive timeout disabled");
    standend();
  }
  line = 5;
  mvprintw(line++, 27, "| Tach Vel: Az ");
  if (fabs(tachAzVel) > 10000) {
    printw("  wacko  ");
  } else {
    printw("%7.4f  ",tachAzVel); 
  }
  printw("El ");
  if (fabs(tachElVel) > 10000) {
    printw("  wacko  ");
  } else {
    printw("%7.4f  ",tachElVel);
  }
  printw("Rocker %s\n",rockerStateStrings[azRockerBits & 3]);
#if 0
  mvprintw(line++, 27, "| Tach Vel: Az %7.4f  El %7.4f  Rocker %s\n",
	tachAzVel, tachElVel, rockerStateStrings[azRockerBits & 3]);
#endif

  mvprintw(line++, 0, "msecCmd %8d	   | msecAccept %8d"
	   " msec %8d ttTimeOuts ", msecCmd, msecAccept, msec);
  if (abs(ttTimeoutCount) >= 100000) {
    printw("wacko");
  } else { 
    printw("%4d\n",ttTimeoutCount);
  }
  if (antName < 9) {
    mvprintw(line++, 0, "m3Cmd = %d = %s  m3State = %d = %s  ",
	     m3Cmd, (m3Cmd == CLOSE_M3_CMD)? "Closed":
	     (m3Cmd == OPEN_M3_CMD)? "open":"Unknown", m3State,
	     m3StateStrings[m3State]);
    if (chkElCollisionLimit==1) {
      standout();
    }
    printw("Collision Check: %s",(chkElCollisionLimit)? "ON": "OFF");
    standend();
    printw(" Sun Az=%3.0f El=%-3.0f\n", sunAz, sunEl);
    
    move(line++, 0);
    addstr("Limits: Az(");
    if(az <= ccwLimit) {
      standout();
    }
    printw("%8.3f,",ccwLimit);
    if(az >= cwLimit) {
      standout();
    } else {
      standend();
    }
    printw(" %7.3f",cwLimit);
    standend();
    addstr(") El(");
    
    if(el <= lowerLimit) {
      /* do not show a false LIMIT condition when az=el=0 which often happens
       * after an RM reallocation */
      if (el != 0 || az != 0) {
	standout();
      }
    }
    if (fabs(lowerLimit) > 1000) {
      printw("wacko,");
    } else {
      printw("%5.3f,",lowerLimit);
    }
    if(el >= upperLimit) {
      standout();
    } else {
      standend();
    }
    if (fabs(upperLimit) > 1000) {
      printw(" wacko");
    } else {
      printw(" %6.3f",upperLimit);
    }
    standend();
    
    printw(") Rocker(");
    if (azRockerCCWLimit <= ccwLimit) {
      standout();
    }
    printw("%+6.2f",azRockerCCWLimit); 
    standend();
    printw(", ");
    if (azRockerCWLimit >= cwLimit) {
      standout();
    }
    printw("%+6.2f", azRockerCWLimit);
    standend();
    printw(")\n");
    mvprintw(line++, 0, "     Gains (IPD)     TBias TDiv EncT EncRev EncOffset"
	     " M1Temp M1Cur M2Temp M2Cur\n");
    mvprintw(line++, 0, "Az (%5.1f %4.1f %4.1f) %5.1f %4d %s   %s  %9.4f"
	     " %6.1f  %4.1f %6.1f  %4.1f\n",
	     azIntegralGain, azProportionalGain, azDerivativeGain, azTorqueBias,
	     azTachDivisor, (azEncType)? "ACC ": "Heid",
	     (azEncoderReversed)? "Yes": "No ", azEncoderOffset * (360.0 / 8388608.),
	     azMot1Temp, azMot1Current, azMot2Temp, azMot2Current);
    mvprintw(line++, 0, "El (%5.1f %4.1f %4.1f)       %4d %s   %s  %9.4f"
	     " %6.1f  %4.1f  M1=CW=left\n",
	     elIntegralGain, elProportionalGain, elDerivativeGain,
	     elTachDivisor, (elEncType)? "ACC ": "Heid",
	     (elEncoderReversed)? "Yes": "No ", elEncoderOffset * (360.0 / 8388608.),
	     elMotTemp, elMotCurrent);
    
    mvprintw(line++, 0, "Lim enc: Az zero %10d, diff %8.4f   "
	     "El zero %10d, diff %8.4f\n",  azLimEncZero, encAz - limAz,
	     elLimEncZero, encEl - limEl);
#if CHECKRMERRORS
    if(rmErrors) {
      mvprintw(line++, 0, "%d rm_read errors", rmErrors);
    } else {
      line++;
    }
#else /* CHECKRMERRORS */
    rms = rm_read(antNo,"RM_AZ_AMAX_F",&dummyFloat);
    mvprintw(line++, 0, "AzAccelMax %.1f deg/s^2",dummyFloat);
    printw("  IRIG-B: ");
    rms = rm_read(antNo,"RM_IRIG_LOCK_ERROR_B",&dummyChar);
    rms = rm_read(antNo,"RM_IRIG_ERROR_COUNT_L",&errorCount);
    switch (dummyChar) {
    case 0:
      printw("Locked");
      break;
    case 1:
      standout();
      printw("Bad phaselock");
      break;
    case 2:
      standout();
      printw("Reference Err");
      break;
    case 3:
      standout();
      printw("Bad lock&ref ");
      break;
    }
    standend();
    printw(" (errors=");
    if (errorCount > 0) {
      standout();
    }
    printw("%d",errorCount);
    standend();
    printw(")");
    
    rms=rm_read(antNo,"RM_SCB_SHUTDOWN_REQUEST_B",&dummyChar);
    printw("  ");
    if (antNo < 7 && stowRequest == 1) {
      standout();
    }
    printw("TelephoneStow:%d",stowRequest);
    standend();
    printw(" shutdown:%d",dummyChar);
    clrtoeol();
#endif /* CHECKRMERRORS */
    
    rms = rm_read(antNo,"RM_SCB_BYPASS_PRELIMS_S",&bypass);
    if (bypass != 0) {
      standout();
      move(16,57); 
      /*    move(line-1,57);*/
      printw("Bypass prelims: %d sec ",((int)bypass*230)/65535);
      standend();
    } else {
      rms = rm_read(antNo,"RM_SCB_BYPASS_SOFTLIMS_S",&bypass);
      if (bypass != 0) {
	standout();
	move(16,56); 
	/*      move(line-1,56);*/
	printw("Bypass softlims: %d sec\n",((int)bypass*230)/65535);
	standend();
      } else {
	printw("\n");
      }
    }
    refresh();
    
    mvprintw(line++, 0, "EPROM 0x%2x  SRAM 0x%2x Palm Code Date %9s  "
	     "SCB resets %d Bad Pkts %d\n",
	     scbEPROMVersion, scbSRAMVersion, palmCodeDate, scbRestarts,
	     nakCount);
    
    mvprintw(line++, 0, "Fault word = 0x%08x  SCB Status 0x%02x  "
	     "padID = ", scbFaultWord, scbStatus);
    if (padID < MIN_ANTENNA_PAD || padID > MAX_ANTENNA_PAD) {
      standout();
    }
    printw("%d",padID);
    standend();
    rms = rm_read(antNo,"RM_PAD_ID_IS_FAKE_B",(char *)&padIDIsFake);
    if (padIDIsFake==1) {
      printw("F");
    }
    printw(", padAzOffset = %.4f\n", padAzOffset);
    parseFaults(scbFaultWord, antNo, encAz, limAz, encEl, limEl);
  }
  move(18,0);

  rm_read(antNo,"RM_GLYCOL_FLOW_D",&glycolFlow);
  rm_read(antNo,"RM_GLYCOL_TEMP_D",&glycolTemp);
  rm_read(antNo,"RM_GLYCOL_PRESSURE_D",&glycolPressure);
  printw("Glycol Flow: %3.2f   Temp:  %3.2f   Pressure %3.1f",glycolFlow,glycolTemp,glycolPressure);
  refresh();
  glycolTemp = rm_read(antNo,"RM_GLYCOL_TEMP_D",&glycolTemp);
  return;
}

char *StateStrings(int state) {
  
  if(state >= 4) {
    return(drvStateStrings[4]);
  } else if(state <= 0) {
    return(drvStateStrings[0]);
  } else {
    return(drvStateStrings[state]);
  };
}
void parseFaults(unsigned long fault, int antNo, float encAz, float limAz, float encEl, float limEl) {
  int i, col, dsmStatus;
  short estop;
  time_t timestamp;
  char antString[10];
  col = 0;
  for(i = 0; i < NFAULTS; i++) {
    if(fault & faults[i].bit) {
      if (faults[i].bit == (1<<EMERGENCY_STOP_FAULT)) {
	sprintf(antString,"acc%d",antNo);
	dsmStatus = dsm_read(antString,"DSM_ESTOP_COMMAND_S",&estop,&timestamp);
	switch (estop) {
	case 1:
	  /* it is either on, or in the process of being turned off */
	  mvprintw(line, col, "%-22s (on Power PC)    ", faults[i].msg);
	  break;
	case 2:
	  mvprintw(line, col, "%-22s (PPC disengaging)", faults[i].msg);
	  break;
	default:
	  standout();
	  mvprintw(line, col, "%-40s", faults[i].msg);
	  standend();
	  break;
	}
      } else {
	mvprintw(line, col, "%-40s", faults[i].msg);
      }
      if(col == 0) {
	col = 40;
      } else {
	line++;
	col = 0;
      }
    }
  }
  if(fabs(encAz - limAz) > 0.2) {
    mvprintw(line, col, "Az Encoders differ %10.4f deg.\n",
	     encAz - limAz);
    if(col == 0) {
      col = 40;
    } else {
      line++;
      col = 0;
    }
  }
  if(fabs(encEl - limEl) > 0.4) {
    mvprintw(line, col, "El Encoders differ %10.4f deg.\n",
	     encEl - limEl);
    if(col == 0) {
      col = 40;
    } else {
      line++;
      col = 0;
    }
  }
  if(col != 0) {
    mvprintw(line, col, "\n");
    line++;
  }
  i = lastLine;
  lastLine = line;
  for(; line < i; line++) {
    mvprintw(line, 0, "\n");
  }
}
