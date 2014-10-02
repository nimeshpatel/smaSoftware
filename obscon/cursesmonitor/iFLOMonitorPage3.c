#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "rm.h"
#include "upspage.h"
#include "monitor.h"
#include "bandwidthDoubler.h"

#define YIG_FREQ_WACKO_HIGH 100
#define YIG_FREQ_WACKO_LOW -10

short bDAControlWord[10];

void iFLODisplayPage3(int count, int *antlist)
{
  short tempShort;
  int ant, rms, dummyInt, unixTime;
  float dFloat;
  float yigfreq[11];

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
  printw("                           Antenna IF/LO Status - Page 2");
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
  move(2,0);
  printw("BDA upper ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms = rm_read(ant, "RM_BWD_CONTROL_WORD_S", &bDAControlWord[ant]);
    tempShort = (bDAControlWord[ant] >> 3*BWD_UPPER) & BWD_BIT_MASK;
    switch (tempShort) {
    case BWD_LRL_MASK:
      printw(" Low4to6 ");
      break;
    case BWD_LRH_MASK:
      printw(" Low6to8 ");
      break;
    case BWD_HRL_MASK:
      printw(" high4to6");
      break;
    case BWD_HRH_MASK:
      printw(" high6to8");
      break;
    default:
      printw(" wacko   ");
    }
  }
  move(3,0);
  printw("BDA bottom");
  for (ant = 1; ant <= numberAntennas; ant++) {
    tempShort = (bDAControlWord[ant] >> 3*BWD_LOWER) & BWD_BIT_MASK;
    switch (tempShort) {
    case BWD_LRL_MASK:
      printw(" Low4to6 ");
      break;
    case BWD_LRH_MASK:
      printw(" Low6to8 ");
      break;
    case BWD_HRL_MASK:
      printw(" high4to6");
      break;
    case BWD_HRH_MASK:
      printw(" high6to8");
      break;
    default:
      printw(" wacko   ");
    }
  }
  move(4,0);
  printw("BDA aux   ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    tempShort = (bDAControlWord[ant] >> 3*BWD_AUX) & BWD_BIT_MASK;
    switch (tempShort) {
    case BWD_LRL_MASK:
      printw(" Low4to6 ");
      break;
    case BWD_LRH_MASK:
      printw(" Low6to8 ");
      break;
    case BWD_HRL_MASK:
      printw(" high4to6");
      break;
    case BWD_HRH_MASK:
      printw(" high6to8");
      break;
    default:
      printw(" wacko   ");
    }
  }
  move(5,0);
  printw("MRG Attn ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_MRG_RX_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(6,0);
  printw("109/200 1");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_109_200_RX1_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(7,0);
  printw("Y1ReqAttn");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_YIG1_ATTENUATOR_VOLTAGE_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  }
  move(8,0);
  printw("Y1ActAttn");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG1_OUTPUT_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(9,0);
  printw("YIG1 IF  ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG1_PLL_IF_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(10,0);
  printw("YIG1 Tune");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG1_CTRL_VOLTAGE_F", &dFloat);
      yigfreq[ant] = (dFloat+yigTuneCurveB)/yigTuneCurveA;
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(11,0);
  printw("YIG1 Freq");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (yigfreq[ant] >= YIG_FREQ_WACKO_HIGH || yigfreq[ant] <= YIG_FREQ_WACKO_LOW) {
	printw("  wacko  ");
      } else {
	printw(" %7.4f ", yigfreq[ant]);
      }
    }
  }

  move(12,0);
  printw("Rx1 Attn ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_DEWAR_POWER1_ATTEN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(13,0);
  printw("Gunn1TECV");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_GUNN1_TEC_FEEDBACK_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %+6.3f  ", dFloat);
    }
  }
  move(14,0);
  printw("YIG_SVC_T");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_YIG_SVC_TIMESTAMP_L", &dummyInt);
      rms = rm_read(ant, "RM_UNIX_TIME_L", &unixTime);
      if (dummyInt < 1) {
	printw("  Wacko  ");
      } else {
	printw("  ");
	printAgeStandoutN(unixTime,dummyInt,3);
      }
    }
  }

  move(15,0);
  printw("109/200 2");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_109_200_RX2_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(16,0);
  printw("Y2ReqAttn");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_YIG2_ATTENUATOR_VOLTAGE_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  }
  move(17,0);
  printw("Y2ActAttn");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG2_OUTPUT_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(18,0);
  printw("YIG2 IF  ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG2_PLL_IF_ATTN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(19,0);
  printw("YIG2 Tune");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_YIG2_CTRL_VOLTAGE_F", &dFloat);
      yigfreq[ant] = (dFloat+yigTuneCurveB)/yigTuneCurveA;
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(20,0);
  printw("YIG2 Freq");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (yigfreq[ant] >= YIG_FREQ_WACKO_HIGH || yigfreq[ant] <= YIG_FREQ_WACKO_LOW) {
	printw("  wacko  ");
      } else {
	printw(" %7.4f ", yigfreq[ant]);
      }
    }
  }

  move(21,0);
  printw("Rx2 Attn ");
  for (ant = 1; ant <= numberAntennas; ant++)
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_DEWAR_POWER2_ATTEN_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %7.4f ", dFloat);
    }
  move(22,0);
  printw("Gunn2TECV");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (!antsAvailable[ant])
      printw("  -----  ");
    else {
      rms = rm_read(ant, "RM_GUNN2_TEC_FEEDBACK_F", &dFloat);
      if ((dFloat < -11.0) || (dFloat > 11.0))
	printw("  Wacko  ");
      else
	printw(" %+6.3f  ", dFloat);
    }
  }
  move(23,0);
  printw("                    --- Type \"+/-\" to cycle through pages ---");
  move(0,79);
  refresh();
}
