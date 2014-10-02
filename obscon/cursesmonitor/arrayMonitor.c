#define HUMIDITY_POSITIVE_WACKO 150
#define HUMIDITY_NEGATIVE_WACKO -10
#define PRESSURE_POSITIVE_WACKO 800
#define PRESSURE_NEGATIVE_WACKO 300
#define WIND_METER_FROZEN_CUTOFF 0.20 /* mph */
#define DSM_WEATHER_HOST "colossus"
#define BDA_YIG_SWITCH_POS 7
#define DEBUG 0
#include <stdio.h>
#include <math.h>
#include <curses.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <commonLib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "esma.h"
#include "rm.h"
#include "dsm.h"
#include "s_cmd2.h"
#include "optics.h" /* found under rxCode/src/include */
#include "deiced.h"
#include "monitor.h"
#include "allanVariance.h"
#include "antMonitor.h"
#include "receiverMonitor.h"
#include "receiverMonitorHighFreq.h"
#include "c1DC.h"
#include "iFLOMonitorPage2.h"
#include "opticsMonitor.h"
#include "tune6status.h"
#include "weather.h"
#include "chopperControl.h"
#include "commandMonitor.h"
#include "commonLib.h"
#include "upspage.h"

#define DESC_STRING_SIZE 256
#define LITTLE_UPDATE_INTERVAL 1
#define TRACKING_ERROR_THRESHOLD 1.0	

int doubleBandwidth = FALSE;
int fullPolarization = FALSE;
int wackyRxTemp[10];
float STANDOUT_TAU(float freqHz);
float computeMedianWindspeed(float *medianDirection, int *invalidWinds);
extern dsm_structure smaWeather, jcmtWeather, irtfWeather, ukirtWeather, cfhtWeather;
extern dsm_structure csoWeather, keckWeather, vlbaWeather ,subaruWeather, uh88Weather;
extern time_t intruderTimestamp;
extern int weatherUnits;
extern int projectLockout;
extern void intruderPage(int count);
extern void messagePage(int count, int kind);
int isDoubleBandwidth(void);
int isFullPolarization(void);
void printNewHotloadPosition(int antenna);
int mirrorDoorIsOpen(int m3stat);
int timeProblems(double error[MAX_NUMBER_ANTENNAS+1]);
double timeDifferential(int antennaNumber);
short mRGLocked[2];
int numberAntennas = 8; /* default to non-eSMA */
extern dsm_structure mRGControl, phasemon_data;
extern dsm_structure crateStatusStructure[13];
unsigned int counterSmoke = 0;
int ignoreSmoking[11];
unsigned long scbFaultWord;
float antElCur[9][31]={{0},{0}};
float antAz1Cur[9][31]={{0},{0}},antAz2Cur[9][31]={{0},{0}};
int sample[8]={0};
int shortestPMBaseline = 0;

void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign);

/* Make a Beep only when a new fault occurs or a fault recurs
 * after TIME_TO CLEAR seconds of being clear.
 * Beepable faults will be tracked by FAULT_NUMBER with antenna number
 * added if appropriate.
 */
#define TIME_TO_CLEAR 60

/* This first batch of faults should have antenna number added to them. */
#define SUN_DISTANCE -1
#define DRIVE_FAULT (SUN_DISTANCE + NUMANTS)
#define CHOPPER_FAULT (SUN_DISTANCE + 2*NUMANTS)
#define M3_FAULT (SUN_DISTANCE + 3*NUMANTS)
#define LO_FAULT (SUN_DISTANCE + 4*NUMANTS)
#define DEWAR_WARM (SUN_DISTANCE + 5*NUMANTS)
/* The remainder are single faults */
#define CORCON_SHUTDOWN (6*NUMANTS)
#define CORR_NOT_SYNCD CORCON_SHUTDOWN+1
#define NUM_FAULTS (CORR_NOT_SYNCD + 1)
unsigned char beepControl[NUM_FAULTS];
int needsBeep = 0;

void printWeatherServer(char *server);
void Beep(int faultNum);
void NoBeep(int faultNum);
int computeUTstale(float hours, float margin, float uthours);
char deiceChar[MAX_NUMBER_ANTENNAS + 1];
int tiltCorrections[MAX_NUMBER_ANTENNAS + 1];
int scbFaultWordArray[MAX_NUMBER_ANTENNAS+1];
void printCalWheelStatus(int orientation);
void printCalWheelStatus3Char(int orientation);
int smaWindAsterisk = 0;
double saoSeeingAmplitude[10];

void checkForNulls(char *string) {
  char junk[DESC_STRING_SIZE];
  
  strcpy(junk, string);
  if (!strtok(junk, " \t"))
    string[0] = (char)0;
  if (!strtok(junk, "\r"))
    string[0] = (char)0;
  if (!strtok(junk, "\n"))
    string[0] = (char)0;
}

int notEmptyString(char *string)
     /*
       Returns TRUE IFF string length is > 0 and string contains at least one
       printable character.
     */
{
  int i, len;
  
  len = strlen(string);
  if (len == 0)
    return(0);
  for (i = 0; i < len; i++)
    if ((string[i] >= '!') &&
	(string[i] < (char)0x7f))
      return(1);
  return(0);
}   

double sunDistance(double az1,double el1,double az2,double el2);


void arrayscreen(
		 char *projectPI, char *projectDescription, int projectID,
		 char *observer, char *operatingLocation,
		 char *antennasInArray,char *timeAndDate, char *source,double *lst_disp,
		 double *utc_disp,double *tjd_disp, float *epoch,
		 double *ra_disp,double *dec_disp, double *ra_cat_disp, 
		 double *dec_cat_disp, int *icount, float *pressure, 
		 float *temperature, float *humidity,
		 float *windspeed, float *winddirection,
		 int *antennaNumber, short *padid,double *planetdistance, double
		 *planetdiameter, double *sourceVel,
		 float *csoTau, float *saoSeeing, float *saoSeeingRatio, long saoSeeingTimestamp,
		 short *driveStatus, short *m3Status, 
		 short *chopperStatus, short *gunn1Status, short *yig1Status, short *gunn2Status,
		 short *yig2Status,float *dewarTemp,short *calWheelStatus,double restFrequency[2],
		 short sideband[2], double mRGFreq[2], double *tsys, double *tsys2, float *az, float *el,
		 int *onSource,
		 double *sunD, int *isAntennaInArray, int *isReceiverInArray, int *trackStatus, double *raoffset,
		 double *decoffset, int *polarMJD, double *polarDx, double *polarDy, double *polarDut,
		 long *dewarTime, time_t *timeStamp, long *calVaneTime,
		 int ignoreIntruders, float *csoTauFromSimonRadfordMeter, 
		 double sunaz, double sunel, int *invalidWinds,
		 int antlist[RM_ARRAY_SIZE]);

int arrayDisplay(int icount, int ignoreIntruders, int antlist[RM_ARRAY_SIZE]) {
  
  FILE *fp;
  char dummyByte, faultState;
  int rms;
  int trackStatus[20];
  extern double radian;
  char source[SOURCE_CHAR_LEN] ;
  double lst_disp, utc_disp, tjd_disp;
  float epoch;
  double ra_disp, dec_disp, ra_cat_disp, dec_cat_disp;
  float pressure, temperature, humidity;
  float windspeed ,winddirection, refraction;
  double planetdistance;
  double planetdiameter;
  int polarMJD=0;
  double polarDx=0.,polarDy=0.,polarDut=0.;
  double sunaz,sunel;
  float dummyFloat, dummyFloat16[16];
  double dummyDouble;
  short dummyShort;
  short padid[MAX_NUMBER_ANTENNAS+1];
  int ant;
  int specialAntenna=1;
  char antennasInArray[200];
  int isAntennaInArray[20];
  int isReceiverInArray[3];
  int projectID;
#define PROJECT_PI_SIZE 30
  char projectPI[PROJECT_PI_SIZE];
#define PROJECT_DESCRIPTION_SIZE 256
  char projectDescription[PROJECT_DESCRIPTION_SIZE];
#define OPERATING_LOCATION_SIZE 256
  char operatingLocation[OPERATING_LOCATION_SIZE];
#define OBSERVER_SIZE 30
  char observer[OBSERVER_SIZE];
  int invalidWinds;
  time_t currentTime;
  char windServer[30];
  long longvalue;
  int rm_status;
  char timeAndDate[50];
  double sourceVel;
  long saoSeeingTimestamp;
  float saoSeeingArray[10][19];
  float csoTau,saoSeeing,saoSeeingRatio,csoTauFromSimonRadfordMeter;
  short driveStatus[MAX_NUMBER_ANTENNAS+1], m3Status[MAX_NUMBER_ANTENNAS+1], chopperStatus[MAX_NUMBER_ANTENNAS+1], 
    gunn1Status[MAX_NUMBER_ANTENNAS+1], yig1Status[MAX_NUMBER_ANTENNAS+1], gunn2Status[MAX_NUMBER_ANTENNAS+1], yig2Status[MAX_NUMBER_ANTENNAS+1];
  float dewarTemp[MAX_NUMBER_ANTENNAS+1];
  long dewarTime[MAX_NUMBER_ANTENNAS+1], calVaneTime[MAX_NUMBER_ANTENNAS+1];
  short calWheelStatus[MAX_NUMBER_ANTENNAS+1], sideband[2];
  double restFrequency[2], mRGFreq[2];
  time_t timestamp;
  double tsys[MAX_NUMBER_ANTENNAS+1],tsys2[MAX_NUMBER_ANTENNAS+1],sunD[MAX_NUMBER_ANTENNAS+1];
  float az[MAX_NUMBER_ANTENNAS+1],el[MAX_NUMBER_ANTENNAS+1];
  int tracktimestamp;
  time_t timeStamp;
  int chopperTimestamp;
  double raoffset[MAX_NUMBER_ANTENNAS+1];
  double decoffset[MAX_NUMBER_ANTENNAS+1];
  float rmsTrackingError;
  int onSource[11];
  FILE *smokeFile;
  doubleBandwidth = isDoubleBandwidth();
  fullPolarization = isFullPolarization();

#if DEBUG
  fprintf(stderr,"Entered arrayDisplay()\n");
#endif
  /* END of variable declarations*/
  
  smokeFile = fopen("/global/configFiles/fire/ignoreAntSmoking.txt","r");
  fscanf(smokeFile,"%d %d %d %d %d %d %d %d",&ignoreSmoking[1],&ignoreSmoking[2],&ignoreSmoking[3],&ignoreSmoking[4],&ignoreSmoking[5],&ignoreSmoking[6],&ignoreSmoking[7],&ignoreSmoking[8]);
  fclose(smokeFile);
  
  for (ant=1;ant<=numberAntennas;ant++) {
    trackStatus[ant]=0;
  }
  
  specialAntenna=getAntennaList(isAntennaInArray);
  getReceiverList(isReceiverInArray);
  if ((specialAntenna<1)||(specialAntenna>8)) {
    /*
      There are no antennas in the project list - chose the
      lowest numbered on which is not in the deadAntennas list.
    */
    specialAntenna = 1;
    while ((specialAntenna < 9) && deadAntennas[specialAntenna])
      specialAntenna++;
    if (specialAntenna > 8) {
      printf("All antennas are flagged offline - cannot produce display\n");
      exit(0);
    }
  }
  time(&currentTime);
  
  strcpy(timeAndDate, asctime(gmtime(&currentTime)));
  if (timeAndDate[8] == ' ')
    timeAndDate[8] = '0';
  timeAndDate[strlen(timeAndDate)-1] = (char)0;
  
#if DEBUG
  fprintf(stderr,"read the current time\n");
#endif
  fp=fopen("/global/projects/antennasInArray","r");
  fgets(antennasInArray,100,fp);
#define ANTENNA_LIST_MAX_LENGTH 19
  if (strlen(antennasInArray) > ANTENNA_LIST_MAX_LENGTH) {
    /* I once saw a file with 19 spaces in it, which caused the final
     * vertical bar on the array page to not appear. -Todd */
    antennasInArray[ANTENNA_LIST_MAX_LENGTH] = 0;
  }
/*   sprintf(antennasInArray, "1 2 3 4 5 6 7 8 9"); */
  fclose(fp);
#if DEBUG
  fprintf(stderr,"parsed antennasInArray file\n");
#endif
  /*
  rms = call_dsm_read("colossus", "DSM_PM_SEEING_8MIN_F", (void *)&saoSeeing, &saoSeeingTimestamp);
  */
  {
    int ii, bb[10], badPMBaseline[10], nBadPMBaselines;

    for (ii = 0; ii < 10; ii++)
      bb[ii] = badPMBaseline[ii] = 0;
    fp = fopen("/global/configFiles/badPhasemonBaselines", "r");
    nBadPMBaselines = fscanf(fp, "%d %d %d %d %d %d %d %d %d %d",
			     &bb[0], &bb[1], &bb[2], &bb[3], &bb[4], &bb[5],
			     &bb[6], &bb[7], &bb[8], &bb[9]);
    if (nBadPMBaselines > 0)
      for (ii = 0; (ii < nBadPMBaselines) && (ii < 10); ii++)
	badPMBaseline[bb[ii]-1] = 1;
    while ((shortestPMBaseline < 10) && badPMBaseline[shortestPMBaseline])
      shortestPMBaseline++;
    fclose(fp);
    rms = dsm_read("phasemon","PHASEMON_DATA_X", &phasemon_data, &saoSeeingTimestamp);
    if (rms != DSM_SUCCESS)
      dsm_error_message(rms,"dsm_read(PHASEMON_DATA_X)");
    rms = dsm_structure_get_element(&phasemon_data, "CORR_AMPLITUDE_V10_D", &saoSeeingAmplitude);
    rms = dsm_structure_get_element(&phasemon_data, "PHASE_STRUCT_V10_V19_F", &saoSeeingArray);
    saoSeeing = saoSeeingArray[shortestPMBaseline][5]*225.0/12.2;
    if (saoSeeingArray[shortestPMBaseline][12] != 0.0)
      saoSeeingRatio = saoSeeingArray[2][12]/saoSeeingArray[shortestPMBaseline][12];
    else
      saoSeeingRatio = 0.0;
  }
  rms = call_dsm_read("hal9000", "DSM_REQUESTED_FREQUENCY_V2_D", &restFrequency[0], &timestamp);
  rms = call_dsm_read("hal9000", "DSM_AS_IFLO_SIDEBAND_V2_S", (char *)&sideband, &timestamp);
  rms = call_dsm_read("hal9000", "DSM_AS_IFLO_MRG_V2_D", &mRGFreq, &timestamp);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_PI_C30",projectPI,&timestamp);
  checkForNulls(projectPI);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_DESCRIPTION_C256", projectDescription,&timestamp);
  checkForNulls(projectDescription);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_ID_L",&projectID,&timestamp);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_OBSERVER_C30",observer,&timestamp);
  checkForNulls(observer);
  rms = call_dsm_read("hal9000","DSM_AS_PROJECT_OPERATINGLOCATION_C256",operatingLocation,&timestamp);
  checkForNulls(operatingLocation);
#if DEBUG
  fprintf(stderr,"read DSM project vars\n");
#endif
  
  for (ant=1;ant<=numberAntennas;ant++) {
    if (antsAvailable[ant]) {
      rm_read(ant,"RM_UNIX_TIME_L",&timeStamp);
      rm_read(ant,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
      if (abs(tracktimestamp-timeStamp)>3L)
	trackStatus[ant]=0;
      else
	trackStatus[ant]=1;
    } else
      trackStatus[ant] = 0;
  }
  
  rms=rm_read(specialAntenna,"RM_SUN_AZ_DEG_F",&dummyFloat);
  sunaz=(double)dummyFloat;
  rms=rm_read(specialAntenna,"RM_SUN_EL_DEG_F",&dummyFloat);
  sunel=(double)dummyFloat;
  sunaz=sunaz*radian;
  sunel=sunel*radian;
  
  rms=rm_read(specialAntenna,"RM_REFRACTION_ARCSEC_D",&refraction);
  // moved here from line 413 due to mystery interference with wind directions DSM value

  
  rms=rm_read(specialAntenna,"RM_WEATHER_TEMP_F",&temperature);
  rms=rm_read(specialAntenna,"RM_WEATHER_HUMIDITY_F",&humidity);
  rms=rm_read(specialAntenna,"RM_WEATHER_MBAR_F",&pressure);
  rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_WIND_SERVER_C7",windServer, &longvalue);
  if (present(windServer,"SMA")) {
    rm_status = dsm_structure_get_element(&smaWeather,"WINDSPEED_F", &windspeed);
     windspeed /= 0.44704; /* convert from m/s to mph BBC 2013-03-18*/ 
    rm_status = dsm_structure_get_element(&smaWeather,"WINDDIR_F", &winddirection);
    if (windspeed > WIND_METER_FROZEN_CUTOFF) {
      /* then assume the SMA wind meter is not frozen, and display
       * its reading on the main page, instead of the summit median */
      smaWindAsterisk = 0;
    } else {
      windspeed = computeMedianWindspeed(&winddirection,&invalidWinds);
      smaWindAsterisk = 1;
    }
  } else if (present(windServer,"UKIRT")) {
    rm_status = dsm_structure_get_element(&ukirtWeather,"WINDSPEED_F", &windspeed);
    rm_status = dsm_structure_get_element(&ukirtWeather,"WINDDIR_F", &winddirection);
  } else if (present(windServer,"CFHT")) {
    rm_status = dsm_structure_get_element(&cfhtWeather,"WINDSPEED_F", &windspeed);
    rm_status = dsm_structure_get_element(&cfhtWeather,"WINDDIR_F", &winddirection);
    if (windspeed < WIND_METER_FROZEN_CUTOFF) {
      windspeed = computeMedianWindspeed(&winddirection,&invalidWinds);
    }
  } else if (present(windServer,"Subaru")) {
    rm_status = dsm_structure_get_element(&subaruWeather,"WINDSPEED_F", &windspeed);
    rm_status = dsm_structure_get_element(&subaruWeather,"WINDDIR_F", &winddirection);
    if (windspeed < WIND_METER_FROZEN_CUTOFF)
      windspeed = computeMedianWindspeed(&winddirection,&invalidWinds);
  } else if (present(windServer,"UH88")) {
    rm_status = dsm_structure_get_element(&uh88Weather,"WINDSPEED_F", &windspeed);
    rm_status = dsm_structure_get_element(&uh88Weather,"WINDDIR_F", &winddirection);
    if (windspeed < WIND_METER_FROZEN_CUTOFF)
      windspeed = computeMedianWindspeed(&winddirection,&invalidWinds);
  } else if (present(windServer,"VLBA")) {
    rm_status = dsm_structure_get_element(&vlbaWeather,"WINDSPEED_F", &windspeed);
    rm_status = dsm_structure_get_element(&vlbaWeather,"WINDDIR_F", &winddirection);
    if (windspeed < WIND_METER_FROZEN_CUTOFF)
      windspeed = computeMedianWindspeed(&winddirection,&invalidWinds);
  } else if (present(windServer,"IRTF")) {
    rm_status = dsm_structure_get_element(&irtfWeather,"WINDSPEED_F", &windspeed);
    rm_status = dsm_structure_get_element(&irtfWeather,"WINDDIR_F", &winddirection);
    if (windspeed < WIND_METER_FROZEN_CUTOFF)
      windspeed = computeMedianWindspeed(&winddirection,&invalidWinds);
  }
  
  if (weatherUnits == METRIC_WEATHER_UNITS)
    windspeed *= 0.44704; /* convert from mph to m/s */
  call_dsm_read(DSM_WEATHER_HOST,"DSM_CSO_225GHZ_TAU_F",&csoTau,&timestamp);
  call_dsm_read(DSM_WEATHER_HOST,"DSM_CSO_350MICRON_TAU_SCALED_F",
		&csoTauFromSimonRadfordMeter,
		&timestamp);
  rms=rm_read(specialAntenna,"RM_RA_APP_HR_D", &ra_disp);
  rms=rm_read(specialAntenna,"RM_DEC_APP_DEG_D",&dec_disp);

  #if DEBUG
 	fprintf(stderr,"@%.0fdeg\n", winddirection); // winddirection is valid at this point!
	#endif
//  rms=rm_read(specialAntenna,"RM_REFRACTION_ARCSEC_D",&refraction);
  #if DEBUG
 	fprintf(stderr,"@%.0fdeg\n", winddirection); // winddirection is invalid at this point!
	#endif

  rms=rm_read(specialAntenna,"RM_PLANET_DIAMETER_ARCSEC_D",
	      &planetdiameter);
  rms=rm_read(specialAntenna,"RM_POLAR_MJD_L", &polarMJD);
  rms=rm_read(specialAntenna,"RM_POLAR_DX_ARCSEC_D", &polarDx);
  rms=rm_read(specialAntenna,"RM_POLAR_DY_ARCSEC_D", &polarDy);
  rms=rm_read(specialAntenna,"RM_POLAR_DUT_SEC_D", &polarDut);
  rms=rm_read(specialAntenna,"RM_SOURCE_C34",source);
  rms=rm_read(specialAntenna,"RM_LST_HOURS_F",&dummyFloat);
  lst_disp=(double)dummyFloat;
  rms=rm_read(specialAntenna,"RM_UTC_HOURS_F",&dummyFloat);
  utc_disp=(double)dummyFloat;
  rms=rm_read(specialAntenna,"RM_TJD_D",&tjd_disp);
  rms=rm_read(specialAntenna,"RM_EPOCH_F",&epoch);
  rms=rm_read(specialAntenna,"RM_RA_CAT_HOURS_F",&dummyFloat);
  ra_cat_disp=(double)dummyFloat;
  rms=rm_read(specialAntenna,"RM_DEC_CAT_DEG_F",&dummyFloat);
  dec_cat_disp=(double)dummyFloat;
  rms=rm_read(specialAntenna,"RM_SVEL_KMPS_D",&sourceVel);
  
  for (ant = 1; ant <= numberAntennas; ant++) {
    rms =rm_read(ant,"RM_RAOFF_ARCSEC_D",&raoffset[ant]);
    rms =rm_read(ant,"RM_DECOFF_ARCSEC_D",&decoffset[ant]);
  }
  
  /* Use the first element of chopperStatus to find out if the focus Curve
   * is on in more than half of the array antennas Array */
  chopperStatus[0] = 0;
  for (ant = 1; ant <= numberAntennas; ant++) {
    (void)rm_read(ant,"RM_PAD_ID_B",&dummyByte);
    padid[ant]=(short)dummyByte;
    if (antsAvailable[ant]) {
      int activeRx;
      unsigned char statusBits[16];
      
      (void)rm_read(ant,"RM_ANTENNA_DRIVE_STATUS_B",&dummyByte);
      (void)rm_read(ant,"RM_SERVO_FAULT_STATE_B",&faultState);
      if (faultState== 0 || dummyByte == 0)
	driveStatus[ant]=(short)dummyByte;
      else if (faultState>0)
	driveStatus[ant]=(short)(faultState+1);
      (void)rm_read(ant,"RM_CALIBRATION_WHEEL_S",&dummyShort);
      calWheelStatus[ant]=dummyShort;
      (void)rm_read(ant,"RM_M3STATE_B",&dummyByte);
      m3Status[ant]=(short)dummyByte;
      (void)rm_read(ant,"RM_CHOPPER_STATUS_BITS_V16_B",statusBits);
      (void)rm_read(ant,"RM_CHOPPER_MONITOR_TIMESTAMP_L",&chopperTimestamp);
      chopperStatus[ant]= statusBits[POS_ERR_BITS];
      if (statusBits[P20] == 2) {
	chopperStatus[ant] |= 0x10; /* Indicate chopping*/
      }
      if (abs(chopperTimestamp - timeStamp) > 4) {
	chopperStatus[ant] |= 0x20;
      }
      if (statusBits[UPDATE_STATUS] == 1) {
	if (isAntennaInArray[ant]==1) chopperStatus[0] += 1;
	chopperStatus[ant] |= 0x40;
      } else if (isAntennaInArray[ant]==1)
	chopperStatus[0] -= 1;
      activeRx = findReceiverNumberLow(ant);
      (void)rm_read(ant,"RM_GUNN1_LOCKED_S",&dummyShort);
      gunn1Status[ant] = (dummyShort == 1)? 1: 0;
      (void)rm_read(ant,"RM_YIG1_LOCKED_S",&dummyShort);
      yig1Status[ant] = (dummyShort == 1)? 1: 0;
      (void)rm_read(ant,"RM_YIG2_LOCKED_S",&dummyShort);
      yig2Status[ant] = (dummyShort == 1)? 1: 0;
      (void)rm_read(ant,"RM_GUNN2_LOCKED_S",&dummyShort);
      gunn2Status[ant] = (dummyShort == 1)? 1: 0;
      (void)rm_read(ant,"RM_DEWAR_TEMPS_V16_F",&dummyFloat16);
      dewarTemp[ant] = dummyFloat16[activeRx+8];
      if ((dewarTemp[ant] < 3.4) || (dewarTemp[ant] > 300)) {
	dewarTemp[ant] = dummyFloat16[DEWAR_TEMP_4K_STAGE];
	wackyRxTemp[ant] = TRUE;
      } else
	wackyRxTemp[ant] = FALSE;
      (void)rm_read(ant,"RM_LAKESHORE_TIMESTAMP_L",&dewarTime[ant]);
      (void)rm_read(ant,"RM_CALIBRATION_WHEEL_TIMESTAMP_L",&calVaneTime[ant]);
      (void)rm_read(ant,"RM_TSYS_D",&dummyDouble);
      tsys[ant]=dummyDouble;
      (void)rm_read(ant,"RM_TSYS2_D",&dummyDouble);
      tsys2[ant]=dummyDouble;
      (void)rm_read(ant,"RM_ACTUAL_AZ_DEG_F",&dummyFloat);
      az[ant]=dummyFloat;
      (void)rm_read(ant,"RM_ACTUAL_EL_DEG_F",&dummyFloat);
      el[ant]=dummyFloat;
      (void)rm_read(ant,"RM_TRACKING_ERROR_ARCSEC_F",&rmsTrackingError);
      if (rmsTrackingError<TRACKING_ERROR_THRESHOLD) {
	onSource[ant]=1;
      } else {
	float ll, ul;
	
	onSource[ant]=0;
	(void)rm_read(ant,"RM_ENCODER_EL_F",&dummyFloat);
	(void)rm_read(ant,"RM_SCB_UP_LIMIT_F",&ul);
	(void)rm_read(ant,"RM_SCB_LOW_LIMIT_F",&ll);
	if (dummyFloat >= ul || dummyFloat <= ll)
	  driveStatus[ant] = 5;
	(void)rm_read(ant,"RM_ENCODER_AZ_F",&dummyFloat);
	(void)rm_read(ant,"RM_SCB_CW_LIMIT_F",&ul);
	(void)rm_read(ant,"RM_SCB_CCW_LIMIT_F",&ll);
	if (dummyFloat >= ul || dummyFloat <= ll)
	  driveStatus[ant] = 6;
      }
      
      (void)rm_read(ant,"RM_REFRACTION_RADIO_FLAG_B",&dummyByte);
      if (dummyByte==0)
	onSource[ant]=0;
      
      sunD[ant]=sunDistance(az[ant]*radian,el[ant]*radian,sunaz,sunel);
    } else {
      driveStatus[ant]=0;
      calWheelStatus[ant]=0;
      m3Status[ant]=0;
      chopperStatus[ant]=-1;
      gunn1Status[ant]=0;
      gunn2Status[ant]=0;
      yig1Status[ant]=0;
      yig2Status[ant]=0;
      dewarTemp[ant]=0;
      dewarTime[ant] = 0;
      calVaneTime[ant] = 0;
      tsys[ant]=0;
      tsys2[ant]=0;
      az[ant]=0;
      el[ant]=0;
      onSource[ant]=0;
      sunD[ant]=180.0;
    }
  }
  
  for (ant = 1; ant <= numberAntennas; ant++) {
    int servoTimestamp;
    
    if (rm_read(ant, "RM_SCB_FAULTWORD_L", &scbFaultWordArray[ant]) !=
       RM_SUCCESS) {
      scbFaultWordArray[ant] = 0;
    } else {
      rm_read(ant, "RM_SERVO_TIMESTAMP_L", &servoTimestamp);
      if (abs(servoTimestamp - timeStamp) > 10) {
	scbFaultWordArray[ant] = 0;
      }
    }
  }
  /* Update the Deice status every 30 sec */
  if ((icount % 30) == 1) for (ant = 1; ant <= numberAntennas; ant++) {
    int deiceStatusBits, deiceTimestamp;
    
    if (antsAvailable[ant]) {
      rms = rm_read(ant, "RM_DEICE_TIMESTAMP_L", &deiceTimestamp);
      rms |= rm_read(ant, "RM_DEICE_STATUS_BITS_L", &deiceStatusBits);
      if (rms || abs(timeStamp - deiceTimestamp) > 300 ||
	 (DEICED_FAULT & deiceStatusBits)) {
	deiceChar[ant] = 'F';
      } else if ((MAIN_CONTACTOR_STATUS_BIT | MAIN_CONTACTOR_BIT) &
		deiceStatusBits)
	deiceChar[ant] = 'D';
      else
	deiceChar[ant] = ' ';
    } else
      deiceChar[ant] = ' ';
  }
#if DEBUG
  fprintf(stderr,"read a whole load of DSM vars\n");
#endif
  
  arrayscreen(projectPI, projectDescription, projectID, observer,
	      operatingLocation, antennasInArray,timeAndDate,source, &lst_disp, &utc_disp, &tjd_disp, &epoch, &ra_disp, &dec_disp,
	      &ra_cat_disp, &dec_cat_disp,
	      &icount, 
	      &pressure, &temperature, &humidity,
	      &windspeed, &winddirection,
	      &specialAntenna,padid,
	      &planetdistance,&planetdiameter,&sourceVel,&csoTau,&saoSeeing,&saoSeeingRatio,
	      saoSeeingTimestamp,
	      driveStatus, m3Status, chopperStatus, gunn1Status, 
	      yig1Status, gunn2Status, yig2Status,dewarTemp,calWheelStatus,
	      restFrequency,sideband,mRGFreq,tsys,tsys2,az,el,onSource,sunD,
	      isAntennaInArray,isReceiverInArray,trackStatus,raoffset,decoffset,&polarMJD,&polarDx,&polarDy,
	      &polarDut,dewarTime,&timeStamp, calVaneTime, ignoreIntruders,
	      &csoTauFromSimonRadfordMeter,sunaz,sunel,&invalidWinds,antlist);
  
  return 0;
}

void arrayscreen(char *projectPI, char *projectDescription, int projectID,
		 char *observer, char *operatingLocation,
		 char *antennasInArray,char *timeAndDate,char *source,double *lst_disp,double *utc_disp,double *tjd_disp, float *epoch,
		 double *ra_disp,double *dec_disp, 
		 double *ra_cat_disp, double *dec_cat_disp,
		 int *icount, float *pressure, float *temperature, float *humidity,
		 float *windspeed, 
		 float *winddirection,
		 int *antennaNumber, short *padid,double *planetdistance,double
		 *planetdiameter, double *sourceVel,
		 float *csoTau, float *saoSeeing, float *saoSeeingRatio, long saoSeeingTimestamp,
		 short *driveStatus, short *m3Status, 
		 short *chopperStatus, short *gunn1Status, short *yig1Status, short *gunn2Status,
		 short *yig2Status,float *dewarTemp,short *calWheelStatus, 
		 double restFrequency[2], short sideband[2], double mRGFreq[2], double *tsys, double *tsys2, float *azfloat,
		 float *elfloat, int *onSource,double *sunD,int *isAntennaInArray,
		 int *isReceiverInArray, int *trackStatus, double raoffset[MAX_NUMBER_ANTENNAS+1],
		 double decoffset[MAX_NUMBER_ANTENNAS+1],int *polarMJD,double *polarDx,double *polarDy,
		 double *polarDut, long *dewarTime, time_t *timeStamp, long *calVaneTime, int ignoreIntruders,
		 float *csoTauFromSimonRadfordMeter,double sunaz,
		 double sunel, int *invalidWinds,   int antlist[RM_ARRAY_SIZE])
{
  static int firstCall = TRUE;
  char weatherServer[7];
  long tau350Minutes, tau225Minutes;  /* in minutes of the present UTC day */
  float tau350Hours, tau225Hours, uthours;
  int jcmtTauStale;
  int tau225stale, tau350stale;
  float windSpeedStandout,windSpeedWacko;
  long getWeatherTimestamp;
  int i,lowestActiveCrate, cratesInArray[14];
#include "antennaServoRmVars.h"
  RV rv[] = {
#include "antennaServoRvList.h"
  };
#define NUMVARS (sizeof(rv) / sizeof(rv[0]))
  float jcmtTau;
  long longvalue;
  int ant;
  static int overTemperature = 0;
  short busy;
  int rms;
  short dec_dum_sign,dec_app_sign,dec_cat_sign,correlatorMode;
  short manualWeather;
  short  opticsBoardPresent[MAX_NUMBER_ANTENNAS+1];
  char crateName[10], shutdownMessage[81], shutdownMessageCopy[81], otherCrateName[10];
  int lsth,lstm,ra_cat_h,ra_cat_m,dec_cat_d,dec_cat_m;
  int dec_app_d,dec_app_m;
  int ra_app_h,ra_app_m;
  int utch,utcm;
  double ra_cat_s,dec_cat_s,ra_app_s,dec_app_s,lsts,utcs;
  int lstsi,utcsi;
  double ha;
  ALLAN_VARIANCE_FLAGS allanVarianceFlags;
  RECEIVER_FLAGS receiver0Flags;
  RECEIVER_FLAGS receiver1Flags;
  OPTICS_FLAGS opticsFlags;
  ANTENNA_FLAGS antennaFlags;
  int antStatsOK[2][MAX_NUMBER_ANTENNAS+1] = {{1,1,1,1,1,1,1,1,1,1,1},{1,1,1,1,1,1,1,1,1,1,1}};
  static int tilting[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,0,0};
  C1DC_FLAGS c1DCFlags;
  IFLO_FLAGS ifloFlags;
  int s,  scanNumber, setScanNo;
  double scanTime, scanLength, otherScanTime;
  short crateStatsOK[13][3][5][9];
  time_t currentTime, timestamp;
  int sourceType, sourceLength;
  double timeError[MAX_NUMBER_ANTENNAS+1];
  int delay;
  int dummyInt;
  char dummyChar;
  extern int defaultTiltFlag[20];
  extern double radian;
  time_t temperatureServerTimestamp;
  long pressureServerTimestamp;
  long dSMScanFlags[11][2];
  long windServerTimestamp;
  long humidityServerTimestamp;
  int staleNess, rx;
  char esma5, esma6;
  time_t dsmTimestamp;
  char lineName[2][41];
  long tsystimestamp, syncdet2timestamp;
  short alarmShort;
  float solarTemp,smaTemp;
  float equipmentRoomTemperature, alarmTemperature;
  FILE *alarmFp;
  int inconsistentRADecOffsets;
  int polarDays=0;
/*  short opticalBreaks[8]; */
  int wackoAntennas;
  int uAnt;
  char online[11];
  FILE *antennaUser;
#ifndef LINUX
  static char obsconAd[256];
  char obsconAdNow[79];
  static int obsconAdInitialized = FALSE;
  static int adShownCount = 0;
#endif
  static int gensetCount = 0;
  int gensetActive;
  float currentA, currentB, currentC, batteryVoltage;
  static dsm_structure gensetStructure;
  char waveplateOrientations[11], dataServer[40];
  FILE *smokeFile;
  
  if (firstCall) {
    dsm_structure_init(&gensetStructure, "DSM_IWATCH_DATA_X");
    firstCall = FALSE;
  }
  currentA = currentB = currentC = 0.0;
  dsm_read("obscon", "DSM_IWATCH_DATA_X", &gensetStructure, &timestamp);
  dsm_structure_get_element(&gensetStructure, "CURRENTL1_F", (char *)(&currentA));
  dsm_structure_get_element(&gensetStructure, "CURRENTL2_F", (char *)(&currentB));
  dsm_structure_get_element(&gensetStructure, "CURRENTL3_F", (char *)(&currentC));
  dsm_structure_get_element(&gensetStructure, "BATTERY_VOLTAGE_F", (char *)(&batteryVoltage));
  gensetActive = 0;
  if ((currentA != 0.0) || (currentB != 0.0) || (currentC != 0.0))
    gensetCount++;
  else if ((currentA == 0.0) && (currentB == 0.0) && (currentC == 0.0))
    gensetCount = 0;
  if (gensetCount > 120)
    gensetActive = 1;
  else
    gensetActive = 0;
    
  if (weatherUnits == METRIC_WEATHER_UNITS) {
    windSpeedWacko = WIND_SPEED_WACKO*MPH_TO_METER_PER_SEC;
    windSpeedStandout = WIND_SPEED_STANDOUT*MPH_TO_METER_PER_SEC;
  } else {
    windSpeedWacko = WIND_SPEED_WACKO;
    windSpeedStandout = WIND_SPEED_STANDOUT;
  }
  
#if DEBUG
  fprintf(stderr,"Entered arrayscreen()\n");
#endif
  ha=*lst_disp-*ra_disp;
  
  if ((*icount%30)==1) initialize();
#ifndef LINUX
  if (adShownCount < 5) {
    if (!obsconAdInitialized) {
      for (i = 0; i < 256; i++)
	obsconAd[i] = '-';
      strncpy(&obsconAd[90], " Why not run monitor on obscon? ", 32);
      obsconAdInitialized = TRUE;
    }
    for (i = 0; i < 78; i++)
      obsconAdNow[i] = obsconAd[(i+((2*(*icount))%256))%256];
    obsconAdNow[78] = (char)0;
    move(23,1);
    printw("%s", obsconAdNow);
    if ((*icount%256) == 10)
      adShownCount++;
  }
#endif
#if DEBUG
  fprintf(stderr,"Completed initialize()\n");
#endif
  move(0,0);
  if (projectLockout)
    printw("Command Defaulting Disabled");
  else
    printw("---------------------------");
  move(0,27);
  if (ignoreIntruders)
    printw(" Operator Messages Disabled ");
  else
    printw("----------------------------");
  hms(ra_disp,&ra_app_h,&ra_app_m,&ra_app_s,&dec_dum_sign);
  hms(dec_disp,&dec_app_d,&dec_app_m,&dec_app_s,&dec_app_sign);
  hms(ra_cat_disp,&ra_cat_h,&ra_cat_m,&ra_cat_s,&dec_dum_sign);
  hms(dec_cat_disp,&dec_cat_d,&dec_cat_m,&dec_cat_s,&dec_cat_sign);
  hms(lst_disp,&lsth,&lstm,&lsts,&dec_dum_sign);
  hms(utc_disp,&utch,&utcm,&utcs,&dec_dum_sign);
  
  lstsi=(int)lsts;
  utcsi=(int)utcs;

#if DEBUG
  fprintf(stderr,"ran hms()\n");
#endif
  
  move(1,2);
  addstr("LastIntruder ");
  currentTime = time((long *)0);
  printInterval(currentTime-intruderTimestamp,5);
  
  move(1,23);
  
  if (timeProblems(timeError) == 1)
   standout();
  addstr(timeAndDate);
  standend();
  move(1,50);
  addstr("TJD ");
  move (1,54);
  printw("%.2f",(*tjd_disp));
  move(1,66);
  addstr("LST");
  move(1,70);
  if (lsth>23 || lsth<0)
    printw("wa:");
  else
    printw("%02d:",lsth);
  if (lstm>59 || lstm<0)
    printw("wa:");
  else
    printw("%02d:",lstm);
  if (lstsi>=60 || lstsi<0)
    printw("wa");
  else
    printw("%02d",lstsi);
  move(2,2);
#if DEBUG
  fprintf(stderr,"About to print the project\n");
#endif
  addstr("Project: ");
  if (projectID < 0 || projectID >= WACKO_PROJECT_ID_MAX)
    printw("wacko - ");
  else
    printw("%d - ",projectID);
#if DEBUG
  refresh();
#endif
#define LINE_LENGTH 55
#define FIELD_LENGTH 29
  if (projectID != 0) {
    if (strlen(projectDescription) > LINE_LENGTH)
      printw("wacko ");
    else
      printw("%s ",projectDescription);
    if (strlen(projectPI) > FIELD_LENGTH)
      printw("(PI: wacko)");
    else
      printw("(PI: %s)",projectPI);
  }
  if (projectID == 0) {
    if (strlen(projectDescription) > LINE_LENGTH)
      printw("wacko ");
    else
      printw("%s",projectDescription);
  }

  move(3,2);
#if DEBUG
  fprintf(stderr,"About to print the Observers name\n");
#endif
  addstr("Observers: ");
  if (strlen(observer) > FIELD_LENGTH)
    printw("wacko ");
  else
    printw("%s ",observer);
  if (strlen(operatingLocation) > LINE_LENGTH)
    printw("@ wacko  ");
  else
    printw("@ %s  ",operatingLocation);
  move(3,50);
  printw("Antennas: ");
  if (strlen(antennasInArray) > 21) {
    printw("wacko");
  } else {
    wackoAntennas = 0;
    for (i=0; i<strlen(antennasInArray); i++) {
      if (antennasInArray[i] != ' ' && (antennasInArray[i] < '0' || antennasInArray[i] > '9')) {
	printw("wacko");
	wackoAntennas = 1;
	break;
      }
    }
    if (wackoAntennas == 0)
      printw("%s",antennasInArray);
  }
  move(3,COLS-1);
  printw("|");
  move(4,2);

  printw("-----------------------------------------------------------------------------");
#ifdef WORKING_FIBER_SWITCH
  if ((isAntennaInArray[9]==0 && isAntennaInArray[10]==0) || doubleBandwidth) {
    rms = call_dsm_read("colossus", "DSM_ANTENNA5_ESMA_STATUS_B", 
			(void *)&esma5, &dsmTimestamp);
    rms = call_dsm_read("colossus", "DSM_ANTENNA6_ESMA_STATUS_B", 
			(void *)&esma6, &dsmTimestamp);
    if (((isAntennaInArray[5]==1 && esma5 != SMA_MODE_READBACK) ||
	 (isAntennaInArray[6]==1 && esma6 != SMA_MODE_READBACK))
         && (isReceiverInArray[2] || doubleBandwidth)) {
      move(4,4);
      standout();
      printw(" The eSMA fiber switches are in the wrong state for this configuration. ");
      standend();
    }
  }
#endif

  move(5,2);
  addstr("Source: ");
#if DEBUG
  fprintf(stderr,"About to print the source name\n");
#endif
  if (!strcmp(source,"unknown")) {
    standout();
    printw("%s",source);
    standend();
  } else {
    /* first char of source is printed in column 10 (starting from 0) */
    printw("%s",source);
    rms = call_dsm_read("hal9000","DSM_AS_SOURCE_TYPE_L", &sourceType, 
			&longvalue);
    if (sourceType == 0) {
      for (i=strlen(source); i<22; i++) {
	printw(" ");
      }
    } else {
      sourceLength = strlen(source);
      for (i=strlen(source)-1; i>0; i--) {
	if (source[i] != ' ') {
	  sourceLength = i+1;
	  break;
	}
      }
      move(5,11+sourceLength);
      printw("(");
      if (sourceType & 1)
	printw("F");
      if (sourceType & 2)
	printw("B");
      if (sourceType & 4)
	printw("G");
      if (sourceType & 8)
	printw("I");
      printw(")");
    }
  }
#if DEBUG
  fprintf(stderr,"Finished printing the source name\n");
#endif
  move(5,32);
  if ((int)abs(*epoch) > 3000)
    printw("RA(wack) ");
  else
    printw("RA(%4d) ",(int)(*epoch));
  if (ra_cat_h > 23 || ra_cat_h < 0)
    printw("wa:");
  else
    printw("%02d:",ra_cat_h);
  if (ra_cat_m > 59 || ra_cat_m < 0)
    printw("wa:");
  else
    printw("%02d:",ra_cat_m);
  if (ra_cat_s >= 60 || ra_cat_s < 0)
    printw("wa.cko");
  else
    printw("%06.3f",ra_cat_s);
#if DEBUG
  fprintf(stderr,"Finished printing the RA\n");
#endif
  printw("  ");
  if ((int)abs(*epoch) > 3000)
    printw("DEC(wack) ");
  else
    printw("DEC(%4d) ",(int)(*epoch));
#if DEBUG
  fprintf(stderr,"Finished printing the DEC epoch\n");
#endif
  if (dec_cat_sign>0) addch('+');
  if (dec_cat_sign<0) addch('-');
  if (dec_cat_sign==0) addch(' ');
#if DEBUG
  fprintf(stderr,"about to print the DEC\n");
#endif
  if (abs(dec_cat_d) > 90)
    printw("wa:");
  else
    printw("%02d:",abs(dec_cat_d));
  if (dec_cat_m > 59 || dec_cat_m < 0)
    printw("wa:");
  else
    printw("%02d:",dec_cat_m);
  if (dec_cat_s >= 60 || dec_cat_s < 0)
    printw("wa:");
  else
    printw("%06.3f",dec_cat_s);
#if DEBUG
  fprintf(stderr,"Finished printing the DEC\n");
#endif
  move(6,32);
  if (ra_app_h > 23 || ra_app_h < 0)
    printw("RA(App.) wa:");
  else
    printw("RA(App.) %02d:",ra_app_h);
#if DEBUG
  fprintf(stderr,"Finished printing the RA(App.) hour = %d\n",ra_app_h);
#endif
  if (ra_app_m > 59 || ra_app_m < 0)
    printw(              "wa:");
  else
    printw(              "%02d:",ra_app_m);
#if DEBUG
  fprintf(stderr,"Finished printing the RA(App.) minute= %d\n",ra_app_m);
  fprintf(stderr,"RA(App.) second= %g\n",ra_app_s);
#endif
  if (ra_app_s < 10000 && ra_app_s > -1)
    printw(                   "%06.3f",ra_app_s);
  else
    printw(                   "wa.cko");
#if DEBUG
  fprintf(stderr,"Finished printing the RA(App.)\n");
#endif
  printw("  ");
  printw("DEC(App.) ");
  if (dec_cat_sign>0) addch('+');
  if (dec_cat_sign<0) addch('-');
  if (dec_cat_sign==0) addch(' ');
  if (abs(dec_app_d) > 90 || abs(dec_app_d) < 0)
    printw("wa:");
  else
    printw("%02d:",abs(dec_app_d));
  if (dec_app_m > 59 || dec_app_m < 0)
    printw("wa:");
  else
    printw("%02d:",dec_app_m);
  if (dec_app_s > 60 || dec_app_s < 0)
    printw("wa.cko");
  else
    printw("%06.3f",dec_app_s);
#if DEBUG
  fprintf(stderr,"Finished printing the DEC(App.)\n");
  refresh();
#endif

  move(6,2);
  addstr("Vel ");
  if (fabs(*sourceVel) >= 300000)
    printw("wacko ");
  else if (fabs(*sourceVel) >= 100000)
    printw("%.0f ",*sourceVel);
  else if (fabs(*sourceVel) >= 1000)
    printw("%.1f ",*sourceVel);
  else
    printw("%.2f ",*sourceVel);
#if DEBUG
  fprintf(stderr,"Finished printing the Vel\n");
  refresh();
#endif
  printw("km/s LSR");
  move(6,22);
  addstr("HA=");
  if (ha < -12.0)
    ha += 24.0;
  if (ha > 12.0)
    ha -= 24.0;
  if (fabs(ha) >= 100)
    printw("wacko");
  else if (fabs(ha) >= 10.0)
    printw("%+.2f",ha);
  else
    printw("%+.3f",ha);
#if DEBUG
  fprintf(stderr,"Finished printing the HA\n");
  refresh();
#endif
  /* check to see if all antennas have a consistent raoff and decoff */
  raoffset[0] = raoffset[*antennaNumber];
  decoffset[0] = decoffset[*antennaNumber];
  inconsistentRADecOffsets = 0;
  
  /* below, I use 8 instead of numberAntennas, because we do not yet
   * have control over CSO/JCMT ra/dec offsets */
  for (ant=*antennaNumber+1;  ant<=8; ant++) {
    if (deadAntennas[ant]==0) {
      if (isAntennaInArray[ant]) {
	if (fabs(raoffset[ant]-raoffset[0]) > 0.02 || 
	    fabs(decoffset[ant]-decoffset[0]) > 0.02) {
	  move(4,31);
	  standout();
	  printw(" RA/DEC offsets inconsistent between antennas!");
	  standend();
	  inconsistentRADecOffsets = 1;
	  break;
	}
      }
    }
  }
  if (inconsistentRADecOffsets == 0) {
    if (raoffset[0] != 0 || decoffset[0] != 0) {
      move(4,31);
      printw(" RA offset(\") ");
      if (fabs(*raoffset) >= 10000) {
	standout();
	printw("wacko");
	standend();
      } else
	printw("%+6.1f",*raoffset);
      printw("     DEC offset(\") ");
      if (fabs(*decoffset) >= 10000) {
	standout();
	printw("wacko ");
	standend();
      } else
	printw("%+6.1f ",*decoffset);
    }
  }
  
#if DEBUG
  refresh();
#endif
  
  move(7,2);
  
  printw("Planet dist.=%.4f AU,dia=%.1f\" ",*planetdistance ,*planetdiameter);
#if DEBUG
  refresh();
#endif
  
  move(7,37);
  {
    struct stat ser7Info;

    if (stat("/global/polar/ser7.dat", &ser7Info) < 0) {
      standout();
      printw(" The UT1-UTC file (ser7.dat) is missing ");
      standend();
    } else {
      time_t age;

      age = time(NULL) - ser7Info.st_mtime;
      if (age > 172800) {
      standout();
      printw(" The UT1-UTC file (ser7.dat) too old ");
      standend();
      } else {
	polarDays= abs( *polarMJD -( (int)(*tjd_disp) - 2400000) ); 
	if (polarDays>2) {
	  standout();
	  printw("Polar parameters are %d days old.",polarDays);
	  standend();
	} else {
	  printw("Polar dx=");
#define WACKO_POLAR 100000
	  if (fabs(*polarDx) > WACKO_POLAR) {
	    standout();
	    printw("wacko");
	    standend();
	  } else
	    printw("%.3f\"",*polarDx);
	  printw(", dy=");
	  if (fabs(*polarDy) > WACKO_POLAR) {
	    standout();
	    printw("wacko");
	    standend();
	  } else
	    printw("%.3f\"",*polarDy);
	  printw(", dut=");
	  if (fabs(*polarDut) > WACKO_POLAR) {
	    standout();
	    printw("wacko");
	    standend();
	  } else
	    printw("%.3f s",*polarDut);
	}
      }
    }
  }
  
#if DEBUG
  refresh();
#endif
  
  rms = dsm_read("colossus","SMA_METEOROLOGY_X", &smaWeather, &temperatureServerTimestamp);
  if (rms != DSM_SUCCESS)
    dsm_error_message(rms,"dsm_read(SMA_METEOROLOGY_X)");
  rms = dsm_structure_get_element(&smaWeather,"SOLAR_TEMP_F", &solarTemp);
  rms = dsm_structure_get_element(&smaWeather,"TEMP_F", &smaTemp);
  if (solarTemp-smaTemp > SUNSHINE_CRITERION) {
    move(2,65);
    printw("*");
  }
  move(2,66);
  printw("Sun: %3.0f ",sunaz/radian);
  if (fabs(sunel/radian) < 1)
     standout();
  printw("%+2.0f",sunel/radian);
  standend();
  move(8,2);
  rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_MANUAL_WEATHER_FLAG_S", &manualWeather, 
		      &longvalue);
  if (manualWeather == 1) {
    strcpy(weatherServer,"Manual");
    temperatureServerTimestamp = longvalue;
    pressureServerTimestamp = longvalue;
    humidityServerTimestamp = longvalue;
    windServerTimestamp = longvalue;
  } else {
#define AUX_WEATHER_HOST 6
    rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_TEMPERATURE_SERVER_C7",weatherServer, &longvalue);
    if (present(weatherServer,"SMAaux")) {
      rms = rm_read(AUX_WEATHER_HOST,"RM_AUX_WEATHER_TIMESTAMP_L",&temperatureServerTimestamp);
    } else if (present(weatherServer,"SMA")) {
      /*
	rms = dsm_read("colossus","SMA_METEOROLOGY_X", &smaWeather, &temperatureServerTimestamp);
	if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(SMA_METEOROLOGY_X)");
	}
      */
    } else if (present(weatherServer,"UKIRT")) {
      rms = dsm_read("colossus","UKIRT_METEOROLOGY_X", &ukirtWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(UKIRT_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"KECK")) {
      rms = dsm_read("colossus","KECK_METEOROLOGY_X", &keckWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(KECK_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"CFHT")) {
      rms = dsm_read("colossus","CFHT_METEOROLOGY_X", &cfhtWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(CFHT_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"Subaru")) {
      rms = dsm_read("colossus","SUBARU_METEOROLOGY_X", &subaruWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(SUBARU_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"JCMT")) {
      rms = dsm_read("colossus","JCMT_METEOROLOGY_X", &jcmtWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(JCMT_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"UH88")) {
      rms = dsm_read("colossus","UH88_METEOROLOGY_X", &uh88Weather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(UH88_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"VLBA")) {
      rms = dsm_read("colossus","VLBA_METEOROLOGY_X", &vlbaWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(VLBA_METEOROLOGY_X)");
      }
    } else if (present(weatherServer,"IRTF")) {
      rms = dsm_read("colossus","IRTF_METEOROLOGY_X", &irtfWeather, &temperatureServerTimestamp);
      if (rms != DSM_SUCCESS) {
	dsm_error_message(rms,"dsm_read(IRTF_METEOROLOGY_X)");
      }
    } else {
      temperatureServerTimestamp = 0;
    }
    rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_HUMIDITY_SERVER_C7",weatherServer, &longvalue);
    if (present(weatherServer,"SMAaux")) {
      rms = rm_read(AUX_WEATHER_HOST,"RM_AUX_WEATHER_TIMESTAMP_L",&humidityServerTimestamp);
    } else if (present(weatherServer,"SMA")) {
      humidityServerTimestamp = temperatureServerTimestamp;
    } else if (present(weatherServer,"UKIRT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"UKIRT_METEOROLOGY_X", &ukirtWeather, &humidityServerTimestamp);
    } else if (present(weatherServer,"KECK")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"KECK_METEOROLOGY_X", &keckWeather, &humidityServerTimestamp);
    } else if (present(weatherServer,"CFHT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"CFHT_METEOROLOGY_X", &cfhtWeather, &humidityServerTimestamp);
    } else if (present(weatherServer,"Subaru")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"SUBARU_METEOROLOGY_X", &subaruWeather, &humidityServerTimestamp);
    } else if (present(weatherServer,"JCMT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"JCMT_METEOROLOGY_X", &jcmtWeather, &humidityServerTimestamp);
    } else if (present(weatherServer,"UH88")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"UH88_METEOROLOGY_X", &uh88Weather, &humidityServerTimestamp);
    } else if (present(weatherServer,"VLBA")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"VLBA_METEOROLOGY_X", &vlbaWeather, &humidityServerTimestamp);
    } else if (present(weatherServer,"IRTF")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"IRTF_METEOROLOGY_X", &irtfWeather, &humidityServerTimestamp);
    } else {
      humidityServerTimestamp = 0;
    }
    rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_WIND_SERVER_C7",weatherServer, &longvalue);
    if (present(weatherServer,"SMA")) {
      windServerTimestamp = temperatureServerTimestamp;
    } else if (present(weatherServer,"UKIRT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"UKIRT_METEOROLOGY_X", &ukirtWeather, &windServerTimestamp);
    } else if (present(weatherServer,"KECK")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"KECK_METEOROLOGY_X", &keckWeather, &windServerTimestamp);
    } else if (present(weatherServer,"CFHT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"CFHT_METEOROLOGY_X", &cfhtWeather, &windServerTimestamp);
    } else if (present(weatherServer,"Subaru")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"SUBARU_METEOROLOGY_X", &subaruWeather, &windServerTimestamp);
    } else if (present(weatherServer,"JCMT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"JCMT_METEOROLOGY_X", &jcmtWeather, &windServerTimestamp);
    } else if (present(weatherServer,"UH88")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"UH88_METEOROLOGY_X", &uh88Weather, &windServerTimestamp);
    } else if (present(weatherServer,"VLBA")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"VLBA_METEOROLOGY_X", &vlbaWeather, &windServerTimestamp);
    } else if (present(weatherServer,"IRTF")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"IRTF_METEOROLOGY_X", &irtfWeather, &windServerTimestamp);
    } else {
      windServerTimestamp = 0;
    }
    rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_PRESSURE_SERVER_C7",weatherServer, &longvalue);
    if (present(weatherServer,"SMAaux")) {
      rms = rm_read(AUX_WEATHER_HOST,"RM_AUX_WEATHER_TIMESTAMP2_L",&pressureServerTimestamp);
    } else if (present(weatherServer,"SMA")) {
      pressureServerTimestamp = temperatureServerTimestamp;
    } else if (present(weatherServer,"UKIRT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"UKIRT_METEOROLOGY_X", &ukirtWeather, &pressureServerTimestamp);
    } else if (present(weatherServer,"KECK")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"KECK_METEOROLOGY_X", &keckWeather, &pressureServerTimestamp);
    } else if (present(weatherServer,"CFHT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"CFHT_METEOROLOGY_X", &cfhtWeather, &pressureServerTimestamp);
    } else if (present(weatherServer,"Subaru")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"SUBARU_METEOROLOGY_X", &subaruWeather, &pressureServerTimestamp);
    } else if (present(weatherServer,"JCMT")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"JCMT_METEOROLOGY_X", &jcmtWeather, &pressureServerTimestamp);
    } else if (present(weatherServer,"UH88")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"UH88_METEOROLOGY_X", &uh88Weather, &pressureServerTimestamp);
    } else if (present(weatherServer,"VLBA")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"VLBA_METEOROLOGY_X", &vlbaWeather, &pressureServerTimestamp);
    } else if (present(weatherServer,"IRTF")) {
      rms = call_dsm_read(DSM_WEATHER_HOST,"IRTF_METEOROLOGY_X", &irtfWeather, &pressureServerTimestamp);
    } else {
      pressureServerTimestamp = 0;
    }
  }
  
  rms = call_dsm_read(DSM_WEATHER_HOST, "DSM_GETWEATHER_TIMESTAMP_L", &getWeatherTimestamp,&longvalue);
  rms = rm_read(*antennaNumber, "RM_UNIX_TIME_L", &unixTime);
  
  /* this needs to be 10 min because if several servers do not respond, we can
   * lose several minutes before the getWeather loop repeats */
  if (unixTime > 720+getWeatherTimestamp && manualWeather==0) {
    standout();
    printw("d.stale: ");
    standend();
  } else {
#define STALE_SECONDS 1200
    if (unixTime > (STALE_SECONDS+temperatureServerTimestamp)
	/*  ||
	    unixTime > (STALE_SECONDS+pressureServerTimestamp) ||
	    unixTime > (STALE_SECONDS+humidityServerTimestamp) ||
	    unixTime > (STALE_SECONDS+windServerTimestamp)
	*/
	) {
      if (manualWeather == 0) {
	printw("s.stale: ");
      } else {
	printw("m.stale: ");
      }
    } else {
      printWeatherServer(weatherServer);
    }
  }
  if ((*temperature > -10.0) && (*temperature < 35.0))
    printw("%+.1fC  ",*temperature);
  else
    printw(" Wacko ");
  
  if (*humidity > HUMIDITY_MAX_STANDOUT || 
      *humidity < HUMIDITY_MIN_STANDOUT) {
    standout();
  }
  if (*humidity > HUMIDITY_POSITIVE_WACKO || *humidity < HUMIDITY_NEGATIVE_WACKO)
    printw("wacko");
  else
    printw("%.0f%% ",*humidity);
  standend();
  
  if (*pressure>PRESSURE_POSITIVE_WACKO || *pressure < PRESSURE_NEGATIVE_WACKO)
    printw(" wacko  ");
  else
    printw(" %3.0fmb  ",*pressure);
  if (*windspeed > windSpeedWacko || *windspeed < -windSpeedStandout) {
    if (weatherUnits == METRIC_WEATHER_UNITS)
      printw("wac m/s");
    else
      printw("wac mph");
  } else {
    if (*windspeed > windSpeedStandout)
      standout();
    if (*windspeed > 150 || *windspeed < -10) {
      printw("wacko");
    } else {
      if (weatherUnits == METRIC_WEATHER_UNITS) {
	printw("%.1fm/s",*windspeed);
      } else { 
	printw("%.1fmph",*windspeed);
      }
    }
    standend();
  }
  if (*winddirection > WIND_DIRECTION_WACKO_PLUS || *winddirection <= WIND_DIRECTION_WACKO_MINUS)
    printw("@wacdeg");
  else
    printw("@%.0fdeg",*winddirection);
  if (smaWindAsterisk == 1) {
    standout();
    printw("avg");
    printw("   ");
    standend();
  } else
    printw("      ");
  
#if DEBUG
  refresh();
#endif
  
  move(8,48);
  call_dsm_read(DSM_WEATHER_HOST,"DSM_CSO_225GHZ_TAU_TSTAMP_L",&tau225Minutes,
		&timestamp);
  call_dsm_read(DSM_WEATHER_HOST,"DSM_CSO_350MICRON_TAU_TSTAMP_L",&tau350Minutes,
		&timestamp);
  rms = rm_read(*antennaNumber,"RM_UTC_HOURS_F",&uthours);
  tau225Hours = tau225Minutes/60.;
  tau350Hours = tau350Minutes/60.;
  tau225stale = computeUTstale(tau225Hours,0.5,uthours);
  tau350stale = computeUTstale(tau350Hours,0.5,uthours);
  if (*csoTau<0 || *csoTau>9.99) {
    printw("CSOTau: wac/");
  } else {
    rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_JCMT_TAU_F",&jcmtTau, &longvalue);
    jcmtTauStale = 0;
    if (unixTime > (STALE_SECONDS+longvalue))
      jcmtTauStale = 1;
    if (tau225stale==1 && tau350stale==1 && jcmtTau > 0.01 && jcmtTauStale == 0) {
      printw("JCMT Tau: ");
      if (jcmtTau > STANDOUT_TAU(restFrequency[0]))
	standout();
      if (jcmtTau<0 || jcmtTau>9.99)
	printw("wacko");
      else
       printw("%.3f",jcmtTau);
      if (jcmtTau > STANDOUT_TAU(restFrequency[0]))
	standend();
    } else if (tau225stale==1) {
      printw("CSOTau:stale/",*csoTau);
    } else {
      printw("CSOTau:");
      if (*csoTau > STANDOUT_TAU(restFrequency[0])) {
	standout();
      }
      if (*csoTau < 0.1) {
	printw("%.3f",*csoTau);
      } else {
	printw(" %.2f",*csoTau);
      }
      if (*csoTau > STANDOUT_TAU(restFrequency[0]))
	standend();
      printw("/");
    }
  }
  if (tau350stale==0 || tau225stale==0) {
    if (*csoTauFromSimonRadfordMeter<0 || *csoTauFromSimonRadfordMeter>9.99) {
      printw("wac");
    } else {
      if (tau350stale==1) {
	printw("stale");
      } else {
	if (*csoTauFromSimonRadfordMeter > STANDOUT_TAU(restFrequency[0]))
	  standout();
	printw("%.2f",*csoTauFromSimonRadfordMeter);
	if (*csoTauFromSimonRadfordMeter > STANDOUT_TAU(restFrequency[0]))
	  standend();
      }
    }
  } else
    printw("stale");
#if DEBUG
  refresh();
#endif
  printw(" PM");
  if (unixTime > (saoSeeingTimestamp+10*86400) ||
      unixTime < (saoSeeingTimestamp-10*86400)) {
    printw(": n/a");
  } else if (unixTime > (saoSeeingTimestamp+3600)) {
    standout();
    printw(" stale");
    standend();
//   } else if (saoSeeingAmplitude[shortestPMBaseline] < 300.0) {
//     standout();
//     printw(" WET! ");
//     standend();
  } else if ((*saoSeeing <0.0) || (*saoSeeing > 1000.0))
    printw(" wacko");
  else if (*saoSeeing > 99.9) {
    printw(" BAD!");
  } else {
    if (*saoSeeingRatio < 10.0) {
      printw(": %3.1f/%3.1f", *saoSeeing, *saoSeeingRatio);
    } else if (*saoSeeingRatio < 100.0) {
      if (*saoSeeing < 10.0)
	printw(": %3.1f/%3.0f", *saoSeeing, *saoSeeingRatio);
      else
	printw(": %3.0f/%3.0f", *saoSeeing, *saoSeeingRatio);
    } else {
      if (*saoSeeing < 10.0)
	printw(": %3.1f/Big", *saoSeeing);
      else
	printw(": %3.0f/Big", *saoSeeing);
    }
  }
#if DEBUG
  refresh();
#endif
  
  move(10,2);
  lowestActiveCrate = getCrateList(cratesInArray);
  s = call_dsm_read("corcon",
		    "DSM_CORCON_HAL_CORR_SHUTDOWN_MESSAGE_C80",
		    (char *)shutdownMessage, &timestamp);
  if (strlen(shutdownMessage) < sizeof(shutdownMessageCopy))
    strcpy(shutdownMessageCopy, shutdownMessage); 
  else
    strncpy(shutdownMessageCopy, shutdownMessage, sizeof(shutdownMessageCopy));
#if DEBUG
  printw("About to call strtok()\n");
  refresh();
#endif
  if (strtok(shutdownMessage, " ")) {
#if DEBUG
    printw("strtok returns non-zero\n");
    refresh();
#endif
    Beep(CORCON_SHUTDOWN);
    if (strstr(shutdownMessageCopy, "corr shutdown:")) {
      int spaces;
      
#if DEBUG
      printw("saw corr shutdown:\n");
      refresh();
#endif
      spaces = (78 - strlen(shutdownMessageCopy))/2;
      if (spaces > 0) {
	int ii;
	
	for (ii = 0; ii < spaces; ii++)
	  printw(" ");
      }
      if (*icount % 2)
      standout();
      printw("%s", shutdownMessageCopy);
    } else {
#if DEBUG
      printw("did not see corr shutdown:\n");
      refresh();
#endif
      if (*icount % 2)
	standout();
      printw("correlator shutdown: %s",
	     shutdownMessageCopy);
#if DEBUG
      refresh();
#endif
    }
    standend();
  } else {
    static int pausedCount = 0;

#if DEBUG
    printw("strtok returns zero\n");
    refresh();
#endif
    NoBeep(CORCON_SHUTDOWN);
    if ((lowestActiveCrate > 0) && (lowestActiveCrate < 13)) {
      int ccc;
      int cratesInSync = TRUE;
      
      sprintf(crateName, "crate%d", lowestActiveCrate);
      dsm_read(crateName, "CRATE_TO_HAL_X", &crateStatusStructure[lowestActiveCrate], &timestamp);
      s = dsm_structure_get_element(&crateStatusStructure[lowestActiveCrate],
				    "IDL_SERVER_C40", dataServer);
      s = dsm_structure_get_element(&crateStatusStructure[lowestActiveCrate],
				    "MODE_S", 
				    (char *)&correlatorMode);
      s = dsm_structure_get_element(&crateStatusStructure[lowestActiveCrate],
				    "SCAN_TIME_D", 
				    (char *)&scanTime);
      s = dsm_structure_get_element(&crateStatusStructure[lowestActiveCrate],
				    "SCAN_NO_L", 
				    (char *)&scanNumber);
#ifdef LINUX
      s = call_dsm_read("hcn",
			"DSM_AS_SCANS_REMAINING_L", 
			(char *)&setScanNo,
			&timestamp);
#else
      s = call_dsm_read("m5",
                        "DSM_AS_SCANS_REMAINING_L",
                        (char *)&setScanNo,
                        &timestamp);
#endif
      s = dsm_structure_get_element(&crateStatusStructure[lowestActiveCrate],
				    "SCAN_LENGTH_D", 
				    (char *)&scanLength);
      for (ccc = 1; ccc < 13; ccc++)
	if (cratesInArray[ccc]) {
	  int tRx, tAnt, tChunk;
	  
	  sprintf(otherCrateName, "crate%d", ccc);
	  dsm_read(otherCrateName, "CRATE_TO_HAL_X", &crateStatusStructure[ccc], &timestamp);
	  s = dsm_structure_get_element(&crateStatusStructure[ccc],
					"SCAN_TIME_D", 
					(char *)&otherScanTime);
	  s = dsm_structure_get_element(&crateStatusStructure[ccc],
					"STATS_OK_V3_V5_V9_S", 
					(char *)&crateStatsOK[ccc]);
	  for (tRx = 1; tRx < 3; tRx++)
	    for (tAnt = 1; tAnt < 11; tAnt++)
	      if (!isReceiverInArray[tRx])
		antStatsOK[tRx-1][tAnt] = 1;
	      else
		for (tChunk = 1; tChunk < 5; tChunk++)
		  if ((crateStatsOK[ccc][tRx][tChunk][tAnt] != 1) &&
		      (((tAnt > 6) && (ccc > 6)) || ((tAnt < 7) && (ccc < 7))))
		    antStatsOK[tRx-1][tAnt] = 0;
	  if ((fabs(scanTime - otherScanTime) > 10.0) &&
	      (fabs(scanTime - otherScanTime) < (scanLength - 10.0)))
	    cratesInSync = FALSE;
	}
      if (correlatorMode != -1) {
	char dummyString[25];
	time_t dataStored;

	currentTime = time((long *)0);
	call_dsm_read(dataServer, "DSM_AS_SCAN_SOURCE_C24", &dummyString[0], &dataStored);
	if (((currentTime - dataStored) > 300) && (scanNumber > 10)) {
	  if (currentTime % 2)
	    standout();
	  printw("dataCatcher has not been storing scans for more than five minutes!          ");
	  if (currentTime % 2)
	    standend();
	} else if (!cratesInSync) {
	  pausedCount = 0;
	  Beep(CORR_NOT_SYNCD);
	  printw("The correlator crates are out of sync.   Please check the \"c\" page.       ");
	  standend();
	} else if (abs((int)timestamp - (int)currentTime) > 60) {
	  int timeDown, dd, hh, mm;
	  
	  pausedCount = 0;
	  NoBeep(CORR_NOT_SYNCD);
	  timeDown = abs((int)timestamp - (int)currentTime);
	  dd = timeDown / 86400;
	  hh = (timeDown - dd*86400) / 3600;
	  mm = (timeDown - dd*86400 - hh*3600) / 60;
	  if ((dd > 0) && (hh > 30)) {
	    hh = 0;
	    dd += 1;
	  }
	  if ((hh > 0) && (mm > 30)) {
	    mm = 0;
	    hh += 1;
	  }
	  else if ((dd == 0) && (hh == 0))
	    if (mm == 1)
	      printw("The correlator is not running.   It was last running 1 minute ago.      ");
	    else
	      printw("The correlator is not running.   It was last running %d minutes ago.     ", mm);
	  else if (dd == 0)
	    if (hh == 1)
	      printw("The correlator is not running.   It was last running about an hour ago.  ");
	    else
	      printw("The correlator is not running.   It was last running about %d hours ago.  ", hh);
	  else
	    if (dd == 1)
	      printw("The correlator is not running.   It was last running about a day ago.    ");
	    else if (dd > 7)
	      printw("The correlator is not running.   It was last running a long time ago.    ");
	    else
	      printw("The correlator is not running.   It was last running about %d days ago.  ", dd);
	} else if ((scanNumber > -1000000) && (scanNumber < 1000000)) {
	  pausedCount = 0;
	  printw("Crate%d scan %d, time %5.1f/%5.1f, ", lowestActiveCrate,
		 scanNumber, scanTime, scanLength);
	  
	  if (setScanNo == 1)
	    printw("1 more scan remains for this source ");
	  else if (setScanNo == -1)
	    printw("scans will be taken indefinitely    ");
	  else if (setScanNo > 1) {
	    if (setScanNo <= 99999999) {
	      printw("%d more scans remain for this source",
		     setScanNo);
	    } else {
	      printw("(wacko) more scans remain for this source");
	    }
	  } else
	    printw("the requested scans have been stored");
	} else {
	  pausedCount = 0;
	  printw("Scans are currently not being stored.");
	}
      } else {
	pausedCount++;
#ifdef LINUX
	if (pausedCount < 120)
#else
	if (pausedCount < 60)
#endif
	  printw("The correlator is in the paused state.                                     ");
	else {
	  standout();
	  printw("The correlator has been paused for more than a minute!                     ");
	  standend();
	}
      }
    } else {
      pausedCount = 0;
      printw("No correlator crates are active.                                           ");
    }
  }
  if (gensetActive) {
    move(10,0);
    standout();
    printw("               Emergency Generator Active - Please Stow Antennas                 ");
    standend();
  }
  
  /* draw separator line */
  move(11,2);
  for (i=2;i<(COLS-1); i++)
    printw("-");
  
  /* if an antenna is being Flagged,or is Smoking, then note it on the separator line */
  rms = call_dsm_read("hal9000",
		      "DSM_SCAN_FLAGS_V11_V2_L", 
		      (char *)&dSMScanFlags,
		      &timestamp);
  for (ant = 1; ant <= numberAntennas; ant++) {
     unsigned char antSmoke;
     short antennaStatus;
     counterSmoke++;
     if(counterSmoke%180 == 0)
     {
	smokeFile = fopen("/global/configFiles/fire/ignoreAntSmoking.txt","r");
	fscanf(smokeFile,"%d %d %d %d %d %d %d %d",&ignoreSmoking[1],&ignoreSmoking[2],&ignoreSmoking[3],&ignoreSmoking[4],&ignoreSmoking[5],&ignoreSmoking[6],&ignoreSmoking[7],&ignoreSmoking[8]);
	fclose(smokeFile);
     }
     //Check if antenna is smoking, $50 fine and 10 hours community service! 
      (void)rm_read(ant,"RM_ANTENNA_SMOKE_WARNING_B",&antSmoke);	
	if(antSmoke && ant <=8 && !ignoreSmoking[ant])
	{
		move(11,8+(ant-1)*9);
		standout();
		//start_color();
		//init_pair(1, COLOR_BLACK, COLOR_RED);
		//attron(COLOR_PAIR(1));
		printw("!SMOKING!" );
		//attroff(COLOR_PAIR(1));
		standend();

		move(23,1);
		standout();
		printw("      Smoke has been detected in ant %d, it will be stowed!!      ",ant);
		standend();
	}    

	//warn if current above 30 for 30 samples. 
      (void)rm_read(ant,"RM_EL_MOT_CUR_AMP_F",&antElCur[ant][sample[1]]);	
      (void)rm_read(ant,"RM_AZ1_MOT_CUR_AMP_F",&antAz1Cur[ant][sample[1]]);	
      (void)rm_read(ant,"RM_AZ2_MOT_CUR_AMP_F",&antAz2Cur[ant][sample[1]]);	
	if(ant<= 8 && (antAz1Cur[ant][0] >=50.0 || antAz2Cur[ant][0] >= 50.0))
	{
		move(23,1);
		standout();
		//printw(" High Az motor current has been detected in ant %d please make a note entry",ant);
		standend();

	}
	if(ant<=8 && (antElCur[ant][0] >=40.0))
	{
		move(23,1);
		standout();
		//printw("High El motor current has been detected in ant %d, please make a note entry",ant);
		standend();

	}

     //Check if antenna palm is disconnected 
      (void)rm_read(ant,"RM_SCB_FAULTWORD_L",&scbFaultWord);	
	if(scbFaultWord&0x01000000 && ant <=8)
	{
		move(11,8+(ant-1)*9);
		standout();
		printw("Palm Off" );
		standend();
	}    


    if (doubleBandwidth && isAntennaInArray[ant]) {
      short bWDDROLocked;
      
      (void)rm_read(ant, "RM_BWD_DRO_LOCKED_S", &bWDDROLocked);
      if (!bWDDROLocked) {
	move(11,8+(ant-1)*9);
	standout();
	printw("BDA Pwr " );
	standend();
      }
 
      /*
      (void)rm_read(ant, "RM_YIG_SWITCH_HIGH_FREQ_S", &bWDDROLocked);
      if (bWDDROLocked != BDA_YIG_SWITCH_POS) {
	move(11,8+(ant-1)*9);
	standout();
	printw("YIGSWCH " );
	standend();
      }
      */
    }

    rms = rm_read(ant, "RM_ANTENNA_STATUS_S", &antennaStatus);
    if ((antennaStatus == 1) && isAntennaInArray[ant]) {
      int iii;
      
      /* I'm here if the antenna is in interferometry mode */
      for (iii = 0; iii < 32; iii++) {
	if ((dSMScanFlags[ant][0] & (1 << iii)) &&
	    (dSMScanFlags[ant][1] & (1 << iii))) {
	  move(11,8+(ant-1)*9);
	  standout();
	  printw("Flagged");
	  standend();
	}
      }
    }
  }
  
  
  /* KLUDGE - list antennas used but not in project */
#if 0
  int rm_status;
  for (uAnt=1; uAnt<=8; uAnt++) {
    if (command_n[0] != 't' && command_n[0] != 'a') {
      tilting[uAnt] = 0;
    }
    rm_status=rm_read(uAnt,"RM_SMASH_TRACK_COMMAND_C30",
		      &command_n);
    if (command_n[0] == 't' || (command_n[0] == 'a' && tilting[uAnt] == 1)) {
      move(9, 9*uAnt);
      printw(" Tilt ");     
      tilting[uAnt] = 1;
    }
  }
#endif
  
  /* if someone is using an antenna, then put their name on it */
  antennaUser = fopen("/global/projects/antennaUser", "r");
  if (antennaUser != NULL) {
    int nTokens;
    char antUser[80];
    
    while ((nTokens = fscanf(antennaUser, "%d %s", &uAnt, antUser)) == 2) {
      if ((uAnt >= 0) && (uAnt < 9)) {
	if (tilting[uAnt] != 1) {
	  if (uAnt == 8) {
	    antUser[6] = (char)0;
	  } else {
	    antUser[7] = (char)0;
	  }
	  if (uAnt == 0) {
	    move(11, 2);
	  } else {
	    move(11, 9*uAnt);
	  }
	  if (projectID <= 0 || projectID >= WACKO_PROJECT_ID_MAX) {
	    standout();
	  }
	  printw(" %s ", antUser);
	  standend();
	  while ((antUser[0] != '\n') && (!feof(antennaUser))) {
	    antUser[0] = (char)getc(antennaUser);
	  }
	}
      }
    }
    fclose(antennaUser);
  }
  
  move(12,1);
  addstr("Ant/Pad "); 
  rms = call_dsm_read("hal9000", "DSM_ONLINE_ANTENNAS_V11_B",
		      &online[0], &timestamp);
  for (ant = 1; ant <= numberAntennas; ant++) {
    move(12, 9*ant - 1);
    if (deadAntennas[ant]) {
      switch (ant) {
      case JCMT_ANTENNA_NUMBER:
	printw("%d/JCMT   ",JCMT_ANTENNA_NUMBER);
	break;
      case CSO_ANTENNA_NUMBER:
	printw("%d/CSO",CSO_ANTENNA_NUMBER);
	break;
      default:
	printw("  ");
	if (antennaFlags.flags == 1)
	  standout();
	printw("%d", ant);
	standend();
	printw("  ");
      } 
    } else {
      if (!online[ant]) {
	standout();
	printw("OFFLIN" );
	standend();
      } else {
	rms = rm_read(ant,"RM_TILT_CORRECTION_FLAG_L",&tiltCorrections[ant]);
	if (tiltCorrections[ant] != defaultTiltFlag[ant]  && ant <= 8) {
	  standout();
	  addch('T');
	  standend();
	} else
	  addch(' ');
	antDisplay(ant, *icount, ANTENNA_PAGE_CHECK_ONLY, &antennaFlags);
	if (deiceChar[ant] != ' ' && ant<=8) {
	  if (deiceChar[ant] == 'F') {
	    standout();
	    addch('D');
	    standend();
	  } else
	    addch(deiceChar[ant]);
	  if (antennaFlags.flags == 1)
	    standout();
	  printw("%d",ant);
	  standend();
	  switch (ant) {
	  case JCMT_ANTENNA_NUMBER:
	    printw("/JCMT ");
	    break;
	  case CSO_ANTENNA_NUMBER:
	    printw("/CSO ");
	    break;
	  default:
	    printw("/");
	    if (padid[ant] < MIN_ANTENNA_PAD || padid[ant]>MAX_ANTENNA_PAD)
	    standout();
	    printw("%-2d", padid[ant]);
	    standend();
	    if (tiltCorrections[ant] == 0)
	      printw(" ");
	  }
	} else {
	  if (tiltCorrections[ant] == 0)
	    printw(" ");
	  if (antennaFlags.flags == 1 && ant<=8)
	    standout();
	  printw("%d",ant);
	  standend();
	  switch (ant) {
	  case JCMT_ANTENNA_NUMBER:
	    printw("/JCMT");
	    break;
	  case CSO_ANTENNA_NUMBER:
	    printw("/CSO");
	  break;
	  default:
	    printw("/");
	  if (padid[ant] < MIN_ANTENNA_PAD || padid[ant]>MAX_ANTENNA_PAD)
	    standout();
	  printw("%-2d", padid[ant]);
	  standend();
	  } 
	  printw(" ");
	}
      }
    }
    switch (ant) {
    case 8:
      if (numberAntennas > 8)
	addstr("   ");
      break;
    case JCMT_ANTENNA_NUMBER:
      break;
    case CSO_ANTENNA_NUMBER:
      addstr(" ");
      break;
    default:
      addstr("   ");
      break;
    }
  }
#if DEBUG
  refresh();
#endif
  
  move(13,1);
  addstr("Az/El");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      standend();
      if (padid[ant] == HANGAR_PAD_ID) {
	printw("  hangar ");
      } else {
	printw("   ----- ");
      }
    } else {
      if ((onSource[ant]==0)&&isAntennaInArray[ant]==1)
	standout();
      else
	standend();
      rms = rm_read(ant,"RM_IRIG_LOCK_ERROR_B",&dummyChar);
      if (trackStatus[ant]==1) {
	if (elfloat[ant]==elfloat[ant]) {
	  printw(" ");
	}
        if (azfloat[ant] <= -100) {
	  if (azfloat[ant] < -999 || azfloat[ant] > 999)
	    printw("wac/");
	  else
	    printw("%4.0f/",azfloat[ant]);
	  if (elfloat[ant]<0 || elfloat[ant]>99)
	    printw("wac");
	  else
	    printw("%-2.0f",elfloat[ant]);
	} else if (ant==numberAntennas) {
	  if (azfloat[ant] < -999 || azfloat[ant] > 999)
	    printw("wac/");
	  else
	    printw("%3.0f/",azfloat[ant]);
	  if (elfloat[ant]<0 || elfloat[ant]>99)
	    printw("wac");
	  else
	    printw("%-2.0f",elfloat[ant]);
	} else {
	  if (azfloat[ant] < -999 || azfloat[ant] > 999)
	    printw(" wac/");
	  else
	    printw(" %3.0f/",azfloat[ant]);
	  if (elfloat[ant]<0 || elfloat[ant]>99)
	    printw("wac");
	  else
	    printw("%-2.0f",elfloat[ant]);
	}
	if (dummyChar!=0) {
	  standout();
	  printw("I"); /* IRIG lock error */
	  standend();
	} else {
	  for (i = 0; i < NUMVARS; i++) {
	    rms = rm_read(ant, rv[i].name, rv[i].a );
	  }
	  if (chkElCollisionLimit==1) {
	    printw("C"); 
	    standend();
	  } else {
	    if (azfloat[ant] > -199.5) {
	      printw(" ");
	    }
	  }
	}
      } else
	printw(" TrkStale");
    }
  }
  
  
#if DEBUG
  refresh();
#endif
  move(14,1);
  standend();
  addstr("SunDist");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      standend(); /*line added by Todd Hunter to prevent highlight of ---- */
      printw(" -----");
    } else {
      if ((sunD[ant] < SUN_DISTANCE_LIMIT) && (trackStatus[ant] == 1))
	Beep(SUN_DISTANCE + ant);
      else
	NoBeep(SUN_DISTANCE + ant);
      printw(" %5.1f",sunD[ant]);
    }
    if (ant < numberAntennas)
      addstr("   ");
  }
  standend(); /*line added by Todd Hunter to prevent highlight of Drives 
                on the next line */
  
#if DEBUG
  refresh();
#endif
  move(15,1);
  printw("Drives ");
  
  standend();
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      standend(); /*line added by Todd Hunter to prevent highlight of ---- */
      printw(" ----- ");
      if (ant < numberAntennas)
	printw("  ");
    } else {
      rms = rm_read(ant,"RM_SERVO_TIMESTAMP_L",&timestamp);
      staleNess = (unixTime - timestamp);
      if (staleNess > 10) {
	if (isAntennaInArray[ant])
	  standout();
	addstr(" SrvS");
	if (staleNess < 1000)
	  printw("%03d ",staleNess); /* display minutes it has been stale */
	else
	  printw("*** ");
	standend();
      } else {
	static char dstat[][10] = {  "  off  ", "  on   ", " fault ",
				     "lockout", "one mot", " EL LMT",
				     " AZ LMT", "one tac", " ESTOP ",
				     "  ???  "};
	if (scbFaultWordArray[ant] & (1 << EMERGENCY_STOP_FAULT)) {
	  driveStatus[ant] = 8;
	}
	switch(driveStatus[ant]) {
	case 0:
	  if (scbFaultWordArray[ant] & (1 << AIR_PRESSURE_SWITCH_FAULT)) {
	    Beep(DRIVE_FAULT + ant);
	    addstr("AZBRAKE");
	  } else {
	    if (isAntennaInArray[ant])
	      Beep(DRIVE_FAULT + ant);
	    else
	      NoBeep(DRIVE_FAULT + ant);
	    addstr(dstat[0]);
	  }
	  break;
	case 1:
	  NoBeep(DRIVE_FAULT + ant);
	  addstr(dstat[1]);
	  break;
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
	case 8:
	  if (isAntennaInArray[ant])
	    Beep(DRIVE_FAULT + ant);
	  else
	    NoBeep(DRIVE_FAULT + ant);
	  addstr(dstat[driveStatus[ant]]);
	  break;
	default:
	  addstr(dstat[9]);
	  break;
	} /* end switch */
        standend();
	if (ant < numberAntennas)
	  addstr("  ");
      } /* endif */
    }
  }
#if DEBUG
  refresh();
#endif
  move(16,1);
  standend();
  printw("Choppers");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      standend();
      printw("----- ");
    } else {
      int st;
      static int chop_err_cnt[MAX_NUMBER_ANTENNAS + 1];

      /* should check that chopperControl is running */
      st = chopperStatus[ant] & 0x3f;
      if (st != 0) {
	if (isAntennaInArray[ant]) {
	  char el_drv_state;

	  rms = rm_read(ant, "RM_EL_DRV_STATE_B", &el_drv_state);
	  if (el_drv_state == 3 && ++chop_err_cnt[ant] > 40) {
	    Beep(CHOPPER_FAULT + ant);
	  }
	  if (el_drv_state > 3) chop_err_cnt[ant] = -5;
	}
	if (st & 0x20)
	  printw(" stale");
	else
          printw(" %c%c%c%c ", (st & 8)? 'X': '-', (st & 4)? 'Y': '-',
		 (st & 2)? 'Z': '-', (st & 0x10)? 'C': ((st & 1)? 'T': '-'));
      } else {
	i = chopperStatus[ant] & 0x40;
	if (isAntennaInArray[ant]) {
	  chop_err_cnt[ant] = 0;
	  if ((i == 0) ^ (chopperStatus[0] <= 0))
	    Beep(CHOPPER_FAULT + ant);
	  else
	    NoBeep(CHOPPER_FAULT + ant);
	}
	if (i) {
	  printw(" OK-FC");
	} else {
	  printw(" OK");
	  standout();
	  printw("NFC");
	  standend();
	}
      }
    }
    if (isAntennaInArray[ant])
      NoBeep(CHOPPER_FAULT + ant);
    if (ant < numberAntennas)
      addstr("   ");
  }
  
#if DEBUG
  refresh();
#endif
  move(17,1);
  standend();
  printw("M3Doors ");
  opticsPage(*icount,antlist,OPTICS_PAGE_CHECK_ONLY,&opticsFlags);
  c1DC(*icount,C1DC_PAGE_CHECK_ONLY,&c1DCFlags);
  rms = dsm_read("hal9000", "DSM_OBS_POLAR_V11_C1", waveplateOrientations, &timestamp);
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      standend(); /*line added by Todd Hunter to prevent highlight of ---- */
      printw("-----  ");
      if (ant < numberAntennas)
	addstr("  ");
    } else {
      static char m3stat[7][7] = { "   ?  ", " close", " open ", "moving",
				   "  ??? ", "close?", " open?"};
      
      if (m3Status[ant] == 1 && isAntennaInArray[ant])
	Beep(M3_FAULT + ant);
      else
	NoBeep(M3_FAULT + ant);
      if (m3Status[ant] > 6 || m3Status[ant] < 0)
	m3Status[ant] = 4;
      if (opticsFlags.flags[ant]!=0 && mirrorDoorIsOpen(m3Status[ant])) {
	standout();
	if (opticsFlags.feedOffsetMismatch[ant] != 0)
	  printw("feedOffs");
	if (opticsFlags.wavePlate[ant]) {
	  char label[8];

	  switch (waveplateOrientations[ant]) {
	  case 'R': strcpy(label, " WP(R)  "); break;
	  case 'L': strcpy(label, " WP(L)  "); break;
	  default:  strcpy(label, " WP(?)  ");
	  }
	  if ((*icount % 10) != 0)
	    standend();
	  printw(label);
	} else
	  printw("'o' page");
	standend();
        if (ant < numberAntennas)
          addstr(" ");
      } else {
        addstr(m3stat[m3Status[ant]]);
        if (ant+1 < numberAntennas)
          addstr("   ");
	else if (ant < numberAntennas)
          addstr(" ");
      }
    }
  }
  
#if DEBUG
  refresh();
#endif
  move(18,1);
  standend();
  printw("G-Y   ");
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (ant <= numberAntennas)
      printw(" ");
    if (deadAntennas[ant]) {
      standend(); /*line added by Todd Hunter to prevent highlight of ---- */
      if (ant==1)
        printw("  ----- ");
      else if (ant==numberAntennas)
        printw(" ----- ");
      else
        printw(" -----   ");
    } else {
      rms = rm_read(ant, "RM_GUNN1_LOCKED_TIMESTAMP_L", &dummyInt);
      delay = unixTime - dummyInt;
      if (((gunn1Status[ant]!=1) || (yig1Status[ant]!=1) ||
           gunn2Status[ant]!=1) &&
	  isAntennaInArray[ant])
        if ((restFrequency[0] > HI_LO_FREQ_CUTOFF_HZ && gunn2Status[ant]==1 && yig1Status[ant]==1) ||
	    (restFrequency[0] < HI_LO_FREQ_CUTOFF_HZ && gunn1Status[ant]==1 && yig1Status[ant]==1))
	  NoBeep(LO_FAULT + ant);
        else
	  Beep(LO_FAULT + ant);
      else
        NoBeep(LO_FAULT + ant);
      rms = rm_read(ant, "RM_TUNE6_COMMAND_BUSY_S", &busy);
      switch (busy) {
      case TUNE6_BUSY_WITH_TUNE_GUNN_LOW:
	printw("tuningL");
	break;
      case TUNE6_BUSY_WITH_TUNE_GUNN_HIGH:
	printw("tuningH");
	break;
      case TUNE6_BUSY_WITH_OPT_LO_POWER_LOW:
	printw("LO adjL");
	break;
      case TUNE6_BUSY_WITH_OPT_LO_POWER_HIGH:
	printw("LO adjH");
	break;
      case TUNE6_BUSY_WITH_IV_SWEEP:
	standout();
	printw("IV ");
	standend();
	printw("/");
	if (gunn2Status[ant] < 0 || gunn2Status[ant] > 1) {
	  printw("w");
	} else {
#define PHASE_LOCK_IS_STALE 90
	  if (delay>PHASE_LOCK_IS_STALE && isAntennaInArray[ant])
	    standout();
	  printw("%d",gunn2Status[ant]);
	  standend();
	} 
	printw("-");
	if (yig2Status[ant] < 0 || yig2Status[ant] > 1)
	  printw("w");
	else
	  printw("%d",yig2Status[ant]);
	break;
      case TUNE6_BUSY_WITH_IV_SWEEP_HIGH_FREQ:
	if (gunn1Status[ant] < 0 || gunn1Status[ant] > 1) {
	  printw("w");
	} else {
	  if (delay>PHASE_LOCK_IS_STALE && isAntennaInArray[ant]) {
	    standout();
	  }
	  printw("%d",gunn1Status[ant]);
	  standend();
	} 
	printw("-");
	if (yig1Status[ant] < 0 || yig1Status[ant] > 1)
	  printw("w/");
	else
	  printw("%d/",yig1Status[ant]);
	printw(" IV");
	break;
      case TUNE6_BUSY_WITH_PB_SWEEP:
	standout();
	printw("PB ");
	standend();
	printw("/");
	if (gunn2Status[ant] < 0 || gunn2Status[ant] > 1) {
	  printw("w");
	} else {
	  if (delay>PHASE_LOCK_IS_STALE && isAntennaInArray[ant])
	    standout();
	  printw("%d",gunn2Status[ant]);
	  standend();
	} 
	printw("-");
	if (yig2Status[ant] < 0 || yig2Status[ant] > 1)
	  printw("w");
	else
	  printw("%d",yig2Status[ant]);
	break;
      case TUNE6_BUSY_WITH_PB_SWEEP_HIGH_FREQ:
	if (gunn1Status[ant] < 0 || gunn1Status[ant] > 1) {
	  printw("w");
	} else {
	  if (delay>PHASE_LOCK_IS_STALE && isAntennaInArray[ant]) {
	    standout();
	  }
	  printw("%d",gunn1Status[ant]);
	  standend();
	} 
	printw("-");
	if (yig1Status[ant] < 0 || yig1Status[ant] > 1)
	  printw("w/");
	else
	  printw("%d/",yig1Status[ant]);
	printw(" PB");
	break;
      default:
	if (doubleBandwidth && !isReceiverInArray[1]) {
	  if ((yig1Status[ant] == 1) || !isAntennaInArray[ant])
	    printw("    ");
	  else {
	    standout();
	    printw("6GHz");
	    standend();
	  }
	} else {
	  if (gunn1Status[ant] < 0 || gunn1Status[ant] > 1) {
	    printw("w");
	  } else {
	    if (delay>PHASE_LOCK_IS_STALE && isAntennaInArray[ant])
	      standout();
	    if (gunn1Status[ant] == 0 && isReceiverInArray[1]==1 && isAntennaInArray[ant]==1)
	      standout();
	    printw("%d",gunn1Status[ant]);
	    standend();
	  } 
	  printw("-");
	  if (yig1Status[ant] < 0 || yig1Status[ant] > 1) {
	    printw("w/");
	  } else {
	    if (yig1Status[ant] != 1 && isReceiverInArray[1]==1 && isAntennaInArray[ant]==1)
	      standout();
	    printw("%d",yig1Status[ant]);
	    standend();
	    printw("/");
	  } 
	}
	if (doubleBandwidth && !isReceiverInArray[2]) {
	  move(18, 11+(ant-1)*9);
	  if ((yig2Status[ant] == 1) || !isAntennaInArray[ant])
	    printw("    ");
	  else {
	    standout();
	    printw("6GHz");
	    standend();
	  }
	} else {
	  if (gunn2Status[ant] < 0 || gunn2Status[ant] > 1) {
	    printw("w");
	  } else {
	    if (gunn2Status[ant] == 0 && isReceiverInArray[2]==1 && isAntennaInArray[ant]==1)
	      standout();
	    printw("%d",gunn2Status[ant]);
	    standend();
	  } 
	  printw("-");
	  if (yig2Status[ant] < 0 || yig2Status[ant] > 1) {
	    printw("w");
	  } else {
	    if (yig2Status[ant] == 0 && isReceiverInArray[2]==1 && isAntennaInArray[ant]==1)
	      standout();
	    printw("%d",yig2Status[ant]);
	    standend();
	  } 
	}
	break;
      }
      if (ant < numberAntennas)
        printw(" ");
    }
  }
  
#define LAKESHORE_STALE 60
#if DEBUG
  refresh();
#endif
  move(19,1);
  standend();
  printw("IF/LO ");
  iFLODisplayPage2(*icount, antlist, IFLO_PAGE_CHECK_ONLY, &ifloFlags);
  
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      printw("  -----");
      printw(" ");
      if (ant < numberAntennas)
	printw(" ");
    } else {
      printw(" ");
      if ((antStatsOK[0][ant] == 1) && (antStatsOK[1][ant] == 1)) {
	printw(" ");
      } else {
	if (isAntennaInArray[ant])
	  standout();
	printw("C");
	standend();
      }
      if (ifloFlags.flags1[ant] == 0 ) {
	printw("-");
      } else {
	if (ifloFlags.mrgRfPower[ant] == 1 
#if 1
	    || ifloFlags.xmit1power[ant]==1
#endif
	    ) {
	  standout();
	}
	printw("I");
	standend();
      }
      if (c1DCFlags.flags1[ant] == 1) {
	/*	standout();*/
	printw("f");
	standend();
      } else
	printw("-");
      
      printw("/");
      
      if (ifloFlags.flags2[ant] == 0 || isReceiverInArray[2] == 0) {
	/* do not bother highlighting a problem with the second IF until we
	 * know whether the second receiver is active */
	printw("-");
      } else {
	if (ifloFlags.mrgRfPower[ant] == 1 
#if 1
	    || ifloFlags.xmit2power[ant]==1
#endif
	    ) {
	  standout();
	}
	printw("I");
	standend();
      }
      if (c1DCFlags.flags2[ant] == 1) {
	/*	standout();*/
	printw("f");
	standend();
      } else
	printw("-");
      printw(" ");
      if (ant < numberAntennas)
	printw(" ");
    }
  }
  
#if DEBUG
  refresh();
#endif
  allanVariancePage(*icount, antlist, ALLAN_VARIANCE_PAGE_CHECK_ONLY,&allanVarianceFlags);
  move(20,1);
  standend();
  if (isReceiverInArray[2] || doubleBandwidth) {
    move(20,1);
    printw("Dwr/Cal");
    for (ant = 1; ant <= numberAntennas; ant++) {
      if (deadAntennas[ant]) {
	standend();
	if (ant==1) {
	  printw(" -----   ");
	} else if (ant==numberAntennas) {
	  printw("----- ");
	} else {
	  printw("-----  ");
	}
      } else {
	if (*timeStamp>(dewarTime[ant]+LAKESHORE_STALE)) {
	  NoBeep(DEWAR_WARM + ant);
	  standout();
	  printw("stl");
	  standend();
	  printw("/");
	} else {
	  if (dewarTemp[ant] > 5.2 && (projectID==0 || isAntennaInArray[ant]))
	    Beep(DEWAR_WARM + ant);
	  else
	    NoBeep(DEWAR_WARM + ant);
	  if (wackyRxTemp[ant]) {
	    if (dewarTemp[ant] < 10)
	      printw("%4.1fD",dewarTemp[ant]);
	    else
	      printw("%4.0fD",dewarTemp[ant]);
	  } else {
	    if (dewarTemp[ant] < 10)
	      printw("%4.1f/",dewarTemp[ant]);
	    else
	      printw("%4.0f/",dewarTemp[ant]);
	  }
	}
      }
      if (!deadAntennas[ant]) {
	rms = rm_read(ant, "RM_OPTICS_BOARD_PRESENT_S",
		      &opticsBoardPresent[ant]);
	if (opticsBoardPresent[ant] == 0) {
	  printw("NoB");
	  if (ant < numberAntennas)
	    addstr(" ");
	} else {
	  printNewHotloadPosition(ant);
	}
      }
      if (ant < numberAntennas)
	addstr(" ");
    } /* end of 'for' loop */ 
    move(21,1);
    standend();
    if (doubleBandwidth)
      printw("Tsys 6-8");
    else
      printw("Tsys2(K)");
    /* see if any of the receiver values is out of range */
    if (!doubleBandwidth)
      receiverMonitorHighFreq(*icount, antlist, RECEIVER_PAGE_CHECK_ONLY, &receiver1Flags);
    for (ant = 1; ant <= numberAntennas; ant++) {
      if (deadAntennas[ant]) {
	printw("-----  ");
      } else {
	rms = rm_read(ant, "RM_SYNCDET2_TIMESTAMP_L",&syncdet2timestamp);
	rms = rm_read(ant, "RM_TSYS_TIMESTAMP_L",&tsystimestamp);
	if (opticsBoardPresent[ant] == 0 && (ant < 9)) {
	  printw("NoBoard");
	  if (ant < numberAntennas)
	    addstr(" ");
	} else {
	  if (tsystimestamp < (unixTime-300) && (ant < 9)) {
	    standout();
	    printw(" stale ");
	    standend();
	  } else {
	    if((tsys2[ant] < 100000.0) && (tsys2[ant] > 10)) {
	      printw("%6.0f",tsys2[ant]);
	      if(syncdet2timestamp < (unixTime-15)) {
	        standout();
	        printw("s");
	        standend();
	      } else if (receiver1Flags.flags[ant] == 1) {
		standout();
		printw("r");
		standend();
	      } else {
		if (allanVarianceFlags.highFreqFlags[ant] == 1) {
		  standout();
		  printw("V");
		  standend();
		} else {
		  if (ant < numberAntennas)
		    printw(" ");
		}
	      }
	    } else
	      printw(" wacko ");
	  }
	}
      }
      if (ant < numberAntennas) {
	if (tsystimestamp < (unixTime-300)) {
	  addstr(" ");
	} else {
	  if (ant < 7)
	    addstr("  ");
	  else
	    addstr(" ");
	}
      }
    } /* end of 'for' loop over antennas */
  } else {
    /* only 1 receiver is active */
    printw("Dewar(K)");
    for (ant = 1; ant <= numberAntennas; ant++) {
      if (deadAntennas[ant]) {
	standend(); /*line added by Todd Hunter to prevent highlight of ---- */
	if (ant==1)
	  printw(" -----   ");
	else if (ant==numberAntennas)
	  printw("----- ");
	else
	  printw("-----    ");
      } else {
	if (*timeStamp>(dewarTime[ant]+LAKESHORE_STALE)) {
	  NoBeep(DEWAR_WARM + ant);
	  printw("stale ");
	} else {
	  if (dewarTemp[ant] > 5.0 && (projectID==0 || isAntennaInArray[ant])) {
	    Beep(DEWAR_WARM + ant);
	  } else {
	    NoBeep(DEWAR_WARM + ant);
	  }
	  printw("%5.1f ",dewarTemp[ant]);
	}
	if (ant < numberAntennas)
	  addstr("   ");
      }
    }
    standend();
    
#if DEBUG
    refresh();
#endif
    move(21,1);
    printw("CalVanes");
    for (ant = 1; ant <= numberAntennas; ant++) {
      if (deadAntennas[ant]) {
	if (ant==1)
	  printw(" -----   ");
	else if (ant==numberAntennas)
	  printw(" -----");
	else
	  printw("-----    ");
      } else {
	rms = rm_read(ant, "RM_OPTICS_BOARD_PRESENT_S",
		      &opticsBoardPresent[ant]);
	if ((opticsBoardPresent[ant] == 0) && (ant < 9)) {
	  printw("NoBoard");
	  if (ant < numberAntennas)
	    addstr(" ");
	} else {
	  /* This looks like it is not needed.
	  if (ant < numberAntennas) {
	    printw(" ");
	  }
	  */
	  printLinearLoadStatusWordSky(ant);
	  if (ant < numberAntennas) {
	    printw("  ");
	  }
	}
      }
    }
  }
#if DEBUG
  refresh();
#endif
  
  move(22,1);
  if (isReceiverInArray[2])
    printw("Tsys1(K)");
  else if (doubleBandwidth)
    printw("Tsys 4-6");
  else
    printw("Tsys (K)");
  
  /* see if any of the receiver values is out of range */
  receiverMonitor(*icount, antlist, RECEIVER_PAGE_CHECK_ONLY, &receiver0Flags);
  if (isReceiverInArray[2]==1) {
    receiverMonitorHighFreq(*icount, antlist, RECEIVER_PAGE_CHECK_ONLY, &receiver1Flags);
  }
  for (ant = 1; ant <= numberAntennas; ant++) {
    if (deadAntennas[ant]) {
      printw("----- ");
      if (ant != numberAntennas)
	printw(" ");
    } else {
      rms = rm_read(ant, "RM_SYNCDET2_TIMESTAMP_L",&syncdet2timestamp);
      rms = rm_read(ant, "RM_TSYS_TIMESTAMP_L",&tsystimestamp);
      if (opticsBoardPresent[ant] == 0 && (ant < 9)) {
	printw("NoBoard");
	if (ant < numberAntennas)
	  addstr(" ");
      } else {
	if (tsystimestamp < (unixTime-15) && (ant < 9)) {
	  standout();
	  printw(" stale ");
	  standend();
	} else {
	  if ((tsys[ant] < 50000.0) && (tsys[ant] > 10)) {
	    printw("%6.0f",tsys[ant]);
	    if (syncdet2timestamp < (unixTime-15)) {
	      standout();
	      printw("s");
	      standend();
	    } else if (receiver0Flags.flags[ant] == 1) {
	      standout();
	      printw("r");
	      standend();
	    } else {
	      if (allanVarianceFlags.lowFreqFlags[ant] == 1) {
		standout();
		printw("V");
		standend();
	      } else {
		if (ant < numberAntennas)
		  printw(" ");
	      }
	    }
	  } else
	    printw(" wacko ");
	}
      }
    }
    if (ant < numberAntennas) {
      if (tsystimestamp < (unixTime-300))
	addstr(" ");
      else
	if (ant < 7)
	  addstr("  ");
	else
	  addstr(" ");
    }
  }
  
  
#if DEBUG
  refresh();
#endif
  rms = call_dsm_read("m5",
		      "MRG_CONTROL_X", 
		      (char *)&mRGControl,
		      &timestamp);
  rms = dsm_structure_get_element(&mRGControl,
				  "YIG_LOCKED_V2_S", 
				  (char *)&mRGLocked);
  for (rx = 1; rx >= 0; rx--) {
    if (!doubleBandwidth && !fullPolarization) {
      move(9,(rx*40)+2);
      printw("                                     ");
      move(9,(rx*40)+2);
    } else if (fullPolarization) {
      move(9, 42);
      if (restFrequency[0] == restFrequency[1])
	printw("Full Polarization                    ");
      else {
	printw("Full Polarization ");
	standout();
	printw("Rx Tuning Mismatch");
	standend();
      }
      move(9, 2);
    } else {
      short c1Bandwidth;

      rms = call_dsm_read("hal9000",
			  "DSM_AS_C1_SOURCE_S",
			  (char *)&c1Bandwidth,
			  &timestamp);
      move(9, 42);
      if (c1Bandwidth)
	printw("1 Receiver, 4 GHz Mode, 4 GHz contin.");
      else
	printw("1 Receiver, 4 GHz Mode, 2 GHz contin.");
      move(9, 2);
    }
    if (isReceiverInArray[rx+1]) {
#define WACKO_HZ (3.0e12)
      if (rx == 1)
	printw("URF");
      else
	printw("LRF");
      if (restFrequency[rx] > WACKO_HZ || restFrequency[rx] < 1.0e6)
	printw("  wacko ");
      else
	printw("  %9.5f GHz ", restFrequency[rx]*1.0e-9);
#if DEBUG
      refresh();
#endif
      if (mRGLocked[rx]) {
	int rms;
	rms = call_dsm_read("hal9000",
			    "DSM_AS_IFLO_LINE_NAME_V2_C41",
			    (char *)lineName, &timestamp);
#if DEBUG
	printw("Finished call_dsm_read(DSM_AS_IFLO_LINE_NAME_V2_C41)\n");
	refresh();
#endif
	lineName[rx][12] = (char)0;
	if (strlen(lineName[rx]) < 1)
	  sprintf(lineName[rx], "unknown");
	if (!strcmp(lineName[rx], "unknown")) {
	  short chunks[2];

	  rms = call_dsm_read("hal9000",
			      "DSM_REQUESTED_CHUNK_V2_S",
			      (char *)&chunks[0], &timestamp);
	  if ((chunks[rx] > 0) && (chunks[rx] <= 48))
	    sprintf(lineName[rx], "s%02d", chunks[rx]);
	}
	printw("(%s) ", lineName[rx]);
	if (sideband[rx] == 1)
	  printw("LSB");
	else if (sideband[rx] == -1)
	  printw("USB");
	else
	  printw("UnknownSB");
      } else {
	standout();
	printw(" MRG YIG Unlckd");
	standend();
      }
    } else if (!doubleBandwidth) {
      if (rx == 0) {
	if (projectID > 0)
	  standout();
	printw("The low frequency Rx is not in use");
	standend();
      } else {
	printw("The high frequency Rx is not in use");
      }
    }
  }
#if DEBUG
  refresh();
#endif
  
  rms = call_dsm_read("colossus","DSM_ANALOG_ROOM_TEMPERATURE_F", &equipmentRoomTemperature, &timestamp);
#if DEBUG
  printw("Finished call_dsm_read(DSM_ANALOG_ROOM_TEMPERATURE_F)\n");
  refresh();
#endif
  rms = call_dsm_read("colossus","DSM_ANALOG_ROOM_HITEMPLIMIT_F", &alarmTemperature, &timestamp);
#if DEBUG
  printw("Finished call_dsm_read(DSM_ANALOG_ROOM_HITEMPLIMIT_F)\n");
  refresh();
#endif
  rms = call_dsm_read("colossus","DSM_ANALOG_ROOM_HITEMPALARM_S", &alarmShort, &timestamp);
#if DEBUG
  printw("Finished call_dsm_read(DSM_ANALOG_ROOM_HITEMPALARM_S)\n");
  refresh();
#endif
  if (alarmShort == 1 && equipmentRoomTemperature>alarmTemperature) {
    move(23,20);
    standout();
    printw(" The summit equipment room is over temperature!!!! ");
    standend();
  } else {
    /* check for over temperature in Hilo */
    move(23,11);
    if ((alarmFp=fopen("/global/hiloOverTemperatureAlarm","r")) != NULL) {
      standout();
      printw(" The Hilo computer room with ulua is over temperature! ");
      overTemperature = 1;
      standend();
      fclose(alarmFp);
    } else {
      if (unixTime-timestamp > 600) {
	printw(" The equipment room temp monitor is stale (colossus) ");
      } else {
	if (overTemperature==1) {
	  if (LINES==24)
	    printw("-------------------------------------------------------");
	  else
	    printw("                                                       ");
	}
      }
      overTemperature = 0;
    }
  }
#if DEBUG
  printw("About to check for a wide screen\n");
  refresh();
#endif
  if (numberAntennas > 8) {
#if DEBUG
    printw("numberAntennas=%d, COLS=%d\n",numberAntennas,COLS);
    refresh();
#endif
    if ((COLS >= MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE) && 0) {
      switch (upperRightWindow) {
      case UR_STDERR:
	commandDisplay(*icount, &pageOffset, &searchMode, searchString,
		       STDERR_LOG,4,80,0,
#define FIXED_WIDTH_EXTENSION 0
#if FIXED_WIDTH_EXTENSION
		       MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE-80
#else
	               COLS-81
#endif
		       );
	break;
      case UR_CORRELATOR:
	commandDisplay(*icount, &pageOffset, &searchMode, searchString,
		       CORR_MONITOR_LOG,4,80,0,
#define FIXED_WIDTH_EXTENSION 0
#if FIXED_WIDTH_EXTENSION
		       MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE-80
#else
		       COLS-81
#endif
		       );
	break;
      case UR_SMASHLOG:
	commandDisplay(*icount, &pageOffset, &searchMode, searchString,
		       SMASH_LOG,4,80,0,
#if FIXED_WIDTH_EXTENSION
		       MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE-80
#else
		       COLS-81
#endif
		       );
	break;
      default:
      case UR_MESSAGES:
	commandDisplay(*icount, &pageOffset, &searchMode, searchString,
		       MESSAGES_LOG,4,80,0,
#if FIXED_WIDTH_EXTENSION
		       MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE-80
#else
		       COLS-81
#endif
		       );
	break;
      }
      
    } else {
      move(1,80);
      printw("Hit 'E' again to   ");
      move(2,80);
      printw("see more features! "); 
      move(3,82);
      printw("    eeeeeee    ");
      move(4,82);
      printw("  eee     ee   ");
      move(5,82);
      printw(" eee      eee  ");
      move(6,82);
      printw(" eeeeeeeeeeee  ");
      move(7,82);
      printw(" eee           ");
      move(8,82);
      printw("  eee      ee  ");
      move(9,82);
      printw("   eeeeeeeee   ");
      move(10,82);
      printw("     eeee      ");
    }
  }
#if DEBUG
  printw("Finished drawing extra lines\n");
  refresh();
#endif
  /* put cursor at bottom so that when you quit, your window is not
   * left in the middle of the page */
  move(23,0);
  refresh();
  if (needsBeep) {
#ifdef LINUX
    beep();
#else
    putchar('\a');
#endif
    fflush(stdout);
    needsBeep = 0;
  }
}

/*
  void hms(double *fx, int *fh, int *fm, double *fs,short *dec_sign)
  {
  double fmt;
  double absfx;
  
  if (*fx<0.) {
  absfx=-*fx;
  *dec_sign=-1;
  }
  if (*fx>=0.) {
  absfx=*fx;
  *dec_sign=1;
  }
  *fh = (int)absfx;
  fmt = (absfx - *fh) * 60.;
  *fm = (int) fmt;
  *fs = (fmt - *fm) * 60.;
  if (*fx < 0.)
  *fh = -(*fh);
  }
*/

double sunDistance(double az1,double el1,double az2,double el2)
{
  extern double radian;
  double cosd,sind,d;
  cosd=sin(el1)*sin(el2)+cos(el1)*cos(el2)*cos(az1-az2);
  sind=pow((1.0-cosd*cosd),0.5);
  d=atan2(sind,cosd);
  d=d/radian;
  return d;
}

void Beep(int faultNum) {
  unsigned char *cp = &beepControl[faultNum];
  if (*cp == 0)
    needsBeep++;
  *cp = TIME_TO_CLEAR;
  standout();
}

void NoBeep(int faultNum) {
  unsigned char *cp = &beepControl[faultNum];
  if (*cp)
    *cp -= 1;
  standend();
}

void printWeatherServer(char *server) {
  char temperatureServer[7];
  char humidityServer[7];
  char windServer[7];
  char pressureServer[7];
  int rms;
  long timestamp;
  
  rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_TEMPERATURE_SERVER_C7",temperatureServer,&timestamp);
  rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_HUMIDITY_SERVER_C7",humidityServer,&timestamp);
  rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_WIND_SERVER_C7",windServer,&timestamp);
  rms = call_dsm_read(DSM_WEATHER_HOST,"DSM_PRESSURE_SERVER_C7",pressureServer,&timestamp);
  
  if (strcmp(temperatureServer,humidityServer)==0 &&
      strcmp(temperatureServer,windServer)==0 &&
      strcmp(temperatureServer,pressureServer)==0
      ) {
    printw("@ %s: ",server);
  } else if (present(temperatureServer,"SMA") && 
	     present(humidityServer,"SMA") &&
	     present(windServer,"SMA") &&
	     present(pressureServer,"SMA")) {
    /* takes care of the case when using some SMAaux vars + SMA vars */
    printw("@ SMA: ");
  } else if ((present(temperatureServer,"SMA") || 
	      present(temperatureServer,"JCMT")) &&
	     (present(humidityServer,"SMA") ||
	      present(humidityServer,"JCMT")) &&
	     (present(windServer,"SMA") ||
	      present(windServer,"JCMT")) &&
	     (present(pressureServer,"SMA") ||
	      present(pressureServer,"JCMT"))) {
    printw("SMA/JCMT ");
  } else if ((present(temperatureServer,"Subaru") || 
	      present(temperatureServer,"JCMT")) &&
	     (present(humidityServer,"Subaru") ||
	      present(humidityServer,"JCMT")) &&
	     (present(windServer,"Subaru") ||
	      present(windServer,"JCMT")) &&
	     (present(pressureServer,"Subaru") ||
	      present(pressureServer,"JCMT"))) {
    printw("JCMT/Sub ");
  } else if ((present(temperatureServer,"Subaru") || 
	      present(temperatureServer,"SMA")) &&
	     (present(humidityServer,"Subaru") ||
	      present(humidityServer,"SMA")) &&
	     (present(windServer,"Subaru") ||
	      present(windServer,"SMA")) &&
	     (present(pressureServer,"Subaru") ||
	      present(pressureServer,"SMA"))) {
    printw("SMA/Suba ");
  } else if ((present(temperatureServer,"CFHT") || 
	      present(temperatureServer,"SMA")) &&
	     (present(humidityServer,"CFHT") ||
	      present(humidityServer,"SMA")) &&
	     (present(windServer,"CFHT") ||
	      present(windServer,"SMA")) &&
	     (present(pressureServer,"CFHT") ||
	      present(pressureServer,"SMA"))) {
    printw("SMA/CFHT ");
  } else if ((present(temperatureServer,"CFHT") || 
	      present(temperatureServer,"JCMT")) &&
	     (present(humidityServer,"CFHT") ||
	      present(humidityServer,"JCMT")) &&
	     (present(windServer,"CFHT") ||
	      present(windServer,"JCMT")) &&
	     (present(pressureServer,"CFHT") ||
	      present(pressureServer,"JCMT"))) {
    printw("JCMT/CFH ");
  } else
    printw("mixture: ");
}

int computeUTstale(float hours, float margin, float uthours) {
  /* note that hours can be negative if the time is from the previous day(s) */
  int stale;
  
  if (uthours-margin > 0) {
    if ((hours>=0 && hours < uthours)  && hours > (uthours-margin))
      stale = 0;
    else
      stale = 1;
  } else {
    if ((hours>=0 && hours < uthours) || hours > (24+uthours-margin))
      stale = 0;
    else
      stale = 1;
  }
  return(stale);
}

void printCalWheelStatus(int orientation) {
  /*  printw(" %2d  ",calWheelStatus[ant]); */
  switch (orientation) {
  case AMBIENT_IN:
    printw(" Vane");
    break;
  case SKY_IN:
    printw("  Sky");
    break;
  case WAVEPLATE_IN: /* this is not actually used */
    printw("Wavep");
    break;
  case UNKNOWN_IN:
    printw(" ??? ");
    break;
  default:
    printw("Error");
    break;
  }
}

void printCalWheelStatus3Char(int orientation) {
  switch (orientation) {
  case AMBIENT_IN:
    printw("Hot");
    break;
  case SKY_IN:
    printw("Sky");
    break;
  case WAVEPLATE_IN: /* this is not actually used */
    printw("Wav");
    break;
  case UNKNOWN_IN:
    printw("??? ");
    break;
  default:
    printw("Err");
    break;
  }
}

int timeProblems(double error[MAX_NUMBER_ANTENNAS+1]) {
  /* compute for all antennas */
  int i;
  int problem = 0;
  
  for (i=1; i<=numberAntennas; i++) {
    error[i] = timeDifferential(i);
    if (fabs(error[i]) > TIME_DIFFERENTIAL_STANDOUT/3600)
      problem |= 1<<i;
  }
  return(problem);
}

double timeDifferential(int antennaNumber) {
  /* compare the system time on hal9000 to the specified antenna's time
   * in reflective memory */
  int status,rms;
  struct timespec tp;
  struct timeval currentTime;
#ifndef LINUX
  struct timezone timezone;
#endif
  struct tm now;
  double differential, utcHours, hours, fractionalSeconds;
  int rightNow;
  
  clock_gettime(CLOCK_REALTIME,&tp);
  rightNow = tp.tv_sec;
  fractionalSeconds = tp.tv_nsec*0.000000001;
#ifdef LINUX
  status = gettimeofday(&currentTime,NULL);
  gmtime_r(&(currentTime.tv_sec), &now);
#else
  status = gettimeofday(&currentTime, &timezone);
  gmtime_r(&now, &(currentTime.tv_sec));
#endif
  hours = now.tm_hour + now.tm_min/60. + now.tm_sec/3600 + fractionalSeconds/3600.;
  rms = rm_read(antennaNumber, "RM_UTC_HR_D", &utcHours);
  differential = hours-utcHours;
  return(differential);
}

float computeMedianWindspeed(float *medianDirection, int *invalidWinds) {
  /* this actually now computes the average */
#define NTELESCOPES 5
  /* do not use the VLBA antenna in the calculation */
  float speed[NTELESCOPES], direction[NTELESCOPES];
  float windAge;
  int rightNow;
  struct timespec tp;
  float dirSum, windSum, dirAvg, windAvg;
  int totalScopes;
  float median;
  int rms,i;
  long upd[NTELESCOPES];
  long irtfTimestamp,ukirtTimestamp,cfhtTimestamp;
  long uh88Timestamp,subaruTimestamp;
  
  clock_gettime(CLOCK_REALTIME,&tp);
  rightNow = tp.tv_sec;
  *invalidWinds = 0, 
    rms = dsm_read(DSM_WEATHER_HOST,"UKIRT_METEOROLOGY_X", &ukirtWeather, &ukirtTimestamp);
  if (rms != DSM_SUCCESS)
    dsm_error_message(rms,"dsm_read(UKIRT_METEOROLOGY_X)");
  rms = dsm_read(DSM_WEATHER_HOST,"SUBARU_METEOROLOGY_X", &subaruWeather, &subaruTimestamp);
  if (rms != DSM_SUCCESS)
    dsm_error_message(rms,"dsm_read(SUBARU_METEOROLOGY_X)");
  rms = dsm_read(DSM_WEATHER_HOST,"CFHT_METEOROLOGY_X", &cfhtWeather, &cfhtTimestamp);
  if (rms != DSM_SUCCESS)
    dsm_error_message(rms,"dsm_read(CFHT_METEOROLOGY_X)");
  rms = dsm_read(DSM_WEATHER_HOST,"IRTF_METEOROLOGY_X", &irtfWeather, &irtfTimestamp);
  if (rms != DSM_SUCCESS)
    dsm_error_message(rms,"dsm_read(IRTF_METEOROLOGY_X)");
  rms = dsm_read(DSM_WEATHER_HOST,"UH88_METEOROLOGY_X", &uh88Weather, &uh88Timestamp);
  if (rms != DSM_SUCCESS)
    dsm_error_message(rms,"dsm_read(UH88_METEOROLOGY_X)");

  rms = dsm_structure_get_element(&irtfWeather,"WINDSPEED_F", &speed[1]);
  rms = dsm_structure_get_element(&irtfWeather,"WINDDIR_F", &direction[1]);
  upd[1] = irtfTimestamp;
  rms = dsm_structure_get_element(&ukirtWeather,"WINDSPEED_F", &speed[2]);
  rms = dsm_structure_get_element(&ukirtWeather,"WINDDIR_F", &direction[2]);
  upd[2] = ukirtTimestamp;
  rms = dsm_structure_get_element(&cfhtWeather,"WINDSPEED_F", &speed[3]);
  rms = dsm_structure_get_element(&cfhtWeather,"WINDDIR_F", &direction[3]);
  upd[3] = cfhtTimestamp;
  rms = dsm_structure_get_element(&subaruWeather,"WINDSPEED_F", &speed[4]);
  rms = dsm_structure_get_element(&subaruWeather,"WINDDIR_F", &direction[4]);
  upd[4] = subaruTimestamp;
  rms = dsm_structure_get_element(&uh88Weather,"WINDSPEED_F", &speed[0]);
  rms = dsm_structure_get_element(&uh88Weather,"WINDDIR_F", &direction[0]);
  upd[0] = uh88Timestamp;
#if 0
  int swap;
  int index[NTELESCOPES];
  /* do a bubble sort by speed, then pick the one in the middle */
  for (i=0; i<NTELESCOPES; i++)
    index[i] = i;
  for (i=0; i<(NTELESCOPES-1); i++)
    for (j=0; j<(NTELESCOPES-1); j++)
      if (speed[index[j]] > speed[index[j+1]]) {
	swap = index[j];
	index[j] = index[j+1];
	index[j+1] = swap;
      }
#endif
  windSum = 0;
  dirSum = 0;
  for (i=0; i<NTELESCOPES; i++) {
    windAge = rightNow-upd[i]; /* in seconds */
    if (speed[i] < WIND_METER_FROZEN_CUTOFF || windAge>1800 || windAge<-1800) { /* mph, seconds */
      (*invalidWinds) = (*invalidWinds) + 1;
    } else {
      windSum += speed[i];
      dirSum += direction[i];
    }
  }
  totalScopes = NTELESCOPES - *invalidWinds;
  windAvg = windSum/totalScopes;
  dirAvg = dirSum/totalScopes;
  if (*invalidWinds < NTELESCOPES) {
    median = windAvg;
    *medianDirection = dirAvg;
  } else {
    median = 0;
    *medianDirection = 0;
  }
  return(median);
}

float STANDOUT_TAU(float freqHz) {
  float GHz = freqHz*1.0e-9;
  if (GHz < 300)
    return(0.40);
  if (GHz < 600)
    return(0.20);
  return(0.10);
}

int mirrorDoorIsOpen(int m3status) {
  if (m3status==2 || m3status==6)
    return(1);
  else
    return(0);
}

void PrintBad(int st) {
  standout();
  if(st == 5) {
    printw("Mov");
  } else {
    printw("Wac");
  }
  standend();
}

void printNewHotloadPosition(int antenna) {
  int rm_status;
  short busy, moverStatus;

  rm_status = rm_read(antenna, "RM_TUNE6_COMMAND_BUSY_S", &busy);
  if (TUNE6_BUSY_WITH_LINEAR_LOAD == busy) {
    standout();
    printw("mov");
    standend();
    return;
  }
  /* Now the heated load is in front of the unheated load as seen by rx, so
   * it must be checked first */
  rm_status = rm_read(antenna, "RM_HEATEDLOAD_STATUS_S", &moverStatus);
  if(moverStatus != 1 && moverStatus != 3 && ignoreHeatedLoad[antenna] == 0) {
    if(moverStatus == 2 || moverStatus == 4) {
      printw("Hot");
    } else {
      PrintBad(moverStatus);
    }
  } else {
    rm_status = rm_read(antenna, "RM_UNHEATEDLOAD_STATUS_S", &moverStatus);
    if(moverStatus != 1 && moverStatus != 3) {
      if(moverStatus == 2 || moverStatus == 4) {
        printw("Amb");
      } else {
        PrintBad(moverStatus);
      }
    } else {
      rm_status = rm_read(antenna, "RM_WAVEPLATE_STATUS_S", &moverStatus);
      if(moverStatus != 1 && moverStatus != 3) {
        if(moverStatus == 2 || moverStatus == 4) {
          printw("WVP");
        } else {
          PrintBad(moverStatus);
        }
      } else {
        printw("Sky");
      }
    }
  }
}

#if 0
void printNewHotloadPosition(short opticalBreaks[8], int antenna) {
  int rm_status;
  short busy;
  rm_status = rm_read(antenna, "RM_TUNE6_COMMAND_BUSY_S", &busy);
  if (TUNE6_BUSY_WITH_LINEAR_LOAD == busy) {
    standout();
    printw("mov");
    standend();
    return;
  }
  /* the unheated load is in front of the heated load as seen by rx, so
   * it must be checked first */
  if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_IN]==0) {
    if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_SKY]==0)
      printw("Wac");
    else
      printw("Amb");
  } else if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_IN]==0) {
    if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_SKY]==0)
      printw("Wac");
    else
      printw("Hot");
  } else if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_SKY]==0
	     && opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_SKY]==0) {
    short wavePlate;

    rm_read(antenna, "RM_WAVEPLATE_STATUS_S", &wavePlate);
    if (wavePlate == 4)
      printw("WVP");
    else
      printw("Sky");
  } else
    printw(" ? ");
}

void printNewHotloadPosition1rx(short opticalBreaks[8], int antenna) {
  int rm_status;
  short busy;
  rm_status = rm_read(antenna, "RM_TUNE6_COMMAND_BUSY_S", &busy);
  if (TUNE6_BUSY_WITH_LINEAR_LOAD == busy) {
    standout();
    printw("moving  ");
    standend();
    return;
  }
  /* the unheated load is in front of the heated load as seen by rx, so
   * it must be checked first */
  if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_IN]==0) {
    if (opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_SKY]==0) {
      printw(" Wacko  ");
    } else {
      if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_SKY]==1
	  && opticalBreaks[OPTICAL_BREAK_WAVEPLATE_IN]==0)
	printw("AmbWave ");
      else
	printw("Ambient ");
    }
  } else if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_IN]==0) {
    if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_SKY]==0) {
      printw(" Wacko  ");
    } else {
      if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_SKY]==1 
	  && opticalBreaks[OPTICAL_BREAK_WAVEPLATE_IN]==0)
	printw("HotWave ");
      else
	printw("Heated  ");
    }
  } else if (opticalBreaks[OPTICAL_BREAK_HEATED_LOAD_SKY]==0
	     && opticalBreaks[OPTICAL_BREAK_UNHEATED_LOAD_SKY]==0) {
    if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_SKY]==1 
	&& opticalBreaks[OPTICAL_BREAK_WAVEPLATE_IN]==0)
      printw("SkyWave ");
    else
      printw("  Sky   ");
  } else {
    if (opticalBreaks[OPTICAL_BREAK_WAVEPLATE_SKY]==1 
	&& opticalBreaks[OPTICAL_BREAK_WAVEPLATE_IN]==0)
      printw(" ?Wave  ");
    else
      printw("   ?    ");
  }
}
#endif
