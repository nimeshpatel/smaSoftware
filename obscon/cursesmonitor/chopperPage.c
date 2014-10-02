#include <curses.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include <math.h>
#include <rm.h>
#include "monitor.h"
#include "chopperControl.h"

#define NANTS 8
#define NAXES 4

extern int chopperUnits;

char statusBitsv[] =	"RM_CHOPPER_STATUS_BITS_V16_B";
char monTimestampv[] =	"RM_CHOPPER_MONITOR_TIMESTAMP_L";
char posMmv[] =		"RM_CHOPPER_POS_MM_V4_F";
char farPosMmv[] = 	"RM_CHOPPER_FAR_POS_MM_V4_F";
char homeChkv[] = 	"RM_CHOPPER_HOME_CHECK_RESULTS_V3_S";
char homeChkTimev[] =	"RM_CHOPPER_HOME_CHECK_TIME_L";
char pmacResetTimev[] =	"RM_CHOPPER_PMAC_RESET_TIME_L";
char unixTimev[] =	"RM_UNIX_TIME_L";
char rlab[][16] = {
  "X rel far  ",
  "Y rel far      ",
  "Z rel far      ",
  "Tilt rel far   ",
  "X rel home ",
  "Y rel home     ",
  "Z rel home     ",
  "Tilt rel hme   ",
  "X home chk(cnt)",
  "Y home chk(cnt)",
  "Z home chk(cnt)",
  "HomeChk time   ",
  "Reset time     ",
  "Home Status    ",
  "XYZ Status     ",
  "Tilt Status    ",
  "PMAC error XYZT",
  "Command        ",
  "Homing Steps   "
};
#define RLABSIZE (sizeof(rlab) / sizeof(rlab[0]))

char homeSteps[][8] = {
  "Stop Ch",
  "Z->NLmt",
  "Y->NLmt",
  "X->NLmt",
  " Home Z",
  "X+=1000",
  " Home Y",
  "Home X",
  "WaitStp",
  "HmeTilt"
};
#define NSTEPS (sizeof(homeSteps) / sizeof(homeSteps[0]))


char homeStateStringes2[][4] = {" OK", " NO", "   ", "ERR", "MOV"};
char xyzorTiltStateStrings[][4] =  {" OK", "MOV", " CH", "ERR", "OL", "STOW"};
char cmdStrings[][8] =  {"   None", "   Home", "   Stow", "Abs Mov", "Rel Mov",
			"Abs Tlt", "Abs tlt", "Strt Ch", "Stop Ch"};
const double mm2cnt[4] = {X_COUNTS_PER_MM, Y_COUNTS_PER_MM, Z_COUNTS_PER_MM,
	TILT_COUNTS_PER_ARCSEC};
char posFmt[][7] = {"%7.3f ", "%7.3f ", "%7.3f ", "%7.2f "};
char posFmt8[][7] = {"%7.3f", "%7.3f", "%7.3f ", "%7.2f"};

extern char *toddtime(time_t *, char *str);
extern char *hsttime(time_t *, char *str);

/* chopperPage.c */
void deltaTime(long sec, char *asciiTime);

void chopperPage(int count, int *rm_list) {
  int rm_status;
  int i, j, ant, row;
  float posMm[NAXES];
  float farMm[NAXES];
  unsigned char statusBits[16];
  int useAnt[NANTS + 1];
  short sarray[NAXES];
  long longvalue;
  short shortvalue, shortvalue2;
  float floatvalue;
  long rightnow, timevalue;
  time_t timestamp;
  time_t system_time;
  int doWeCare[11];
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  char *cp;

  if ((count % 30) == 1) {
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
  for(ant = 1, i = 0; ant <= NANTS; ant++) {
    if(rm_list[i] > ant) {
      useAnt[ant] = 0;
    } else {
      useAnt[ant] = 1;
      i++;
    }
  }
  rm_status = rm_read(rm_list[0], unixTimev, &rightnow);
  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("  Chopper Status on %s UT = %s HST",timeString,timeString2);
  move(1,0);
  printw("Antenna             1       2       3       4       5       6       7       8\n");

  row = 3;
  for(i = 0; i < RLABSIZE; i++) {
    mvprintw(row + i, 0, rlab[i]);
  }
  for (ant=1; ant<=8; ant++) {
    row = 3;
    rm_status = rm_read(ant, monTimestampv, &longvalue);
    if (!useAnt[ant] || abs(longvalue - rightnow) > 4) {
      char *badmsg;

      badmsg = (useAnt[ant])? "  stale ": "  ----- ";
      for(i = 0; i < RLABSIZE; i++) {
        mvprintw(row + i, 8 + ant * 8, badmsg);
      }
    } else {
      rm_status = rm_read(ant, posMmv, posMm);
      rm_status = rm_read(ant, farPosMmv, farMm);
      for(i = 0; i < 4; i++) {
	if(chopperUnits) {
	  mvprintw(row + i, 8 + ant * 8, (ant == 8)? "%7d": "%7d ",
		  (int)floor((double)(posMm[i] - farMm[i]) * mm2cnt[i] + 0.5));
	  mvprintw(row + 4 + i, 8 + ant * 8, (ant == 8)? "%7d": "%7d ",
		  (int)floor(posMm[i] * mm2cnt[i] + 0.5));
	} else {
	  mvprintw(row + i, 8 + ant * 8, (ant == 8)? posFmt8[i]: posFmt[i],
		(double)(posMm[i] - farMm[i]));
	  mvprintw(row + 4 + i, 8 + ant * 8, (ant == 8)? posFmt8[i]: posFmt[i],
		(double)(posMm[i]));
	}
      }
      row = 11;
      rm_status = rm_read(ant, homeChkv, sarray);
      for(i = 0; i < 3; i++) {
        mvprintw(row + i, 8 + ant * 8, (ant == 8)? "%7d": "%7d ", sarray[i]);
      }
      rm_status = rm_read(ant, homeChkTimev, &longvalue);
      deltaTime(rightnow - longvalue, timeString);
      mvprintw(14, 8 + ant * 8, (ant == 8)? "%7s": "%7s ", timeString);
      rm_status = rm_read(ant, pmacResetTimev, &longvalue);
      if(rm_status != RM_SUCCESS) longvalue = 0;
      deltaTime(rightnow - longvalue, timeString);
      mvprintw(15, 8 + ant * 8, (ant == 8)? "%7s": "%7s ", timeString);
      rm_status = rm_read(ant, statusBitsv, statusBits);
      mvprintw(16, 8 + ant * 8, (ant == 8)? "    %3s": "    %3s ",
		homeStateStringes2[statusBits[P0]]);
      mvprintw(17, 8 + ant * 8, (ant == 8)? "    %3s": "    %3s ",
		xyzorTiltStateStrings[statusBits[P3]]);
      mvprintw(18, 8 + ant * 8, (ant == 8)? "    %3s": "    %3s ",
		xyzorTiltStateStrings[statusBits[P20]]);
      for(i = 0; i < 4; i++) {
        if(statusBits[X_MOTOR_STATUS + i] == 0) {
	  timeString[i] = '_';
	} else {
	  static char errs[] = "OPHF-+";

	  for(j = 5; j >= 0; j--) {
	    if(statusBits[X_MOTOR_STATUS + i] & (1 << j) ) {
	      timeString[i] = errs[j];
	      break;
	    }
	  }
	}
      }
      timeString[4] = 0;
      mvprintw(19, 8 + ant * 8, (ant == 8)? "   %4s": "   %4s ",timeString);
      cp = cmdStrings[(i = statusBits[P2])];
      if(i == 0 && (j = statusBits[UPDATE_STATUS]) != 0) {
	if(j == 255) cp = "Freeze";
	else if(j == 1) cp = "F Curve";
      }
      mvprintw(20, 8 + ant * 8, (ant == 8)? "%7s": "%7s ", cp);
      i = statusBits[P1];
      if((statusBits[P0] > 2) && (i >= 0) && (i < NSTEPS)) {
        mvprintw(21, 8 + ant * 8, (ant == 8)? "%7s": "%7s ", homeSteps[i]);
      } else {
        mvprintw(21, 8 + ant * 8, (ant == 8)? "       ": "        ");
      }
    }
  }
  mvprintw(23, 0, (chopperUnits)? "counts ('+' for mm and arc sec) %s":
	"mm and arc sec ('+' for counts) %s",
	"PMAC errs (-+)lim (F)ollow (H)ome (P)os (O)penL\n");
  refresh();
}

void deltaTime(long sec, char *asciiTime) {

  if((sec > (86400 * 10)) || (sec < 0)) {
    strcpy(asciiTime, "  stale");
  } else if(sec < 3600) {
    int minutes, seconds;

    minutes = sec / 60;
    seconds = sec - minutes * 60;
    sprintf(asciiTime, "%02dm %02ds", minutes, seconds); 
  }else if(sec < 86400) {
    int hours, minutes;

    hours = sec / 3600;
    minutes = (sec - (hours * 3600)) / 60;
    sprintf(asciiTime, "%02dh %02dm", hours, minutes);
  } else {
    int days, hours;

    days = sec / 86400;
    hours = (sec - (days * 86400)) / 3600;
    sprintf(asciiTime, "% 1dd %02dh", days, hours);
  }
}
