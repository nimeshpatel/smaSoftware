#define SIS1_MIXER_CURRENT_MIN 20
#define SIS1_MIXER_CURRENT_MAX 45
#define MIXER_BOARD_WACKO_VOLTAGE 999.9
#define DEBUG 0
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include <math.h>
#include <curses.h>
#include <dsm.h>
#include "commonLib.h"
#include "chopperControl.h"
#include "monitor.h"
#include "opticsMonitor.h"
#include "optics.h"
#include "tune6status.h"
#include "rscanpage.h"
#include "upspage.h"
#include "receiverMonitor.h"
#include "receiverMonitorHighFreq.h"

extern int translateBandNumberToGHz(int band);
extern void noBoard(int); 
extern short slaveToRxNumber(short slave);
extern void checkStatus(int,char *);
extern char *getLoBoardTypeStringBrief(int);
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *t, char *str);
extern void checkDSMStatus(int status, char *string);
#define INITVAL 999999.9
static  double lastazoff[9] = {INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL};
static double lasteloff[9] = {INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL,INITVAL};

int rscanpage(int count, int *rm_list, int pageMode) {
  float dummyFloat;
  float temperature;
  short shortarray[8][8];
  short dummyShort;
  double azoff, eloff;
  int rm_status, rms;
  int ant, antennaNumber;
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int doWeCare[11];
  unsigned char statusBits[16];
  float posMm[4];		/* x, y, z, and t of chopper n mm and arcsec */
  short network[MAX_NUMBER_ANTENNAS+1][15];
  static int avoid = 0;
  long longvalue, longvalueMirror;
  short shortvalue;
  char source[35];
  char dummyByte;
  int m3Status;
  static char m3stat[8][8] = { "  m3?  ", "m3closd", " open ", "m3movng",
				   " m3??? ", "m3clsd?", " open?"};
  int radio_flag;
  long rightnow;
  float syncdet_channels[2];

  short actPLLrx[9], opticsBoardPresent[9];
  short busy;
  long timevalue;
  int i;
  float cont1det1,cont2det1;
  short nightlyPointing;
  short mixerBoardPresent[9],actPLL[9];
  float cntVal;
  float mixerCurrent[9];
  long lastIpoint[11];
  long feed;
  static int ipoint[10];
  static float sunaz[10], sunel[10], ambientTempC[10];
  int problem = 0;
  int antsum;
  int isReceiverInArray[3];

  getReceiverList(isReceiverInArray);
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  if (pageMode == RSCAN_CHECK_ONLY) {
    return(problem);
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
  getAntennaList(doWeCare);

  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw(" Rscan Status on %s UT = %s HST",timeString,timeString2);
  if (DEBUG) { 
    refresh(); 
  }
  move(1,0);
  printw("Ant/Feed/p");
  for (ant=1; ant<9; ant++) {
    printw("  ");
    if (ant==avoid) {
      standout();
    }
    rm_status = rm_read(ant,"RM_FEED_L",&feed);
    printw("%d/",ant);
    if (feed < 100 || feed > 999) {
      addstr("wac");
    } else {
      printw("%3d",feed);
    }
    rm_status = rm_read(ant,"RM_REFRACTION_RADIO_FLAG_B",&dummyByte);
    radio_flag=(int)dummyByte; 
    if (radio_flag==1) {
      printw("r");
    } else {
      printw("o");
    }
    if (ant==avoid) {
      standend();
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(2,0);
  printw("Source     ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    short style;
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      (void)rm_read(antennaNumber,"RM_M3STATE_B",&dummyByte);
      m3Status = (short)dummyByte;
      rms = rm_read(antennaNumber,"RM_SOURCE_C34",source);
      for (i=strlen(source); i<7; i++) {
	source[i] = ' ';
      }
      source[7] = 0;
      if (m3Status != 2 && m3Status != 6) {
	printw(" ");
	standout();
	printw("%s",m3stat[m3Status]);
	standend();
      } else {
	/* check for loads */
	rms = rm_read(antennaNumber, "RM_HOTLOAD_STYLE_S",&style);
	if (style == ORIGINAL_HOTLOAD_STYLE) {
	  (void)rm_read(antennaNumber,"RM_CALIBRATION_WHEEL_S",&dummyShort);
	  if (dummyShort == AMBIENT_IN) {
	    printw(" ");
	    standout();
	    printw("Ambient");
	    standend();
	  } else {
	    printw(" %s",source);
	  }
	} else if (style == NEW_HOTLOAD_STYLE) {
	  if (printLinearLoadStatusWordStandout(antennaNumber) != 0) {
	    printw(" %s",source);
	  } else {
	    printw(" ");
	  }
	}
      }
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(3,0);
  printw("AZ command ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_COMMANDED_AZ_DEG_F",&dummyFloat);
      if (fabs(dummyFloat) > 999) {
	printw("  wacko ");
      } else {
	printw(" %+6.1f ",dummyFloat);
      }
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(4,0);
  printw("AZ actual  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_ACTUAL_AZ_DEG_F",&dummyFloat);
      if (dummyFloat > 500 || dummyFloat < -240) {
	printw("  wacko  ");
      } else {
	printw(" %+6.1f ",dummyFloat);
      }
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(5,0);
  printw("AZ offset  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_AZOFF_D",&azoff);
      if (fabs(azoff) > 999999) {
	printw("  wacko ");
      } else {
	if (lastazoff[antennaNumber] != azoff && lastazoff[antennaNumber]!=INITVAL) {
	  standout();
	}
	if (fabs(azoff) >= 10000) {
	  printw("%+.1e",azoff);
	} else if (fabs(azoff) >= 1000) {
	  printw(" %+.0f ",azoff);
	} else {
	  printw(" %+6.1f ",azoff);
	}
	lastazoff[antennaNumber] = azoff;
	standend();
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(9,0);
#if 0
  printw("scan rate  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_SMARTS_OFFSET_UNIT_ARCSEC_S",&rate);
      printw(" %+6d ",rate);
    }
  }
#else
  printw("RA/DECoffst ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    double decoff,raoff;
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_DECOFF_ARCSEC_D",&decoff);
      rms = rm_read(antennaNumber,"RM_RAOFF_ARCSEC_D",&raoff);
      if (fabs(raoff) > 999999) {
	printw("wac/");
      } else {
	if (fabs(decoff)>=100 && fabs(raoff)<10) {
	  printw("%2.0f/",raoff);
	} else {
	  printw("%3.0f/",raoff);
	}
      }
      if (fabs(decoff) > 999999) {
	printw("wac ");
      } else {
	if (fabs(raoff)>=100 && fabs(decoff)<10) {
	  printw("%2.0f ",decoff);
	} else {
	  printw("%3.0f ",decoff);
	}
      }
    }
  }
#endif
  if (DEBUG) { 
    refresh(); 
  }
  move(7,0);
  printw("EL command ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      if (fabs(dummyFloat) > 999) {
	printw("  wacko ");
      } else {
	rms = rm_read(antennaNumber,"RM_COMMANDED_EL_DEG_F",&dummyFloat);
	if (fabs(dummyFloat) > 9999) {
	  printw("  wacko ");
	} else {
	  printw(" %6.1f ",dummyFloat);
	}
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(8,0);
  printw("EL actual  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_ACTUAL_EL_DEG_F",&dummyFloat);
      if (fabs(dummyFloat) > 9999) {
	printw(" wacko  ");
      } else {
	printw(" %6.1f ",dummyFloat);
      }
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(6,0);
  printw("EL offset  ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_ELOFF_D",&eloff);
      if (fabs(eloff) > 999999) {
	printw("  wacko ");
      } else {
	if (lasteloff[antennaNumber] != eloff && lasteloff[antennaNumber]!=INITVAL) {
	  standout();
	}
	printw(" %+6.1f ",eloff);
	standend();
      }
      lasteloff[antennaNumber] = eloff;
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(10,0);
  printw("Grid/Mirror");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    rms = rm_read(antennaNumber, "RM_OPTICS_BOARD_PRESENT_S",
		    &opticsBoardPresent[antennaNumber]);
    if (opticsBoardPresent[antennaNumber] == 0) {
      printw(" NoBoard");
      continue;
    }
    if (!antsAvailable[antennaNumber]) {
      printw("  -----  ");
    } else {
      rms = rm_read(antennaNumber,"RM_WIRE_GRID_RECEIVER_LOCATION_L", 
		    &longvalue);
      checkStatus(rms,"rm_read(RM_WIRE_GRID_RECEIVER_LOCATION_L)");
      rms = rm_read(antennaNumber,"RM_COMBINER_MIRROR_MOTORIZED_S", 
		    &shortvalue);
      checkStatus(rms,"rm_read(RM_COMBINER_MIRROR_MOTORIZED_S)");
      if (shortvalue) {
	rms = rm_read(antennaNumber,"RM_COMBINER_MIRROR_RECEIVER_LOCATION_L", 
		      &longvalueMirror);
	checkStatus(rms,"rm_read(RM_COMBINER_MIRROR_RECEIVER_LOCATION_L)");
      } else {
	longvalueMirror = 6; /* first units were fixed illuminating the 600s */
      }

      rms = rm_read(antennaNumber, "RM_CALIBRATION_WHEEL_TIMESTAMP_L", &timevalue);
      if (timevalue < rightnow-GRID_STALE_INTERVAL) {
	if (doWeCare[antennaNumber]==1) {
	  standout();
	}
	printw("  stale ");
	if (doWeCare[antennaNumber]==1) {
	  standend();
	}
      } else {
        rms = rm_read(antennaNumber, "RM_TUNE6_COMMAND_BUSY_S", &busy);
	switch (busy) {
        case TUNE6_BUSY_WITH_WIRE_GRID:
	  standout();
	  printw("turn");
	  standend();
	  printw("/%3d",translateBandNumberToGHz(longvalueMirror));
	  break;
        case TUNE6_BUSY_WITH_COMBINER_MIRROR:
	  printw("%3d/",translateBandNumberToGHz(longvalue));
	  standout();
	  printw("turn");
	  standend();
	  break;
	default:
          printw(" %3d/%3d",
             translateBandNumberToGHz(longvalue),
             translateBandNumberToGHz(longvalueMirror));
	  break;
	}
      }
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(11,0);
  printw("SIS0 MixCur");
  for (ant = 1; ant <= 8; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      rms = rm_read(ant, "RM_MIXER_BOARD_PRESENT_S", &mixerBoardPresent[ant]);
      if (mixerBoardPresent[ant]) {
	rms = rm_read(ant, "RM_SIS_MIXER0_CURRENT_CALIB_F", &dummyFloat);
	mixerCurrent[ant] = dummyFloat;
	if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
            dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
	} else {
          if (doWeCare[ant]==1 && isReceiverInArray[1]==1) {
	    /* Commented out, because it won't compile:
	    if (fabs(dummyFloat) > sis0MixerCurrentMax[ant] ||
		fabs(dummyFloat) < sis0MixerCurrentMin[ant]) {
	      standout();
	    }
	    */
          }
  	  printw("%7.2f ", dummyFloat);
          standend();
	}
      } else {
        printw(" NoBoard"); 
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

  move(12,0);
  printw("SIS1 MixCur");
  for (ant = 1; ant <= 8; ant++) {
    if (!antsAvailable[ant]) {
      printw("  -----  ");
    } else {
      if (mixerBoardPresent[ant]) {
        rms = rm_read(ant, "RM_SIS_MIXER1_CURRENT_CALIB_F", &dummyFloat);
        mixerCurrent[ant] = dummyFloat;
        if (dummyFloat > MIXER_BOARD_WACKO_VOLTAGE ||
          dummyFloat < -MIXER_BOARD_WACKO_VOLTAGE) {
          printw(" wacko ");
        } else {
	  /* Commented out, because it won't compile
 	  if (doWeCare[ant]==1 && isReceiverInArray[2]==1 &&
              (fabs(dummyFloat)<sis1MixerCurrentMin[ant] || 
               fabs(dummyFloat)>sis1MixerCurrentMax[ant])) {
	    standout();
	  }
	  */
	  printw("%7.2f ", dummyFloat);
          standend();
	}
      } else {
	printw(" NoBoard");
      }
    }
  }

  move(13,0);
  printw("Gunn0/Gunn1");
  for (ant = 1; ant <= 8; ant++) {
    rms = rm_read(ant, "RM_LO_BOARD_EPROM_VERSION_V8_S", shortarray[ant-1]);
    rms = rm_read(ant, "RM_RX_MICROCONTROLLER_NETWORK_V15_S", network[ant]);
    if (!antsAvailable[ant]) {
      printw(" -----   ");
    } else {
      rms = rm_read(ant, "RM_SLAVE1_PLL_NO_S", &actPLL[ant]);
      actPLLrx[ant] = slaveToRxNumber(actPLL[ant]);
#define YIG_BOARD_TYPE 6
      if (actPLLrx[ant] < 0) {
	printw(" rm off  ");
      } else if (shortarray[ant-1][actPLLrx[ant]] < 0
		 && network[ant][actPLL[ant]] != YIG_BOARD_TYPE) {
	printw(" NoBoard");
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
        rms = rm_read(ant, "RM_GUNN2_ON_S", &dummyShort);
        switch (dummyShort) {
        case 0:
	  if (doWeCare[ant]==1) {
	    standout();
	  }
	  printw("off ");
  	  if (doWeCare[ant]==1) {
	    standend();
	  }
	  break;
        case 1:
	  printw("on  ");
	  break;
        default:
	  if (doWeCare[ant]==1) {
	    standout();
	  }
	  printw("---");
	  if (doWeCare[ant]==1) {
	    standend();
  	  }
	  printw(" ");
        }
      }
    }
  }

  move(14,0);
  printw("SUBREF stat");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_CHOPPER_STATUS_BITS_V16_B",statusBits);
      printw("  %c%c%c%c  ",
	(statusBits[POS_ERR_BITS] & 8)? 'X': '-',
	(statusBits[POS_ERR_BITS] & 4)? 'Y': '-',
	(statusBits[POS_ERR_BITS] & 2)? 'Z': '-',
	(statusBits[POS_ERR_BITS] & 1)? 'C': '-');
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(15,0);
  printw("SUBTILT_cnt");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_CHOPPER_POS_MM_V4_F",posMm);
      cntVal = posMm[3] * TILT_COUNTS_PER_ARCSEC + 32768;
      if (cntVal < 0 || cntVal >= 1000000) {
	printw("  wacko ");
      } else {
	printw(" %6.0f ",cntVal);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(16,0);
  printw("SUBTILT_sec");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_CHOPPER_POS_MM_V4_F",posMm);
      if (posMm[3] >= 10000 || posMm[3] < -10000) {
	printw("   wacko  ");
      } else { 
	printw(" %+6.2f ",posMm[3]);
      }
    }
  }


  if (DEBUG) { 
    refresh(); 
  }
  move(17,0);
  printw("CONT1DET1mV");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_CONT1_DET1_F",&cont1det1);
      cont1det1 *= 1000;
      if (cont1det1>=100000) {
	printw("  wacko ");
      } else if (cont1det1>=10000) {
	printw(" %6.0f ",cont1det1);
      } else {
	printw(" %6.1f ",cont1det1);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(18,0);
  printw("CONT2DET1mV");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_CONT2_DET1_F",&cont2det1);
      cont2det1 *= 1000;
      if (cont2det1>=100000) {
	printw("  wacko ");
      } else if (cont2det1>=10000) {
	printw(" %6.0f ",cont2det1);
      } else {
	printw(" %6.1f ",cont2det1);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(19,0);
  printw("SYNCDET2_1 ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_SYNCDET2_CHANNELS_V2_F",&syncdet_channels[0]);
      checkStatus(rms,"rm_read(RM_SYNCDET2_CHANNELS_V2_F)");
      if ((syncdet_channels[0] > SYNCDET_CHANNELS_WACKO_LOW) && (syncdet_channels[0] < SYNCDET_CHANNELS_WACKO_HIGH)) {
	printw(" %6.3f ",syncdet_channels[0]);
      } else {
	printw(" Wacko  ");
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(20,0);
  printw("SYNCDET2_2 ");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_SYNCDET2_CHANNELS_V2_F",&syncdet_channels[0]);
      checkStatus(rms,"rm_read(RM_SYNCDET2_CHANNELS_V2_F)");
      if ((syncdet_channels[1] > SYNCDET_CHANNELS_WACKO_LOW) && (syncdet_channels[1] < SYNCDET_CHANNELS_WACKO_HIGH)){
	printw(" %6.3f ",syncdet_channels[1]);
      } else {
	printw(" Wacko  ");
      }
    }
  }

  move(21,0);
  printw("Last Ipoint ");
  refresh(); 
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = call_dsm_read("hal9000","DSM_HAL_HAL_LAST_IPOINT_V11_L",lastIpoint,&longvalue);
      printw(" ");
      printAgeStandoutN(rightnow,lastIpoint[antennaNumber],3);
    }
  } /* end of the 'for' loop over antennas */
  move(22,0);
  printw("Last Cpoint ");
  if (DEBUG) { 
    refresh(); 
  }
  rms = rm_read(rm_list[0],"RM_SUN_EL_DEG_F",&dummyFloat);
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = call_dsm_read("hal9000","DSM_HAL_HAL_LAST_CPOINT_V11_L",lastIpoint,&longvalue);
      printw(" ");
      printAgeStandoutN(rightnow,lastIpoint[antennaNumber],3);
    }
  }
  move(23,0);
  printw("LastNitely");
  refresh(); 
  rms = call_dsm_read("hal9000","DSM_HAL_HAL_NIGHTLY_POINTING_S",&nightlyPointing,&longvalue);
#define NUMBER_OF_ANTENNAS 8
  for (antennaNumber=1; antennaNumber<=NUMBER_OF_ANTENNAS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber, "RM_UNIX_TIME_L", &rightnow);
      printw(" ");
      printAgeStandoutN(rightnow,longvalue,3);
      if (nightlyPointing > 0 && nightlyPointing < 3000) {
	printw(" at %d GHz",nightlyPointing);
      } else {
	printw(" at wacko GHz");
      }
      refresh();
      break;
    }
  } /* end of the 'for' loop over antennas */
#if 0
  move(23,0);
  printw("IpointTempC\n ");
  if (DEBUG) { 
    refresh(); 
  }
  rms = rm_read(rm_list[0],"RM_WEATHER_TEMP_F",&temperature);
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (fabs(ambientTempC[antennaNumber]-temperature) > 5) {
      standout();
    } 
    printw(" %+5.1f",ambientTempC[antennaNumber]);
    standend();
    printw("  ");
  }
#endif

#if 0
  printw("PatchChan 4");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber, "RM_TOTAL_POWER_VOLTS_D", &dummyDouble);
      if (fabs(dummyDouble) >= 100) {
	printw("  wacko ");
      } else {
	printw(" %6.1f ",dummyDouble*1000);
      }
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  move(22,0);
  printw("PatchChan 5");
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber, "RM_TOTAL_POWER_VOLTS2_D", &dummyDouble);
      if (fabs(dummyDouble) >= 100) {
	printw("  wacko ");
      } else {
	printw(" %6.1f ",dummyDouble*1000.);
      }
    }
  }
  move(23,0);
  printw("ScansComplete");
  rms = call_dsm_read("hal9000","DSM_DUALRXPOINT_TOTAL_SCANS_V8_L",scans,&timestamp);
  rms = call_dsm_read("hal9000","DSM_DUALRXPOINT_SCAN_NUMBER_V8_L",scan,&timestamp);
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      if (scan[antennaNumber-1] > 99 || scan[antennaNumber-1] < 0) {
	printw("wac/");
      } else {
	printw(" %2d/",scan[antennaNumber-1]);
      }
      if (scans[antennaNumber-1] > 99 || scans[antennaNumber-1] < 0) {
        printw("wac ");
      } else {
	printw("%2d  ",scans[antennaNumber-1]);
      }
    }
  }
#endif
  move(24,0);
  refresh();
  return(problem);
}

int tokenize(char *input, char *tokenArray[MAX_TOKENS], char *divider) {
  int i;
  int non_blanks = 0;
  int tokens = 0;

  if (strlen(input) > 0) {
    for (i=0; i<strlen(input); i++) {
      if (input[i] != ' ') {
        non_blanks = 1; break;
      }
    }
    if (non_blanks == 0) return(0);
    tokenArray[tokens++] = strtok(input,divider);
    while ((tokenArray[tokens] = strtok(NULL,divider)) != NULL) {
      tokens++;
    }
  }
  return(tokens);
}
