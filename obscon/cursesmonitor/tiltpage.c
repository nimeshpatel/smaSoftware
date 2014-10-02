#define TILT_METER_DC_STANDOUT 60 /* arcsec */
#define WACKO_TEMPERATURE_HIGH 1500
#define WACKO_TEMPERATURE_LOW  -1500
#define DEBUG 0
#include <stdio.h>
#include <termio.h>
#include <sys/types.h>
#include <time.h>
#include <rm.h>
#include <string.h>
#include <math.h>
#include <curses.h>
#include <dirent.h>
#include "commonLib.h"
#include "monitor.h"
#define DO_FIT 0
#if DO_FIT
  #include "nr.h"
  #include "nrutil.h"
  void tiltfn(float x, float p[], int np);
  void tiltfitall(char *filename);
  void tiltfit(char *filename, int tilt_number);
#endif

extern int tiltCorrections[NUMANTS + 1];
extern int printAgeStandoutN(long rightnow, long longvalue, int agelevel);
/*
void printTiltScanAge(int antenna, int rightnow);
*/
static int newestTiltUnixTime[9] = {0,0,0,0,0,0,0,0,0};
static char tiltFilename[9][200] = {"","","","","","","","",""};
void LocateTilts(int targetAntenna);
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *t, char *str);
extern void checkDSMStatus(int status, char *string);
char tiltscanSense[9];
int tiltscanElevation[9];
int tiltDate[NUMANTS+1], tiltTime[NUMANTS+1];
long ConvertTiltDateTimeToUnixTime(int day,char month[5],int year,int time);
int ConvertToUnixTime(int trueyear,int month,int day,int hr,int min,int sec);
static int needToReadTiltFits[9] = {1,1,1,1,1,1,1,1,1};
int getTiltFitData(int i, short padid[], int day[], int hour[], int antarray[], 
		   int pad[], int meter[], int scandir[], 
		   float elev[], float sunaz[], float cabintemp[], float ambient[], 
		   float wind[], float winddir[], float dc[], float tilt[], float tiltdir[], 
		   float sin[], float cos[], int nofile[], float dc2[], float sin2[], 
		   float cos2[], float tilt2[]);

void tiltpage(int count, int *rm_list) {
  int rm_status, rms;
  int ant, antennaNumber;
  time_t system_time;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int doWeCare[11];
  long rightnow,timestamp;
  int unixTime;
  int i;
  double tiltx, tilty;
  float tiltVolts[NUMANTS+1][4];
  int antarray[NUMANTS+1];
  short padid[NUMANTS+1];
  float temp;
  int nofile[NUMANTS+1];
  int stale[NUMANTS+1];
  int day[NUMANTS+1], hour[NUMANTS+1], pad[NUMANTS+1], meter[NUMANTS+1];
  int scandir[NUMANTS+1];
  static int reduced[NUMANTS+1];
  static float elev[NUMANTS+1], sunaz[NUMANTS+1], cabintemp[NUMANTS+1];
  static float ambient[NUMANTS+1], dc[NUMANTS+1], tilt[NUMANTS+1], tiltdir[NUMANTS+1];
  static float sin[NUMANTS+1], cos[NUMANTS+1], sin2[NUMANTS+1], cos2[NUMANTS+1];
  static float dc2[NUMANTS+1], wind[NUMANTS+1], winddir[NUMANTS+1], tilt2[NUMANTS+1];
  char dummyByte;

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
#define TILT_SEARCH_INTERVAL 60
  if ((count % TILT_SEARCH_INTERVAL) == 1) {
    /* cause a new search for recent tilt data */
    for (i=0; i<=NUMANTS; i++) { 
      strcpy(tiltFilename[i],"");
    }
  }

  move(0,0);
  system_time = time(NULL);
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("Tilt meter status on %s UT = %s HST",timeString,timeString2);
  if (DEBUG) { 
    refresh(); 
  }
  move(1,0);
  printw("Antenna/Pad  ");
  for (ant=1; ant<=NUMANTS; ant++) {
    rms = rm_read(ant,"RM_TILT_CORRECTION_FLAG_L",&tiltCorrections[ant]);
    (void)rm_read(ant,"RM_PAD_ID_B",&dummyByte);
    padid[ant]=(short)dummyByte;
    if (tiltCorrections[ant] != 0) {
      printw("%d/%d",ant,padid[ant]);
      standout();
      printw("TltCor");
      standend();
      printw(" ");
    } else {
      printw("   %d/%2d ",ant,padid[ant]);
    }
  }

  if (DEBUG) { 
    refresh(); 
  }
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      stale[antennaNumber] = 1;
    } else {
      rms = rm_read(antennaNumber,"RM_TILT_TIMESTAMP_L",&timestamp);
      rms = rm_read(ant, "RM_UNIX_TIME_L", &unixTime);
#define PAST 1081149340
      if (timestamp+60 < unixTime || timestamp < PAST) {
	stale[antennaNumber] = 1;
      } else {
	stale[antennaNumber] = 0;
      }
    }
  }

  for (i=0; i<4; i++) {
    move(i+2,0);
    printw("Tilt[%d] Volt ",i);
    for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
      if (antsAvailable[antennaNumber] == 0) {
	printw(" ----- ");
      } else {
	if (i==0) {
	  rms = rm_read(antennaNumber,"RM_TILT_VOLTS_V4_F",&tiltVolts[antennaNumber]);
	}
	if (stale[antennaNumber]) {
	  printw("  stale ");
	} else {
	  if (tiltVolts[antennaNumber][i] != tiltVolts[antennaNumber][i]) {
	    printw("  wacko ");
	  } else {
	    printw("%+7.3f ",tiltVolts[antennaNumber][i]);
	  }
	}
      }
    }
    printw("\n");
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(6,0);
  /*  printw("TILT X arcsec");*/
  printw("F/A up arcsec");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      /*      rms = rm_read(antennaNumber,"RM_TILTX_ARCSEC_D",&tiltx);*/
      rms = rm_read(antennaNumber,"RM_AFT_FORWARD_TILT_UPPER_ARCSEC_D",&tilty);
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	printw("%+7.1f ",tiltx);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(7,0);
  /*  printw("TILT Y arcsec");*/
  printw("L/R up arcsec");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      /*      rms = rm_read(antennaNumber,"RM_TILTY_ARCSEC_D",&tilty);*/
      rms = rm_read(antennaNumber,"RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D",&tilty);
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	printw("%+7.1f ",tilty);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(8,0);
  printw("F/A lo arcsec");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_AFT_FOREWARD_TILT_LOWER_ARCSEC_D",&tiltx);
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	printw("%+7.1f ",tiltx);
      }
    }
  }
  move(9,0);
  printw("L/R lo arcsec");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_LEFT_RIGHT_TILT_LOWER_ARCSEC_D",&tiltx);
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	printw("%+7.1f ",tiltx);
      }
    }
  }


  if (DEBUG) { 
    refresh(); 
  }
  move(11,0);  
  printw("Lo Xelev Temp");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_TILT1_LO_XELEV_TEMPERATURE_F",&temp);
      /*      temp *= 100;  factor moved to logTilts */
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	if (temp > WACKO_TEMPERATURE_HIGH || temp < WACKO_TEMPERATURE_LOW) {
	  printw("  wacko ");
	} else if (temp != temp) {
	  printw("  wacko ");
	} else {
	  printw("%+7.1f ",temp);
	}
      }
    }
  }
  move(10,0);  
  printw("Lo Pelev Temp");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_TILT2_LO_PELEV_TEMPERATURE_F",&temp);
      /*      temp *= 100;  factor moved to logTilts */
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	if (temp > WACKO_TEMPERATURE_HIGH || temp < WACKO_TEMPERATURE_LOW) {
	  printw("  wacko ");
	} else if (temp != temp) {
	  printw("  wacko ");
	} else {
	  printw("%+7.1f ",temp);
	}
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(13,0);  
  printw("Hi Xelev Temp");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_TILT3_HI_XELEV_TEMPERATURE_F",&temp);
      /*      temp *= 100; factor moved to logTilts */
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	if (temp > WACKO_TEMPERATURE_HIGH || temp < WACKO_TEMPERATURE_LOW) {
	  printw("  wacko ");
	} else if (temp != temp) {
	  printw("  wacko ");
	} else {
	  if (antennaNumber == 4 || antennaNumber == 6) {
	    printw("%+7.1f ",temp);
	  } else {
	    printw("  n/a   ");
	  }
	}
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
  move(12,0);  
  printw("Hi Pelev Temp");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_TILT4_HI_PELEV_TEMPERATURE_F",&temp);
      /*      temp *= 100;  factor moved to logTilts */
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	if (temp > WACKO_TEMPERATURE_HIGH || temp < WACKO_TEMPERATURE_LOW) {
	  printw("  wacko ");
	} else if (temp != temp) {
	  printw("  wacko ");
	} else {
	  printw("%+7.1f ",temp);
	}
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
#if 0
  move(15,0);
  printw("Last tiltscan");
  for (i=1; i<=NUMANTS; i++) {
    printTiltScanAge(i,rightnow);
  }
  if (DEBUG) { 
    refresh(); 
  }
#endif
  move(14,0);
  printw("ScanElev/Sense");
  for (i=1; i<=NUMANTS; i++) {
    if (tiltscanSense[i] == 'c') {
      printw(" %2d/CCW ",tiltscanElevation[i]);
    } else {
      printw(" %2d/CW  ",tiltscanElevation[i]);
    }
  }
  if (DEBUG) { 
    refresh(); 
  }

  move(18,0);
  printw("TltMag1(asec)");
  for (i=1; i<=NUMANTS; i++) {
    if (needToReadTiltFits[i] == 1 && ((count % 10) == 1)) {
      /*
      (void)rm_read(i,"RM_PAD_ID_B",&dummyByte);
      padid[i] = (short)dummyByte;
      */
      reduced[i] = getTiltFitData(i,padid,day,hour,antarray, pad,meter,
				  scandir, elev,sunaz, cabintemp, ambient, wind,
				  winddir,dc, tilt, tiltdir, sin, cos, nofile,dc2,
				  sin2,cos2,tilt2);
#if DO_FIT
      refresh();
      if (reduced[i] == 0) {
	if (strlen(tiltFilename[i]) > 0) {
	  tiltfitall(tiltFilename[i]);
	  reduced[i] = getTiltFitData(i,padid,day,hour,antarray, pad,meter,
				      scandir, elev,sunaz, cabintemp, ambient, wind,
				      winddir,dc, tilt, tiltdir, sin, cos,nofile,
				      dc2,sin2,cos2,tilt2);
	}
      }
#endif
    } /* endif needToReadTiltFits */
    if (reduced[i] == 0) {
      printw(" Unreduc");
    } else {
      needToReadTiltFits[i] = 0;
      if (fabs(tilt[i]) > TILT_METER_DC_STANDOUT) {
	standout();
      }
      printw(" %+6.1f ",tilt[i]);
      standend();
    }
  } /* end of 'for' loop over antennas */ 


  move(16,0);
  printw("DCtilt1(asec)");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
	if (fabs(dc[antennaNumber]) > TILT_METER_DC_STANDOUT) {
	  standout();
	}
	printw("%+7.1f ",dc[antennaNumber]);
	standend();
  }
#if 0
  printw("RM_TILTX_DC arc");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_TILTX_DC_ARCSEC_D",&tiltx);
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	printw("%+7.1f ",tiltx);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
#endif
  move(17,0);
  printw("DCtilt2(asec)");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
	if (fabs(dc2[antennaNumber]) > TILT_METER_DC_STANDOUT) {
	  standout();
	}
	printw("%+7.1f ",dc2[antennaNumber]);
	standend();
  }
#if 0
  printw("RM_TILTY_DC arc");
  for (antennaNumber=1; antennaNumber<=NUMANTS; antennaNumber++) { 
    if (antsAvailable[antennaNumber] == 0) {
      printw(" ----- ");
    } else {
      rms = rm_read(antennaNumber,"RM_TILTY_DC_ARCSEC_D",&tilty);
      if (stale[antennaNumber]) {
	printw("  stale ");
      } else {
	printw("%+7.1f ",tilty);
      }
    }
  }
  if (DEBUG) { 
    refresh(); 
  }
#endif


#if 0
  move(15,0);
  printw("ScanAmbientTmp");
  for (i=1; i<=NUMANTS; i++) {
    if (nofile[i] == 1) {
      printw("        ");
    } else {
      if (i > 1) {
	printw(" ");
      }
      if (reduced[i] == 0) {
	printw("Unreduc");
      } else {
	printw(" %+5.1f ",ambient[i]);
      }
    }
  }
#endif

  move(19,0);
  printw("TltMag2(asec)");
  for (i=1; i<=NUMANTS; i++) {
    if (nofile[i] == 1) {
      printw("        ");
    } else {
      if (reduced[i] == 0) {
	printw(" Unreduc");
      } else {
	if (fabs(tilt2[i]) > TILT_METER_DC_STANDOUT) {
	  standout();
	}
	printw(" %+6.1f ",tilt2[i]);
	standend();
      }
    }
  }

  move(20,0);
  printw("sin (meter 1)");
  for (i=1; i<=NUMANTS; i++) {
    if (nofile[i] == 1) {
      printw("        ");
    } else {
      if (reduced[i] == 0) {
	printw(" Unreduc");
      } else {
	printw(" %+6.1f ",sin[i]);
      }
    }
  }

  move(21,0);
  printw("sin (meter 2)");
  for (i=1; i<=NUMANTS; i++) {
    if (nofile[i] == 1) {
      printw("        ");
    } else {
      if (reduced[i] == 0) {
	printw(" Unreduc");
      } else {
	printw(" %+6.1f ",sin2[i]);
      }
    }
  }

  move(22,0);
  printw("cos (meter 1)");
  for (i=1; i<=NUMANTS; i++) {
    if (nofile[i] == 1) {
      printw("        ");
    } else {
      if (reduced[i] == 0) {
	printw(" Unreduc");
      } else {
	printw(" %+6.1f ",cos[i]);
      }
    }
  }

  move(23,0);
  printw("cos (meter 2)");
  for (i=1; i<=NUMANTS; i++) {
    if (nofile[i] == 1) {
      printw("        ");
    } else {
      if (reduced[i] == 0) {
	printw(" Unreduc");
      } else {
	printw(" %+6.1f ",cos2[i]);
      }
    }
  }

  move(23,79);
  refresh();
}

void LocateTilts(int targetAntenna) {
  DIR *dirPtr;
  struct dirent *nextEnt;
  int day, time, antenna, elevation, narg;
  char sense;
  int file = 0;
  long longvalue;
  int year;
  char month[5];
  char TILT_DIR[50];

  sprintf(TILT_DIR, "/data/engineering/tilt/ant%d",targetAntenna);
  newestTiltUnixTime[targetAntenna] = 0;
  dirPtr = opendir(TILT_DIR);
  while ((nextEnt = readdir(dirPtr)) != NULL) {
    if (DEBUG) {
      move(1,0);
      printw("%d: %s\n",file,nextEnt->d_name);
      refresh();
    }
    file++;
      /* tilt_ant1_20040129_041026_el_75_cw */
    /*
    if (present(nextEnt->d_name, "tilt_ant") &&
	!present(nextEnt->d_name, ".bak") &&
	!present(nextEnt->d_name, ".ps")) {
      narg = sscanf(nextEnt->d_name,"tilt_ant%d_%d_%d_el_%d_c%c",&antenna,
		    &day,&time,&elevation,&sense);
    */
    if (present(nextEnt->d_name, ".tilt")) {
      narg= sscanf(nextEnt->d_name,"ant%d.azscan.%02d%3s%02d_%d.tilt",&antenna,
		    &day,month,&year,&time);
      if (antenna > 0 && antenna < 9 && narg==5) {
	if (antenna == targetAntenna) {
	  year += 2000;
	  longvalue = ConvertTiltDateTimeToUnixTime(day,month,year,time);
	  if (longvalue > newestTiltUnixTime[antenna]) {
	    newestTiltUnixTime[antenna] = longvalue;
	    strcpy(tiltFilename[antenna],nextEnt->d_name);
	    tiltscanSense[antenna] = sense;
	    tiltscanElevation[antenna] = elevation;
	  }
	}
      }
    }
  }
  closedir(dirPtr);
}

#define DAYSEC 86400

/*
void printTiltScanAge(int antenna, int rightnow) {
  int month,day,hr,min,narg,trueyear,sec;
  int y,x,elevation;
  char sense;
  long longvalue;

  if (strlen(tiltFilename[antenna]) < 1) {
    LocateTilts(antenna);
    needToReadTiltFits[antenna] = 1;
  }
  if (strlen(tiltFilename[antenna]) > 0) {
    narg = sscanf(tiltFilename[antenna],"tilt_ant%*d_%d_%d_el_%d_c%c",
		  &tiltDate[antenna],&tiltTime[antenna],&elevation,&sense);
    if (narg == 4) {
      longvalue = ConvertTiltDateTimeToUnixTime(tiltDate[antenna],tiltTime[antenna]);
      if (antenna == 1 && DEBUG) {
	getyx(stdscr,y,x);
	move(y-1,x-10);
	printw("%s: %d: %d %d %d %d %d %d",tiltFilename[antenna],
	       longvalue,trueyear,month,day,hr,min,sec);
	move(y,x);
      }
      printw(" ");
      printAgeStandoutN(rightnow,longvalue,3);    
    } else {
      printw("  wacko ");
    }
  } else {
    printw("no file");
  }
}
*/

int ConvertToUnixTime(int trueyear,int month,int day,int hr,int min,int sec) {
  int longvalue;
  longvalue = 1072915200; /* 2004-Jan-01 00:00 */
  longvalue += DAYSEC*365*(trueyear-2004);
  longvalue += DAYSEC*floor((trueyear-2001)/4);  /* past leap years */
  if (month > 1) longvalue += 31*DAYSEC;
  if (month > 2) {
    longvalue += 28*DAYSEC;
    if ((trueyear % 4) == 0) {
      longvalue += DAYSEC;  /* present leap year */
    }
  }
  if (month > 3) longvalue += 31*DAYSEC;
  if (month > 4) longvalue += 30*DAYSEC;
  if (month > 5) longvalue += 31*DAYSEC;
  if (month > 6) longvalue += 30*DAYSEC;
  if (month > 7) longvalue += 31*DAYSEC;
  if (month > 8) longvalue += 31*DAYSEC;
  if (month > 9) longvalue += 30*DAYSEC;
  if (month > 10) longvalue += 31*DAYSEC;
  if (month > 11) longvalue += 30*DAYSEC;
  longvalue += DAYSEC*(day-1);
  longvalue += hr*3600;
  longvalue += min*60;
  longvalue += sec;
  return(longvalue);
}

long ConvertTiltDateTimeToUnixTime(int day, char month[5], int year, int time){
  return(0);
}

/*  old-style
long ConvertTiltDateTimeToUnixTime(int date,int time) {
  int trueyear,year,month,day,hr,min,sec;
  long longvalue;

  year = date/10000;
  trueyear = year;
  month = date/100 - year*100;
  day = date-year*10000-month*100;
  hr = time/10000;
  min = (time-hr*10000)/100;
  sec = (time-hr*10000-min*100);
  longvalue = ConvertToUnixTime(trueyear,month,day,hr,min,sec);
  return(longvalue);
}
*/

#if DO_FIT
#define NP 7
#define MAX 20000
float           az[MAX], el[MAX], del[MAX], daz[MAX];
float           tiltarray[7][MAX];

void tiltfitall(char *filename) {
  char fullfilename[200];
  sprintf(fullfilename,"/data/engineering/tilt/%s",filename);
  tiltfit(fullfilename,1);
  tiltfit(fullfilename,2);
  tiltfit(fullfilename,3);
  tiltfit(fullfilename,4);
}

void tiltfit(char *filename, int tilt_number) {
#define NAMELEN 100
    char outfile[NAMELEN];
    int             i;
    float           chisq, *x, *y, *sig, *a, *w, **cvm, **u, **v, *elres;
    int             npt;
    FILE           *fpin, *fpout;
    char           time[20];
    char           startTime[20];
    float           theta, radian, tilt_minus_dc, theoretical, residual;
    static float    pi;
    float utc;
    float magnitude, angle ;
    float sunaz,sunel,cabintemp,ambienttemp,windspeed,winddir;
    int antenna, pad;
    int scandirection;

    pi = 4.0 * atan(1.0);
    radian = pi / 180.;
    fpin = fopen(filename, "r");
    if (fpin == NULL) {
      printw("Could not open input datafile = %s\n",filename);
      return;
    }
    i = 1;

    while (fscanf(fpin, "%s %d %d %f %f %f %f %f %f %f %f %f %f %f %f %f",
		  time, &antenna, &pad, &utc,&az[i], &el[i], &tiltarray[1][i], 
		  &tiltarray[2][i],&tiltarray[3][i],&tiltarray[4][i],&sunaz,
		  &sunel,&cabintemp,&ambienttemp,
		  &windspeed,&winddir) != EOF)
    {
       if (i==1) {
	 strcpy(startTime,time);
       }
       i++;
    }
    fclose(fpin);
    npt = i - 1;
    x = vector(1, npt);
    y = vector(1, npt);
    sig = vector(1, npt);
    elres = vector(1, npt);
    a = vector(1, NP);
    w = vector(1, NP);
    cvm = matrix(1, NP, 1, NP);
    u = matrix(1, npt, 1, NP);
    v = matrix(1, NP, 1, NP);

    for (i = 1; i <= npt; i++)
    {
	x[i] = (float) i;
	y[i] = (float) tiltarray[tilt_number][i];
    }
    svdfit(x, y, sig, npt, a, NP, u, v, w, &chisq, tiltfn);
    svdvar(v, NP, w, cvm);
    magnitude =  (float)pow(((double)a[2]*(double)a[2]+(double)a[3]*(double)a[3]),0.5);
    angle=(float)atan2((double)a[2],(double)a[3]);
    angle=angle/radian;

    for (i = 1; i <= npt; i++)    {
	theta = az[i] * radian;
	tilt_minus_dc = tiltarray[tilt_number][i] - a[1];
	theoretical = a[2] * sin(theta) + a[3] * cos(theta) + a[4] * sin(2 * theta) + 
	  a[5] * cos(2 * theta) + a[6] * sin(3 * theta) + a[7] * cos(3 * theta);
	residual = tilt_minus_dc - theoretical;
    }

    fclose(fpin);
    sprintf(outfile,"/data/engineering/tilt/antenna%d/tilt.ant%d.pad%d.summary",
	    antenna,antenna,pad);
    fpout = fopen(outfile,"a");
    if (fpout == NULL) return;

    if (strstr(filename,"ccw") == NULL) {
      scandirection = +1;
    } else {
      scandirection = -1;
    }
    for (i=0; i<strlen(startTime); i++) {
      if (startTime[i] == '_') {
	startTime[i] = ' ';
      }
    }
    fprintf(fpout,"%s %1d %2d %1d %+1d %4.1f %+5.1f %+4.1f %+4.1f %4.1f %3.0f %+5.1f %+6.1f %+6.1f %+6.1f %+6.1f\n", 
	    startTime, antenna, pad, tilt_number, scandirection, el[1],
	    sunaz, cabintemp, ambienttemp, windspeed, winddir, magnitude,
	    angle, a[1], a[2], a[3]);
    fclose(fpout);
    free_matrix(v, 1, NP, 1, NP);
    free_matrix(u, 1, npt, 1, NP);
    free_matrix(cvm, 1, NP, 1, NP);
    free_vector(w, 1, NP);
    free_vector(a, 1, NP);
}


void tiltfn(float x, float p[], int np) {
	int i;
	static float pi;
	float radian;

	pi=4.0*atan(1.0);
	radian=pi/180.;

	i=(int)x;
	p[1]=1.0;
	p[2]=sin(az[i]*radian);
	p[3]=cos(az[i]*radian);
	p[4]=sin(2.0*az[i]*radian);
	p[5]=cos(2.0*az[i]*radian);
	p[6]=sin(3.0*az[i]*radian);
	p[7]=cos(3.0*az[i]*radian);
}
#endif


/* i is the antenna number */
int getTiltFitData(int i, short padid[], int day[], int hour[], int antarray[], 
		   int pad[], int meter[], int scandir[], 
		   float elev[], float sunaz[], float cabintemp[], float ambient[], 
		   float wind[], float winddir[], float dc[], float tilt[], float tiltdir[], 
		   float sin[], float cos[], int nofile[], float dc2[], float sin2[], 
		   float cos2[], float tilt2[]) {
  char filename[150];
  char lastbuf[150], lastbuf2[150];
  char buf[150];
  char *ptr;
  FILE *fp;
  int reduced;

  /* now look for the file that will hold the fitted results */
  sprintf(filename,"/data/engineering/tilt/antenna%d/tilt.ant%d.pad%d.summary",i,i,padid[i]);
  fp = fopen(filename,"r");
  if (fp == NULL) {
    printw(" no file");
    nofile[i] = 1;
    reduced = 0;
    return(reduced);
  }
  nofile[i] = 0;
  strcpy(lastbuf,"");
  strcpy(lastbuf2,"");
  ptr = fgets(buf,sizeof(buf),fp);
  /* I should really find the most recent scan, not just the one 
   * at the bottom of the file */
  while (ptr != NULL) {
    if (ptr != NULL) {
      sscanf(buf,"%d %d %*d %*d %d %*d %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f %*f",
	     &day[0], &hour[0], &meter[0]);
      if (day[0] == tiltDate[i] && hour[0] == tiltTime[i]) {
	switch (meter[0]) {
	case 1:
	  strcpy(lastbuf,buf);
	  break;
	case 2:
	  strcpy(lastbuf2,buf);
	  break;
	}
	if (strlen(lastbuf)>0 && strlen(lastbuf2)>0) break; /* out of the while loop */
      }
    }
    ptr = fgets(buf,sizeof(buf),fp);
  }
  fclose(fp);
  reduced = 0;
  if (strlen(lastbuf) > 0) {
    sscanf(lastbuf,"%d %d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f",
	   &day[i], &hour[i], &antarray[i], &pad[i],&meter[i],&scandir[i],
	   &elev[i], &sunaz[i], &cabintemp[i], &ambient[i], &wind[i],
	   &winddir[i], &tilt[i], &tiltdir[i], &dc[i], &sin[i], &cos[i]);
    reduced = 1;
  }
  if (strlen(lastbuf2) > 0) {
    sscanf(lastbuf2,
	   "%*d %*d %*d %*d %*d %*d %*f %*f %*f %*f %*f %*f %f %*f %f %f %f",
	   &tilt2[i], &dc2[i], &sin2[i], &cos2[i]);
  }
  return(reduced);
}
