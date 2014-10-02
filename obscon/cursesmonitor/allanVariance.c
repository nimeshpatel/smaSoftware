#include <curses.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include <dsm.h>
#include <math.h>
#include "monitor.h"
#include "allanVariance.h"

#define TUNE6_STALE_INTERVAL 10
#define NUMBER_OF_TIME_BINS 10

extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *, char *);

/* named after David W. Allan */

void allanVariancePage(int count, int *rm_list, int pageMode, ALLAN_VARIANCE_FLAGS *flags) {
  long timestamp;
  int rm_status,i,dsm_status;
  int ant, antennaNumber, j, bin;
  short shortarray[15];
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  long rightnow;
  float allan[RM_ARRAY_SIZE][2][NUMBER_OF_TIME_BINS];
  int doWeCare[11];
  int isReceiverInArray[3];

#define VARIANCE_HIGHLIGHT_MIN 7.0
#define VARIANCE_A_PAGE_HIGHLIGHT_MIN 9.0

  getAntennaList(doWeCare);
  getReceiverList(isReceiverInArray);
  for (antennaNumber=1; antennaNumber<=8; antennaNumber++) { 
    rm_status = rm_read(antennaNumber, "RM_SYNCDET2_ALLAN_VARIANCE_V2_V10_F", 
			&allan[antennaNumber][0][0]);
  }
  if (pageMode == ALLAN_VARIANCE_PAGE_CHECK_ONLY) {
    for (ant = 1; ant <= numberAntennas; ant++) {
      flags->lowFreqFlags[ant] = 0;
      flags->highFreqFlags[ant] = 0;
      flags->lowFreqShortTimescale[ant] = 0;
      flags->highFreqLongTimescale[ant] = 0;
      flags->lowFreqShortTimescale[ant] = 0;
      flags->highFreqLongTimescale[ant] = 0;
      for (j=0; j<2; j++) {
	for (bin=1; bin<NUMBER_OF_TIME_BINS; bin++) { 
	  if (allan[ant][j][bin] < 0 || allan[ant][j][bin] > VARIANCE_A_PAGE_HIGHLIGHT_MIN) {
	    if (j==0) {
	      if (bin < (NUMBER_OF_TIME_BINS/2)) {
		flags->lowFreqShortTimescale[ant] = 1;
	      } else {
		flags->lowFreqLongTimescale[ant] = 1;
	      }
	    } else {
	      if (bin < (NUMBER_OF_TIME_BINS/2)) {
		flags->highFreqShortTimescale[ant] = 1;
	      } else {
		flags->highFreqLongTimescale[ant] = 1;
	      }
	    }
	  }
	}
      }
      if (doWeCare[ant] == 1) {
	flags->lowFreqFlags[ant] = flags->lowFreqShortTimescale[ant];
	flags->highFreqFlags[ant]= flags->highFreqShortTimescale[ant];
      }
    } /* end for loop over antennas */
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
  ant = 0;
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  /*  printw("RxAllanVariance page: %s UT, %s HT",timeString,timeString2);*/
  printw("AllanVariance 1        2        3        4        5        6        7        8\n");
  move(1,0);
  /*
  printw("Antenna      1        2        3        4        5        6        7        8\n");
  move(2,0);
  */
  for (j=0; j<2; j++) {
    for (i=0; i<NUMBER_OF_TIME_BINS; i++) {
      move(1+i+j*13,0);
      switch (i) {
      case 0: printw("0.1 sec"); break;
      case 1: printw("0.2 sec"); break;
      case 2: printw("0.4 sec"); break;
      case 3: printw("0.8 sec"); break;
      case 4: printw("1.6 sec"); break;
      case 5: printw("3.2 sec"); break;
      case 6: printw("6.4 sec"); break;
      case 7: printw("12.8sec"); break;
      case 8: printw("25.6sec"); break;
      case 9: printw("51.2sec"); break;
      }
      
      for (antennaNumber=1; antennaNumber<=8; antennaNumber++) {
	if (!antsAvailable[antennaNumber]) {
	  printw("   ----- ");
	} else {
	  if (allan[antennaNumber][j][i] < 0) {
	    printw("   wacko ");
	  } else if (allan[antennaNumber][j][i] < VARIANCE_HIGHLIGHT_MIN) {
	    printw("   %.3f ",allan[antennaNumber][j][i]);
	  } else if (allan[antennaNumber][j][i] < 99.995) {
	    if (i < 9) {
	      standout();
	    }
	    if (allan[antennaNumber][j][i] < 10) {
	      printw("   %5.3f ",allan[antennaNumber][j][i]);
	    } else {
	      printw("   %5.2f ",allan[antennaNumber][j][i]);
	    }
	    standend();
	  } else if (allan[antennaNumber][j][i] < 999.95) {
	    standout();
	    printw("   %.1f ",allan[antennaNumber][j][i]);
	    standend();
	  } else {
	    printw("   wacko ");
	  }
	}
      }
      clrtoeol();
    }
  }
/*  clrtobot(); */
  move(11,0);

  printw("Tsys age");
  for (ant = 1; ant <= numberAntennas; ant++) {
    time_t tsysTime;
    if (antsAvailable[ant]) { 
      printw("  ");
      rm_status = rm_read(ant, "RM_UNHEATEDLOAD_TIMESTAMP_L",&tsysTime);
      if (doWeCare[ant]==1) {
	printAgeStandoutN(rightnow,tsysTime,(int)1);
      } else {
	printAgeNoStandout(rightnow,tsysTime);
      }
    } else {
      printw("  ----- ");
    }
  }
  clrtoeol();
  move(12,0);
  printw("Trx age   ");
  for (i=1; i<=8; i++) {
    time_t timestamp;
    rm_status = rm_read(i, "RM_TRX_TIMESTAMP_L", &timestamp);
    printAgeStandoutN(rightnow,timestamp,1);
    if (i<8) {
      printw("  ");
    }
  }
  clrtoeol();
  move(13,0);
  printw("High-frequency receiver (IF2):\n");
  refresh();
}

