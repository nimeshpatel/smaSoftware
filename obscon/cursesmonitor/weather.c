#define USE_VLBA_IN_AVERAGE 0
#define SKYDIP_DIR "/data/engineering/dip_scan"
#define PWV_MAX 50
#define PWV_MIN 0
#define DEBUG 0
#define DSM_HOST "colossus"
#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <dirent.h>
#include <curses.h>
#include <math.h>
#include <rm.h>
#include <dsm.h>
#include "weather.h"
#include "monitor.h"

#define MAX_SKYDIP_POINTS 1000
int PrintSkydip(int ant, int band, int correctForFreq, int rightnow);
void CallLinefit(int npts, float *x, float *y, float *slope, float *intercept,
  float vhot);
float skydip(char *filename, int band, int antenna);
void printDayOfWeek(int day);
void printYear(int year, int days);
long ConvertSkydipDateTimeToUnixTime(int date,int time);
void printAgeNoStandoutSkykdip(int rightnow, int longvalue);
extern int ConvertToUnixTime(int trueyear,int month,int day,int hr,int min,int sec);
extern int printAgeNoStandout(long rightnow, long longvalue);
extern int distanceToForecast(int i, int now);
extern void printDistanceToForecast(int i, int now);
extern float STANDOUT_TAU(float freqHz);
long printSkydipAge(int antenna, int rightnow);
extern float findMedian(float *, int);
extern int weatherUnits;
extern int printAge(long rightnow, long longvalue);
extern char *toddtime(time_t *, char *);
extern char *hsttime(time_t *t, char *str);
extern void checkDSMStatus(int status, char *string);
void PrintAverageSkydip(int band, int correctForFreq, float avgage);
void printForecastTime(float,int);
int LocateSkydips(int ant, int rightnow);
static int newestSkydipUnixTime[9] = {0,0,0,0,0,0,0,0,0};
static char skydipFilename[9][200] = {"","","","","","","","",""};
static float smatau[9] = {-2,-2,-2,-2,-2,-2,-2,-2,-2};
static float smatauCorrectedTo225[9] = {-2,-2,-2,-2,-2,-2,-2,-2,-2};
static float smatauHighFreq[9] = {-2,-2,-2,-2,-2,-2,-2,-2,-2};
static float skydipFrequency[9];
static float skydipFrequencyHigh[9];
static int skydipLocates = 0;
int usingServer(char *telescope);
char server[8];
float skydipTime[9];
char temperatureServer[8];
char pressureServer[8];
char humidityServer[8];
char windServer[8];
#include "taufactor.h"
extern dsm_structure smaWeather, jcmtWeather, irtfWeather, ukirtWeather, cfhtWeather;
extern dsm_structure csoWeather, keckWeather, vlbaWeather ,subaruWeather, uh88Weather;
/********************** added for aux weather display ************************/
extern dsm_structure AntWeather;
float AntTemp, AntRH, AntPres;
/*****************************************************************************/

void weather(int count, int *rm_list) {
  float tau225, tau350, uthours, jcmtTau, scaledTau;
  int tau225Minutes, tau350Minutes;
  int jcmtStale,subaruStale,smaStale,cfhtStale,ukirtStale,uh88Stale,vlbaStale;
  int irtfStale, irtfServerStale, keckStale, keckServerStale;
  int jcmtServerStale,subaruServerStale,smaServerStale,cfhtServerStale;
  int ukirtServerStale,uh88ServerStale,vlbaServerStale;
#define WINDFIELDS 24
  short wind[WINDFIELDS];
  short windAvg, road;
  int whichForecast;
  int rm_status, dsm_status;
  time_t system_time, distanceToClosestForecast;
  int incr;
  char timeString[27]; /* according to 'man ctime', string length = 26 */
  char timeString2[27]; /* according to 'man ctime', string length = 26 */
  int i;
  float floatvalue[10], average;
  static int firstTime = 1;
  float solarTemp,smaTemp;
/********************* added for " iced " anemometer display *****************/
  float smaHumidity;
/*****************************************************************************/
 int tempctr, avgctr;
  long longvalue, jcmtTauLongvalue;
  long smaTimestamp,jcmtTimestamp,irtfTimestamp,ukirtTimestamp,cfhtTimestamp;
  long csoTimestamp,keckTimestamp,vlbaTimestamp,uh88Timestamp,subaruTimestamp;
/*********************** added for aux weather display ************************/
 long AntTimestamp; 
/******************************************************************************/
  long rightnow,timestamp;
  int inHilo; /* is the SMA station in Hilo? */
  float windSpeedStandout;
  float wackoWindSpeedMax;
  float wackoWindSpeedMin;
  double restFrequency[2];
  float temperature[7];
  float humidity[7];
  float pressure[7];
  float direction[7];
  float speed[7];
  int correctForFreq;
  float avgtime;

  correctForFreq = 0;
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
  if (firstTime == 1) {
    firstTime = 0;
  } else {
    /*    fprintf(stderr,"Skipped dsm structure initializations\n");*/
  }
  dsm_status = call_dsm_read("hal9000", "DSM_AS_IFLO_REST_FR_V2_D", &restFrequency[0], &timestamp);
#define SKYDIP_SEARCH_INTERVAL 60
  if ((count % SKYDIP_SEARCH_INTERVAL) == 1) {
    /* cause a new search for recent skydip data */
    for (i=0; i<9; i++) { 
      strcpy(skydipFilename[i],"");
      smatau[i] = -2; 
      smatauHighFreq[i] = -2; 
    }
  }
  rm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);

/********************** added for aux weather display ************************/
  rm_status = dsm_read("hal9000","ANT_WEATHER_DATA_X", &AntWeather, &AntTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(ANT_WEATHER_DATA_X)");
  }  
/*****************************************************************************/
  rm_status = dsm_read(DSM_HOST,"SMA_METEOROLOGY_X", &smaWeather, &smaTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(SMA_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"JCMT_METEOROLOGY_X", &jcmtWeather, &jcmtTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(JCMT_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"UKIRT_METEOROLOGY_X", &ukirtWeather, &ukirtTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(UKIRT_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"SUBARU_METEOROLOGY_X", &subaruWeather, &subaruTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(SUBARU_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"KECK_METEOROLOGY_X", &keckWeather, &keckTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(KECK_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"CFHT_METEOROLOGY_X", &cfhtWeather, &cfhtTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(CFHT_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"IRTF_METEOROLOGY_X", &irtfWeather, &irtfTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(IRTF_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"UH88_METEOROLOGY_X", &uh88Weather, &uh88Timestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(UH88_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"CSO_METEOROLOGY_X", &csoWeather, &csoTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(CSO_METEOROLOGY_X)");
  }
  rm_status = dsm_read(DSM_HOST,"VLBA_METEOROLOGY_X", &vlbaWeather, &vlbaTimestamp);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_read(VLBA_METEOROLOGY_X)");
  }
  /*
  fprintf(stderr,"Finished reading dsm structures\n");
  */

  move(0,0);
  system_time = time(NULL);
  rightnow = system_time;
  toddtime(&system_time,timeString);
  hsttime(&system_time,timeString2);
  printw("Weather: %s UT = %s HT = %d",timeString,timeString2,rightnow);
#if DEBUG
  refresh();
#endif
  /*
  move(1,0);
  call_dsm_read(DSM_HOST,"DSM_JCMT_TIMESTAMP_L",&longvalue, &timestamp);
  printw("%d",longvalue);
  */
  move(1,8);
  rm_status = rm_read(rm_list[0], "RM_WEATHER_SERVER_SOURCE_C7",server);
  call_dsm_read(DSM_HOST,"DSM_TEMPERATURE_SERVER_C7",temperatureServer, &timestamp);
  call_dsm_read(DSM_HOST,"DSM_HUMIDITY_SERVER_C7",humidityServer, &timestamp);
  call_dsm_read(DSM_HOST,"DSM_WIND_SERVER_C7",windServer, &timestamp);
  call_dsm_read(DSM_HOST,"DSM_PRESSURE_SERVER_C7",pressureServer, &timestamp);
  if (usingServer("SMA")==1) {
    standout();
  }
  printw("SMA_Roof");
  if (usingServer("SMA")==1) {
    standend();
  }
  move(1,17);
  if (usingServer("JCMT")==1) {
    standout();
  }
  printw("JCMTbldg");
  if (usingServer("JCMT")==1) {
    standend();
  }
  move(1,27);
  if (usingServer("Subaru")==1) {
    standout();
  }
  printw("Subaru");
  if (usingServer("Subaru")==1) {
    standend();
  }
  move(1,35);
  if (usingServer("UKIRT")==1) {
    standout();
  }
  printw("UKIRT");
  if (usingServer("UKIRT")==1) {
    standend();
  }
  move(1,43);
  if (usingServer("CFHT")==1) {
    standout();
  }
  printw("CFHT");
  if (usingServer("CFHT")==1) {
    standend();
  }
  move(1,51);
  if (usingServer("UH88")==1) {
    standout();
  }
  printw("UH88");
  if (usingServer("UH88")==1) {
    standend();
  }
  move(1,59);
  if (usingServer("IRTF")==1) {
    standout();
  }
  printw("IRTF");
  standend();
  move(1,67);
  printw("VLBA");
  move(1,74);
  printw("Median");
#if DEBUG
  refresh();
#endif

#if DEBUG
  refresh();
#endif

  inHilo = 0; /* can use this to prevent values from being used in averages */

  /* Print the pressure data first, so that I can tell if the SMA weather 
   * station is in Hilo. */
  move(4,0);
  printw("Pressure ");
  i = 0;
  average = 0;
  avgctr = 0;
  tempctr = 0;
  /*
  call_dsm_read(DSM_HOST,"DSM_SMA_MBAR_F",&floatvalue[i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMA,MBAR_F)");
  }
#define STALE_INTERVAL 600

  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    if (floatvalue[i] < 1030 && floatvalue[i] > 970) {
      /* It is in Hilo! */
      inHilo = 1;
      if (floatvalue[i] >= 1000) {
	printw("%7.2f ",floatvalue[i]);
      } else {
	printw(" %6.2f ",floatvalue[i]);
      }
    } else {
      printw("  wacko  ");
    }
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f",floatvalue[i]);
    if (strcmp(pressureServer,"SMA")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    average += floatvalue[i];
    avgctr++;
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_JCMT_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&jcmtWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(JCMT,MBAR_F)");
  }
  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f",floatvalue[i]);
    if (strcmp(pressureServer,"JCMT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    average += floatvalue[i];
    avgctr++;
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_SUBARU_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&subaruWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Subaru,MBAR_F)");
  }
  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f",floatvalue[i]);
    if (strcmp(pressureServer,"Subaru")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    average += floatvalue[i];
    avgctr++;
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UKIRT_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&ukirtWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UKIRT,MBAR_F)");
  }

  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f",floatvalue[i]);
    if (strcmp(pressureServer,"UKIRT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    average += floatvalue[i];
    avgctr++;
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_CFHT_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&cfhtWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CFHT,MBAR_F)");
  }
  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f",floatvalue[i]);
    if (strcmp(pressureServer,"CFHT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    average += floatvalue[i];
    avgctr++;
  }
#if 0
  /*
  call_dsm_read(DSM_HOST,"DSM_UH88_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&uh88Weather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UH88,MBAR_F)");
  }
  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f",floatvalue[i]);
    if (strcmp(pressureServer,"UH88")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    average += floatvalue[i];
    avgctr++;
  }
#else /* UH88 has no pressure meter */
  printw("   n/a  ");
#endif

#if 0
  /*
  call_dsm_read(DSM_HOST,"DSM_IRTF_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&irtfWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(IRTF,MBAR_F)");
  }
  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    pressure[tempctr++] = floatvalue[i];
    printw(" %6.2f ",floatvalue[i]);
    if (irtfStale==0 && irtfServerStale==0) {
      average += floatvalue[i];
      avgctr++;
    }
  }
#else /* IRTF has not pressure meter */
  printw("   n/a  ");
#endif
  /*
  call_dsm_read(DSM_HOST,"DSM_VLBA_MBAR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&vlbaWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(VLBA,MBAR_F)");
  }
  if (floatvalue[i] > WACKO_MBAR_MAX || floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  ----- ");
  } else {
    printw(" %6.2f ",floatvalue[i]);
    if (vlbaStale==0 && vlbaServerStale==0 && USE_VLBA_IN_AVERAGE) {
      average += floatvalue[i];
      avgctr++;
    }
  }

  move(19,74);
  /*
  rm_status = call_dsm_read(DSM_HOST,"DSM_KECK_MBAR_F",&floatvalue[0],&longvalue);
  */
  rm_status = dsm_structure_get_element(&keckWeather,"MBAR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(KECK,MBAR_F)");
  }
  if (floatvalue[0] > WACKO_MBAR_MAX || floatvalue[0]<WACKO_MBAR_MIN) {
    printw("wacko");
  } else {
    pressure[tempctr++] = floatvalue[0];
    printw("%.2f",floatvalue[0]);
    if (keckStale==0 && keckServerStale==0) {
      average += floatvalue[0];
      avgctr++;
    }
  }

  move(4,74);
  average /= avgctr;
  average = findMedian(pressure,tempctr);
  if (average > WACKO_MBAR_MAX || average < WACKO_MBAR_MIN || average != average) {
    printw("  ------ ");
  } else {
    printw(" %5.1f",average);
  }


  move(7,0);
#if DEBUG
  refresh();
#endif
  printw("LastQuery ");
  i = 0;
  average = 0;
  /*
  call_dsm_read(DSM_HOST,"DSM_SMA_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = smaTimestamp;
#define WACKO_TIMESTAMP_MIN 1040000000
#define WACKO_TIMESTAMP_MAX 2040000000

  if (longvalue>WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    smaStale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_JCMT_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = jcmtTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    jcmtStale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_SUBARU_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = subaruTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    subaruStale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UKIRT_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = ukirtTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    ukirtStale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_CFHT_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = cfhtTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    cfhtStale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UH88_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = uh88Timestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    uh88Stale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_IRTF_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = irtfTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    irtfStale = printAge(rightnow,longvalue);
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_VLBA_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = vlbaTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw("  stale ");
  } else {
    printw(" ");
    vlbaStale = printAge(rightnow,longvalue);
  }
  move(7,74);
  call_dsm_read(DSM_HOST,"DSM_JCMT_TAU_TIMESTAMP_L",&longvalue, &timestamp);
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw(" stale ");
  } else {
    printAge(rightnow,longvalue);
  }


  move(16,68);
  if (usingServer("CSO")==1) {
    standout();
  }
  printw("CSOdome");
  standend();
  printw(" ");
  if (usingServer("KECK")==1) {
    standout();
  }
  printw("Keck");
  standend();
  
//   move(20,74);
// 
//   rm_status = dsm_structure_get_element(&keckWeather,"SERVER_TIMESTAMP_L", &longvalue);
//   if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
//     printw("------");
//   } else {
//     keckServerStale = printAge(rightnow,longvalue);
//   }

  move(20,67);
  rm_status = dsm_structure_get_element(&csoWeather,"SERVER_TIMESTAMP_L", &longvalue);
//  printAge(uthours*3600,longvalue*3600); //of CSO dome weather data
  printAge(rightnow,longvalue);

  move(20,74);
  /*
  call_dsm_read(DSM_HOST,"DSM_KECK_TIMESTAMP_L",&longvalue, &timestamp);
  */
  longvalue = keckTimestamp;
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    printw(" stale  ");
  } else {
    keckStale = printAge(rightnow,longvalue);
  }
 
  move(8,0);
#if DEBUG
  refresh();
#endif
  printw("LastUpdate");

  i = 0;
  average = 0;
  rm_status = dsm_structure_get_element(&smaWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMA,SERVER_TIMESTAMP_L)");
  }

#define WACKO_TIMESTAMP_MIN 1040000000
#define WACKO_TIMESTAMP_MAX 2040000000
  printw(" ");
  if (longvalue>WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  smaServerStale = printAge(rightnow,longvalue);
  standend();

  rm_status = dsm_structure_get_element(&jcmtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(JCMT,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  jcmtServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = dsm_structure_get_element(&subaruWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Subaru,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  subaruServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = dsm_structure_get_element(&ukirtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UKIRT,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  ukirtServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = dsm_structure_get_element(&cfhtWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CFHT,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  cfhtServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = dsm_structure_get_element(&uh88Weather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UH88,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  uh88ServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = dsm_structure_get_element(&irtfWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(IRTF,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  irtfServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = dsm_structure_get_element(&vlbaWeather,"SERVER_TIMESTAMP_L", &longvalue);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(VLBA,SERVER_TIMESTAMP_L)");
  }
  printw(" ");
  if (longvalue > WACKO_TIMESTAMP_MAX || longvalue < WACKO_TIMESTAMP_MIN) {
    standout();
  }
  vlbaServerStale = printAge(rightnow,longvalue);
  standend();
  rm_status = call_dsm_read(DSM_HOST,"DSM_JCMT_TAU_F",&jcmtTau, &longvalue);
  rm_status = call_dsm_read(DSM_HOST,"DSM_JCMT_TAU_SERVER_TIMESTAMP_L", 
		       &jcmtTauLongvalue, &timestamp);
  move(8,74);
  /*
  printf("\n\n\n\nrightnow = %d, longvalue = %d, jcmtTauLongvalue = %d\n",
	 rightnow, longvalue, jcmtTauLongvalue);exit(0);
  */
  printAge(rightnow,jcmtTauLongvalue);
  move(9,74);
  if (jcmtTau >= STANDOUT_TAU(restFrequency[0])) {
    standout();
  }
  if (jcmtTau >= 0) {
    printw(" ");
  } 
  if (fabs(jcmtTau) >= 1000.0) {
    printw("wacko",jcmtTau);
  } else if (fabs(jcmtTau) >= 100.0) {
    printw("%.1f",jcmtTau);
  } else if (fabs(jcmtTau) >= 10.0) {
    printw("%.2f",jcmtTau);
  } else {
    printw("%.3f",jcmtTau);
  }
  standend();

  move(2,0);
  printw("Temp (C) ");
  i = 0;
  average = 0;
  avgctr = 0;
  tempctr = 0;
  /*
  call_dsm_read(DSM_HOST,"DSM_SMA_TEMP_F",&floatvalue[i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"TEMP_F", &floatvalue[i]);
  smaTemp = floatvalue[i];
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMA,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN
      || floatvalue[i] != floatvalue[i]) {
    printw("   wacko ");
  } else {
    printw(" %+5.1f",floatvalue[i]);
    if (strcmp(temperatureServer,"SMA")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (inHilo == 0 && smaServerStale==0 && smaStale==0) {
      temperature[tempctr++] = floatvalue[i];
      avgctr++;
      average += floatvalue[i];
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_JCMT_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&jcmtWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(JCMT,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f",floatvalue[i]);
    if (strcmp(temperatureServer,"JCMT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
/*
    if (jcmtStale==0 && jcmtServerStale==0) {
      avgctr++;
      average += floatvalue[i];
    }
*/
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_SUBARU_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&subaruWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Subar,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f",floatvalue[i]);
    if (strcmp(temperatureServer,"Subaru")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (subaruStale==0 && subaruServerStale==0) {
      temperature[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UKIRT_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&ukirtWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UKIRT,TEMP_TIMESTAMP_L)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f",floatvalue[i]);
    if (strcmp(temperatureServer,"UKIRT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (ukirtStale==0 && ukirtServerStale==0) {
      temperature[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_CFHT_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&cfhtWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CFHT,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f",floatvalue[i]);
    if (strcmp(temperatureServer,"CFHT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (cfhtStale==0 && cfhtServerStale==0) {
      temperature[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UH88_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&uh88Weather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UH88,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f",floatvalue[i]);
    if (strcmp(temperatureServer,"UH88")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (uh88Stale==0 && uh88ServerStale==0) {
      temperature[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_IRTF_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&irtfWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(IRTF,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f ",floatvalue[i]);
    if (irtfStale==0 && irtfServerStale==0) {
      temperature[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }

  /*
  call_dsm_read(DSM_HOST,"DSM_VLBA_TEMP_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&vlbaWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(VLBA,TEMP_F)");
  }
  if (floatvalue[i] > WACKO_TEMP_MAX || floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko ");
  } else {
    printw("  %+5.1f ",floatvalue[i]);
    if (vlbaStale==0 && vlbaServerStale==0 && USE_VLBA_IN_AVERAGE) {
      average += floatvalue[i];
      avgctr++;
    }
  }

//    uncomment when sensor repaired
//   move(9,0);       
//   rm_status = dsm_structure_get_element(&smaWeather,"SOLAR_TEMP_F", &solarTemp);
//   if (solarTemp-smaTemp > SUNSHINE_CRITERION) {
//     standout();
//   }
//   printw("Sunlight");
//   standend();
//  printw("(C) %4.1f",solarTemp);

  move(17,74);
  /*
  rm_status = call_dsm_read(DSM_HOST,"DSM_KECK_TEMP_F",&floatvalue[0],&longvalue);
  */
  rm_status = dsm_structure_get_element(&keckWeather,"TEMP_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Keck,TEMP_F)");
  }
  if (floatvalue[0] > WACKO_TEMP_MAX || floatvalue[0] < WACKO_TEMP_MIN) {
    printw("wacko");
  } else {
    printw("%+.1f",floatvalue[0]);
    if (keckStale==0 && keckServerStale==0) {
      temperature[tempctr++] = floatvalue[i];
      average += floatvalue[0];
      avgctr++;
    }
  }

  move(2,74);
  average /= avgctr;
  average = findMedian(temperature,tempctr);
  if (average>WACKO_TEMP_MAX || average<WACKO_TEMP_MIN || average != average) {
    printw("  wacko");
  } else {
    printw(" %+4.1f\n",average);
  }

#if DEBUG
  refresh();
#endif
  move(3,0);
  printw("Humidity ");
  i = 0;
  average = 0;
  avgctr = 0;
  tempctr = 0;
  /*
  call_dsm_read(DSM_HOST,"DSM_SMA_HUMIDITY_F",&floatvalue[i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMA,HUMIDITY_F)");
  }

  if (floatvalue[i]>WACKO_HUMIDITY_MAX || 
      /*        floatvalue[i] < WACKO_HUMIDITY_MIN || */
      floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (strcmp(humidityServer,"SMA")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (inHilo == 0 && smaStale==0 && smaServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_JCMT_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&jcmtWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(JCMT,HUMIDITY_F)");
  }

  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  ------ ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (strcmp(humidityServer,"JCMT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    /*
    if (jcmtStale==0 && jcmtServerStale==0) {
      average += floatvalue[i];
      avgctr++;
    }
    */
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_SUBARU_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&subaruWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Subaru,HUMIDITY_F)");
  }
  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (strcmp(humidityServer,"Subaru")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (subaruStale==0 && subaruServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UKIRT_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&ukirtWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UKIRT,HUMIDITY_F)");
  }
  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (strcmp(humidityServer,"UKIRT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (ukirtStale==0 && ukirtServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_CFHT_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&cfhtWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CFHT,HUMIDITY_F)");
  }
  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (strcmp(humidityServer,"CFHT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (cfhtStale==0 && cfhtServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UH88_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&uh88Weather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UH88,HUMIDITY_F)");
  }
  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (strcmp(humidityServer,"UH88")==0) {
      printw("*");
    } else {
      printw(" ");
    }
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (uh88Stale==0 && uh88ServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_IRTF_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&irtfWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(IRTF,HUMIDITY_F)");
  }
  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%% ",floatvalue[i]);
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (irtfStale==0 && irtfServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_VLBA_HUMIDITY_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&vlbaWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(VLBA,HUMIDITY_F)");
  }
  if (floatvalue[i] > WACKO_HUMIDITY_MAX || floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] != floatvalue[i]) {
    printw(" ----- ");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %5.1f%%",floatvalue[i]);
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (vlbaStale==0 && vlbaServerStale==0 && USE_VLBA_IN_AVERAGE) {
      average += floatvalue[i];
      avgctr++;
    }
  }

  move(18,74);
  /*
  rm_status = call_dsm_read(DSM_HOST,"DSM_KECK_HUMIDITY_F",&floatvalue[0],&longvalue);
  */
  rm_status = dsm_structure_get_element(&keckWeather,"HUMIDITY_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(KECK,HUMIDITY_F)");
  }
  if (floatvalue[0] > WACKO_HUMIDITY_MAX || floatvalue[0]<WACKO_HUMIDITY_MIN) {
    printw("wacko");
  } else {
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw("%.1f%%",floatvalue[0]);
    if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
    if (keckStale==0 && keckServerStale==0) {
      humidity[tempctr++] = floatvalue[i];
      average += floatvalue[0];
      avgctr++;
    }
  }

  move(3,74);
  average /= avgctr;
  average = findMedian(humidity,tempctr);
  if (average > WACKO_HUMIDITY_MAX || average < WACKO_HUMIDITY_MIN || average != average) {
    printw("  -----");
  } else {
    if (average > HUMIDITY_MAX_STANDOUT ||
	average < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw(" %3.0f%%\n",average);
    standend();
  }

  move(5,0);
#if DEBUG
  refresh();
#endif
  i = 0;
  average = 0;
  avgctr = 0;
  tempctr = 0;
  /*
  call_dsm_read(DSM_HOST,"DSM_SMA_WINDSPEED_F",&floatvalue[i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMA,WINDSPEED_F)");
/**********************added for " iced " anemometer display *****************/
  	smaHumidity = floatvalue[i];
/*****************************************************************************/
 }

  windSpeedStandout = WIND_SPEED_STANDOUT;
  wackoWindSpeedMax = WACKO_WINDSPEED_MAX;
  wackoWindSpeedMin = WACKO_WINDSPEED_MIN;
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    printw("Wind(m/s)");
    windSpeedStandout *= MPH_TO_METER_PER_SEC;
    wackoWindSpeedMax *= MPH_TO_METER_PER_SEC;
    wackoWindSpeedMin *= MPH_TO_METER_PER_SEC;
  } else {
    printw("Wind(mph)");
    floatvalue[i] /= MPH_TO_METER_PER_SEC;
  }
  
/**********************added for " iced " anemometer display *****************/
  if (floatvalue[i]==WINDSPEED_ICED && smaTemp<TEMP_ICED && smaHumidity>HUMIDITY_ICED) {
    printw("  iced   ");
  } else {  
/*****************************************************************************/
   if (floatvalue[i]>wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
   } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"SMA")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
    printw(" ");
#endif
    standend();
    if (inHilo == 0 && smaServerStale==0 && smaStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
   }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_JCMT_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&jcmtWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(JCMT,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  printw("   n/a  ");
  /* The Hawaiians have removed the JCMT weather tower. */ 
#if 0
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"JCMT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (jcmtStale==0 && jcmtServerStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
#endif
  /*
  call_dsm_read(DSM_HOST,"DSM_SUBARU_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&subaruWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Subaru,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"Subaru")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (subaruStale==0 && subaruServerStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UKIRT_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&ukirtWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UKIRT,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"UKIRT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (ukirtStale==0 && ukirtServerStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_CFHT_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&cfhtWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CFHT,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"CFHT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (cfhtStale==0 && cfhtServerStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UH88_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&uh88Weather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UH88,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"UH88")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (uh88Stale==0 && uh88ServerStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_IRTF_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&irtfWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(IRTF,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f ",floatvalue[i]);
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (irtfStale==0 && irtfServerStale==0) {
      speed[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }

  /*
  call_dsm_read(DSM_HOST,"DSM_VLBA_WINDSPEED_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&vlbaWeather,"WINDSPEED_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(VLBA,WINDSPEED_F)");
  }
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    floatvalue[i] *= MPH_TO_METER_PER_SEC;
  }
  if (floatvalue[i] > wackoWindSpeedMax || floatvalue[i] < wackoWindSpeedMin || floatvalue[i] != floatvalue[i]) {
    printw("  ------ ");
  } else {
    if (floatvalue[i] > windSpeedStandout) {
      standout();
    } 
    printw("  %5.2f ",floatvalue[i]);
    if (floatvalue[i] > windSpeedStandout) {
      standend();
    } 
    if (vlbaStale==0 && vlbaServerStale==0 && USE_VLBA_IN_AVERAGE) {
      average += floatvalue[i];
      avgctr++;
    }
  }
  average /= avgctr;
  average = findMedian(speed,tempctr);
  if (average > wackoWindSpeedMax || average < wackoWindSpeedMin || average != average) {
    printw("  -----");
  } else {
    if (average >= windSpeedStandout) {
      standout();
    } 
    printw(" %4.1f\n",average);
    if (average >= windSpeedStandout) {
      standend();
    } 
  }


  move(6,0);
#if DEBUG
  refresh();
#endif
  printw("Direction");
  avgctr = 0;
  i = 0;
  average = 0;
  tempctr = 0;
  /*
  call_dsm_read(DSM_HOST,"DSM_SMA_WINDDIR_F",&floatvalue[i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&smaWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMA,WINDDIR_F)");
  }

  if (floatvalue[i]>WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"SMA")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (inHilo == 0 && smaStale==0 && smaServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_JCMT_WINDDIR_F",&floatvalue[++i], &timestamp);
  rm_status = dsm_structure_get_element(&jcmtWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(JCMT,WINDDIR_F)");
  }
  */
  /* The Hawaiians have removed the JCMT weather tower. */ 
  printw("   n/a  ");
#if 0
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"JCMT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (jcmtStale==0 && jcmtServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
#endif
  /*
  call_dsm_read(DSM_HOST,"DSM_SUBARU_WINDDIR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&subaruWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(Subaru,WINDDIR_F)");
  }
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"Subaru")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (subaruStale==0 && subaruServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UKIRT_WINDDIR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&ukirtWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UKIRT,WINDDIR_F)");
  }
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"UKIRT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (ukirtStale==0 && ukirtServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_CFHT_WINDDIR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&cfhtWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CFHT,WINDDIR_F)");
  }
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"CFHT")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (cfhtStale==0 && cfhtServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_UH88_WINDDIR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&uh88Weather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(UH88,WINDDIR_F)");
  }
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f",floatvalue[i]);
#if 0
    if (strcmp(windServer,"UH88")==0) {
      printw("*");
    } else {
      printw(" ");
    }
#else
      printw(" ");
#endif
    if (uh88Stale==0 && uh88ServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }
  /*
  call_dsm_read(DSM_HOST,"DSM_IRTF_WINDDIR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&irtfWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(IRTF,WINDDIR_F)");
  }
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw("  wacko  ");
  } else {
    printw("  %5.1f ",floatvalue[i]);
    if (irtfStale==0 && irtfServerStale==0) {
      direction[tempctr++] = floatvalue[i];
      average += floatvalue[i];
      avgctr++;
    }
  }

  /*
  call_dsm_read(DSM_HOST,"DSM_VLBA_WINDDIR_F",&floatvalue[++i], &timestamp);
  */
  rm_status = dsm_structure_get_element(&vlbaWeather,"WINDDIR_F", &floatvalue[i]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(VLBA,WINDDIR_F)");
  }
  if (floatvalue[i] > WIND_DIRECTION_WACKO_PLUS || floatvalue[i] < WIND_DIRECTION_WACKO_MINUS || floatvalue[i] != floatvalue[i]) {
    printw(" ------ ");
  } else {
    printw(" %5.1f ",floatvalue[i]);
    if (vlbaStale==0 && vlbaServerStale==0 && USE_VLBA_IN_AVERAGE) {
      average += floatvalue[i];
      avgctr++;
    }
  }

  average /= avgctr;
  average = findMedian(direction,tempctr);
  if (average > WIND_DIRECTION_WACKO_PLUS || average < WIND_DIRECTION_WACKO_MINUS || average != average) {
    printw("  -----");
  } else {
    printw("  %3.0f\n",average);
  }


  move(10,0);
  printw("Antenna   1      2      3      4      5    ");
  if (present(pressureServer,"SMAaux") ||
      present(humidityServer,"SMAaux") ||
      present(temperatureServer,"SMAaux")
     ) {
    standout();
  }
  printw(" 6 ");
  standend();
  printw("     7      8   SMA");

  move(11,0);
  printw("DipFrq");
  for (i=1; i<=8; i++) {
    if (skydipFrequency[i] < 1000 && skydipFrequency[i] > 0) {
      printw(" %5.1f ",skydipFrequency[i]);
    } else {
      printw(" wacko");
      if (i<8) printw(" ");
    }
  }
  move(11,63);
  printw("Avg      CSO350um");
  move(12,68);
  printw("CSOTau");

#if 0
  move(10,0);
  printw("Last acc1 dip: ");
  if (strlen(skydipFilename[1]) > 0) {
    printw("20%s UTC",skydipFilename[1]);
  }
  printw(" at %.3f GHz ",skydipFrequency[1]);

  printw(" (search#%d in %d)",
    skydipLocates,SKYDIP_SEARCH_INTERVAL-count%SKYDIP_SEARCH_INTERVAL);
  /*  printw(" at 230.65GHz and 657.5GHz");*/
#endif
  call_dsm_read(DSM_HOST,"DSM_CSO_225GHZ_TAU_TSTAMP_L",&tau225Minutes,&timestamp);
  call_dsm_read(DSM_HOST,"DSM_CSO_350MICRON_TAU_TSTAMP_L",&tau350Minutes,&timestamp);
  rm_read(rm_list[0],"RM_UTC_HOURS_F",&uthours);
  call_dsm_read(DSM_HOST,"DSM_CSO_225GHZ_TAU_F",&tau225,&timestamp);
  call_dsm_read(DSM_HOST,"DSM_CSO_350MICRON_TAU_SCALED_F",&tau350,&timestamp);

  move(13,68);
  if (tau225 >= STANDOUT_TAU(restFrequency[0])) {
    standout();
  } 
  if (tau225 <-10 || tau225>99) {
    printw("wacko");
  } else {
    printw("%.3f",tau225);
  }
  standend();
  printw("  ");

  if (tau350 >= STANDOUT_TAU(restFrequency[0])) {
    standout();
  } 
  scaledTau = 23.0*tau350;
  if (tau350 >= 100.0) {
    printw("%.1f",tau350);
    standend();
  } else if (tau350>= 10.0) {
    printw("%.2f",tau350);
    standend();
    printw(" ");
  } else {
    printw("%.3f",tau350);
    standend();
    printw(" ");
  }

  move(12,75);
  if (tau350 >= STANDOUT_TAU(restFrequency[0])) {
    standout();
  } 
  if (scaledTau >= 100.0) {
    printw("%.1f",scaledTau);
  } else if (scaledTau >= 10.0) {
    printw("%.2f",scaledTau);
  } else {
    printw("%.3f",scaledTau);
  }
  standend();
  move(14,67);
  printAge(uthours*3600,tau225Minutes*60);
  move(14,74);
  printAge(uthours*3600,tau350Minutes*60);

  correctForFreq = 0;
  move(14,0);
#if DEBUG
  refresh();
#endif
  printw("AgeDip ");
  avgtime = 0;
  for (i=1; i<=8; i++) {
    skydipTime[i] = printSkydipAge(i,rightnow);
    avgtime += skydipTime[i];
  }
  avgtime /= 8;

  move(12,0);
#if DEBUG
  refresh();
#endif
  for (i=1; i<=8; i++) {
    if(PrintSkydip(i,0,correctForFreq,rightnow) < 0) {
      move(13,0);
      printw("The Sky dip dir = %s could not be opened! ", SKYDIP_DIR);
      goto NO_SKY_DIP;
    }
  }
  printw("TauFrq");
  PrintAverageSkydip(0,correctForFreq,avgtime);
  /*
  mvprintw(9, 1, (weatherUnits)? "wind shown in mph ('+' for m/s)":
	"wind shown in m/s ('+' for mph)");
  */
  mvprintw(9, 19, (weatherUnits)? "('+' for m/s)":
	"('+' for mph)");

  printw("  * = values used by antennas    JCMT Tau:"); /* do not put \n here*/


  move(13,0);
  printw("Tau225");
  correctForFreq = 1;
  for (i=1; i<=8; i++) {
    PrintSkydip(i,0,correctForFreq,rightnow);
  }
  PrintAverageSkydip(0,correctForFreq,avgtime);


  move(15,0);
  printw("HiFrRx");
  for (i=1; i<=8; i++) {
    PrintSkydip(i,1,correctForFreq,rightnow);
  }
  PrintAverageSkydip(1,correctForFreq,avgtime);

  move(16,0);
  printw("HiFreq");
  for (i=1; i<=8; i++) {
    if (skydipFrequencyHigh[i] < 1000 && skydipFrequencyHigh[i] > 0) {
      printw(" %5.1f ",skydipFrequencyHigh[i]);
    } else {
      printw(" wacko");
      if (i<8) printw(" ");
    }
  }
NO_SKY_DIP:

  move(17,0);
#if DEBUG
  refresh();
#endif
  printw("Temp(C)");
/********************** added for aux temperature display ********************/
for (i=1; i<=8; i++) {
    rm_status = dsm_structure_get_element(&AntWeather,"TEMPERATURE_V9_F", &floatvalue[0]);
   if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMAaux,TEMPERATURE_V9_F)");
  }
//  rm_status = dsm_structure_get_element(&sma5Weather,"SERVER_TIMESTAMP_L", &longvalue);
//   rm_status = dsm_structure_get_element(&AntWeather,"TIME_STAMP_V9_L", &longvalue);
//   if (rm_status != DSM_SUCCESS) {
//     dsm_error_message(rm_status,"dsm_structure_get_element(SMA,TIME_STAMP_V9_L)");
// }
if (floatvalue[i] < WACKO_TEMP_MIN || floatvalue[i] > WACKO_TEMP_MAX) {
	printw("wacko  ");
      } else {
	printw(" %+5.1f",floatvalue[i]);
      }
      if (i == 1) {
	printw(" ");
      } else {
	printw(" ");
      }
  }
/*****************************************************************************/
  move(17,68);
  /*
  rm_status = call_dsm_read(DSM_HOST,"DSM_CSO_TEMP_F",&floatvalue[0],&longvalue);
  */
  rm_status = dsm_structure_get_element(&csoWeather,"TEMP_F", &floatvalue[0]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CSO,TEMP_F)");
  }
  if (floatvalue[0] > WACKO_TEMP_MAX || floatvalue[0] < WACKO_TEMP_MIN) {
    printw("wacko  ");
  } else {
    if (fabs(floatvalue[0]) >= 10) {
      printw("%+.1f ",floatvalue[0]);
    } else {
      printw("%+.1f  ",floatvalue[0]);
    }
  }

  move(18,0);
#if DEBUG
  refresh();
#endif
#if 0
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    printw("Wind(m/s)");
  } else {
    printw("Wind(mph) ");
  }
  for (i=0; i<8; i++) {
    rm_read(rm_list[i],"RM_WEATHER_WINDSPEED_F",&floatvalue[i]);
    if (weatherUnits == METRIC_WEATHER_UNITS) {
      floatvalue[i] *= 0.44704; /* convert from mph to m/s */
    }
    printw(" %4.1f ",floatvalue[i]);
  }
#else
  printw("Humidty");
#endif
/********************** added for aux humidity display ***********************/
  for (i=1; i<=8; i++) {
  rm_status = dsm_structure_get_element(&AntWeather,"HUMIDITY_V9_F", &floatvalue[0]);
   if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMAaux,HUMIDITY_V9_F)");
    }
      if (floatvalue[i] < WACKO_HUMIDITY_MIN || floatvalue[i] > WACKO_HUMIDITY_MAX) {
	printw("wacko  ");
      } else {
	if (floatvalue[i] > HUMIDITY_MAX_STANDOUT ||
	    floatvalue[i] < HUMIDITY_MIN_STANDOUT) {
	  standout();
	} 
	printw("%5.1f%%",floatvalue[i]);
	printw(" ");
	standend();
      }
       if (i == 1) {
//	printw(" ");
//      } else if (i == 7) {
// 	printAge(rightnow,AntWeatherTimestamp);
//      } else {
	printw("");
      }
  }
/*****************************************************************************/
  move(18,67);
  /*
  rm_status = call_dsm_read(DSM_HOST,"DSM_CSO_HUMIDITY_F",&floatvalue[0],&longvalue);
  */
  rm_status = dsm_structure_get_element(&csoWeather,"HUMIDITY_F", &floatvalue[0]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CSO,HUMIDITY_F)");
  }
  if (floatvalue[0] > WACKO_HUMIDITY_MAX || floatvalue[0]<WACKO_HUMIDITY_MIN) {
    printw("wacko");
  } else {
    if (floatvalue[0] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[0] < HUMIDITY_MIN_STANDOUT) {
      standout();
    } 
    printw("%.1f%% ",floatvalue[0]);
    if (floatvalue[0] > HUMIDITY_MAX_STANDOUT ||
	floatvalue[0] < HUMIDITY_MIN_STANDOUT) {
      standend();
    } 
  }

  move(19,0);
#if DEBUG
  refresh();
#endif
  printw("Pressur");
/********************** added for aux pressure display ***********************/
 for (i=1; i<=8; i++) {
  rm_status = dsm_structure_get_element(&AntWeather,"PRESSURE_V9_F", &floatvalue[0]);
   if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(SMAaux,PRESSURE_V9_F)");
    }
      if (floatvalue[i] < WACKO_MBAR_MIN || floatvalue[i] > WACKO_MBAR_MAX) {
	printw(" wacko ");
      } else {
	printw(" %5.1f ",floatvalue[i]);
	}
}

/*****************************************************************************/
  move(19,67);
  /*
  rm_status = call_dsm_read(DSM_HOST,"DSM_CSO_MBAR_F",&floatvalue[0],&longvalue);
  */
  rm_status = dsm_structure_get_element(&csoWeather,"MBAR_F", &floatvalue[0]);
  if (rm_status != DSM_SUCCESS) {
    dsm_error_message(rm_status,"dsm_structure_get_element(CSO,MBAR_F)");
  }
  if (floatvalue[0] > WACKO_MBAR_MAX || floatvalue[0]<WACKO_MBAR_MIN) {
    printw("wacko ");
  } else {
    printw("%.2f ",floatvalue[0]);
  }


  move(20,0);
  printw("LastUpd");
  for (i=1; i<=8; i++) {
//     if (i != 6) {
//       printw("       ");
//     } else {
// #if 1
//       printw("       ");
// #else
//       if (1) {
	printAge(rightnow,AntTimestamp);
 //      } else {
// 	printw("stale  ");
//       }
// #endif
//     }
  }

  
// restore at a later date - lost it when ported to private network
  
//   move(22,0);
//   printw("HST fcast: ");
//   distanceToClosestForecast = distanceToForecast(0,rightnow);
//   whichForecast = 0;
//   incr = 3;
//   for (i=3; i<=10*3; i+=incr) {
//     if (abs(distanceToForecast(i,rightnow)) < abs(distanceToClosestForecast)) {
//       whichForecast = i/incr;
//       distanceToClosestForecast = abs(distanceToForecast(i,rightnow));
//     }
//   }
//   incr = 3;
//   for (i=0; i<=12; i+=incr) {
//     if (whichForecast == i/incr) {
//       standout();
//     }
// #if DEBUG
//     printw("About to printForecastTime %d\n",i);
//     refresh();
// #endif
// 
//     printDistanceToForecast(i,rightnow);
//    
//     printForecastTime(pwv[i],1);
//     if (whichForecast == i/incr) {
//       standend();
//     }
//     printw(" ");
//   }
  move(22,0);
// #if DEBUG
//   refresh();
// #endif
  printw("PWV, Wmph: ");
  incr = 1;
  for (i=0; i<5; i+=incr) {
    if (pwv[i*3+1] > PWV_MAX || pwv[i*3+2] > PWV_MAX ||
        pwv[i*3+1] < PWV_MIN || pwv[i*3+2] < PWV_MIN) {
      printw("    wacko    "); 
    } else {
      if (whichForecast == i/incr) {
        standout();
      }
      windAvg = (wind[i*2] + wind[i*2+1]) / 2;
      if (pwv[i*3+1] >= 10) {
        printw("%.1f-%.1f, ",pwv[i*3+1], pwv[i*3+2]); 
	if (wind[i*2+1] >= WIND_SPEED_STANDOUT) {
	  standout();
	}
        printw("%2d", windAvg); 
	if (wind[i*2+1] >= WIND_SPEED_STANDOUT) {
	  standend();
	}
      } else if (pwv[i*3+2] >= 10) {
        printw("%.1f -%.1f, ",pwv[i*3+1], pwv[i*3+2]); 
	if (wind[i*2+1] >= WIND_SPEED_STANDOUT) {
	  standout();
	}
        printw("%2d", windAvg); 
	if (wind[i*2+1] >= WIND_SPEED_STANDOUT) {
	  standend();
	}
      } else {
        printw("%.1f-%.1f,",pwv[i*3+1], pwv[i*3+2]); 
	if (wind[i*2+1] >= WIND_SPEED_STANDOUT) {
	  standout();
	}
	if (wind[i*2] < 100 && wind[i*2+1] < 100) {
          printw("%2d-%2d", wind[i*2],wind[i*2+1]); 
	} else {
          printw(" %3d ", windAvg); 
	}
	if (wind[i*2+1] >= WIND_SPEED_STANDOUT) {
	  standend();
	}
      }
      if (whichForecast == i/incr) {
        standend();
      }
      printw(" ");
    }
  }
  move(23,0);
  rm_status = call_dsm_read(DSM_HOST,"DSM_SUMMIT_ROAD_STATUS_S", &road, &longvalue);
  if (road == 1) {
    printw("Road: open");
  } else {
    standout();
    printw("Road: closed");
    standend();
  }
  move(24,0);
  refresh();
}

void printForecastTime(float t, int space) {
  double secondsSince2003;
  double days, hours;
  int feb, year = 2003;
  int dayOfWeek, daysInYear;
  int todd;
  char m;
  int hour,day,month,daynum;

  if (t < 1041415200 || t > 2041415200) {
    printw("     junk    ");
    return;
  }
  secondsSince2003 = t-1041415200;  /* Jan 01 midnight HST 2003 = Wednesday */
  daysInYear = 365;
  days = secondsSince2003/86400;
  dayOfWeek = ((int)(days+2)) % 7;  /* 2004 after leap day*/

  while (days > daysInYear+1) {
    days -= daysInYear; 
    year++;
    if ((year % 4) == 0) {
      daysInYear = 366;
    } else {
      daysInYear = 365;
    }
  }
  if ((year%4)==0) {
    feb=29; 
  } else {
    feb=28;
  }
  todd = days;
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
  if (daynum>30)  { month++; days-=30; }
  day = (int)floor(days);
  /*  printw("%d ",day);*/
  hours = 24.0*(days-day);
  /*  printw("%f ",hours);*/
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

void printYear(int year,int days) {
  int x,y;
  getyx(stdscr,y,x);
  move(y-1,x);
  printw("%4d %d",year,days);
  move(y,x);
}

void printDayOfWeek(int day) {
  switch (day) {
  case 0:
    printw("Sun");
    break;
  case 1:
    printw("Mon");
    break;
  case 2:
    printw("Tue");
    break;
  case 3:
    printw("Wed");
    break;
  case 4:
    printw("Thu");
    break;
  case 5:
    printw("Fri");
    break;
  case 6:
    printw("Sat");
    break;
  default:
    printw("   ");
  }
}

float skydip(char *filename, int band, int antenna) {
  float vhot, vcold, vhotHigh, vcoldHigh;
  float voltage[MAX_SKYDIP_POINTS];
  float voltageHigh[MAX_SKYDIP_POINTS];
  float airmass[MAX_SKYDIP_POINTS];

    float elev, treceiver;
    char *ptr;
    int i, npoints, narg;
    int coldPresent = 0;
    float vhcdiff, gain, tauz, intercept, etahot, tcold, tspill, yfactor;
    FILE *fp;
    char inputline[100];
    float thot = -1;

/* Calculate the appropriate log values */

    fp = fopen(filename,"r");
    if (fp == NULL) {
      if (DEBUG) {
        move(13,0);
        printw("File not found, Returning with -1\n");
        refresh();
      }
      return(-1);
    }
    i = 0;
    do {
      ptr = fgets(inputline,sizeof(inputline),fp);
      if (ptr != NULL) {
        if (present(inputline,"#")) {
	  if (present(inputline,"thot")) {
	    sscanf(inputline,"%*s %*s %f",&thot);
	  } else if (present(inputline,"hot")) {
	    sscanf(inputline,"%*s %*s %f %f",&vhot,&vhotHigh);
	  } else if (present(inputline,"tcold")) {
	    coldPresent = 1;
	    sscanf(inputline,"%*s %*s %f",&tcold);
	  } else if (present(inputline,"cold")) {
	    coldPresent = 1;
	    sscanf(inputline,"%*s %*s %f %f",&vcold,&vcoldHigh);
	  } else if (present(inputline,"freq")) {
	    sscanf(inputline,"%*s %*s %f %f",&skydipFrequency[antenna],
              &skydipFrequencyHigh[antenna]);
	  }
	} else {
  	  narg = sscanf(inputline,"%f %f %f",&elev,&voltage[i],&voltageHigh[i]);
	  if (narg >= 2) {
#define PI (4.0*atan(1.0))
#define DEG_TO_RAD (PI/180.)
            airmass[i] = 1/cos(DEG_TO_RAD*(90-elev));
            i++;
	  }
        }
      }
    } while (ptr != NULL);
    fclose(fp);
    npoints = i;
    if (coldPresent == 1) {
      if (band == 0) {
        vhcdiff = vhot - vcold;
      } else {
        vhcdiff = vhotHigh - vcoldHigh;
      }
    } else {
      if (band == 0) {
        vhcdiff = 0.5*vhot;
      } else {
        vhcdiff = 0.5*vhotHigh;
      }
    }
    if (band == 0) {
      CallLinefit(npoints, airmass, voltage, &tauz, &intercept,vhot);
    } else {
      CallLinefit(npoints, airmass, voltageHigh, &tauz, &intercept,vhotHigh);
    }
    if (coldPresent==1) {/* thot, tcold are given so calculate everything */
      etahot = ( 1 - tcold/thot) / exp( intercept );
      tspill = ( 1 - etahot ) * thot;
      gain = vhcdiff / ( thot - tcold );
      if (band == 0) {
        yfactor = vhot / vcold;
      } else {
        yfactor = vhotHigh / vcoldHigh;
      }
      treceiver = ( thot - yfactor * tcold ) / ( yfactor - 1 );
    }
    return(tauz);
}

		 /* x = airmass, y = voltage */
void CallLinefit(int npts, float *x, float *y, float *slope, float *intercept,
                 float vhot){
    int i;
    float logvalue;
    double xnpts = 0., sumx = 0., sumx2 = 0., sumy = 0.;
    double sumy2 = 0, sumxy = 0.;

    for ( i = 0; i < npts; i++ ) {
      if ( y[i] < vhot ) {
        xnpts += 1.;
        logvalue = log( vhot / ( vhot - y[i] ) );
        sumx  += x[i];
        sumx2 += x[i] * x[i];
        sumy  += logvalue;
        sumy2 += logvalue * logvalue;
        sumxy += x[i] * logvalue;
      }
    }
    *slope = ( xnpts*sumxy - sumx*sumy ) / ( xnpts*sumx2 - sumx*sumx );
    *intercept = ( sumy*sumx2 - sumx*sumxy ) / ( xnpts*sumx2 - sumx*sumx );
}

int PrintSkydip(int ant, int band, int correctForFreq, int rightnow) {
  char filename[sizeof(skydipFilename)];
  float tau;
  float freqdiff;
  char tauString[20];

  if (ant != 1) {
    printw(" ");
  }
  if (band == 0) {
    if (smatau[ant] == -2) {
      if (strlen(skydipFilename[ant]) < 1) {
        if(LocateSkydips(ant,rightnow) != 0) {
	    return(-1);
	}
        ++skydipLocates;
      }
      sprintf(filename,"%s/%s",SKYDIP_DIR,skydipFilename[ant]);
      if (DEBUG) {
        move(4+ant,0);
        printw("calling skydip(%s)\n",filename);
        refresh();
      }
      smatau[ant] = skydip(filename,0,ant);
      smatauHighFreq[ant] = skydip(filename,1,ant);
    }
    if (smatau[ant] < 0) {
      printw(" wacko");
    } else {
      tau = smatau[ant];
      if (correctForFreq) {
	if (skydipFrequency[ant] > 0 && skydipFrequency[ant]<TOPFREQ) {
	  freqdiff = skydipFrequency[ant]-(int)skydipFrequency[ant];
	  tau /= 0.5*(taufactor[(int)skydipFrequency[ant]+5]*(1-freqdiff) +
                      taufactor[(int)skydipFrequency[ant]+6]*freqdiff) +
#if 0
	         0.1*(taufactor[(int)skydipFrequency[ant]+4]*(1-freqdiff) +
                      taufactor[(int)skydipFrequency[ant]+5]*freqdiff) +
		 0.1*(taufactor[(int)skydipFrequency[ant]+6]*(1-freqdiff) +
                      taufactor[(int)skydipFrequency[ant]+7]*freqdiff) +
#endif
		 0.5*(taufactor[(int)skydipFrequency[ant]-5]*(1-freqdiff) +
                      taufactor[(int)skydipFrequency[ant]-4]*freqdiff)
#if 0
    	         + 0.1*(taufactor[(int)skydipFrequency[ant]-4]*(1-freqdiff) +
    	                taufactor[(int)skydipFrequency[ant]-4]*freqdiff) +
                   0.1*(taufactor[(int)skydipFrequency[ant]-6]*(1-freqdiff) +
                        taufactor[(int)skydipFrequency[ant]-6]*freqdiff);
#endif
	  ;
	  smatauCorrectedTo225[ant] = tau;
	}
      } else {
	smatauCorrectedTo225[ant] = tau;
      }
      sprintf(tauString,"%f",tau);
      if (present(tauString,"NaN")) {
	printw("  NaN ");
      } else if (tau < 10) {
	printw(" %.3f",tau);
      } else if (tau < 100) {
	printw(" %.2f",tau);
      } else {
	printw(" %.1f",tau);
      }
    }
  } else { /* high frequency band */
    if (smatauHighFreq[ant] < 0) {
      printw(" wacko");
    } else {
      sprintf(tauString,"%f",smatauHighFreq[ant]);
      if (present(tauString,"NaN")) {
	printw("  NaN ");
      } else if (smatauHighFreq[ant] < 10) {
	printw(" %.3f",smatauHighFreq[ant]);
      } else if (smatauHighFreq[ant] < 100) {
	printw(" %.2f",smatauHighFreq[ant]);
      } else {
	printw(" %.1f",smatauHighFreq[ant]);
      }
    }
  }
}


int LocateSkydips(int targetAntenna, int rightnow) {
 /*
    Set most recent directory to be the default
  */
  DIR *dirPtr;
  struct dirent *nextEnt;
  int day, time, antenna;
  long longvalue;

  dirPtr = opendir(SKYDIP_DIR);
  if(dirPtr == NULL) return(-1);
  newestSkydipUnixTime[targetAntenna] = 0;
  while ((nextEnt = readdir(dirPtr)) != NULL) {
    if (DEBUG) {
      move(1,0);
      printw("%s\n",nextEnt->d_name);
      refresh();
    }
    if (present(nextEnt->d_name, ".dip")) {
      sscanf(nextEnt->d_name,"%d_%d.dip%d",&day,&time,&antenna);
      if (antenna > 0 && antenna < 9) {
	if (antenna == targetAntenna) {
	  longvalue = ConvertSkydipDateTimeToUnixTime(day,time);
	  if (DEBUG && antenna==1) {
	    move(17,10);
	    printw("%s",skydipFilename[antenna]);
	    refresh();
	    sleep(1);
	  }
#define SKYDIP_TIME 240 /* time needed to complete a dip on 7 ants=14m40s */

	  if (longvalue > newestSkydipUnixTime[antenna] && (rightnow-longvalue)>SKYDIP_TIME) {
	    newestSkydipUnixTime[antenna] = longvalue;
	    strcpy(skydipFilename[antenna],nextEnt->d_name);
	    if (DEBUG && antenna==1) {
	      printw(" newest");
	    }
	  }
	  printw("\n");
	}
      }
    }
  }
#if 0
  if (strlen(skydipFilename[targetAntenna]) > 0) {
    skydipFilename[targetAntenna][strlen(skydipFilename[targetAntenna])-1] = 0;
  }
#endif
  closedir(dirPtr);
  if (DEBUG) {
    move(3,0);
    printw("returning with %s\n",skydipFilename[targetAntenna]);
    refresh();
  }
  return(0);
}

int usingServer(char *telescope) {
  if (strcmp(server,telescope)==0 ||
      strcmp(temperatureServer,telescope)==0 ||
      strcmp(humidityServer,telescope)==0 ||
      strcmp(pressureServer,telescope)==0 ||
      strcmp(windServer,telescope)==0
     ) {
    return(1);
  } else {
    return(0);
  }
}

void PrintAverageSkydip(int band, int correctForFreq, float avgtime) {
  float avg = 0;
  int i,ctr = 0;
  float maxTau = 0;
  float minTau = 9e+9;
#define OLDEST_ALLOWABLE -7200
  if (band == 0) {
    if (correctForFreq) {
      for (i=1; i<=8; i++) {
	if (smatauCorrectedTo225[i] == smatauCorrectedTo225[i] && 
	    smatauCorrectedTo225[i] > 0 && smatauCorrectedTo225[i]<3 &&
            (skydipTime[i] - avgtime > OLDEST_ALLOWABLE) ) {
	  avg += smatauCorrectedTo225[i];
	  ctr++;
	  if (smatauCorrectedTo225[i] > maxTau) {
	    maxTau = smatauCorrectedTo225[i];
	  }
	  if (smatauCorrectedTo225[i] < minTau) {
	    minTau = smatauCorrectedTo225[i];
	  }
	}
      }
    } else {
      for (i=1; i<=8; i++) {
	if (smatau[i] == smatau[i] && smatau[i] > 0 && smatau[i]<3 &&
            (skydipTime[i] - avgtime > OLDEST_ALLOWABLE)) {
	  avg += smatau[i];
	  ctr++;
	  if (smatau[i] > maxTau) {
	    maxTau = smatau[i];
	  }
	  if (smatau[i] < minTau) {
	    minTau = smatau[i];
	  }
	}
      }
    }
    avg -= maxTau;
    avg -= minTau;
    ctr -= 2;
    if (ctr > 0) {
      avg /= ctr;
    }
    if (avg <= -10 || avg > 100) {
      printw(" wacko");
    } else if (avg < 0) {
      printw(" %.2f",avg);
    } else if (avg < 10) {
      printw(" %.3f",avg);
    } else if (avg < 100) {
      printw(" %.2f",avg);
    } else {
      printw(" %.1f",avg);
    }
  } else { /* this is the high-frequency band data */
    for (i=1; i<=8; i++) {
      if (smatauHighFreq[i] == smatauHighFreq[i] && 
	  smatauHighFreq[i] > 0 && smatauHighFreq[i]<3 && 
          (skydipTime[i] - avgtime > OLDEST_ALLOWABLE)) {
	avg += smatauHighFreq[i];
	ctr++;
	if (smatauHighFreq[i] > maxTau) {
	  maxTau = smatauHighFreq[i];
	}
	if (smatauHighFreq[i] < minTau) {
	  minTau = smatauHighFreq[i];
	}
      }
    }
    avg -= maxTau;
    avg -= minTau;
    ctr -= 2;
    if (ctr > 0) {
      avg /= ctr;
    }
    if (avg <= -10) {
      printw(" wacko");
    } else if (avg < 0) {
      printw(" %.2f",avg);
    } else if (avg < 10) {
      printw(" %.3f",avg);
    } else if (avg < 100) {
      printw(" %.2f",avg);
    } else {
      printw(" %.1f",avg);
    }
  }
}

long ConvertSkydipDateTimeToUnixTime(int date,int time) {
  long longvalue;
  int year,month,day,hr,min,trueyear;
  int sec = 0;
#define DAYSEC 86400
  year = date/10000;
  trueyear = year+2000;
  month = date/100 - year*100;
  day = date-year*10000-month*100;
  hr = time/100;
  min = time-hr*100;
  longvalue = ConvertToUnixTime(trueyear,month,day,hr,min,sec);
  return(longvalue);
}

long printSkydipAge(int antenna, int rightnow) {
  int date,time,narg;
  int y,x;
  struct tm *now;
  long longvalue;

  narg = sscanf(skydipFilename[antenna],"%d_%d",&date,&time);
  if (narg == 2) {
    longvalue = ConvertSkydipDateTimeToUnixTime(date,time);
    printAgeNoStandoutSkykdip(rightnow,longvalue);    
  } else {
    printw("wacko ");
  }
  return(longvalue);
}

void printAgeNoStandoutSkykdip(int rightnow, int longvalue) {
  if (rightnow-longvalue < SKYDIP_TIME) {
    standout();
    printw("dippng ");
    standend();
  } else {
    printAgeNoStandout(rightnow,longvalue);    
  }
}   

float pwvToTau(float pwv) {
  float tau225;
  /* old formula derived by Dan from the old incorrect version of the am program: */
  /*  tau225 = 0.012 + 0.0575*pwv + 0.001*pow(pwv,2);*/

  /* new fit to the radiosonde PWV vs CSO tau */
  tau225 = 0.013 + 0.046*pwv;
  return(tau225);
}

