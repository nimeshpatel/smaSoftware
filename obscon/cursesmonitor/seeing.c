#include <curses.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <termio.h>
#include <time.h>
#include <rm.h>
#include "dsm.h"
#define PRINT_DSM_ERRORS 0
#define RADIUS_EARTH_M (6.378164e6)  /* equator, Allen 3rd edition */

#define NUMANTS 8
#define MAX_NUMBER_ANTENNAS 10  // only needed for antsAvailable array
#define MAX_ANTENNA_PAD 24
#define MAX_PADS (MAX_ANTENNA_PAD+1)
#define MAXSTATIONS 5
#define MAXBASE ((MAXSTATIONS*(MAXSTATIONS-1))/2)
#define dsmhost "phasemon"

/* direction of Ciel-2 from SMA (in degrees) */
char SATELLITE_NAME[50]; // "Ciel2"
float SATELLITE_AZ = 0; // 123.708
float SATELLITE_EL = 0; // 51.64  /* 29 day average from Dec 6, 2009 */

extern int antsAvailable[MAX_NUMBER_ANTENNAS+1];
double cosd(double a);
double sind(double a);
typedef struct {
    double latitudeDegrees;
    double longitudeDegrees;
    double heightFeet;
    double heightMeter;
    double deltaLatitudeMeters;
    double deltaLongitudeMeters;
    double deltaHeightMeter;
    int covered;
} PADLOCATION;
void loadPadLocations(PADLOCATION *p);

void loadbit(char *c, unsigned char v) {
  if (v > 1) {
    sprintf(c,"?");
  } else {
    sprintf(c,"%d",v);
  }
}

void timestring(int t, char *a) {
  int sec = pow(2.0,t);
  sprintf(a,"%d",sec);
  if (pow(2,t) < 10) {
    strcat(a," sec");
  } else if (pow(2,t) < 100) {
    strcat(a,"sec");
  } else {
    strcat(a," s");
  }
}

void seeing(int count, int *rm_list) {
  static int firstCall = TRUE;
  FILE *fp;
  static dsm_structure ds1, ds2;
  char b[100], c[100];
  int i, j, s, t, row, dsm_status, temperatureRow, temperatureColumn;

/* The following four arrays are dynamically read (or created) 
 * from the contents of the configFile. */
  static int pad[MAXSTATIONS];
  static int RF[MAXBASE];
  static int LO[MAXBASE];

  static int nbase, nstations;
  static struct  {
    double x;
    double y;
    double z;
    double projected;
    double unprojected;
    double elevation;
    double azimuth;
  } baselineVector[MAXBASE];
  static PADLOCATION padLocation[MAX_PADS];

  double amplitude[MAXBASE], atmosphericPhase[MAXBASE];
  double rawPhase[MAXBASE], unwrappedPhase[MAXBASE];
  double rms[MAXBASE][12], cosTheta;
  float rfpower[MAXSTATIONS];
  unsigned char synthLock;
  unsigned char rackOptTx[MAXSTATIONS];
  unsigned char rackOptRx[MAXSTATIONS];
  unsigned char padOptTx[MAXSTATIONS];
  unsigned char padOptRx[MAXSTATIONS];
  char seconds[10];
  float temperature[MAXSTATIONS+1];
  float humidity[MAXSTATIONS+1];
  float fiveVolts[MAXSTATIONS];
  float minusFiveVolts[MAXSTATIONS];
  float minusFifteenVolts[MAXSTATIONS];
  float twelveVolts[MAXSTATIONS];
  float twentyFiveVolts[MAXSTATIONS];
  float fifteenVoltsA[MAXSTATIONS];
  float fifteenVoltsB[MAXSTATIONS];
  char string1[100], *ptr, linebuf[100];
  static long rightnow;
  int narg, ant;
  time_t timestamp, curTime, timestampData;
  static short padid[9];

  if (firstCall) {
    for (i=0; i<MAXSTATIONS; i++) {
      pad[i] = 0;
    }
    for (i=0; i<MAXBASE; i++) {
      RF[i] = 0;
      LO[i] = 0;
    }
    loadPadLocations(padLocation);
    for(ant = 1; ant <= NUMANTS; ant++) {
      if(antsAvailable[ant]) {
	unsigned char dummyByte;
	(void)rm_read(ant,"RM_PAD_ID_B",&dummyByte);
	padid[ant]=(short)dummyByte;
      }
    }

    /* initialize the 2 dsm structures for the phase monitor */
    
    s = dsm_structure_init(&ds1, "PHASEMON_DATA_X");
    if(s!=DSM_SUCCESS) {
      dsm_error_message(s, "dsm_structure_init()");
      exit(1);
    }
    
    s = dsm_structure_init(&ds2, "PHASEMON_HRDWR_STAT_X");
    if(s!=DSM_SUCCESS) {
      dsm_error_message(s, "dsm_structure_init()");
      exit(1);
    }
    fp = fopen("/global/configFiles/phaseMonitor.config","r");
    /* format should be:
     # any number of initial comment line(s)
     3 # number of stations installed
     22
     10
     14
     3 # number of active baselines (first number is RF, second is LO)
     22 10
     10 14
     14 22
    */
    if (fp == NULL) {
      nbase = 6;
      nstations = 4;
      pad[0] = 22;
      pad[1] = 10;
      pad[2] = 14;
      pad[3] = 15;
      RF[0] = 22;
      LO[0] = 10;
    } else {
      i=-1;
      /* read through any initial comments */
      do {
	ptr = fgets(linebuf,sizeof(linebuf),fp);
	if (ptr != NULL) {
	  if (strstr(ptr,"#") == NULL) break;
	}
      } while (ptr != NULL);

      /* read the satellite name and position */
      narg = sscanf(linebuf,"%s %f %f",SATELLITE_NAME, &SATELLITE_AZ, &SATELLITE_EL);

      /* read the number of installed stations */
      ptr = fgets(linebuf,sizeof(linebuf),fp);
      narg = sscanf(linebuf,"%d",&nstations);
      /* read the pad numbers */
      for (i=0; i<nstations; i++) {
	ptr = fgets(linebuf,sizeof(linebuf),fp);
	if (ptr != NULL) {
	  narg = sscanf(linebuf,"%d",&pad[i]);
	}
      }
      /* read the number of active baselines */
      ptr = fgets(linebuf,sizeof(linebuf),fp);
      narg = sscanf(linebuf,"%d",&nbase);
      /* read the RF LO configurations */
      for (i=0; i<MAXBASE; i++) {
	ptr = fgets(linebuf,sizeof(linebuf),fp);
	if (ptr != NULL) {
	  narg = sscanf(linebuf,"%d %d",&RF[i],&LO[i]);
	} else {
	  RF[i] = LO[i] = 0;
	}
      }
    }
    /* check for covered pads */
    for (i=0; i<=MAX_ANTENNA_PAD; i++) {
      padLocation[i].covered = 0;
    }
    for (i=0; i<nstations; i++) {
      for (ant=1; ant<=NUMANTS; ant++) {
	if (padid[ant] == pad[i]) {
	  padLocation[pad[i]].covered = ant;
	}
      }
    }
    // test:
    //padLocation[10].covered = 1;
    //padLocation[14].covered = 2;
    for (i=0; i<MAXBASE; i++) {
      if (padLocation[RF[i]].deltaLongitudeMeters < padLocation[LO[i]].deltaLongitudeMeters) {
	baselineVector[i].x = padLocation[LO[i]].deltaLongitudeMeters-padLocation[RF[i]].deltaLongitudeMeters;
	baselineVector[i].y = padLocation[LO[i]].deltaLatitudeMeters-padLocation[RF[i]].deltaLatitudeMeters;
	baselineVector[i].z = padLocation[LO[i]].deltaHeightMeter-padLocation[RF[i]].deltaHeightMeter;
      } else {
	baselineVector[i].x = padLocation[RF[i]].deltaLongitudeMeters-padLocation[LO[i]].deltaLongitudeMeters;
	baselineVector[i].y = padLocation[RF[i]].deltaLatitudeMeters-padLocation[LO[i]].deltaLatitudeMeters;
	baselineVector[i].z = padLocation[RF[i]].deltaHeightMeter-padLocation[LO[i]].deltaHeightMeter;
      }
      baselineVector[i].unprojected = pow(pow(baselineVector[i].x,2)+pow(baselineVector[i].y,2)+pow(baselineVector[i].z,2),0.5);
      baselineVector[i].elevation = asin(baselineVector[i].z / baselineVector[i].unprojected)*45/atan(1.0);
      baselineVector[i].azimuth = 90 - (45/atan(1.0))*atan(baselineVector[i].y / baselineVector[i].x);
      //      fprintf(stderr,"%f %f %f --> %f, %f\n", baselineVector[i].x, baselineVector[i].y, baselineVector[i].z,
      //	      baselineVector[i].azimuth, baselineVector[i].elevation);
      /* formula from http://saj.matf.bg.ac.yu/177/pdf/115-124.pdf */
      cosTheta = cosd(baselineVector[i].elevation)*cosd(SATELLITE_EL)*cosd(baselineVector[i].azimuth-SATELLITE_AZ) +
	sind(baselineVector[i].elevation)*sind(SATELLITE_EL);
      baselineVector[i].projected = baselineVector[i].unprojected*sin(acos(cosTheta));
    }
    firstCall = FALSE;
  }

  if ((count % 60) == 1) {
    /*
      Initialize the Curses Display
    */
    initscr();
    clear();
    move(1,1);
    refresh();
  }
  move(0,0);
  curTime = time((long *)0);
  strcpy(string1, ctime(&curTime));
  string1[strlen(string1)-1] = (char)0;
  printw("12 GHz Phase Monitor Status at %s.  %s (az=%.0f,el=%.0f)", 
	 string1,SATELLITE_NAME,SATELLITE_AZ,SATELLITE_EL); 

  dsm_status = dsm_read(dsmhost,"PHASEMON_DATA_X",&ds1,&timestampData);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(PHASEMON_DATA_X)");
  } 
  s = dsm_structure_get_element(&ds1, "CORR_AMPLITUDE_V10_D", amplitude);
  if(s!=DSM_SUCCESS) {
    dsm_error_message(s, "dsm_get_element()");
    exit(1);
  }
  s = dsm_structure_get_element(&ds1, "RAW_PHASE_V10_D", rawPhase);
  if(s!=DSM_SUCCESS) {
    dsm_error_message(s, "dsm_get_element()");
    exit(1);
  }
  s = dsm_structure_get_element(&ds1, "UNWRPD_PHASE_V10_D", unwrappedPhase);
  if(s!=DSM_SUCCESS) {
    dsm_error_message(s, "dsm_get_element()");
    exit(1);
  }
  s = dsm_structure_get_element(&ds1, "ATMOSPHERIC_PHASE_V10_D", atmosphericPhase);
  if(s!=DSM_SUCCESS) {
    dsm_error_message(s, "dsm_get_element()");
    exit(1);
  }

  dsm_status = dsm_structure_get_element(&ds1,"RMS_PHASE_V10_V12_D",rms);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(RMS_PHASE_V10_V12_D)");
  }
  

  /****************************/
     
  dsm_status = dsm_read(dsmhost,"PHASEMON_HRDWR_STAT_X",&ds2,&timestamp);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(PHASEMON_HARDWARE_STAT_X)");
  }
 
  dsm_status = dsm_structure_get_element(&ds2,"RF_PWR_V5_F",rfpower);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(RF_PWR_V5_F)");
  } 
  /* dev1 AI21 = -5V  (-0.967 at ADC) */
  dsm_status = dsm_structure_get_element(&ds2,"NEG_5V_RACK_F",minusFiveVolts);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(NEG_5V_RACK_F)");
  } 
  /* dev1 AI23 = +5V (+0.957 at ADC) */
  dsm_status = dsm_structure_get_element(&ds2,"POS_5V_RACK_F",fiveVolts);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(POS_5V_RACK_F)");
  } 
  /* dev1 AI22 = +12V (+2.35 at ADC) */
  dsm_status = dsm_structure_get_element(&ds2,"POS_12V_RACK_F",twelveVolts);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(POS_12V_RACK_F)");
  } 
  /* dev1 AI29 = +25V (+4.83 at ADC) */
  dsm_status = dsm_structure_get_element(&ds2,"POS_25V_RACK_F",twentyFiveVolts);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(POS_25V_RACK_F)");
  } 
  /* dev2 AI21 = -15V (-2.92 at ADC) */
  dsm_status = dsm_structure_get_element(&ds2,"NEG_15V_RACK_F",minusFifteenVolts);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(NEG_15V_RACK_F)");
  } 
  /* dev2 AI22, 23 = +15V_A, B (which is which ?)  +2.89 & +2.88 at ADC */
  dsm_status = dsm_structure_get_element(&ds2,"POS_15VA_RACK_F",fifteenVoltsA);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(POS_15VA_RACK_F)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"POS_15VB_RACK_F",fifteenVoltsB);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(POS_15VB_RACK_F)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"SYNTH_LOCK_B",&synthLock);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(SYNTH_LOCK_B)");
  } 
  /* note the spelling error on the next line */
  dsm_status = dsm_structure_get_element(&ds2,"TX_STAT_RACK_V5_B",rackOptTx);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(TX_STAT_RACK_V5_B)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"RX_STAT_RACK_V5_B",rackOptRx);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(RX_STAT_RACK_V5_B)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"TX_STAT_PAD_V5_B",padOptTx);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(TX_STAT_PAD_V5_B)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"RX_STAT_PAD_V5_B",padOptRx);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(RX_STAT_PAD_V5_B)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"HUM_SENSE_V6_F",humidity);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(HUM_SENSE_V6_F)");
  } 
  dsm_status = dsm_structure_get_element(&ds2,"TEMP_SENSE_V6_F",temperature);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_read(TEMP_SENSE_V6_F)");
  } 

  move(row=1,0);
  if (fp == NULL) {
    addstr("BASELINE:   1-2    1-3    1-4    1-5    2-3    2-4    2-5    3-4    3-5    4-5");
  } else {
    sprintf(c,"BASELINE:");
    addstr(c);
    for (i=0; i<MAXBASE; i++) {
      sprintf(c,"  %02d-%02d",RF[i],LO[i]);
      addstr(c);
    }
  }

  /* baseline lengths */
  move(++row,0);

  addstr("Unprojected");
  for (i=0; i<MAXBASE; i++) {
    if (padLocation[RF[i]].covered || padLocation[LO[i]].covered) {
      printw("Ant#%d  ",
	     (padLocation[RF[i]].covered>0? padLocation[RF[i]].covered: padLocation[LO[i]].covered));
    } else {
      sprintf(b," %3.0fm  ",baselineVector[i].unprojected);
      addstr(b);
    }
  }
  clrtoeol();

  move(++row,0);
  addstr("Projected  ");
  for (i=0; i<MAXBASE; i++) {
    if (padLocation[RF[i]].covered || padLocation[LO[i]].covered) {
      sprintf(b,"       ");
    } else {
      sprintf(b," %3.0fm  ",baselineVector[i].projected);
    }
    addstr(b);
  }
  clrtoeol();
  extern dsm_structure smaWeather;
  long smaTimestamp, longvalue;
  float floatvalue,winddir;
#define DSM_HOST "colossus"
  dsm_status = dsm_read(DSM_HOST,"SMA_METEOROLOGY_X", &smaWeather, &smaTimestamp);
  dsm_status = dsm_structure_get_element(&smaWeather,"WINDDIR_F", &winddir);
  dsm_status = dsm_structure_get_element(&smaWeather,"SERVER_TIMESTAMP_L", &longvalue);

  move(++row,0);
  addstr("Azimuth(deg)");
  for (i=0; i<MAXBASE; i++) {
    sprintf(b," %3.0f   ",baselineVector[i].azimuth);
    addstr(b);
  }

  move(++row,0);
  addstr("AngleToWind ");
  dsm_status = rm_read(rm_list[0], "RM_UNIX_TIME_L", &rightnow);
  for (i=0; i<MAXBASE; i++) {
    floatvalue = fabs(baselineVector[i].azimuth - winddir);
    if (floatvalue > 180) {
      floatvalue = floatvalue-180;
    } 
    if (floatvalue > 90) {
      floatvalue = 180-floatvalue;
    }
    if (rightnow - longvalue > 60*30) {
      sprintf(b," stale ");
    } else {
      sprintf(b," %3.0f   ",floatvalue);
    }
    addstr(b);
  }
  clrtoeol();

  move(++row,0);

  printw("Amplitude");
  for (i=0; i<nbase; i++) {
    if ((padLocation[RF[i]].covered || padLocation[LO[i]].covered) && 1) {
      sprintf(c,"       ");
    } else {
      if (rightnow-timestampData > 3) {
	standout();
	sprintf(c," stale ");
      } else if (fabs(amplitude[i])>=100000) {
	sprintf(c,"  wacko ");
      } else if (amplitude[i]>=1000) {
	sprintf(c,"%7.1f",amplitude[i]);
      } else {
	sprintf(c,"%7.1f",amplitude[i]);
      }
      if (amplitude[i] < 300) {
	standout();
      }
    }
    addstr(c);
    standend();
  }
  if (nbase < MAXBASE) {
    addstr("   --- Future baselines ---");
  }
  clrtoeol();

  move(++row,0);
  strcpy(b,"Raw Phase");
  for (i=0; i<nbase; i++) {
    if ((padLocation[RF[i]].covered || padLocation[LO[i]].covered) && 1) {
      sprintf(c,"       ");
    } else {
      if (fabs(rawPhase[i])>=100000) {
	sprintf(c," wacko ");
      } else {
	sprintf(c," %+6.1f",rawPhase[i]);
      }
    }
    strcat(b,c);
  }
  addstr(b);
  clrtoeol();

  move(++row,0);
  strcpy(b,"Unwrapped");
  for (i=0; i<nbase; i++) {
    if (padLocation[RF[i]].covered || padLocation[LO[i]].covered) {
      sprintf(c,"       ");
    } else {
      if (fabs(unwrappedPhase[i]) >= 100000) { 
	sprintf(c," wacko ");
      } else if (fabs(unwrappedPhase[i]) >= 1000) { 
	sprintf(c," %+6.0f",unwrappedPhase[i]);
      } else {
	sprintf(c," %+6.1f",unwrappedPhase[i]);
      }
    }
    strcat(b,c);
  }
  addstr(b);
  clrtoeol();

  move(++row,0);
  strcpy(b,"Atmospher");
  for (i=0; i<nbase; i++) {
    if (padLocation[RF[i]].covered || padLocation[LO[i]].covered) {
      sprintf(c,"       ");
    } else {
      if (fabs(atmosphericPhase[i]) >= 100000) { 
	sprintf(c," wacko ");
      } else if (fabs(atmosphericPhase[i]) < 10) {
	sprintf(c," %+6.3f",atmosphericPhase[i]);
      } else if (fabs(atmosphericPhase[i]) < 100) {
	sprintf(c," %+6.2f",atmosphericPhase[i]);
      } else if (fabs(atmosphericPhase[i]) < 1000) {
	sprintf(c," %+6.1f",atmosphericPhase[i]);
      } else if (fabs(atmosphericPhase[i]) < 10000) {
	sprintf(c," %+6.0f",atmosphericPhase[i]);
      } else {
	sprintf(c," wacko ");
      }
    }
    strcat(b,c);
  }
  addstr(b);
  clrtoeol();
     

  for (t=0; t<10; t++) {
    int mt = t;
    move(++row,0);
    timestring(t,seconds);
    sprintf(b,"Rms %s",seconds);
    for (i=0; i<nbase; i++) {
      if (padLocation[RF[i]].covered || padLocation[LO[i]].covered) {
        sprintf(c,"       ");
      } else if (rms[i][mt] < 100) {
	sprintf(c," %6.3f",rms[i][mt]);
      } else if (rms[i][mt] < 1000) {
	sprintf(c," %6.2f",rms[i][mt]);
      } else if (rms[i][mt] < 10000) {
	sprintf(c," %6.1f",rms[i][mt]);
      } else if (rms[i][mt] < 100000) {
	sprintf(c," %6.0f",rms[i][mt]);
      } else {
	sprintf(c," wacko ");
      }
      strcat(b,c);
    }
    addstr(b);
    clrtoeol();
  }
  move(++row,0);
  strcpy(b,"STATION/Pad:");
  for (i=0; i<MAXSTATIONS; i++) {
    if (pad[i] < 1) {
#if 1
      sprintf(c," %d/18  ",i+1);
      strcat(b,c);
#endif
    } else {
      sprintf(c," %d/%02d  ",i+1,pad[i]);
      strcat(b,c);
    }
  }
  addstr(b);

  temperatureRow = row;
#if 0
  move(++row,0);
  strcpy(b,"Temper(C)");
  for (i=0; i<nstations; i++) {
    sprintf(c," %+6.2f",temperature[i]);
    strcat(b,c);
  }
  addstr(b);
  if (nstations < 5) {
    for (i=nstations; i<5; i++) {
      addstr("       ");
    }
  }
  /*
    sprintf(c,"%+6.2f",temperature[MAXSTATIONS]);
    addstr(c);
  */

  move(++row,0);
  strcpy(b,"Humidity%");
  for (i=0; i<nstations; i++) {
    sprintf(c," %6.2f",humidity[i]);
    strcat(b,c);
  }
  addstr(b);
  if (nstations < 5) {
    for (i=nstations; i<5; i++) {
      addstr("       ");
    }
  }
  /*
    sprintf(c,"%+6.2f",humidity[MAXSTATIONS]);
    addstr(c);
  */
#endif

  move(++row,0);
  printw("RFPower_dBm");
  for (i=0; i<nstations; i++) {
    for (j=0; j<nbase; j++) {
      if (pad[i]==RF[j]) {
	if (rfpower[i] <= 0.0001) {
	  sprintf(b," %+6.1f",100*(rfpower[i]-1));
	} else {
	  sprintf(b," %+6.2f",100*(rfpower[i]-1));
	}
	break;
      }
      if (j==nbase-1) {
	strcpy(b,"  n/a  ");
      }
    }
    // do not highlight if antenna covers pad - Todd
    if (padLocation[pad[i]].covered == 0) {
      if (rfpower[i] <= 0.6) {  // +0.6 == -40 dBm
	standout();
      }
    }
    addstr(b);
    standend();
  }
  move(++row,0);
  strcpy(b,"RackOptTx/Rx");
  for (i=0; i<nstations; i++) {
    strcat(b," ");
    loadbit(c,rackOptTx[i]);
    strcat(b,c);
    strcat(b,"/");
    loadbit(c,rackOptRx[i]);
    strcat(b,c);
    strcat(b,"   ");
  }
  addstr(b);

  move(++row,0);
  strcpy(b,"Pad OptTx/Rx");
  for (i=0; i<nstations; i++) {
    strcat(b," ");
    loadbit(c,padOptTx[i]);
    strcat(b,c);
    strcat(b,"/");
    loadbit(c,padOptRx[i]);
    strcat(b,c);
    strcat(b,"   ");
  }
  addstr(b);

  temperatureColumn = 48;
  //  printw("Temp %+5.1fC  Humid %3.0f%%", 
  //	  temperature[MAXSTATIONS], humidity[MAXSTATIONS]);
  move(20,temperatureColumn-1);
  addstr("Vault Rack Voltages  SynthLock=");
  loadbit(c,synthLock);
  addstr(c);

#if 0
  move(++row,4);
  addstr("**** 12 GHz phase monitor page ****");
#endif
#if 0
  move(temperatureRow-2,48);
  bzero(b,sizeof(b));
  for (i=0; i<nstations; i++) {
    sprintf(c,"     %d",i+1);
    strcat(b,c);
  }
  addstr(b);
#endif

  //  move(row=temperatureRow,64);
  //  printw("Rack Voltages");

  move(row=temperatureRow+1,temperatureColumn);
  sprintf(b,"5V: ");
  int nvoltages = 1;
  for (i=0; i<nvoltages; i++) {
    if (fabs(minusFiveVolts[i]) > 100) {
      sprintf(c,"wacko");
    } else {
      sprintf(c,"%+5.2f,",minusFiveVolts[i]);
    }
    strcat(b,c);
  }
  for (i=0; i<nvoltages; i++) {
    if (fabs(fiveVolts[i]) > 100) {
      sprintf(c,"wacko");
    } else {
      sprintf(c,"%+5.2f",fiveVolts[i]);
    }
    strcat(b,c);
  }
  addstr(b);
  int temperatureColumn2 = temperatureColumn+17;
  move(row,temperatureColumn2);
  sprintf(b," -15V:");
  for (i=0; i<nvoltages; i++) {
    if (fabs(minusFifteenVolts[i]) > 100) {
      sprintf(c," wacko");
    } else {
      sprintf(c," %+6.2f",minusFifteenVolts[i]);
    }
    strcat(b,c);
  }
  addstr(b);

  move(++row,temperatureColumn);
  sprintf(b,"+12V:");
  for (i=0; i<nvoltages; i++) {
    if (fabs(twelveVolts[i]) > 100) {
      sprintf(c," wacko");
    } else {
      sprintf(c," %+6.2f",twelveVolts[i]);
    }
    strcat(b,c);
  }
  addstr(b);

  move(++row,temperatureColumn);
  //  sprintf(b,"+25V:");
  sprintf(b,"+25V:");
  for (i=0; i<nvoltages; i++) {
    if (fabs(twentyFiveVolts[i]) > 100) {
      sprintf(c," wacko");
    } else {
      sprintf(c," %+6.2f",twentyFiveVolts[i]);
    }
    strcat(b,c);
  }
  addstr(b);

  move(--row,temperatureColumn2);
  sprintf(b,"+15Va:");
  for (i=0; i<nvoltages; i++) {
    if (fabs(fifteenVoltsA[i]) > 100) {
      sprintf(c," wacko");
    } else {
      sprintf(c," %+6.2f",fifteenVoltsA[i]);
    }
    strcat(b,c);
  }
  addstr(b);

  move(++row,temperatureColumn2);
  sprintf(b,"+15Vb:");
  for (i=0; i<nvoltages; i++) {
    if (fabs(fifteenVoltsB[i]) > 100) {
      sprintf(c," wacko");
    } else {
      sprintf(c," %+6.2f",fifteenVoltsB[i]);
    }
    strcat(b,c);
  }
  addstr(b);
 
  move(23,79);
  refresh();
  return;
}

void loadPadLocations(PADLOCATION *p) {
  int referencePad = 1;
  int i=0;
  p[++i].latitudeDegrees = 19+ 49/60.+ 27.13895/3600.;
  p[i].longitudeDegrees =   -155 -28/60. -39.08279/3600.;
  p[i].heightFeet = 13398.78;

  p[++i].latitudeDegrees = 19+ 49/60.+ 26.94917/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.33943/3600.;
  p[i].heightFeet =    13398.76;

  p[++i].latitudeDegrees = 19+ 49/60.+ 26.87152/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.89930/3600.;
  p[i].heightFeet =    13398.58;

  p[++i].latitudeDegrees = 19+ 49/60.+ 27.18375/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.94725/3600.;
  p[i].heightFeet =    13398.66;

  p[++i].latitudeDegrees = 19+ 49/60.+ 27.67941/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.73510/3600.;
  p[i].heightFeet =    13398.53;

  p[++i].latitudeDegrees = 19+ 49/60.+ 27.56212/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.43020/3600.;
  p[i].heightFeet =    13398.74;

  p[++i].latitudeDegrees = 19+ 49/60.+ 26.62748/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.77310/3600.;
  p[i].heightFeet =    13398.45;

  p[++i].latitudeDegrees = 19+ 49/60.+ 26.42186/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -41.27819/3600.;
  p[i].heightFeet =    13388.25;

  p[++i].latitudeDegrees = 19+ 49/60.+ 27.32098/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -41.42026/3600.;
  p[i].heightFeet =    13383.08;

  /* 10 */
  p[++i].latitudeDegrees = 19+ 49/60.+ 28.59855/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -40.83621/3600.;
  p[i].heightFeet =    13376.86;

  p[++i].latitudeDegrees = 19+ 49/60.+ 28.26256/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -40.01168/3600.;
  p[i].heightFeet =    13381.95;

  p[++i].latitudeDegrees = 19+ 49/60.+ 25.76368/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -40.92714/3600.;
  p[i].heightFeet =    13389.77;

  p[++i].latitudeDegrees = 19+ 49/60.+ 25.23423/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -45.30245/3600.;
  p[i].heightFeet =    13361.10;

  /* 14 */
  p[++i].latitudeDegrees = 19+ 49/60.+ 27.24870/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -45.22794/3600.;
  p[i].heightFeet =    13361.20;

  p[++i].latitudeDegrees = 19+ 49/60.+ 31.00812/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -43.78229/3600.;
  p[i].heightFeet =    13360.15;

  p[++i].latitudeDegrees = 19+ 49/60.+ 30.24568/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -41.54153/3600.;
  p[i].heightFeet =    13363.22;

  p[++i].latitudeDegrees = 19+ 49/60.+ 23.65967/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -43.67432/3600.;
  p[i].heightFeet =    13364.74;

  p[++i].latitudeDegrees = 19+ 49/60.+ 28.52391/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -51.09963/3600.;
  p[i].heightFeet =    13317.79;

  p[++i].latitudeDegrees = 19+ 49/60.+ 32.60242/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -55.54445/3600.;
  p[i].heightFeet =    13280.12;

  p[++i].latitudeDegrees = 19+ 49/60.+ 38.05364/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -47.72787/3600.;
  p[i].heightFeet =    13337.83;

  p[++i].latitudeDegrees = 19+ 49/60.+ 37.58490/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -41.60842/3600.;
  p[i].heightFeet =    13332.20;

  /* 22 */
  p[++i].latitudeDegrees = 19+ 49/60.+ 27.43838/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -39.89831/3600.;
  p[i].heightFeet =    13398.69;

  p[++i].latitudeDegrees = 19+ 49/60.+ 28.25633/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -41.12983/3600.;
  p[i].heightFeet =    13376.99;

  p[++i].latitudeDegrees = 19+ 49/60.+ 30.70662/3600.;
  p[i].longitudeDegrees =    -155 -28/60. -44.92058/3600.;
  p[i].heightFeet =    13360.28;
  for (i=1; i<MAX_PADS; i++) {
    p[i].heightMeter = p[i].heightFeet*12/39.37;
    /* meters = degrees * rad/deg * radius_meters * cos(latitude*rad/deg)  */
    p[i].deltaLongitudeMeters = RADIUS_EARTH_M*cos(p[i].latitudeDegrees*atan(1.0)/45.)*(p[i].longitudeDegrees-p[referencePad].longitudeDegrees)*atan(1.0)/45.0;
    p[i].deltaLatitudeMeters = RADIUS_EARTH_M*(p[i].latitudeDegrees-p[referencePad].latitudeDegrees)*atan(1.0)/45.0;
    p[i].deltaHeightMeter = p[i].heightMeter - p[referencePad].heightMeter;
  }
}

double sind(double a) {
  return(sin(a*atan(1.0)/45.));
}

double cosd(double a) {
  return(cos(a*atan(1.0)/45.));
}
