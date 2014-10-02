#define USE_VLBA_IN_AVERAGE 0
#define SKYDIP_DIR "/data/engineering/dip_scan"
#define PWV_MAX 50
#define PWV_MIN 0
#define DEBUG 0
#define DSM_HOST "colossus"
#include <curses.h>
#include <stdio.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <dirent.h>
#include <curses.h>
#include <math.h>
#include <rm.h>
#include <dsm.h>
#include "weather.h"
#include "upspage.h"
#include "monitor.h"

#define MAX_SKYDIP_POINTS 1000
#define STALE_INTERVAL 180

void PrintSkydip(int ant, int band);
void CallLinefit(int npts, float *x, float *y, float *slope, float *intercept,
  float vhot);
float skydip(char *filename, int band);
extern void printDayOfWeek(int day);
extern void printYear(int year, int days);
int distanceToForecast(int i, int now);
void printDistanceToForecast(int i, int now);
extern float STANDOUT_TAU(float freqHz);
extern float findMedian(float *, int);
extern int weatherUnits;
extern int printAge(long rightnow, long longvalue);
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *t, char *str);
extern void checkDSMStatus(int status, char *string);
void printForecastTime2(float,int);
void LocateSkydips(void);
static char skydipFilename[200] = {""};
static float smatau[9] = {-2,-2,-2,-2,-2,-2,-2,-2,-2};
static float smatauHighFreq[9] = {-2,-2,-2,-2,-2,-2,-2,-2,-2};
char server[8];
char temperatureServer[8];
extern dsm_structure microdustStructure;


char pressureServer[8];
char humidityServer[8];
char windServer[8];
char dust[12];
long dust_timestamp;
float pwv[PWVFIELDS];
float floatvalue;
int s;


void weatherPage2(int count, int *rm_list) {
  int rm_status, dsm_status;
  time_t system_time; //, distanceToClosestForecast;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int i;
  float tau,freq,eta,tatm;
  long longvalue;
  long rightnow,timestamp;
  double restFrequency;

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
  dsm_status = call_dsm_read("m5", "DSM_AS_IFLO_REST_FR_D", (char *)&restFrequency, &timestamp);
#define SKYDIP_SEARCH_INTERVAL 60
  if ((count % SKYDIP_SEARCH_INTERVAL) == 1) {
    /* cause a new search for recent skydip data */
    strcpy(skydipFilename,"");
    for (i=0; i<9; i++) { 
      smatau[i] = -2; 
      smatauHighFreq[i] = -2; 
    }
  }
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
 
  dsm_status = call_dsm_read("colossus", "DSM_MICRODUST_X", (char *)&microdustStructure, &dust_timestamp);
   system_time = time(NULL);
  rightnow = system_time;
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
//  printw("Weather: %s UT = %s HT = %d",timeString,timeString2,rightnow);
  move(0,0);  
#if DEBUG
  refresh();
#endif

FILE *ifpw;
char R1C1[6], R1C3[6], R1C5[6], R1C6[6], R1C8[6], R1C9[6];
char R2C1[6], R2C3[6], R2C5[6], R2C6[6], R2C8[6], R2C9[6];
char R3C1[6], R3C3[6], R3C5[6], R3C6[6], R3C8[6], R3C9[6];
char R4C1[6], R4C3[6], R4C5[6], R4C6[6], R4C8[6], R4C9[6];
char R6C1[6], R6C3[6], R6C5[6], R6C6[6], R6C8[6], R6C9[6];
char R7C3[5], R7C4[5], R8C3[5], R8C4[5], R9C3[5], R9C4[4];
int  R11C1;
float  R1C2, R1C4, R1C7, R1C10;	
float  R2C2, R2C4, R2C7, R2C10;	
float  R3C2, R3C4, R3C7, R3C10;	
float  R4C2, R4C4, R4C7, R4C10;	
float  R5C2, R5C4, R5C7, R5C10;	
float  R6C2, R6C4, R6C7, R6C10;	
float  R7C1, R7C2, R8C1, R8C2, R9C1, R9C2, R10C1, R10C2;	

//float current, average, maximum;
float current;
float average;
float maximum;

  ifpw = fopen("/tmp/archive.txt","r");
  fscanf(ifpw, "%s", R1C1);
  fscanf(ifpw, "%f", &R1C2);
  fscanf(ifpw, "%s", R1C3);
  fscanf(ifpw, "%f", &R1C4);
  fscanf(ifpw, "%s", R1C5);
  fscanf(ifpw, "%s", R1C6);
  fscanf(ifpw, "%f", &R1C7);
  fscanf(ifpw, "%s", R1C8);
  fscanf(ifpw, "%s", R1C9);
  fscanf(ifpw, "%f", &R1C10);
  fscanf(ifpw, "%s", R2C1);
  fscanf(ifpw, "%f", &R2C2);
  fscanf(ifpw, "%s", R2C3);
  fscanf(ifpw, "%f", &R2C4);
  fscanf(ifpw, "%s", R2C5);
  fscanf(ifpw, "%s", R2C6);
  fscanf(ifpw, "%f", &R2C7);
  fscanf(ifpw, "%s", R2C8);
  fscanf(ifpw, "%s", R2C9);
  fscanf(ifpw, "%f", &R2C10);
  fscanf(ifpw, "%s", R3C1);
  fscanf(ifpw, "%f", &R3C2);
  fscanf(ifpw, "%s", R3C3);
  fscanf(ifpw, "%f", &R3C4);
  fscanf(ifpw, "%s", R3C5);
  fscanf(ifpw, "%s", R3C6);
  fscanf(ifpw, "%f", &R3C7);
  fscanf(ifpw, "%s", R3C8);
  fscanf(ifpw, "%s", R3C9);
  fscanf(ifpw, "%f", &R3C10);
  fscanf(ifpw, "%s", R4C1);
  fscanf(ifpw, "%f", &R4C2);
  fscanf(ifpw, "%s", R4C3);
  fscanf(ifpw, "%f", &R4C4);
  fscanf(ifpw, "%s", R4C5);
  fscanf(ifpw, "%s", R4C6);
  fscanf(ifpw, "%f", &R4C7);
  fscanf(ifpw, "%s", R4C8);
  fscanf(ifpw, "%s", R4C9);
  fscanf(ifpw, "%f", &R4C10);
  fscanf(ifpw, "%f", &R5C2);
  fscanf(ifpw, "%f", &R5C4);
  fscanf(ifpw, "%f", &R5C7);
  fscanf(ifpw, "%f", &R5C10);
  fscanf(ifpw, "%s", R6C1);
  fscanf(ifpw, "%f", &R6C2);
  fscanf(ifpw, "%s", R6C3);
  fscanf(ifpw, "%f", &R6C4);
  fscanf(ifpw, "%s", R6C5);
  fscanf(ifpw, "%s", R6C6);
  fscanf(ifpw, "%f", &R6C7);
  fscanf(ifpw, "%s", R6C8);
  fscanf(ifpw, "%s", R6C9);
  fscanf(ifpw, "%f", &R6C10);  

  fscanf(ifpw, "%f", &R7C1); // wind chill
  fscanf(ifpw, "%f", &R8C1); // heat index 
  fscanf(ifpw, "%f", &R9C1); // dew point
  fscanf(ifpw, "%f", &R10C1);// peak ws m/s
  fscanf(ifpw, "%f", &R7C2); // rain daily
  fscanf(ifpw, "%f", &R8C2); // rain mo
  fscanf(ifpw, "%f", &R9C2); // rain term
  fscanf(ifpw, "%s", R7C3);  // 2 min past
  fscanf(ifpw, "%s", R8C3);  // prev min
  fscanf(ifpw, "%s", R9C3);  // curr min
  fscanf(ifpw, "%s", R7C4);  // 6 min avg
  fscanf(ifpw, "%s", R8C4);  // max hr
  fscanf(ifpw, "%s", R9C4);  // min of max
  fscanf(ifpw, "%d", &R11C1); // SAS
  fclose (ifpw);
   
  dsm_status = dsm_structure_get_element(&microdustStructure, "DUST_F", (char *)&floatvalue);
   if (dsm_status != DSM_SUCCESS) {
     dsm_error_message(dsm_status,"dsm_structure_get_element(DUST_F)");
   }
	current = floatvalue;
	
  dsm_status = dsm_structure_get_element(&microdustStructure, "AVG_DUST_F", (char *)&floatvalue);
   if (dsm_status != DSM_SUCCESS) {
     dsm_error_message(dsm_status,"dsm_structure_get_element(AVG_DUST_F)");
   }
	average = floatvalue;
	
  dsm_status = dsm_structure_get_element(&microdustStructure, "MAX_DUST_F", (char *)&floatvalue);
   if (dsm_status != DSM_SUCCESS) {
     dsm_error_message(dsm_status,"dsm_structure_get_element(MAX_DUST_F)");
   }
	maximum = floatvalue;

  dsm_status = dsm_structure_get_element(&microdustStructure, "SERVER_TIMESTAMP_L", (char *)&longvalue);
   if (dsm_status != DSM_SUCCESS) {
     dsm_error_message(dsm_status,"dsm_structure_get_element(MAX_DUST_F)");
   }
	dust_timestamp = longvalue;
// move(12,0);  
//	printw("Dust Time Stamp = %d Now = %d",dust_timestamp, rightnow);  

  R10C2 = R10C1 * 2.2369;
  move(0,0);
  printw("             Daily Max      Daily Min         Term  Max            Term  Min");
  move(1,0);
  printw("Temp (C)    %s  %+3.1f    %s  %+3.1f    %s %s %+3.1f    %s %s %+3.1f", R1C1, R1C2, R1C3, R1C4, R1C5, R1C6, R1C7, R1C8, R1C9, R1C10);
  move(2,0);
  printw("Humidity    %s %4.1f%%    %s %4.1f%%    %s %s %4.1f%%   %s %s %4.1f%%", R2C1, R2C2, R2C3, R2C4, R2C5, R2C6, R2C7, R2C8, R2C9, R2C10);
  move(3,0);
  printw("Wind (m/s)  %s %5.1f    %s %5.1f    %s %s %5.1f    %s %s %5.1f", R4C1, R4C2, R4C3, R4C4, R4C5, R4C6, R4C7, R4C8, R4C9, R4C10);
  move(4,0);
  printw("Direction         %4.1f          %4.1f                %4.1f                %4.1f", R5C2, R5C4, R5C7, R5C10);
  move(6,0);
  printw("Calculated Values");
  move(7,0);
  printw("Wind Chill (C)   %+3.1f", R7C1);
  move(8,0);
  printw("Heat Index (C)   %+3.1f", R8C1);
  move(9,0);
  printw("Dew Point  (C)   %+3.1f", R9C1);
  move(5,0);
  if (R10C2 > WIND_SPEED_STANDOUT) {
  	printw("Peak Wind Spd (last hour) ");
  	standout();
  	printw("%3.1f",R10C1);
  	standend();
  	printw(" (m/s), ");
   	standout();
  	printw("%3.1f",R10C2);
  	standend();
  	printw(" (mph)");
  } else {
   	printw("Peak Wind Spd (last hour)  %3.1f (m/s), %3.1f (mph)", R10C1, R10C2);
  }
  move(11,0);
// 	R11C1 = 4; //for debugging
  printw("SAS Alert Level ");
  
 	switch(R11C1)
		{
		case 0:
			printw("0: There are no lightning storm alerts at this time.");
			break;
		case 1:
			printw("1: Some lightning activity is indicated in the vicinity of \n");
			printw("the Mauna Kea summit. If activity increases more warnings will follow.");
			break;
		case 2:
			printw("2: Moderate lightning activity is indicated at or near");
			printw("the Mauna Kea summit. If activity increases more warnings will follow.");
			break;
		case 3:
			standout();
			printw("3: There is now heavy lightning activity in the vicinity\n");
			printw("of the Mauna Kea summit. Please take necessary precautions.");
			standend();
			break;
		case 4:
			standout();
			printw("4: Warning! Severe lightning activity is currently taking\n");
			printw("place at the Mauna Kea summit. Take necessary precautions.");
			standend();
			break;
		}
   
  move(10,0);
	if (abs(dust_timestamp - rightnow) > STALE_INTERVAL) {
		printw("Dust Concentration (mg/m^3)  current: ");
		standout();
		printw("stale");
		standend();
		printw(" 1-min avg: ");
		standout();
		printw("stale");
		standend();
		printw(" 1-min max: ");
		standout();
		printw("stale");
		standend();	
		} else {
		printw("Dust Concentration (mg/m^3)  current: %5.3f 1-min avg: %5.3f 1-min max: %5.3f", current, average, maximum);
		}
	move(6,24);
  printw("Rain Totals");
  move(7,24);
  printw("Daily      %3.1f mm", R7C2);
  move(8, 24);
  printw("Monthly   %3.1f mm", R8C2);
  move(9, 24);
  printw("Rate       %3.1f mm", R9C2);

  move(6,45);
  printw("Lightning Strikes");  
  move(7,45);
  printw("2nd min past %s", R7C3);
  move(8,45);
  printw("Prev min     %s", R8C3);
  move(9,45);
  printw("Current min  %s", R9C3);
  move(7,63);
  printw("6 min avg    %s", R7C4);
  move(8,63);
  printw("Max last hr  %s", R8C4);
  move(9,63);
  printw("Min of Max   %s", R9C4);
 
  move(13,0);
  printw("Antenna         1      2      3      4      5      6      7      8");
  move(14,0);
  printw("DipfitTauLow ");
#define N_ANTS 8
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_DIPFIT_TAU_ZENITH_F", &tau);
    if (fabs(tau)<10) {
      printw(" %5.3f ",tau);
    } else {
      printw(" wacko ");
    }
  }
  move(15,0);
  printw("DpftTatmLow  ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_DIPFIT_TATMOSPHERE_F", &tatm);
    if (fabs(tatm)<340) {
      printw(" %5.1f ",tatm);
    } else {
      printw(" wacko ");
    }
  }
  move(16,0);
  printw("DipfitEtaLow ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_DIPFIT_ETA_SPILLOVER_F", &eta);
    if (fabs(eta) < 10 && eta > 0) {
      printw(" %5.3f ",eta);
    } else {
      printw(" wacko ");
    }
  }
  move(17,0);
  printw("DpftFreqLow  ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_DIPFIT_FREQUENCY_F", &freq);
    if (freq < 1000 && freq > 0) {
      printw(" %5.1f ",freq);
    } else {
      printw(" wacko ");
    }
  }
  move(18,0);
  printw("DipfitAgeLow ");
  for (i=1; i<=N_ANTS; i++) {
    if (0)
      /* Note: the following RM variable does not exist, so I'm setting the
	 value to 0, so that the results will be predictable */
      rm_status = rm_read(i, "RM_DIPFIT_IF2_TAU_ZENITH_TIMESTAMP_L", &timestamp);
    else
      timestamp = 0;
    printAge(rightnow,timestamp);
  }

  move(19,0);
  printw("DpftTauHigh  ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_IF2_DIPFIT_TAU_ZENITH_F", &tau);
    if (fabs(tau)<10) {
      printw(" %5.3f ",tau);
    } else {
      printw(" wacko ");
    }
  }
  move(20,0);
  printw("DpftTatmHigh ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_DIPFIT_TATMOSPHERE_F", &tatm);
    if (fabs(tatm)<340) {
      printw(" %5.1f ",tatm);
    } else {
      printw(" wacko ");
    }
  }
  move(21,0);
  printw("DpftEtaHigh  ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_IF2_DIPFIT_ETA_SPILLOVER_F", &eta);
    if (eta < 10 && eta>0) {
      printw(" %5.3f ",eta);
    } else {
      printw(" wacko ");
    }
  }
  move(22,0);
  printw("DpftFreqHigh ");
  for (i=1; i<=N_ANTS; i++) {
    rm_status = rm_read(i, "RM_DIPFIT_IF2_FREQUENCY_F", &freq);
    if (freq < 1000 && freq > 0) {
      printw(" %5.1f ",freq);
    } else {
      printw(" wacko ");
    }
  }
  move(23,0);
  printw("DpftAgeHigh  ");
  for (i=1; i<=N_ANTS; i++) {
    if (0)
      /* Note: the following RM variable does not exist, so I am setting
	 the value to 0 in order to have predictable results */
      rm_status = rm_read(i, "RM_DIPFIT_IF2_TAU_ZENITH_TIMESTAMP_L", &timestamp);
    else
      timestamp = 0;
    printAge(rightnow,timestamp);
  }
  refresh();
}

void printForecastTime2(float t, int space) {
  float secondsSince2003;
  float days, hours;
  int feb, year;
  int dayOfWeek;
  int daysInYear;
  int todd;
  char m;
  int hour,day,month,daynum;

  if (t < 1041415200 || t > 2041415200) {
    printw("     junk    ");
    return;
  }
  year = 2003;
  secondsSince2003 = t-1041415200;  /* Jan 01 midnight HST 2003 = Wednesday */
  daysInYear = 365;
  days = secondsSince2003/86400;
  dayOfWeek = ((int)(days+2)) % 7;  /* 2004 after leap day */
  /*  printw("%f ",days);*/
  while (days > daysInYear+1) {
    days -= daysInYear; 
    year += 1;
    if ((year % 4) == 0) {
      daysInYear = 366;
    } else {
      daysInYear = 365;
    }
  }
  todd = days;
  if ((year%4) == 0) {
    feb = 29; 
  } else {
    feb = 28;
  }
  month = 1;
  daynum = floor(days);
  if (daynum>31)  { month++; days-=31; }
  daynum = floor(days);
  if (daynum>feb) { month++; days-=feb; }
  daynum = floor(days);
  if (daynum>31)  { month++; days-=31; }
  daynum = floor(days);
  if (daynum>30)  { month++; days-=30; }
  daynum = floor(days);
  if (daynum>31)  { month++; days-=31; }
  daynum = floor(days);
  if (daynum>30)  { month++; days-=30; }
  daynum = floor(days);
  if (daynum>31)  { month++; days-=31; }
  daynum = floor(days);
  if (daynum>31)  { month++; days-=31; }
  daynum = floor(days);
  if (daynum>30)  { month++; days-=30; }
  daynum = floor(days);
  if (daynum>31)  { month++; days-=31; }
  daynum = floor(days);
  if (daynum>30)  { month++; days-=30; /* Nov */ }
  day = (int)floor(days);

  hours = 24.0*(days-day);

  hour = (int)floor(hours+0.5);
  m = 'A'; 
  if (hour > 11) { 
    hour -= 12; 
    m = 'P'; 
  } else if (hour < 1) {
    hour += 12; 
  }
#if 1
  printDayOfWeek(dayOfWeek);
  /*  printYear(year,todd);*/
  if (space) {
    printw("%2d%cM %02d/%02d",hour,m,month,day);
  } else {
    printw("%1d%cM %02d/%02d",hour,m,month,day);
  }
#else
  printw("%2d%cM %02d/%02d  ",hour,m,month,day);
#endif
}

void printDistanceToForecast(int i,int now) {
  int y,x;

  getyx(stdscr,y,x);
  move(y-1,x);

  printw("%d",distanceToForecast(i,now));

  move(y,x);

}

int distanceToForecast(int i, int now) {
  /* i must be of the value 0,3,6,9, etc */
  int seconds;
  seconds = pwv[i]-now-86400;
  return(seconds);
}
