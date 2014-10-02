#define HIGHLIGHT_200MHZ_POWER_ERROR 0.4 /* volts */
/* 0.2V is about 10deg of phase at 345 GHz */

#define NO_PRINT 0
#define YES_PRINT 1
#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "rm.h"
#include "optics.h"
#include "monitor.h"
#include "iFLOMonitorPage2.h"
#define LOW_QUADRATURE_THRESHOLD 4.0

int printVoltageAsLogicState(float dFloat);
int getVoltageAsLogicState(float dFloat);
int doVoltageAsLogicState(float dFloat, int state);
extern void checkStatus(int,char *);
extern int iFLOUnits;

void iFLODisplayPage2(int count, int *antlist, int pageMode, IFLO_FLAGS *flags)
{
#define CONT1_DET2_CRITERION (1.0) /* volts */
#define CONT2_DET2_CRITERION (1.0) /* volts */
  int ant, rms, dummyInt;
  short dummyShort;
  char esma;
  time_t dsmTimestamp;
  float cont1det2coldskymin = 1.5;
  float cont2det2coldskymin = 1.5;
  static float quadratureThresholdLowFreqRx[MAX_NUMBER_ANTENNAS+1] = {
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD
  };
  static float quadratureThresholdHighFreqRx[MAX_NUMBER_ANTENNAS+1] = {
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD,
    LOW_QUADRATURE_THRESHOLD
  };

  float dFloat;
  int doWeCare[11];
  int state;
  int curTime,delay;
  int isReceiverInArray[3];

  getReceiverList(isReceiverInArray);
  getAntennaList(doWeCare);
  for (ant = 1; ant <= numberAntennas; ant++) {
    flags->flags1[ant] = 0;
    flags->flags2[ant] = 0;
    flags->quad1[ant] = 0;
    flags->xmit1power[ant] = 0;
    flags->xmit1OpAlarm[ant] = 0;
    flags->xmit1TempAlarm[ant] = 0;
    flags->quad2[ant] = 0;
    flags->xmit2power[ant] = 0;
    flags->xmit2OpAlarm[ant] = 0;
    flags->xmit2TempAlarm[ant] = 0;
    flags->mrgAlarm[ant] = 0;
    flags->mrgRfPower[ant] = 0;
    flags->power1_109_200[ant] = 0;
    flags->power2_109_200[ant] = 0;
  }
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (doWeCare[ant] == 1) {
      rms = rm_read(ant, "RM_109MHZ1_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_109MHZ1_POWER_F)");
      if (fabs(dFloat-power200MHzTarget[ant])>HIGHLIGHT_200MHZ_POWER_ERROR) {
	flags->power1_109_200[ant] = 1;
      }
      rms = rm_read(ant, "RM_MRG_OP_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_MRG_OP_POWER_F)");
      state = getVoltageAsLogicState(dFloat);
      if (state != 0) {
	flags->mrgAlarm[ant] = 1;
      }
      rm_read(ant,"RM_CALIBRATION_WHEEL_S",&dummyShort);
      checkStatus(rms,"rm_read(RM_CALIBRATION_WHEEL_S)");
      rms = rm_read(ant, "RM_CONT1_DET2_F", &dFloat);
      checkStatus(rms,"rm_read(RM_CONT1_DET2_F)");
      if ((fabs(dFloat-cont1det2target[ant]) > CONT1_DET2_CRITERION) &&
	  dummyShort == AMBIENT_IN) {
	flags->xmit1power[ant] = 1;
      }
      if (dFloat < cont1det2coldskymin && dummyShort == SKY_IN) {
	flags->xmit1power[ant] = 1;
      }
      rms = rm_read(ant, "RM_YIG1_QUADRATURE_F", &dFloat);
      checkStatus(rms,"rm_read(RM_YIG1_QUADRATURE)");
      if (fabs(dFloat) < quadratureThresholdLowFreqRx[ant]) {
	flags->quad1[ant] = 1;
      }
      if (isReceiverInArray[2]) {
	rms = rm_read(ant, "RM_109MHZ2_POWER_F", &dFloat);
	checkStatus(rms,"rm_read(RM_109MHZ2_POWER_F)");
	if (fabs(dFloat-power200MHzTarget[ant])>HIGHLIGHT_200MHZ_POWER_ERROR) {
	  flags->power2_109_200[ant] = 1;
	}
	rms = rm_read(ant, "RM_YIG2_QUADRATURE_F", &dFloat);
	checkStatus(rms,"rm_read(RM_YIG2_QUADRATURE_F)");
	if (fabs(dFloat) < quadratureThresholdHighFreqRx[ant]) {
	  flags->quad2[ant] = 1;
	}
	rms = rm_read(ant, "RM_XMIT2_OP_ALARM_F", &dFloat);
	checkStatus(rms,"rm_read(RM_XMIT2_OP_ALARM_F)");
	state = getVoltageAsLogicState(dFloat);
	if (state != 0) {
	  flags->xmit2OpAlarm[ant] = 1;
	}
	rms = rm_read(ant, "RM_XMIT2_TEMP_ALARM_F", &dFloat);
	checkStatus(rms,"rm_read(RM_XMIT2_TEMP_ALARM_F)");
	state = getVoltageAsLogicState(dFloat);
	if (state != 0) {
	  flags->xmit2TempAlarm[ant] = 1;
	}
      }
      rms = rm_read(ant, "RM_XMIT1_OP_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_XMIT1_OP_ALARM_F)");
      state = getVoltageAsLogicState(dFloat);
      if (state != 0) {
	flags->xmit1OpAlarm[ant] = 1;
      }
      rms = rm_read(ant, "RM_XMIT1_TEMP_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_XMIT1_TEMP_ALARM_F)");
      state = getVoltageAsLogicState(dFloat);
      if (state != 0) {
	flags->xmit1TempAlarm[ant] = 1;
      }
      if (isReceiverInArray[2]) {
	rms = rm_read(ant, "RM_CONT2_DET2_F", &dFloat);
	checkStatus(rms,"rm_read(RM_CONT2_DET2_F)");
	if ((fabs(dFloat-cont2det2target[ant]) > CONT2_DET2_CRITERION) &&
	    dummyShort == AMBIENT_IN) {
	  flags->xmit2power[ant] = 1;
	}
	if ((dFloat < cont2det2coldskymin) && dummyShort == SKY_IN) {
	  flags->xmit2power[ant] = 1;
	}
      }
      rms = rm_read(ant, "RM_MRG_RF_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_MRG_RF_POWER_F)");
      if (fabs(mrgRfPowerTarget[ant]-dFloat) > 1.0) {
	flags->mrgRfPower[ant] = 1;
      }
      flags->flags1[ant] = flags->quad1[ant] || flags->xmit1power[ant] ||
	flags->mrgAlarm[ant] || flags->xmit1OpAlarm[ant] || 
	flags->mrgRfPower[ant] ||
	flags->xmit1TempAlarm[ant] || flags->power1_109_200[ant];
      flags->flags2[ant] = flags->quad2[ant] || flags->xmit2power[ant] ||
	flags->mrgAlarm[ant] || flags->xmit2OpAlarm[ant] || 
	flags->mrgRfPower[ant] ||
	flags->xmit2TempAlarm[ant] || flags->power2_109_200[ant];
    }
  }
  if (pageMode == IFLO_PAGE_CHECK_ONLY) {
    return;
  }
  if ((count % 60) == 1) {
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
  move(0,0);
  printw("   Antenna IF/LO Status - Page 1  (\"+/-\" to cycle pages, \"#\" to change units)\n");
  move(1,0);
  printw("Antenna      1");
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
  clrtoeol();
  move(2,0);
  printw("MRG Alarm");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_MRG_OP_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_MRG_OP_POWER_F)");
      state = printVoltageAsLogicState(dFloat);
    }
  }
  clrtoeol();
  move(3,0);
  printw("MRG RFPwr");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_MRG_RF_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_MRG_RF_POWER_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	if (flags->mrgRfPower[ant] == 1) {
	  standout();
	}
	if (fabs(dFloat)<10) {
	  printw(" %+7.4f ", dFloat);
	} else {
	  printw("%+7.4f ", dFloat);
	}
	standend();
      }
    }
  }
  clrtoeol();
  move(4,0);
  printw("MRGTarget");
  for (ant = 1; ant <= numberAntennas; ant++) {
    printw(" %+7.4f ",mrgRfPowerTarget[ant]);
  }
  clrtoeol();
  move(5,0);
  printw("Quad RX1 ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_YIG1_QUADRATURE_F", &dFloat);
      checkStatus(rms,"rm_read(RM_YIG1_QUADRATURE_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	if (flags->quad1[ant] == 1) {
	  standout();
	}
	printw(" %7.4f ", dFloat);
	standend();
      }
    }
  }
  clrtoeol();
  move(6,0);
  printw("Stress   ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG1_STRESS_F", &dFloat);
      checkStatus(rms,"rm_read(RM_YIG1_STRESS_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  clrtoeol();
  move(7,0);
  printw("200 Power");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_109MHZ1_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_109MHZ1_POWER_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	if (flags->power1_109_200[ant] == 1) {
	  standout();
	}
	printw(" %7.4f ", dFloat);
	standend();
      }
    }
  }
  clrtoeol();
  move(8,0);
  printw("200Target");
  for (ant = 1; ant <= numberAntennas; ant++) {
    printw(" %7.4f ",power200MHzTarget[ant]);
  }
  move(9,0);
  printw("Rcvr Powr");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_109_200_RX1_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_109_200_RX1_ALARM_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	switch (iFLOUnits) {
	case IFLO_UNITS_DBM:
	  if (fabs(10*log10(dFloat)) >= 10) {
	    printw(" %+5.1fdBm", 10*log10(dFloat));
	  } else {
	    printw(" %+5.2fdBm", 10*log10(dFloat));
	  }
	  break;
	case IFLO_UNITS_MILLIWATT:
	  printw(" %6.3fmW", dFloat);
	  break;
	default:
        case IFLO_UNITS_VOLTS:
	  printw(" %7.4fV", dFloat);
	  break;
	}
      }
    }
  }
  clrtoeol();
  move(10,0);
  printw("Dewar Pwr");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_CONT1_DET1_F", &dFloat);
      checkStatus(rms,"rm_read(RM_CONT1_DET1_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  clrtoeol();
  move(11,0);
  printw("XmitPower");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT1_DET2_F", &dFloat);
      checkStatus(rms,"rm_read(RM_CONT1_DET2_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	printw(" ");
        rm_read(ant,"RM_CALIBRATION_WHEEL_S",&dummyShort);
	checkStatus(rms,"rm_read(RM_CALIBRATION_WHEEL_S)");
	if (flags->xmit1power[ant] == 1) {
	  standout();
	}
	printw("%7.4f", dFloat);
	standend();
	printw(" ");
      }
    }
  }
  clrtoeol();
  move(12,0);
  printw("TargetPow");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (fabs(cont1det2target[ant]) > 11) {
	printw("  wacko  ");
      } else {
	printw("  %6.4f ",cont1det2target[ant]);
      }
    }
  }
  clrtoeol();
  move(13,0);
  printw("XOpt/Temp");
  /*  printw("XOptiAlrm");*/
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_XMIT1_OP_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_XMIT1_OP_ALARM_F)");
      printw(" ");
      state = printVoltageAsThreeCharLogicState(dFloat);
      printw("/");
      rms = rm_read(ant, "RM_XMIT1_TEMP_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_XMIT1_TEMP_ALARM_F)");
      state = printVoltageAsThreeCharLogicState(dFloat);
      printw(" ");
    }
  }
  clrtoeol();
  /*  printw("Receiver 2");*/

  move(14,48);
  rms = call_dsm_read("colossus", "DSM_ANTENNA5_ESMA_STATUS_B", 
		      (void *)&esma, &dsmTimestamp);
  curTime = time((long *)0);
  delay = curTime-dsmTimestamp;
  if (delay > 60) {
    printw("stale");
  } else {
#define POWERED_OFF 2
    switch (esma) {
    case ESMA_MODE_READBACK:
      printw(" CSO ");
      break;
    case SMA_MODE_READBACK:
      printw(" SMA ");
      break;
    case POWERED_OFF:
      printw(" SMA?");
      break;
    default:
      standout();
      printw("wacko");
      standend();
    }
  }
  move(14,56);
  rms = call_dsm_read("colossus", "DSM_ANTENNA6_ESMA_STATUS_B", 
		      (void *)&esma, &dsmTimestamp);
  delay = curTime-dsmTimestamp;
  if (delay > 60) {
    standout();
    printw("stale");
    standend();
  } else {
    switch (esma) {
    case ESMA_MODE_READBACK:
      printw(" JCMT");
      break;
    case SMA_MODE_READBACK:
      printw(" SMA ");
      break;
    case POWERED_OFF:
      printw(" SMA?");
      break;
    default:
      standout();
      printw("wacko");
      standend();
    }
  }

  move(15,0);
  printw("Quad RX2 ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_YIG2_QUADRATURE_F", &dFloat);
      checkStatus(rms,"rm_read(RM_YIG2_QUADRATURE_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	if (flags->quad2[ant] == 1) {
	  standout();
	}
	printw(" %7.4f ", dFloat);
	standend();
      }
    }
  }
  clrtoeol();
  move(16,0);
  printw("Stress   ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG2_STRESS_F", &dFloat);
      checkStatus(rms,"rm_read(RM_YIG2_STRESS_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  clrtoeol();
  move(17,0);
  printw("200 Power");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_109MHZ2_POWER_F", &dFloat);
      checkStatus(rms,"rm_read(RM_109MHZ2_POWER_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	if (flags->power2_109_200[ant] == 1) {
	  standout();
	}
	printw(" %7.4f ", dFloat);
	standend();
      }
    }
  }
  clrtoeol();
  move(18,0);
  printw("200Target");
  for (ant = 1; ant <= numberAntennas; ant++) {
    printw(" %7.4f ",power200MHzTarget2[ant]);
  }
  move(19,0);
  printw("Rcvr Powr");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_109_200_RX2_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_109_200_RX2_ALARM_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	switch (iFLOUnits) {
	case IFLO_UNITS_DBM:
	  if (fabs(10*log10(dFloat)) >= 10) {
	    printw(" %+5.1fdBm", 10*log10(dFloat));
	  } else {
	    printw(" %+5.2fdBm", 10*log10(dFloat));
	  }
	  break;
	case IFLO_UNITS_MILLIWATT:
	  printw(" %6.3fmW", dFloat);
	  break;
	default:
        case IFLO_UNITS_VOLTS:
	  printw(" %7.4fV", dFloat);
	  break;
	}
      }
    }
  }
  clrtoeol();
  move(20,0);
  printw("Dewar Pwr");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT2_DET1_F", &dFloat);
      checkStatus(rms,"rm_read(RM_CONT2_DET1_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	printw(" %7.4f ", dFloat);
      }
    }
  }
  clrtoeol();
  move(21,0);
  printw("XmitPower");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_CONT2_DET2_F", &dFloat);
      checkStatus(rms,"rm_read(RM_CONT2_DET2_F)");
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	printw("  Wacko  ");
      } else {
	printw(" ");
	if (flags->xmit2power[ant] == 1) {
	  standout();
	}
	printw("%7.4f", dFloat);
	standend();
	printw(" ");
      }
    }
  }
  clrtoeol();
  move(22,0);
  printw("TargetPow");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (fabs(cont2det2target[ant]) > 11) {
	printw("  wacko  ");
      } else {
	printw("  %6.4f ",cont2det2target[ant]);
      }
    }
  }
  clrtoeol();
  move(23,0);
  /*  printw("XOptiAlrm");*/
  printw("XOpt/Temp");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_XMIT2_OP_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_XMIT2_OP_ALARM_F)");
      printw(" ");
      state = printVoltageAsThreeCharLogicState(dFloat);
      printw("/");
      rms = rm_read(ant, "RM_XMIT2_TEMP_ALARM_F", &dFloat);
      checkStatus(rms,"rm_read(RM_XMIT2_TEMP_ALARM_F)");
      state = printVoltageAsThreeCharLogicState(dFloat);
      printw(" ");
    }
  }
  clrtoeol();
  move(0,79);
  refresh();
}

int printVoltageAsLogicState(float dFloat) {
  return(doVoltageAsLogicState(dFloat,YES_PRINT));
}

int printVoltageAsThreeCharLogicState(float dFloat) {
  return(doVoltageAsThreeCharLogicState(dFloat,YES_PRINT));
}

int getVoltageAsLogicState(float dFloat) {
  return(doVoltageAsLogicState(dFloat,NO_PRINT));
}

int doVoltageAsLogicState(float dFloat, int print) {
#define HEALTHY_RANGE 0.20
  int state = 0;
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	if (print==YES_PRINT) {
	  printw("  Wacko  ");
	}
	return(-1);
      } else if (fabs(dFloat-5) < HEALTHY_RANGE) {
	if (print==YES_PRINT) {
	  printw("   good  ");
	}
	return(0);
      } else if (fabs(dFloat) < HEALTHY_RANGE) {
	if (print==YES_PRINT) {
	  standout();
	  printw("   bad   ");
	  standend();
	}
	return(-2);
      } else {
	if (print==YES_PRINT) {
	  standout();
	  printw(" %7.4f ", dFloat);
	  standend();
	}
	return(-3);
      }
}

int doVoltageAsThreeCharLogicState(float dFloat, int print) {
#define HEALTHY_RANGE 0.20
  int state = 0;
      if ((dFloat < -11.0) || (dFloat > 11.0)) {
	if (print==YES_PRINT) {
	  printw("wac");
	}
	return(-1);
      } else if (fabs(dFloat-5) < HEALTHY_RANGE) {
	if (print==YES_PRINT) {
	  printw(" ok");
	}
	return(0);
      } else if (fabs(dFloat) < HEALTHY_RANGE) {
	if (print==YES_PRINT) {
	  standout();
	  printw("bad");
	  standend();
	}
	return(-2);
      } else {
	if (print==YES_PRINT) {
	  standout();
	  printw("%3.2f", dFloat);
	  standend();
	}
	return(-3);
      }
}
