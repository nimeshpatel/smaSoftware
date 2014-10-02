#define PRINT_DSM_ERRORS 0 /* set this to 1 to debug DSM problems
			    * with the use of call_dsm_read() */
#define N_LINES 20
#define MIN_LINES_FOR_BOTTOM_2OP 25
#define MIN_LINES_FOR_BOTTOM_OPMSG 26
#define DEBUG 0
#include <stdio.h>
#include <sys/utsname.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#ifdef LINUX
  #include <sys/stat.h>
  #include <unistd.h>
#endif
#include <sys/file.h>
#include <termio.h>
#include <time.h>
#include <curses.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <termios.h>
#include "smapopt.h"
#include "commonLib.h"
#include "rm.h"
#include "dsm.h"
#include "monitor.h"
#include "allanVariance.h"
#include "weather.h"
#include "receiverMonitor.h"
#include "receiverMonitorHighFreq.h"
#include "c1DC.h"
#include "opticsMonitor.h"
#include "antMonitor.h"
#include "iFLOMonitorPage2.h"
#include "rscanpage.h"
#include "deiced.h"
#include "coherence.h"
#include "upspage.h"
#include "stateCounts.h"
/*#include "bdc.h"*/

enum {
/* to add a page, insert a new name at the top and decrement the initializor */
  FLAGGING_DISPLAY = -44, YIG_DISPLAY, SWARM_DISPLAY, BDC812_DISPLAY,GENSET_DISPLAY, EFF_DISPLAY, GPS_DISPLAY,
HANGAR_DISPLAY,USERS_DISPLAY,MRG_DISPLAY,DRIVERS_DISPLAY,             /* -34 */
IPCENSUS_DISPLAY,WEATHER2_DISPLAY,SEEING_DISPLAY,UPTIMES_DISPLAY,     /* -30 */
TILT_DISPLAY,MAP_DISPLAY,RSCAN_DISPLAY,OPERATOR_DISPLAY,CHOPPER_DISPLAY,/*-25*/
AIR_DISPLAY,WEATHER_DISPLAY,UPS_DISPLAY,POLAR_DISPLAY,COHERENCE_DISPLAY,/*-20*/
MESSAGE_DISPLAY,PROJECT_DISPLAY,C1DC_DISPLAY,PMODEL_DISPLAY,          /* -16 */
DEWAR_DISPLAY,OPTICS_DISPLAY,DEICE_DISPLAY,CORRSUM_DISPLAY,AC_DISPLAY, /*-11 */
ANT_IFLO_DISPLAY,DEAD_DISPLAY,SMAINIT_DISPLAY,RX_DISPLAY,HELP_DISPLAY, /* -6 */
CORRELATOR_DISPLAY, /*-5*/
SMASH_DISPLAY,STDERR_DISPLAY,IFLO_DISPLAY,DDS_DISPLAY,ARRAY_DISPLAY,   /* 0 */
ANTENNAPAGE_DISPLAY /* 1  (going up 10) */
};

#define TRUE 1
#define FALSE 0
#define TIMEOUT 3600

#include "dDSCursesMonitor.h"
#include "commandMonitor.h"

int colorFlag = FALSE;
int bottom2op = FALSE;
int bottomOpMsg = FALSE;
int projectLockout;
int defaultTiltFlag[20];
time_t intruderTimestamp;
void readIFServerInfo(void);
void handlerForSIGINT(int signum);
void deiceWarningPage(int count, int on[NUMANTS]);
void setAllPageModesToZero(void);
void goto80width(void);
void goto80WidthAndExit(int value);
void goto132width(void);
void allow132width(void);
enum {
  NOT_DRIVERS_IPCENSUS_SMAINIT=0, NOT_IPCENSUS
};
void setMostPageModesToZero(int type);
dsm_structure phasemon_data;
dsm_structure upsStructure[MAX_NUMBER_ANTENNAS+1];
//dsm_structure upsStructureColossus[NUMBER_OF_UPSs_ON_COLOSSUS];
dsm_structure upsStructureObscon[NUMBER_OF_UPSs_ON_OBSCON];
dsm_structure upsStructureObsconX;
dsm_structure microdustStructure;
dsm_structure roachStructure;
dsm_structure bdc812Structure;
dsm_structure dDSStatusStructure;
//dsm_structure smaWeather, sma5Weather, jcmtWeather, irtfWeather, ukirtWeather, cfhtWeather;
dsm_structure smaWeather, AntWeather, jcmtWeather, irtfWeather, ukirtWeather, cfhtWeather;
dsm_structure Ant5Temps;
dsm_structure csoWeather, keckWeather, vlbaWeather ,subaruWeather, uh88Weather;
dsm_structure crateStatusStructure[13];
dsm_structure mRGControl, mRGStatus;
void screen(char *source, double *lst_disp,
	    double *utc_disp, double *tjd_disp,
	    double *ra_disp, double *dec_disp,
	    double *ra_cat_disp, double *dec_cat_disp,
	    double *az_disp, double *el_disp, int *icount,
	    double *azoff, double *eloff, 
	    double *az_actual_corrected, double *el_actual_disp, double *tiltx,
	    double *tilty, float *pressure,
	    float *temperature, float *humidity,
	    double *az_error, double *el_error,
	    double *scan_unit, char *messg, 
	    double *focus_counts, double *subx_counts, double *suby_counts,
	    double *subtilt_counts, double *total_power_disp, 
	    double *syncdet_disp,
	    int *integration, float *windspeed,
	    float *winddirection, float *refraction, float *pmdaz, 
	    float *pmdel,
	    double *smoothed_tracking_error, double *tsys, int *ant, 
	    float *plsAzRdg,
	    int *radio_flag, short *padid,double *planetdistance,
	    double *Az_cmd, 
	    double *El_cmd);

double sunDistance(double az1,double el1,double az2,double el2);

int user, lastUser;
int baselineInfo = UNPROJECTED;
double radian=0.01745329;
struct termio tio, tin;
int beepFlag=1;
int antsAvailable[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,0,0};
int deadAntennas[MAX_NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,1,1};
int ignoreHeatedLoad[MAX_NUMBER_ANTENNAS+1] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1};
int chopperUnits = 0;
int airHandlerUnits = 0;
int weatherUnits = METRIC_WEATHER_UNITS;
int deiceCheckCount;
int lastDeiceWarn = 0;
void openDsmUPSStructures(void);
void openDsmMeteorologyStructures(void);
int lastIpointWarn = 0;
int iFLOUnits = IFLO_UNITS_VOLTS;
int opMessagePending = FALSE;
int opMsgHash = -1;
int deiceWattage;

/* the following will be read from the antIFServerInfo.txt files */
float yigTuneCurveA = 2.5; /* at present, these are all the same */
float yigTuneCurveB = 13.7328; /* at present, these are all the same */

float cont2det2target[MAX_NUMBER_ANTENNAS+1] = {0,6.44,6.74,5.48,7.57,6.43,6.14,6.09,6.46,0,0};
float cont1det2target[MAX_NUMBER_ANTENNAS+1] = {0,5.76,5.71,5.25,5.37,6.93,5.50,6.64,5.89,0,0};
float mrgRfPowerTarget[MAX_NUMBER_ANTENNAS+1] = {0,5.76,5.71,5.25,5.37,6.93,5.50,6.64,5.89,4.40,0};
float power200MHzTarget[MAX_NUMBER_ANTENNAS+1] = {0,5.76,5.71,5.25,5.37,6.93,5.50,6.64,5.89,2.97,0};
float power200MHzTarget2[MAX_NUMBER_ANTENNAS+1] = {0,5.76,5.71,5.25,5.37,6.93,5.50,6.64,5.89,0,0};
float c1dcTargetIF1[MAX_NUMBER_ANTENNAS+1] = {0,6.45,8.1,6.72,7.30,7.82,7.21,6.85,7.29,0,0};
float c1dcTargetIF2[MAX_NUMBER_ANTENNAS+1] = {0,6.47,7.50,6.68,7.38,7.40,8.00,7.20,7.40,0,0};

int smainitMode = 0; /* Set to 1 when displaying smainit pages */
int corrSumMode = 0;
int ipcensusMode = 0; 
int driversMode = 0; 
int chopperMode = 0;
int airHandlerMode = 0;
int weatherMode = 0;
int antMode = 0;
int opticsMode = 0;
int opMsgMode = 0;
int upsMode = 0;
int antIFLOMode = 0;
int corrMode = 0;
int aCMode = 0;
int rxMode = 0;
int dDSMode = 0;
int yIGMode = 0;
int commandMode = 0;
int searchMode = 0;
int messageMode = 0;
int errorMode = 0; 
/* remember to add any new entries to the functions SetAllPageModesToZero() 
 * and SetMostPageModesToZero() */

int smainitPage = 0; /* Selects computer within smainit pages */
int corrSumPage = 0;
int ipcensusPage = 0;
int driversPage = 0;
int antPage = 0;
int opPage = 0;
int antIFLOPage = 0;
int statesPage = FALSE;
int corrPage = 0;
int aCPage = 0;
int rxPage = 0;
int opMsgPage = 0;
int upsPage = 0;
int pageOffset = 0;
char searchString[100];
int ignoreMessages = FALSE;
int ignoreOpMessages = FALSE;
int opBeepCount = 3;
int forceActiveAntennas[SMAPOPT_MAX_ANTENNAS + 1];

extern void flagging(int count);
extern void solveGains(int count);
extern void opTel(int count, int *rm_list) ;
extern void help(int count);
extern void correlatorDisplay(int count, int *crateOffset);
extern void iFLODisplay(int count);
extern int arrayDisplay(int icount, int ignoreIntruders, int antlist[RM_ARRAY_SIZE]);
extern void timeStamp(void);
extern void messagePage(int count, int kind);
extern int projectpage(int icount);
extern void hangarPage(int count);
extern void intruderPage(int count);
extern void laserDisplay(int icount);
extern void gpsPage(int count);
extern void antPage2(int count, int *antlist, int antNo, int antName);
extern int users(int count);
extern void mRG(int count);
extern void driversLoaded(int count, int page);
extern void ipcensus(int count, int page);
extern void weatherPage2(int count, int *rm_list);
extern void genset(int count);
extern void bdc812(int count);
extern void swarmPage(int count);
extern void seeing(int count, int *rm_list);
extern void uptimes(int count);
extern void tiltpage(int count, int *rm_list);
extern void arraymap(int count, int *rm_list);
extern int opMsg(int count, int bottom);
extern void chopperPage(int count, int *rm_list);
extern void airHandler(int count, int *rm_list); 
extern void weather(int count, int *rm_list);
extern void upspage2(int count, int *rm_list);
extern void polar(int *count);
extern int pmodelspage(int icount,int *antlist);
extern void dewarpage(int count, int *rm_list);
extern void deicemon(int count, int *antlist);
extern void correlatorSummary(int count, int rx);
extern void aCDisplay(int count);
extern void iFLODisplayPage3(int count, int *antlist);
extern void iFLODisplayPage4(int count, int *antlist);
extern void listDead(int count);
extern void smainitMonitor(int count, int page);
extern void networkPage(int count, int *rm_list);
extern void lomotorpage(int count, int *rm_list);

/* on the wide 'a' page, this tells which window to start with in the
 * upper right corner */
int upperRightWindow = UR_MESSAGES;

void usage(char *a);
void usage(char *a) {
  printf("Usage: %s -H -h --help\n",a);
  /*
    printf("Usage: %s -i -z -Z -H --help\n");
    printf("      -i  ignore messages from 2op\n");
    printf("      -z  ignore messages from 2op, intruders and Op\n");
    printf("      -Z  ignore messages from intruders and Op\n");
*/
  printf(" -a or --antenna display values for antenna(s) even if 'dead'\n");
  printf("           -H  show the hangar display page only\n");
  printf(" -h or --help  show this message\n");
  exit(0);
}

int computeDeIceWattage(int *deiceOn) {
  int totPwr;
  int i,rm_status;
  static int peakPwr[7] = {0, 5470, 10939, 15553, 20167, 24781, 29394};
  static char powerCmdv[] = "RM_DEICE_POWER_CMD_V5_S";
  short powerCmd[5];      

  totPwr = 0;
  /* Limit list to requested antennas with active deiced */
  for(i=1; i<=8; i++) {
#if DEBUG
    fprintf(stderr,"About to rm_read(%s)\n",powerCmdv);
#endif
    rm_status = rm_read(i, powerCmdv, powerCmd);
#if DEBUG
    fprintf(stderr,"finished\n");
#endif
    if (rm_status == RM_SUCCESS) {
      if (powerCmd[MAIN_P] >= 0 && powerCmd[MAIN_P] < 100) {
	totPwr += peakPwr[powerCmd[MAIN_P]];
	if (peakPwr[powerCmd[MAIN_P]] > 0) {
	  deiceOn[i] = 1;
	} else {
	  deiceOn[i] = 0;
	}
      }
    }
  }
  return(totPwr);
}

void CheckIgnoreHeatedLoad(void) {
  char fname[64];
  int i;

  for(i = 1; i <= NUMANTS; i++) {
    sprintf(fname,"/otherInstances/acc/%d/configFiles/ignoreHeatedLoad",i);
    if(access(fname, F_OK) == 0) {
      ignoreHeatedLoad[i] = 1;
    }
  }
}

main(int argc, char *argv[])
{
  int i;
  time_t intruderTime;
  int esmaMode = -1; /* -1 means not yet specified by the user */
  int statsRx = 1;
  int dDSRx = 0;
  int yIGRx = 0;
  int ant=ARRAY_DISPLAY; /* start up on the 'a' page */
  int ignoreIntruders = FALSE;
  short intruder = 0;
  int cycle = FALSE;
  int delay = 1;
#ifdef LINUX
  int spinCount = 0;
#endif
  char *outputFile = NULL;
  struct utsname unamebuf;
  int antlist[RM_ARRAY_SIZE], rms;
  struct sigaction action, old_action; 
  int sigactionInt;
  int icount=0;
  int firstUpdate = TRUE;
  int  rc; 
  int lastIntruder = FALSE;
  int startDay;
  int message = 0;
  int deiceWarn = 0;
  int ipointWarn = 0;
  int deiceOn[NUMANTS+1];
  int ignoreFlag;
  int hangarDefault = FALSE;
  RECEIVER_FLAGS receiverFlags;
  ALLAN_VARIANCE_FLAGS allanVarianceFlags;
  OPTICS_FLAGS opticsFlags;
  C1DC_FLAGS c1DCFlags;
  IFLO_FLAGS ifloFlags;
  ANTENNA_FLAGS antennaFlags;
  struct stat messageStat, oldMessageStat;
  FILE *deadList;

  /* The following variables are for the default Tilt flag read from
     the pointing model files */
   FILE *fpMountModel;
   int itilt;
   char mountModelFile[100],line[BUFSIZ];
   char pointingParameter[20],opticalValue[20],radioValue[20];

  smapoptContext optCon;
  struct smapoptOption options[] = {
    {"cycle", 'c', SMAPOPT_ARG_NONE, &cycle, 0, "Cycle through all pages in rapid succession"},
    {"delay", 'd', SMAPOPT_ARG_INT, &delay, 0, "Delay between screen updates in seconds"},
    {"file", 'f', SMAPOPT_ARG_STRING, &outputFile, 0, "File to which output should be sent"},
    {"operator", 'o', SMAPOPT_ARG_NONE, &ignoreFlag, 0, "Ignore the operator messages"},
    {"rgb", 'r', SMAPOPT_ARG_NONE, &colorFlag, 0, "rgb color"},
    {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, forceActiveAntennas, 0,
		"antenna(s) to force active"},
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 } 
  };  
  
  /* secret code written by Todd */
  for (i=1; i<argc; i++) {
    if (strstr(argv[i],"-Z") != NULL) {
      ignoreIntruders = TRUE;
      ignoreOpMessages = TRUE;
      continue;
    }
    if (strstr(argv[i],"-o") != NULL) {
      ignoreOpMessages = TRUE;
      continue;
    }
    if (strstr(argv[i],"-i") != NULL) {
      ignoreMessages = TRUE;
      continue;
    }
    if (strstr(argv[i],"-z") != NULL) {
      ignoreIntruders = TRUE;
      ignoreMessages = TRUE;
      ignoreOpMessages = TRUE;
      continue;
    }
    if (strstr(argv[i],"-H") != NULL) {
      ignoreIntruders = TRUE;
      ignoreMessages = TRUE;
      ignoreOpMessages = TRUE;
      hangarDefault = TRUE;
      ant = HANGAR_DISPLAY;
      continue;
    }
    if (strstr(argv[i],"help") != NULL) {
      usage(argv[0]);
    }
    if (strstr(argv[i],"-h") != NULL) {
      usage(argv[0]);
    }
  }
  if (ignoreIntruders == FALSE) {
    optCon = smapoptGetContext("smapopttest", argc, argv, options, 0);
  
    if ((rc = smapoptGetNextOpt(optCon)) < -1) {
      fprintf(stderr, "smapopttest: bad argument %s: %s\n", 
	      smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS), 
	      smapoptStrerror(rc));
      return 2;
    }
  }

  if (getenv("PROJECT_LOCKOUT") == NULL)
    projectLockout = FALSE;
  else {
    projectLockout = TRUE;
    putenv("PROJECT_LOCKOUT=0");
  }
  if (cycle) {
    ignoreOpMessages = TRUE;
    if (processPresent("excl_monitor")) {
      fprintf(stderr, "A logging version of this program is already running.\n");
      exit(-1);
    }
    if (outputFile == NULL) {
      time_t curTime;
      struct tm *gmt;

      outputFile = malloc(81);
      if (outputFile == NULL) {
	perror("malloc on outputFile");
	exit(-1);
      }
      curTime = time((long *)0);
      gmt = gmtime(&curTime);
      strftime(outputFile, 80, "/data/engineering/monitorLogs/%d%b%y", gmt);
      startDay = gmt->tm_yday;
    }
    freopen(outputFile, "a", stdout);
    if (stdout == NULL) {
      perror("reopening stdout");
      exit(-1);
    }
  }
  if (delay < 1)
    delay = 1;
  /* signal handler for control C */
  action.sa_flags=0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = handlerForSIGINT;
  sigactionInt = sigaction(SIGINT,&action, &old_action);
  
  uname(&unamebuf);
#ifndef LINUX
  if(strcmp(unamebuf.nodename,"hal9000")) {
    printf("This program will run only on hal9000. Bye.\n");
    exit(-1);
  }
#endif
  /* initializing ref. mem. */
  rms=rm_open(antlist);
  if(rms != RM_SUCCESS) {
    rm_error_message(rms,"rm_open()");
    exit(1);
  }
#if DEBUG
  fprintf(stderr,"Finished opening RM\n");
#endif
  i = 0;
  while (antlist[i] != RM_ANT_LIST_END) {
    antsAvailable[antlist[i]] = 1;
    deadAntennas[antlist[i]] = 0;
    i++;
  }
 
  deadList= fopen("/global/configFiles/deadAntennas", "r");
  if (deadList != NULL) {
    int ii, nDead, dead[MAX_NUMBER_ANTENNAS+1];
    
    nDead = fscanf(deadList, "%d %d %d %d %d %d %d %d %d %d",
		   &dead[1], &dead[2], &dead[3], &dead[4],
		   &dead[5], &dead[6], &dead[7], &dead[8], &dead[9],&dead[10]);
    fclose(deadList);
    if (nDead > 0) {
      for (ii = 1; ii <= nDead; ii++) {
	if(!forceActiveAntennas[dead[ii]])
	  deadAntennas[dead[ii]] = TRUE;
      }
    }
    for (ii = 1; ii <= 8; ii++)
      if (deadAntennas[ii])
	antsAvailable[ii] = FALSE;
  }

  defaultTiltFlag[0]=0;
   for(itilt=1;itilt<=8;itilt++) {
   defaultTiltFlag[itilt]=0;
   sprintf(mountModelFile,"/otherInstances/acc/%d/configFiles/pointingModel",itilt);
   fpMountModel=fopen(mountModelFile,"r");
      if(fpMountModel!=NULL) {
         while(fgets(line,sizeof(line),fpMountModel) != NULL) {
            line[strlen(line)-1]='\0';
                if(line[0]!='#') {
          sscanf(line,"%s %s %s", pointingParameter, opticalValue, radioValue);
                     if(!strcmp(pointingParameter,"TiltFlag")) {
                     defaultTiltFlag[itilt]=(int)atof(radioValue);
                     }
                }
         }
      }
   fclose(fpMountModel);
   }

  CheckIgnoreHeatedLoad();
  rms = dsm_open();
  if(rms != DSM_SUCCESS) {
    dsm_error_message(rms, "dsm_open");
    exit(-1);
  }
#if DEBUG
  fprintf(stderr,"Finished opening DSM\n");
#endif

	openDsmMeteorologyStructures();
	openDsmUPSStructures();

	rms = dsm_structure_init(&dDSStatusStructure,"DDS_TO_HAL_X");
	if (rms != DSM_SUCCESS) 
		dsm_error_message(rms,"dsm_structure_init(DDS_TO_HAL_X)");
	rms = dsm_structure_init(&bdc812Structure,"BDC_X");
	if (rms != DSM_SUCCESS) 
		dsm_error_message(rms,"dsm_structure_init(BDC_X)");
	rms = dsm_structure_init(&roachStructure,"ROACH2_TEMPS_X");
	if (rms != DSM_SUCCESS) 
		dsm_error_message(rms,"dsm_structure_init(ROACH2_TEMPS_X)");
	rms = dsm_structure_init(&microdustStructure,"DSM_MICRODUST_X");
	if (rms != DSM_SUCCESS) 
		dsm_error_message(rms,"dsm_structure_init(DSM_MICRODUST_X)");
	rms = dsm_structure_init(&Ant5Temps,"DSM_ANTENNA5_TEMPS_X");
	if (rms != DSM_SUCCESS)
		dsm_error_message(rms,"dsm_structure_init(DSM_ANTENNA5_TEMPS_X)");    
	rms = dsm_structure_init(&mRGControl,"MRG_CONTROL_X");
	if (rms != DSM_SUCCESS)
		dsm_error_message(rms,"dsm_structure_init(MRG_CONTROL_X)");
	rms = dsm_structure_init(&phasemon_data,"PHASEMON_DATA_X");
	if (rms != DSM_SUCCESS)
		dsm_error_message(rms,"dsm_structure_init(PHASEMON_DATA_X)");
	rms = dsm_structure_init(&mRGStatus,"MRG_STATUS_X");
	if (rms != DSM_SUCCESS)
		dsm_error_message(rms,"dsm_structure_init(MRG_STATUS_X)");
	for (i = 1; i < 13; i++) 
	{
		rms = dsm_structure_init(&crateStatusStructure[i], "CRATE_TO_HAL_X");
		if (rms != DSM_SUCCESS)
	  		dsm_error_message(rms,"dsm_structure_init(CRATE_TO_HAL_X)");
	}

  /*
   * This is to get the user input as a single unbuffered char and
   * zero-wait
   */
  
  ioctl(0, TCGETA, &tio);
  
  tin = tio;
  tin.c_lflag &= ~ECHO;
  tin.c_lflag &= ~ICANON;
  
  tin.c_cc[VMIN] = 0;
  tin.c_cc[VTIME] = 0;
  
  ioctl(0, TCSETA, &tin);

  stat(MESSAGES_LOG, &oldMessageStat);

#if DEBUG
  fprintf(stderr,"1. Finished stat() on messages file\n");
#endif
  /* starting infinite loop */
  deiceCheckCount = 0;
  /* begin while loop every 1 second */
  while (1) {
    time_t lastKeystrokeTime, curTime;

#ifndef LINUX
    yield();
#endif
#if DEBUG
    fprintf(stderr,"Finished yield()\n");
#endif
    ioctl(0, TCSETA,&tin);
    
    icount++;
    /* Check for operator messages */
    if (!opMessagePending) {
      int tempOpMsgHash = 0;
      int ii, s;
#define MAX_NUMBER_OP_MSG 22
      short severity[MAX_NUMBER_OP_MSG], priority[MAX_NUMBER_OP_MSG];
      long unixTime[MAX_NUMBER_OP_MSG];
      char text[MAX_NUMBER_OP_MSG][60], pPC[MAX_NUMBER_OP_MSG][20];
      time_t timestamp;

      s = dsm_read("colossus",
		   "DSM_OPMSG_PPC_V22_C20", 
		   (char *)pPC,
		   &timestamp);
#if DEBUG
      fprintf(stderr,"Finished dsm_read of OPMSG_PPC_V22_C20\n");
#endif
      for (i = 0; i < MAX_NUMBER_OP_MSG; i++)
	for (ii = 0; ii < 20; ii++)
	  tempOpMsgHash += (int)pPC[i][ii];
      s = dsm_read("colossus",
		   "DSM_OPMSG_TEXT_V22_C60", 
		   (char *)text,
		   &timestamp);
#if DEBUG
      fprintf(stderr,"Finished dsm_read of OPMSG_TEXT_V22_C60\n");
#endif
      for (i = 0; i < MAX_NUMBER_OP_MSG; i++)
	for (ii = 0; ii < 20; ii++)
	  tempOpMsgHash += (int)text[i][ii];
      s = dsm_read("colossus",
		   "DSM_OPMSG_SEVERITY_V22_S", 
		   (char *)severity,
		   &timestamp);
#if DEBUG
      fprintf(stderr,"Finished dsm_read of OPMSG_SEVERITY_V22_S\n");
#endif
      for (i = 0; i < MAX_NUMBER_OP_MSG; i++)
	tempOpMsgHash += (int)severity[i];
      s = dsm_read("colossus",
		   "DSM_OPMSG_PRIORITY_V22_S", 
		   (char *)priority,
		   &timestamp);
#if DEBUG
      fprintf(stderr,"Finished dsm_read of OPMSG_PRIORITY_V22_S\n");
#endif
      for (i = 0; i < MAX_NUMBER_OP_MSG; i++)
	tempOpMsgHash += (int)priority[i];
      s = dsm_read("colossus",
		   "DSM_OPMSG_TIME_V22_L", 
	       (char *)unixTime,
		   &timestamp);
      for (i = 0; i < MAX_NUMBER_OP_MSG; i++)
	tempOpMsgHash += (int)unixTime[i];
      if (tempOpMsgHash != opMsgHash) {
	if ((opMsgHash != -1) && (unixTime[0] != 0))
	  opMessagePending = TRUE;
	opMsgHash = tempOpMsgHash;
      }
    }
    if (((!ignoreMessages) && (!cycle)) || (bottom2op)) {
      stat(MESSAGES_LOG, &messageStat);
#if DEBUG
      fprintf(stderr,"2. Finished stat() on messages file\n");
#endif
      if (messageStat.st_mtime != oldMessageStat.st_mtime) {
	if (!messageMode) {
	  icount = 0;
	  message = 1;
	} else
	  oldMessageStat.st_mtime = messageStat.st_mtime;
      }
    }
    lastUser = user;
    if (!cycle) {
      int s;
      time_t timestamp;
      if (deiceCheckCount == 0) {
	deiceWarn = 0;
#if DEBUG
	fprintf(stderr,"about to call computeDeIceWattage()\n");
#endif
	deiceWattage = computeDeIceWattage(deiceOn);
#if DEBUG
	fprintf(stderr,"Finished computeDeIceWattage()\n");
#endif
	if (deiceWattage > 0) {
	  icount = 0;
	  deiceWarn = 1;
	}
      }
      deiceCheckCount++;
      if (deiceCheckCount > 900) {/* warning period in seconds */
	deiceCheckCount = 0;
      }
      if (deiceWarn == 1) {
#define DEICE_WARN_SECONDS 6
	if (lastDeiceWarn == 0) {
	  icount = 0;
	} 
	if (lastDeiceWarn++ >= DEICE_WARN_SECONDS) {
	  lastDeiceWarn = 0;
	  deiceWarn = 0;
	  icount = 0;
	}
      }
      if (ipointWarn == 1) {
#define IPOINT_WARN_SECONDS 8
	if (lastIpointWarn == 0) {
	  icount = 0;
	} 
	if (lastIpointWarn++ >= IPOINT_WARN_SECONDS) {
	  lastIpointWarn = 0;
	  ipointWarn = 0;
	  icount = 0;
	}
      }
      s = dsm_read("colossus", "DSM_INTRUDER_STATUS_S", &intruder,
		   &timestamp);
      /* because dsm_intruder_status_s is written to at the startup
       * time of the intruder daemon, its timestamp cannot be used
       * as the time of the last intruder.  We must keep track of that
       * separately. */
      s = dsm_read("colossus", "DSM_LAST_INTRUDER_TIME_L", &intruderTime,
		   &intruderTimestamp);
#if DEBUG
      fprintf(stderr,"Finished dsm_read of intruder status\n");
#endif
      if (ignoreIntruders) {
	intruder = FALSE;
      }
      if (intruder == 1) {
	if (!lastIntruder) {
	  icount = 0;
	  lastIntruder = TRUE;
	}
      } else if (lastIntruder) {
	icount = 0;
	lastIntruder = FALSE;
      }
#if DEBUG
      fprintf(stderr, "Calling getchar()\n");
#endif
      user = getchar();
#if DEBUG
      fprintf(stderr, "Returned from getchar()\n");
#endif
      if (user != lastUser) {
	lastKeystrokeTime = time((long *)0);
      } else {
	curTime = time((long *)0);
#ifndef LINUX
	if (((int)curTime - (int)lastKeystrokeTime) > TIMEOUT) {
	  initscr();
#ifdef LINUX
	  clear();
#endif
 	  move(12, 15);
	  printw("There have been no page changes in %d seconds - exiting.", TIMEOUT);
	  move(14, 15);
	  printw("(This doesn't happen if you run the monitor on obscon.)");
	  move(23,0);
	  refresh();
	  user = (int)'q';
	}
#endif
      }
    }
    else {
      user = 0;
    }
#if DEBUG
      fprintf(stderr,"approaching switch(user)\n");
#endif
    switch (user) {
#if 0 
      char key;
#endif
    case 'q':
      move(LINES-1,0);
      printw("Bye.\n");
      refresh();
      ioctl(0, TCSETA, &tio);
      goto80width();
      printf("\n");
      exit(0);
      break;
    case '0':
      if (smainitMode) {
	smainitPage = 0;
	icount = 1;
	setAllPageModesToZero();
	smainitMode = 1;
      } else if (ipcensusMode) {
	ipcensusPage = 0;
	icount = 1;
	setAllPageModesToZero();
	ipcensusMode = 1;
      } else if (driversMode) {
	driversPage = 0;
	icount = 1;
	setAllPageModesToZero();
	driversMode = 1;
      } else if (numberAntennas>8) {
	ant=10;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      break;
    case '1':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = airHandlerMode = 0;
      if (smainitMode) {
	smainitPage = 1;
	icount = 1;
      } else if (opMsgMode) {
        opMsgPage = 1;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 1;
	icount = 1;
      } else if (driversMode) {
        driversPage = 1;
	icount = 1;
      } else {
	ant = 1;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '2':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 2;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 2;
	icount = 1;
      } else if (driversMode) {
        driversPage = 2;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 2;
	icount = 1;
      } else {
	ant = 2;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '3':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 3;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 3;
	icount = 1;
      } else if (driversMode) {
        driversPage = 3;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 3;
	icount = 1;
      } else {
	ant = 3;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '4':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 4;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 4;
	icount = 1;
      } else if (driversMode) {
        driversPage = 4;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 4;
	icount = 1;
      } else {
	ant = 4;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '5':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 5;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 5;
	icount = 1;
      } else if (driversMode) {
        driversPage = 5;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 5;
	icount = 1;
      } else {
	ant=5;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '6':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 6;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 6;
	icount = 1;
      } else if (driversMode) {
        driversPage = 6;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 6;
	icount = 1;
      } else {
	ant=6;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '7':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 7;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 7;
	icount = 1;
      } else if (driversMode) {
        driversPage = 7;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 7;
	icount = 1;
      } else {
	ant=7;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '8':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 8;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 8;
	icount = 1;
      } else if (driversMode) {
        driversPage = 8;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 8;
	icount = 1;
      } else {
	ant=8;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case '9':
      commandMode = messageMode = errorMode = pageOffset = chopperMode = weatherMode = 0;
      if (smainitMode) {
	smainitPage = 25;
	icount = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 27;
	icount = 1;
      } else if (driversMode) {
        driversPage = 27;
	icount = 1;
      } else if (opMsgMode) {
	opMsgPage = 25;
	icount = 1;
      } else if (numberAntennas>8) {
	ant=9;
	icount=0;
	if (!antMode) {
	  antMode = 1;
	}
      }
      user = -1;
      break;
    case 'F':
      setMostPageModesToZero(NOT_DRIVERS_IPCENSUS_SMAINIT);
      ant = FLAGGING_DISPLAY;
      icount = 1;
      break;
    case 'j':
      setMostPageModesToZero(NOT_DRIVERS_IPCENSUS_SMAINIT);
      ant = GENSET_DISPLAY;
      icount = 1;
      break;
    case '*':
      setMostPageModesToZero(NOT_DRIVERS_IPCENSUS_SMAINIT);
      ant = EFF_DISPLAY;
      icount = 1;
      break;
    case 'k':
      setMostPageModesToZero(NOT_DRIVERS_IPCENSUS_SMAINIT);
      if (smainitMode) {
	smainitPage = 10;
      } else if (ipcensusMode) {
        ipcensusPage = 10;
      } else if (driversMode) {
        driversPage = 10;
      } else {
	ant = COHERENCE_DISPLAY;
      }
      icount=1;
      break;
    case 'd':
      setMostPageModesToZero(NOT_DRIVERS_IPCENSUS_SMAINIT);
      if (smainitMode) {
	smainitPage = 10;
      } else if (ipcensusMode) {
        ipcensusPage = 10;
      } else if (driversMode) {
        driversPage = 10;
      } else {
	ant = DDS_DISPLAY;
	dDSMode = 1;
      }
      icount=1;
      break;
    case 'D':
      setAllPageModesToZero();
      icount=0;
      if (ipcensusMode) {
        ipcensusPage = 13; /* second page for newDds */
	icount = 1;
      } else {
        ant = DEICE_DISPLAY;
      }
      break;
    case 'i':
      setAllPageModesToZero();
      icount=1;
      ant = IFLO_DISPLAY;
      break;
    case 'B':
      setAllPageModesToZero();
      icount=1;
      ant = BDC812_DISPLAY;
      break;
    case 'I':
      setAllPageModesToZero();
      icount=1;
      antIFLOMode = 1;
      ant = ANT_IFLO_DISPLAY;
      break;
    case 'e':
      setAllPageModesToZero();
      icount = 1;
      errorMode = 1;
      ant = STDERR_DISPLAY;
      break;
    case 'l':
      if (dDSMode) {
	switch (baselineInfo) {
	case MAS:
	  baselineInfo = MAS_SORTED_BY_LENGTH;
	  break;
	case PROJECTED:
	  baselineInfo = PROJECTED_SORTED_BY_LENGTH;
	  break;
	case UNPROJECTED:
	  baselineInfo = UNPROJECTED_SORTED_BY_LENGTH;
	  break;
	}
      } else {
	setAllPageModesToZero();
	commandMode = 1;
	icount = 1;
	ant = SMASH_DISPLAY;
      }
      break;
    case 'z':
      if (ignoreIntruders) {
	ignoreIntruders = FALSE;
	ignoreMessages = FALSE;
      } else {
	ignoreIntruders = TRUE;
	ignoreMessages = TRUE;
      }
      break;
    case 'Z':
      if (ignoreIntruders) {
	ignoreIntruders = FALSE;
      } else {
	ignoreIntruders = TRUE;
      }
      break;
    case 'C':
      icount = 1;
      statesPage = FALSE;
      if (smainitMode) {
	smainitPage = 9;
      } else if (ipcensusMode) {
        ipcensusPage = 9;
      } else if (driversMode) {
        driversPage = 9;
      } else {
	setAllPageModesToZero();
	corrMode = 1;
	corrPage = 0;
	ant = CORRELATOR_DISPLAY;
      }
      break;
    case 'A':
      if (smainitMode) {
	smainitPage = 26;
	icount = 1;
	user = -1;
      } else if (ipcensusMode) {
        ipcensusPage = 28;
	icount = 1;
	user = -1;
      } else if (driversMode) {
        driversPage = 28;
	icount = 1;
	user = -1;
      } else {
	setAllPageModesToZero();
	icount = 1;
	ant = AC_DISPLAY;
	aCMode = 1;
      }
      break;
    case 'c':
      if (smainitMode) {
	smainitPage = 13;
	icount = 1;
      } else if (ipcensusMode) {
	ipcensusPage = 15;
	icount = 1;
      } else if (driversMode) {
	driversPage = 15;
	icount = 1;
      } else {
	setAllPageModesToZero();
	corrSumMode = 1;
	icount = 1;
	ant = CORRSUM_DISPLAY;
      }
      break;
    case '%':
      setAllPageModesToZero();
      icount = 1;
      ant = POLAR_DISPLAY;
      break;
    case 'u':
      if (dDSMode) {
	if (baselineInfo > 2) {
	  baselineInfo = UNPROJECTED_SORTED_BY_LENGTH;
	} else {
	  baselineInfo = UNPROJECTED;
	}
      } else {
	setAllPageModesToZero();
	upsMode = 1;
	icount = 1;
	ant = UPS_DISPLAY;
      }
      break;
    case 'R':
      if (corrMode) {
	icount = 1;
	if (statesPage) {
	  statsRx++;
	  if (statsRx > 2)
	    statsRx = 1;
	} else
	  statesPage = TRUE;
      } else {
	setAllPageModesToZero();
	icount = 1;
	ant = RSCAN_DISPLAY;
      }
      break;
    case 'w':
      setAllPageModesToZero();
      weatherMode = 1;
      icount = 1;
      ant = WEATHER_DISPLAY;
      break;
    case 'W':
      setAllPageModesToZero();
      weatherMode = 1;
      icount = 1;
      ant = WEATHER2_DISPLAY;
      break;
    case 'b':
      setAllPageModesToZero();
      airHandlerMode = 1;
      icount = 1;
      ant = AIR_DISPLAY;
      break;
    case 't':
      setAllPageModesToZero();
      icount = 1;
      chopperMode = 1;
      ant = CHOPPER_DISPLAY;
      break;
    case 'h':
      if (smainitMode) {
        smainitPage = 29;
        icount = 1;
	break;
      }
    case '?':
      setAllPageModesToZero();
      icount = 1;
      ant = HELP_DISPLAY;
      break;
    case '/':
      setAllPageModesToZero();
      icount = 1;
      ant = USERS_DISPLAY;
      break;
    case 'r':
      setAllPageModesToZero();
      icount = 1;
      ant = RX_DISPLAY;
      rxMode = 1;
      /*      rxPage = 0;*/
      break;
    case '\t':
      icount = 0;
      break;
    case 'O':
      if (smainitMode) {
	smainitPage = 27;
	icount = 1;
      } else {
	setAllPageModesToZero();
	icount = 1;
	opMsgMode = 1;
	opMsgPage = 1;
	ant = OPERATOR_DISPLAY;
	opMessagePending = FALSE;
	opMsgHash = -1;
      }
      break;
    case 'o':
      setAllPageModesToZero();
      icount = 1;
      opticsMode = 1;
      ant = OPTICS_DISPLAY;
      break;
    case 'y':
      setAllPageModesToZero();
      icount = 1;
      ant = DEWAR_DISPLAY;
      break;
    case 'Y':
      setAllPageModesToZero();
      icount = 1;
      ant = YIG_DISPLAY;
      yIGMode = 1;
      break;
    case 'P':
      setAllPageModesToZero();
      icount = 1;
      ant = PMODEL_DISPLAY;
      break;
    case 27: /* escape */
#if 1    
      if (++upperRightWindow >= UR_STDERR) {
	upperRightWindow = UR_MESSAGES;
      }
      break;
#else
      /* this works in UAP but not in monitor.  Need to investigate */
      key = getch();
      key = getch();
      switch (key) {
      case 13: /* ^m */
	upperRightWindow = UR_MESSAGES;
	break;
      case 12: /* ^l */
	upperRightWindow = UR_SMASHLOG;
	break;
      case 5: /* ^e */
	upperRightWindow = UR_STDERR;
	break;
      }
      break;
#endif
    case 'M':
      setMostPageModesToZero(NOT_IPCENSUS);
      if (ipcensusMode) {
        ipcensusPage = 14; /* second page for m5 */
      } else {
	ant = MAP_DISPLAY;
      }
      icount = 1;
      break;
    case 'T':
      setAllPageModesToZero();
      icount = 1;
      ant = TILT_DISPLAY;
      break;
    case 'U':
      setAllPageModesToZero();
      icount = 1;
      ant = UPTIMES_DISPLAY;
      break;
    case 'm':
      if (smainitMode) {
 	smainitPage = 12;
	icount = 1;
	setAllPageModesToZero();
	smainitMode = 1;
      } else if (ipcensusMode) {
        ipcensusPage = 12;
	icount = 1;
	setAllPageModesToZero();
	ipcensusMode = 1;
      } else if (driversMode) {
        driversPage = 12;
	icount = 1;
	setAllPageModesToZero();
	driversMode = 1;
      } else {
	setAllPageModesToZero();
	icount = 1;
	ant = MESSAGE_DISPLAY;
	message = 0;
	messageMode = 1;
	oldMessageStat.st_mtime = messageStat.st_mtime;
      }
      break;
    case 'f':
      setAllPageModesToZero();
      icount = 1;
      ant = C1DC_DISPLAY;
      break;
    case '"':
      if (dDSMode) {
	if (baselineInfo > 2) {
	  baselineInfo = MAS_SORTED_BY_LENGTH;
	} else {
	  baselineInfo = MAS;
	}
      }
      break;
    case 'p':
      if (smainitMode) {
	smainitPage = 28;
	icount = 1;
      } else if (dDSMode) {
	if (baselineInfo > 2) {
	  baselineInfo = PROJECTED_SORTED_BY_LENGTH;
	} else {
	  baselineInfo = PROJECTED;
	}
      } else {
	setAllPageModesToZero();
	icount = 1;
	ant = PROJECT_DISPLAY;
      }
      break;
    case '~':
      if (smainitMode) {
	smainitPage = 11;
	icount = 1;
      } else if (ipcensusMode) {
	ipcensusPage = 11;
	icount = 1;
      } else if (driversMode) {
	driversPage = 11;
	icount = 1;
      }
      break;
    case 's':
      setAllPageModesToZero();
      ant = SMAINIT_DISPLAY;
      icount = 1;
      smainitMode = 1;
      break;
    case 'n':
      if (dDSMode) {
	switch (baselineInfo) {
	case MAS_SORTED_BY_LENGTH:
	  baselineInfo = MAS;
	  break;
	case PROJECTED_SORTED_BY_LENGTH:
	  baselineInfo = PROJECTED;
	  break;
	case UNPROJECTED_SORTED_BY_LENGTH:
	  baselineInfo = UNPROJECTED;
	  break;
	}
      } else {
	setAllPageModesToZero();
	ant = IPCENSUS_DISPLAY;
	icount = 1;
	ipcensusMode = 1;
      }
      break;
    case 'H':
      setAllPageModesToZero();
      ant = HANGAR_DISPLAY;
      icount = 1;
      break;
    case 'g':
      setAllPageModesToZero();
      ant = MRG_DISPLAY;
      icount = 1;
      break;
    case 'E':
      endwin();
      switch (esmaMode) {
      case 2:
	goto80width();
	esmaMode = 0;
	COLS = 80;
	break;
      case 1:
	esmaMode = 2;
	COLS = 132;
	break;
      case -1:
	allow132width();
	/* continue with the next case */
      case 0:
	goto132width();
	esmaMode = 1;
	COLS = MINIMUM_SCREEN_WIDTH_FOR_ESMA;
	break;
      }
      initialize();
      break;
    case 'G':
      setAllPageModesToZero();
      ant = GPS_DISPLAY;
      icount = 0;
      break;
    case 'V':
      setAllPageModesToZero();
      ant = RX_DISPLAY;
      rxMode = 1;
#define RX_ALLAN_VARIANCE_PAGE 4
#define MAX_RECEIVER_PAGES 5
#define MAX_ANTENNA_PAGES 2
#define MAX_IFLO_PAGES 3
#define MAX_UPS_PAGES 2
      rxPage = RX_ALLAN_VARIANCE_PAGE;
      break;
    case 'v':
      setAllPageModesToZero();
      ant = DRIVERS_DISPLAY;
      icount = 1;
      driversMode = 1;
      break;
    case 'S':
      if (commandMode || errorMode || messageMode || (aCMode && (aCPage == 1))) {
	ioctl(0, TCSETA, &tio);
	printf("\nEnter search string: "); fflush(stdout);
	scanf("%s", searchString);
	searchMode = TRUE;
	ioctl(0, TCGETA, &tio);
	tin = tio;
	tin.c_lflag &= ~ECHO;
	tin.c_lflag &= ~ICANON;
	tin.c_cc[VMIN] = 0;
	tin.c_cc[VTIME] = 0;
	ioctl(0, TCSETA, &tin);
	icount = 1;
      } else {
	setAllPageModesToZero();
	ant = SEEING_DISPLAY;
	icount = 1;
      }
      break;
    case 'x':
      setAllPageModesToZero();
      ant = DEAD_DISPLAY;
      icount = 1;
      break;
    case 'X':
      setAllPageModesToZero();
      icount = 1;
      ant = SWARM_DISPLAY;
      break;
    case ' ':
    case 'a':
      setAllPageModesToZero();
      icount=0;
      ant = ARRAY_DISPLAY;
      beepFlag=1;
      break;
    case '-':
      if (commandMode || errorMode || messageMode || (aCMode && (aCPage == 1))) {
	pageOffset++;
      }
      if (corrSumMode) {
	corrSumPage--;
	if (corrSumPage < 0) {
	  corrSumPage = 1;
	}
      } else if (corrMode) {
	icount = 1;
        corrPage--;
	if (corrPage < 0)
	  corrPage = 11;
      } else if (rxMode == 1) {
	icount = 1;
	rxPage--;
	if (rxPage < 0) {
          rxPage = MAX_RECEIVER_PAGES-1;
	}
      } else if (opticsMode) {
	icount = 1;
	opPage--;
	if (opPage < 0)
	  opPage = 1;
      } else if (antMode) {
	icount = 0;
	antPage--;
	if (antPage < 0) {
	  antPage = MAX_ANTENNA_PAGES-1;
	}
      } else if (antIFLOMode) {
	icount = 1;
	antIFLOPage--;
	if (antIFLOPage < 0) {
	  antIFLOPage = MAX_IFLO_PAGES-1;
	}
      } else if (upsMode) {
	upsPage--;
	if (upsPage < 0) {
	  upsPage = MAX_UPS_PAGES-1;
	}
      } else if (dDSMode) {
	dDSRx--;
	if (dDSRx < 0) {
	  dDSRx = 1;
	}
      } else if (yIGMode) {
	yIGRx--;
	if (yIGRx < 0) {
	  yIGRx = 1;
	}
      } else if (smainitMode) {
	icount = 1;
	smainitPage--;
	if (smainitPage < 13)
	  smainitPage = 24;
      } else if (upsMode) {
	upsPage--;
	if (upsPage < 0)
	  upsPage = MAX_UPS_PAGES;
      } else if (ipcensusMode) {
	icount = 1;
	ipcensusPage--;
	if (ipcensusPage < 15)
	  ipcensusPage = 26;
      } else if (driversMode) {
	icount = 1;
	driversPage--;
	if (driversPage < 15)
	  driversPage = 26;
      }
      break;
    case '.':
      if (commandMode || errorMode || messageMode || (aCMode && (aCPage == 1))) {
	pageOffset = 0;
        icount = 1;
      }
      break;
    case '#':
      iFLOUnits++;
      if (iFLOUnits>=IFLO_UNITS_LAST) {
	iFLOUnits = 0;
      }
      break;
    case '+':
      if (corrSumMode) {
	corrSumPage++;
	if (corrSumPage > 1) {
	  corrSumPage = 0;
	}
      } else if (opticsMode) {
	icount = 1;
	opPage++;
	if (opPage > 1)
	  opPage = 0;
      } else if (antMode) {
	icount = 0;
	antPage++;
	if (antPage >= MAX_ANTENNA_PAGES) {
	  antPage = 0;
	}
      } else if (antIFLOMode) {
	icount = 1;
	antIFLOPage++;
	if (antIFLOPage >= MAX_IFLO_PAGES) {
	  antIFLOPage = 0;
	}
      } else if (corrMode) {
	icount = 1;
        corrPage++;
      } else if (aCMode) {
	aCPage++;
	if (aCPage > 1) {
	  aCPage = 0;
	  icount = 1;
	}
      } else if (rxMode) {
	icount = 1;
	rxPage++;
	if (rxPage >= MAX_RECEIVER_PAGES) {
          rxPage = 0;
	}
      } else if (commandMode || messageMode) {
	pageOffset--;
	if (pageOffset < 0) {
	  pageOffset = 0;
	}
      } else if(chopperMode) {
	chopperUnits ^= 1;
      } else if(airHandlerMode) {
	airHandlerUnits ^= 1;
      } else if(weatherMode) {
	weatherUnits ^= 1;
      } else if(opMsgMode) {
	opMsgPage++;
	if (opMsgPage > 8) {
	  opMsgPage = 1;
	}
      } else if(upsMode) {
	upsPage++;
	if (upsPage >= MAX_UPS_PAGES) {
	  upsPage = 0;
	}
      } else if (smainitMode) {
	icount = 1;
	smainitPage++;
	if (smainitPage > 24) {
	  smainitPage = 13;
	}
      } else if (ipcensusMode) {
	icount = 1;
	ipcensusPage++;
	if (ipcensusPage > 26) {
	  ipcensusPage = 15;
	}
      } else if (driversMode) {
	icount = 1;
	driversPage++;
	if (driversPage > 26) { /* crate 12 */
	  driversPage = 15; /* crate 1 */
	}
      } else if (dDSMode) {
	dDSRx++;
	if (dDSRx > 1) {
	  dDSRx = 0;
	}
      } else if (yIGMode) {
	yIGRx++;
	if (yIGRx > 1) {
	  yIGRx = 0;
	}
      }
      break;
    }			/* end of switch */
    if (intruder == TRUE) {
#if DEBUG
      fprintf(stderr,"About to call intruderPage()\n");
#endif
      intruderPage(icount++);
    } else if ((message) && (!bottom2op)) {
      if (ant == ARRAY_DISPLAY) { /* we are on the 'a' page */
	if ((COLS < MINIMUM_SCREEN_WIDTH_FOR_MESSAGES_ON_MAIN_PAGE) && FALSE) {
	  messagePage(icount++, 1);
	} else if (upperRightWindow != UR_MESSAGES) {
	  upperRightWindow = UR_MESSAGES;
	} else {
	  /* we are already seeing the new message on the extended width 'a' page */
	}
      } else {
	messagePage(icount++, 1);
      }
    } else if ((!ignoreOpMessages) && opMessagePending && (ant != -26)) {
      icount = 0;
      messagePage(icount++, 2);
    } else if (deiceWarn == 1 && ignoreIntruders==FALSE) {
      deiceWarningPage(icount++,deiceOn);
    } else {
      if (ant == ARRAY_DISPLAY) {
	if (cycle) {
	  if (!firstUpdate) {
	    sleep(delay);
	  }
	  firstUpdate = FALSE;
	  timeStamp();
	}
	if (icount < 1) {
	  icount = 1;
	}
#if DEBUG
        fprintf(stderr,"About to call arrayDisplay() with icount=%d, ignoreIntruders=%d\n",icount,ignoreIntruders);
#endif
	arrayDisplay(icount, ignoreIntruders, antlist); 
      }
      else if (ant == DDS_DISPLAY) {
	dDSDisplay(icount, dDSRx);
      } else if (ant == IFLO_DISPLAY) {
	iFLODisplay(icount);
      } else if (ant == STDERR_DISPLAY) {
	commandDisplay(icount, &pageOffset, &searchMode, searchString,
		     STDERR_LOG,N_LINES,0,0,COLS);
      } else if (ant == SMASH_DISPLAY) {
	commandDisplay(icount, &pageOffset, &searchMode, searchString,
		       SMASH_LOG,N_LINES,0,0,COLS);
      } else if (ant == CORRELATOR_DISPLAY) {
	if (!statesPage)
	  correlatorDisplay(icount, &corrPage);
	else
	  stateCounts(icount, statsRx, &corrPage);
      } else if (ant == HELP_DISPLAY) {
	help(icount);
      } else if (ant == RX_DISPLAY) {
	switch (rxPage) {
        case 0:
	  receiverMonitor(icount,antlist,RECEIVER_PAGE_PRINT,&receiverFlags);
	  break;
        case 1:
	  receiverMonitorHighFreq(icount, antlist, RECEIVER_PAGE_PRINT,&receiverFlags);
	  break;
	case 2:
	  lomotorpage(icount, antlist);
	  break;
	case 3:
	  networkPage(icount, antlist);
	  break;
	case 4:
	  allanVariancePage(icount, antlist, ALLAN_VARIANCE_PAGE_PRINT,&allanVarianceFlags);
	  break;
	}
      } else if (ant == SMAINIT_DISPLAY) {
	smainitMonitor(icount, smainitPage);
      } else if (ant == DEAD_DISPLAY) {
	listDead(icount);
      } else if (ant == ANT_IFLO_DISPLAY) {
	if (antIFLOPage == 0) {
	  iFLODisplayPage2(icount, antlist, IFLO_PAGE_PRINT, &ifloFlags);
	} else if (antIFLOPage == 1) {
	  iFLODisplayPage3(icount, antlist);
	} else {
	  iFLODisplayPage4(icount, antlist);
	}
      } else if (ant == AC_DISPLAY) {
	if (aCPage == 0) {
	  aCDisplay(icount);
	} else {
	  commandDisplay(icount, &pageOffset, &searchMode, searchString,
			 CORR_MONITOR_LOG,N_LINES,0,0,COLS);	  
	}
      } else if (ant == CORRSUM_DISPLAY) {
	correlatorSummary(icount, corrSumPage);
      } else if (ant == DEICE_DISPLAY) {
	deicemon(icount, antlist);
      } else if (ant == OPTICS_DISPLAY) {
	if (opPage == 0)
	  opticsPage(icount, antlist,OPTICS_PAGE_PRINT,&opticsFlags);
	else
	  opTel(icount, antlist);
      } else if (ant == DEWAR_DISPLAY) {
	dewarpage(icount, antlist);
      } else if (ant == PMODEL_DISPLAY) {
	pmodelspage(icount, antlist);
      } else if (ant == C1DC_DISPLAY) {
	c1DC(icount,C1DC_PAGE_PRINT,&c1DCFlags);
      } else if (ant == PROJECT_DISPLAY) {
	projectpage(icount);
      } else if (ant == MESSAGE_DISPLAY) {
	commandDisplay(icount, &pageOffset, &searchMode, searchString,
		     MESSAGES_LOG,N_LINES,0,0,COLS);
      } else if (ant == COHERENCE_DISPLAY) {
	coherence(icount);
      } else if (ant == POLAR_DISPLAY) {
	polar(&icount);
      } else if (ant == UPS_DISPLAY) {
	if (upsPage == 0) {
	  upspage(icount, antlist);
	} else {
	  upspage2(icount, antlist);
	}
      } else if (ant == WEATHER_DISPLAY) {
	weather(icount, antlist);
      } else if (ant == AIR_DISPLAY) {
	airHandler(icount, antlist);
      } else if (ant == CHOPPER_DISPLAY) {
	chopperPage(icount, antlist);
      } else if (ant == OPERATOR_DISPLAY) {
	opMsg(icount, FALSE);
	/*
	opointScreen(icount, opointPage);
	*/
      } else if (ant == RSCAN_DISPLAY) {
	rscanpage(icount, antlist,RSCAN_PRINT);
      } else if (ant == MAP_DISPLAY) {
	arraymap(icount, antlist);
      } else if (ant == TILT_DISPLAY) {
	tiltpage(icount, antlist);
      } else if (ant == UPTIMES_DISPLAY) {
	uptimes(icount);
      } else if (ant == SEEING_DISPLAY) {
	seeing(icount,antlist);
      } else if (ant == WEATHER2_DISPLAY) {
	weatherPage2(icount,antlist);
      } else if (ant == IPCENSUS_DISPLAY) {
	ipcensus(icount,ipcensusPage);
      } else if (ant == DRIVERS_DISPLAY) {
	driversLoaded(icount,driversPage);
      } else if (ant == MRG_DISPLAY) {
	mRG(icount);
      } else if (ant == USERS_DISPLAY) {
	users(icount);
      } else if (ant == HANGAR_DISPLAY) {
	hangarPage(icount);
      } else if (ant == GPS_DISPLAY) {
	gpsPage(icount);
      } else if (ant == GENSET_DISPLAY) {
	genset(icount);
      } else if (ant == BDC812_DISPLAY) {
        bdc812(icount);
      } else if (ant == SWARM_DISPLAY) {
	swarmPage(icount);
      } else if (ant == YIG_DISPLAY) {
	yIGFrequencies(icount, yIGRx);
      } else if (ant == EFF_DISPLAY) {
	solveGains(icount);
      } else if (ant == FLAGGING_DISPLAY) {
	flagging(icount);
      } else {
	if (antPage == 1) {
	  antPage2(icount, antlist, ant, ant);
	} else {
          antDisplay(ant, icount, ANTENNA_PAGE_PRINT, &antennaFlags);
	}
      }
      if (cycle) {
	time_t curTime;
	struct tm *gmt;
	
	curTime = time((long *)0);
	gmt = gmtime(&curTime);
	if (startDay != gmt->tm_yday) {
	  goto80WidthAndExit(0);
	}
	icount = 0;
	if (ant == RX_DISPLAY) {
	  if (rxPage == 0) {
	    rxPage = 1;
	  } else if (rxPage == 1) {
	    rxPage = 2;
	  } else if (rxPage == 2) {
	    rxPage = 3;
	  } else {
	    rxPage = 0;
	    ant++;
	  }
	} else {
	  ant++;
	}
	if ((ant > 7) && (antPage > 0)) {
	  ant = PMODEL_DISPLAY;
	  antMode = commandMode = messageMode = errorMode = pageOffset = corrMode = corrSumMode = aCMode = dDSMode = corrPage = opticsMode = yIGMode = 0;
	  antPage = 1;
	} else if (ant == DEICE_DISPLAY) {
	  ant++;
	} else if (ant == AC_DISPLAY)
	  ant = ANT_IFLO_DISPLAY;
	else if (ant == SMAINIT_DISPLAY)
	  ant = RX_DISPLAY;
	else if (ant == HELP_DISPLAY)
	  ant = CORRELATOR_DISPLAY;
	else if (ant == SMASH_DISPLAY)
	  ant = IFLO_DISPLAY;
	else if (ant == ANTENNAPAGE_DISPLAY) {
	  antMode = 1;
	  antPage = 1;
	} else if (ant == 2222) {
	  printf("See antenna = 2\n");
	}
	if (antMode) {
	  if (antPage == 0) {
	    antPage++;
	    ant--;
	  } else {
	    antPage = 0;
	  }
	}
      } else {
	int opLines;

	if (bottomOpMsg && (opMessagePending)) {
	  int ii;

	  if (opBeepCount-- > 0) {
#ifdef LINUX
	    beep();
#else
            printw("\a");
#endif
	    refresh();
	  }
	  opLines = opMsg(icount, TRUE);
	  for (ii = 0; ii < COLS; ii++) {move(24+opLines+2,ii);printw("-");}
	  opLines +=3;
	} else {
	  opBeepCount = 3;
	  opLines = 0;
	}
	if (bottom2op) {
	  if (message) {
#ifdef LINUX
	    beep();
#else
	    printw("\a");
#endif
	    refresh();
	    message = FALSE;
	    oldMessageStat.st_mtime = messageStat.st_mtime;
	  }
	  {
	    int zero = 0;

	    commandDisplay(icount, &zero, &searchMode, searchString,
		      MESSAGES_LOG, LINES-opLines-24, 0, 22+opLines, COLS);
	  }
	}
#ifndef LINUX
	sleep(delay);
#else
	usleep(delay*500000);
	spinCount++;
	if ((spinCount % 2) && ((icount % 30) > 1))
	  icount--;
#endif
      }
    }
  }				/* this is the big while loop */
  ioctl(0, TCSETA, &tio);
}				/* end of main Loop */


void handlerForSIGINT(int signum)
{
  user='q'; /* 'q' for quit command */
  fprintf(stderr,"Got the control C signal. Quitting.\n");
  printf("Bye.\n");
  ioctl(0, TCSETA, &tio);
  goto80WidthAndExit(0);
}

/* The following are variables to be added to screen at some point
   rms=rm_read(ant,"RM_EPOCH_F",&epoch);
   rms=rm_read(ant,"RM_SVEL_KMPS_D",&dummydouble);
   rms=rm_read(ant,"RM_ANTENNA_DRIVE_STATUS_S", &antenna_drive_fault);
   rms=rm_read(ant,"RM_HOUR_ANGLE_HR_D", &hourangle);
   sundistance = 
   sunDistance(az_actual_disp*radian,el_actual_disp*radian,sunaz,sunel);
   rms=rm_read(ant,"RM_TILTX_DC_ARCSEC_D",&tiltxdc);
   rms=rm_read(ant,"RM_TILTY_DC_ARCSEC_D",&tiltydc);
   sundistance;
   lastcommand;
   mirrorstate;
   syncdetvolts;
   source velocity
*/

void deiceWarningPage(int count, int *deiceOn) {
  float barrels;
  int i;

  if ((count % 120) == 0) {
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
  /*
  move(1,40);
  printw("%d %d\n",deiceCheckCount,lastDeiceWarn);
  */
  move(1,15);
  printw("Just a friendly, periodic reminder from the electric company....\n");
  move(2,15);
  printw("The De-Ice energy consumption is presently at %d Watts\n",deiceWattage);
  /* the following formula comes from 41.868GJ=1 metric ton crude=7.3 barrels*/
  barrels = ((float)deiceWattage)/66250;
  barrels /= .3034;  /* for efficiency of a HECO's steam turbine generators */
  move(3,15);
  printw("This is equivalent to %.2f barrels of crude oil per day!\n",barrels);
  for (i=1; i<=8; i++) {
    if (deiceOn[i] != 0) {
      move(5+2*i,28);
      printw("DE-ICE is ACTIVE on antenna %d\n",i);
    }
  }
  move(23,0);
  if (!(count % 3)) {
#ifdef LINUX
    beep();
#else
    printw("\a");
#endif
  }
  refresh();
  usleep(600000);
}

void readIFServerInfo(void) {
  int i, narg,ant,rx;
  char filename[120], *ptr;
  FILE *fp;
  char line[120];
  float floatvalue, mrgtarget, target200, openloop,closedloop;
  int found;

  strcpy(filename,"/otherInstances/storage/1/configFiles/iFServerInfoNew.txt");
  fp = fopen(filename,"r");
  if (fp != NULL) {
    found = 0;
    do {
      ptr = fgets(line,sizeof(line),fp);
      if (ptr == NULL) break;
      if (!present(line,"#") && !present(line,"TYPE")&&!present(line,"CLOSED")) {
	narg = sscanf(line,"%d %d %f %f",&ant,&rx,&openloop,&closedloop);
	if (narg == 4) {
	  if (rx == 1) {
	    c1dcTargetIF1[ant] = closedloop;
	    found++;
	  }
	  if (rx == 2) {
	    c1dcTargetIF2[ant] = closedloop;
	    found++;
	  }
	}
      }
    } while (ptr != NULL && found < 16);
    fclose(fp);
  } /* end if (fp != NULL) */
  for (i=1; i<=MAX_NUMBER_ANTENNAS; i++) {
    found = 0;
    sprintf(filename,"/otherInstances/acc/%d/configFiles/antIFServerInfo.txt",i);
    fp = fopen(filename,"r");
    if (fp != NULL) {
      do {
	ptr = fgets(line,sizeof(line),fp);
	if (ptr == NULL) break;
	if (!present(line,"#")) {
	  sscanf(line,"%*s %*f %f %*d %*f %f %*d %*f %*f %*d %*f %*f %*d %*f %*f %*d %*f %f %*d",&mrgtarget,&target200,&floatvalue);
          if (present(line,"A1")) {
	    cont1det2target[i] = floatvalue;
	    mrgRfPowerTarget[i] = mrgtarget;
	    power200MHzTarget[i] = target200;
	    found++;
	  } else if (present(line,"C")) {
	    cont2det2target[i] = floatvalue;
	    power200MHzTarget2[i] = target200;
	    found++;
	  }
	}
      } while (ptr != NULL && found < 2);
      fclose(fp);
    } /* end if 'found the info file' */
  } /* end of 'for' loop over antennas */

}

int call_dsm_read(char *machine, char *variable, void *ptr, time_t *tstamp) {
  char buf[256];
  int rms;
  rms = dsm_read(machine,variable,ptr,tstamp);
  if (rms != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      sprintf(buf,"dsm_read - %s",variable);
      dsm_error_message(rms, buf);
    }
  }
  return(rms);
}

int call_dsm_structure_get_element(dsm_structure *ds, char *name, void *ptr) {
  char buf[256];
  int rms;
  rms = dsm_structure_get_element(ds, name, ptr);
  if (rms != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS) {
      sprintf(buf,"dms_structure_get_element - %s",name);
      dsm_error_message(rms, buf);
    }
  }
  return(rms);
}

void setAllPageModesToZero(void) {
/* 4 */      antMode = commandMode = messageMode = errorMode = pageOffset = yIGMode = 0;
/* 4+5=9 */  corrMode = corrSumMode = aCMode = dDSMode = corrPage = chopperMode = 0;
/* 9+5=14 */ weatherMode = opMsgMode = upsMode = antIFLOMode = airHandlerMode = 0;
/*14+4=18 */ driversMode = ipcensusMode = smainitMode = rxMode = opticsMode = 0;

/* not cleared: searchMode */
}

void setMostPageModesToZero(int type) {
  switch (type) {
  case NOT_DRIVERS_IPCENSUS_SMAINIT:
    antMode = commandMode = messageMode = errorMode = pageOffset = rxMode = 0;
    corrMode = corrSumMode = aCMode = dDSMode = corrPage = chopperMode = yIGMode = 0;
    weatherMode = opMsgMode = upsMode = antIFLOMode = airHandlerMode = opticsMode = 0;
    break;
  case NOT_IPCENSUS:
    antMode = commandMode = messageMode = errorMode = pageOffset = rxMode = 0;
    corrMode = corrSumMode = aCMode = dDSMode = corrPage = chopperMode = 0;
    weatherMode = opMsgMode = upsMode = antIFLOMode = airHandlerMode = 0;
    smainitMode = driversMode = opticsMode = yIGMode = 0;
    break;
  }
}

void allow132width(void) {
  char name[20];
  sprintf(name,"%c?40h",0x9b);
  printf(name);
}

void goto132width(void) {
  char name[20];
  sprintf(name,"%c?3h",0x9b);
  printf(name);
}

void goto80width(void) {
  char name[20];
  sprintf(name,"%c?3l",0x9b);
  printf(name);
}

void initialize() {
  int i;

  initscr();
#ifdef LINUX
  if(colorFlag) {
	start_color();
  }
  nonl();
  clear();
#endif
   /* LINES and COLS are now defined */
  if (COLS > 132)
    COLS = 132;
  if ((COLS >= MINIMUM_SCREEN_WIDTH_FOR_ESMA) && FALSE){
    numberAntennas = 10;
  } else {
    numberAntennas = 8;
  }
  readIFServerInfo();
  if (LINES >= MIN_LINES_FOR_BOTTOM_2OP) {
    bottom2op = TRUE;
    ignoreMessages = TRUE;
  }
  if (LINES >= MIN_LINES_FOR_BOTTOM_OPMSG) {
    bottomOpMsg = TRUE;
    ignoreOpMessages = TRUE;
  }
  if ((COLS >= MINIMUM_SCREEN_WIDTH_FOR_ESMA) && FALSE) {
    numberAntennas = 10;
  } else {
    numberAntennas = 8;
  }
  /*
  box(stdscr, '|','-');
  */

  for (i = 1; i < COLS-1; i++) {move(0,i);printw("-");}
  for (i = 1; i < COLS-1; i++) {move(23,i);printw("-");}
  for (i = 1; i < 23; i++) {move(i,0);printw("|");}
  for (i = 1; i < 23; i++) {move(i,COLS-1);printw("|");}
#if 0
  move(0,0);
  printw("COLS=%d",COLS);
#endif
  move(2,2);
}

void goto80WidthAndExit(int value) {
  goto80width();
  exit(value);
}

#define LYNX_FINGER  0
#define LINUX_FINGER 1
int checkAsiaaIdleTime(void) {
  return(checkIdleTime("/global/obscon.asiaa.users",LYNX_FINGER));
}

int checkObsconIdleTime(void) {
  return(checkIdleTime("/global/obscon.users",LINUX_FINGER));
}

int checkObsconhIdleTime(void) {
  return(checkIdleTime("/global/obsconh.users",LINUX_FINGER));
}

int checkObsconhpIdleTime(void) {
  return(checkIdleTime("/global/obsconhp.users",LINUX_FINGER));
}

int checkObsconcIdleTime(void) {
  return(checkIdleTime("/global/obsconc.users",LINUX_FINGER));
}

int checkIdleTime(char *filename, int ostype) {
  int dateStartChar = 0;
  FILE *tempfd;
  int narg,idleMinutes,minIdleMinutes,idleHours;
  char line[100],idle[20],month[50];
  int year,doy,hour,min,sec;
  int seconds, gmtseconds;
  time_t curTime;
  struct tm *gmt;

  switch (ostype) {
  case LINUX_FINGER:
    dateStartChar = 39;
    break;
  case LYNX_FINGER:
    dateStartChar = 34;
    break;
  }
#define MIN_IDLE 9999999
  idleMinutes = MIN_IDLE;
  /* return the value in minutes,  give -1 if no one logged onto console */
  if ((tempfd = fopen(filename,"r")) != NULL) {
    /* first line holds the date/time, should check it for up-to-date-ness */
    fgets(line,sizeof(line),tempfd);
    narg = sscanf(line,"%d %d %d %d %d",&year,&doy,&hour,&min,&sec);
#define SEC_PER_YEAR 31556925 /* no need to be accurate, just consistent */
    seconds = (year-2000)*SEC_PER_YEAR + doy*86400 + hour*3600 + min*60 + sec;
    curTime = time((long *)0);
    gmt = gmtime(&curTime);
    gmtseconds = (gmt->tm_yday+1)*86400+gmt->tm_hour*3600+gmt->tm_min*60+gmt->tm_sec;
    if (gmt->tm_year > 2000) {
      gmtseconds += (gmt->tm_year-2000)*SEC_PER_YEAR;
    } else if (gmt->tm_year > 100) {
      /* i.e. 2005 is 105 */
      gmtseconds += (gmt->tm_year-100)*SEC_PER_YEAR;
    } else {
      /* i.e. 2005 is 5 */
      gmtseconds += gmt->tm_year*SEC_PER_YEAR;
    }
    minIdleMinutes = MIN_IDLE;
    /* second line and succeeding lines hold entries for console ttys */
    narg = 1;
    while (narg > 0) {
      bzero(line,sizeof(line));
      fgets(line,sizeof(line),tempfd);
      /* The following is needed to compensate for an unexplained line
       * getting into the finger output file. */
      if (present(line,"No one logged")) continue;
      narg = 0;
      if (strlen(line) > 0) {
	narg = 1;
	narg = sscanf(&line[dateStartChar],"%s",month);
	/* if first non-blank chars are a month, then there is no idle time */
	if (present(month,"Jan") ||
	    present(month,"Feb") ||
	    present(month,"Mar") ||
	    present(month,"Apr") ||
	    present(month,"May") ||
	    present(month,"Jun") ||
	    present(month,"Jul") ||
	    present(month,"Aug") ||
	    present(month,"Sep") ||
	    present(month,"Oct") ||
	    present(month,"Nov") ||
	    present(month,"Dec")
	    /* ||
	    present(month,"Mon") ||
	    present(month,"Tue") ||
	    present(month,"Wed") ||
	    present(month,"Thu") ||
	    present(month,"Fri") ||
	    present(month,"Sat") ||
	    present(month,"Sun")
	    */
	    ) {
	  idleMinutes = 0;
	  minIdleMinutes = 0;
	} else {
	  narg = sscanf(&line[dateStartChar],"%d:%2d",&idleHours,&idleMinutes);
	  /*
	  if (ostype == LYNX_FINGER) {
	    printw("str=%s narg=%d ",&line[dateStartChar],narg);
	  }
	  */
	  if (narg == 0) {
	    narg = 1;
	    continue;
	  }
	  if (narg < 2) {
	    idleMinutes = idleHours;
	  } else {
	    idleMinutes = idleHours*60+idleMinutes;
	  }
	  if (narg > 0) {
	    bzero(idle,sizeof(idle));
	    strncpy(idle,&line[dateStartChar],4);
	    if (present(idle,"d")) {
	      idleMinutes *= 1440;
	    }
	    if (idleMinutes < minIdleMinutes) {
	      minIdleMinutes = idleMinutes;
	    }
	  }
	}
      }
    }
    fclose(tempfd);
  }
  if (idleMinutes == MIN_IDLE) {
    minIdleMinutes = OBSCON_NO_ONE_LOGGED_ON;
  }
  if ((gmtseconds - seconds) > 300) {
    minIdleMinutes = OBSCON_INFO_STALE;
  }
  return(minIdleMinutes);
}

int present(char *a, char *b) {
  if (strstr(a,b) == NULL) {
    return(0);
  } else {
    return(1);
  }
}

void printObsconIdleTime(int idleMinutes) {
  printw("obscon: ");
  printIdleTime(idleMinutes);
}

void printObsconhIdleTime(int idleMinutes) {
  printw("obsconh: ");
  printIdleTime(idleMinutes);
}

void printObsconhpIdleTime(int idleMinutes) {
  printw("obsconhp: ");
  printIdleTime(idleMinutes);
}

void printAsiaaIdleTime(int idleMinutes) {
  printw("ASIAA: ");
  printIdleTime(idleMinutes);
}

void printObsconcIdleTime(int idleMinutes) {
  printw("obsconc: ");
  printIdleTime(idleMinutes);
}

void printIdleTime(int idleMinutes) {
  if (idleMinutes >= 0) {
    if (idleMinutes > 30) {
      standout();
    }
    if (idleMinutes > 1440*10) {
      standend();
      printw("(no one present)");
    } else if (idleMinutes > 1440) {
      printw("%.1f days",(float)idleMinutes/1440.);
    } else {
      printw("%d minutes",idleMinutes);
    }
    standend();
  } else {
    switch (idleMinutes) {
    case OBSCON_NO_ONE_LOGGED_ON:
      printw("(no one present)");
      break;
    case OBSCON_INFO_STALE:
    default:
      printw("(stale)");
    }
  }
  printw("\n");
}

int parseReceiverName(char *dummyString1) {
 if (present(dummyString1, "A1")) return(0);
 /* if (!strcmp(dummyString1, "A1")) return(0);*/
 if (!strcmp(dummyString1, "A2")) return(1);
 if (!strcmp(dummyString1, "B1")) return(2);
 if (!strcmp(dummyString1, "B2")) return(3);
 if (!strcmp(dummyString1, "C")) return(4);
 if (!strcmp(dummyString1, "D")) return(5);
 if (!strcmp(dummyString1, "E")) return(6);
 if (!strcmp(dummyString1, "F")) return(7);
 return(-1);
}

char *getLoBoardTypeStringBrief(int type) {
    switch (type) {
    case LO_BOARD_A1_TYPE:   return("A1");
    case LO_BOARD_A2_TYPE:   return("A2");
    case LO_BOARD_B1_TYPE:   return("B1");
    case LO_BOARD_B2_TYPE:   return("B2");
    case LO_BOARD_C_TYPE:    return("C");
    case LO_BOARD_D_TYPE:    return("D");
    case LO_BOARD_E_TYPE:    return("E");
    case LO_BOARD_F_TYPE:    return("F");
    case LO_BOARD_UNKNOWN_TYPE: 
    default:                 return("??");
    }
}

int computeMonthFrom3CharString(char *month) {
  int mon;
  if (!strcmp("Jan", month))
    mon = 0;
  else if (!strcmp("Feb", month))
    mon = 1;
  else if (!strcmp("Feb", month))
    mon = 1;
  else if (!strcmp("Mar", month))
    mon = 2;
  else if (!strcmp("Apr", month))
    mon = 3;
  else if (!strcmp("May", month))
    mon = 4;
  else if (!strcmp("Jun", month))
    mon = 5;
  else if (!strcmp("Jul", month))
    mon = 6;
  else if (!strcmp("Aug", month))
    mon = 7;
  else if (!strcmp("Sep", month))
    mon = 8;
  else if (!strcmp("Oct", month))
    mon = 9;
  else if (!strcmp("Nov", month))
    mon = 10;
  else
    mon = 11;
  return(mon);
}

int oldDate(char *string, int rightnow) {
  int day,year,monthNumber;
  char month[10];
  float yearFraction, presentYearFraction;
  struct tm *now;

  sscanf(string,"%2d%3s%2d",&day,month,&year);
  year += 2000;
#if 0
  printw("%2d%3s%2d= ",day,month,year);
#endif
  monthNumber = computeMonthFrom3CharString(month);
  yearFraction = computeYearFraction(day,monthNumber,year);
#if 0
  printw("%.3f =",yearFraction);
#endif
  /* Now compare it to the present time */
  now = gmtime(&rightnow);
  presentYearFraction = computeYearFraction(now->tm_mday, now->tm_mon, 1900+now->tm_year);  
  if (fabs(presentYearFraction - yearFraction) > 0.1) {
    return(1);  
  } else {
    return(0);  
  }
}

float computeYearFraction(int day, int month, int year) {
  /* day should be 1..31, month should be 0..11, year should be >2000 */
  /* returns a value such as 2005.0534 (for some time in January 2005) */
  float doy, yearFraction; 
  int m;

  doy = day;
  for (m=0; m<month; m++) {
    doy += 30;
    if (m==2) {
      doy--;
      if ((year % 4) != 0) {
	doy--;
      }
    }
    if (m==1 || m==3 || m==5 || m==7 || m==8 || m==10) {
      doy++;
    }
  }  
  if ((year % 4) == 0) {
    yearFraction = year + doy/366;
  } else {
    yearFraction = year + doy/365;
  }
  return(yearFraction);
}

int translateBandNumberToGHz(int band) {
  switch (band) {
  case 0: return(230);
  case 1: return(230);
  case 2: return(345);
  case 3: return(345);
  case 4: return(400);
  case 5: return(490);
  case 6: return(690);
  case 7: return(183); /* could be 800 in the future */
  default: return(0);
  }
}

void openDsmUPSStructures(void) {
  int rms,i;
  for (i=1; i<=8; i++) {
    rms = dsm_structure_init(&upsStructure[i],"UPS_DATA_X");
    if (rms != DSM_SUCCESS) {
      dsm_error_message(rms,"dsm_structure_init(UPS_DATA_X,acc)");
    }
  }
  
//   rms = dsm_structure_init(&upsStructureObscon[0],"UPS_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS_DATA_X,obscon)");
//   }
//   rms = dsm_structure_init(&upsStructureObscon[1],"UPS2_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS2_DATA_X,obscon)");
//   }
//   rms = dsm_structure_init(&upsStructureObscon[2],"UPS3_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS3_DATA_X,obscon)");
//   }
//   rms = dsm_structure_init(&upsStructureObscon[3],"UPS4_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS4_DATA_X,obscon)");
//   }
//     rms = dsm_structure_init(&upsStructureObscon[4],"UPS5_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS5_DATA_X,obscon)");
//   }
//   rms = dsm_structure_init(&upsStructureObscon[5],"UPS6_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS6_DATA_X,obscon)");
//   }
  rms = dsm_structure_init(&upsStructureObsconX,"UPS_ENET_DATA_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(UPS_ENET_DATA_X,obscon)");
  }


  
//   rms = dsm_structure_init(&upsStructureColossus[0],"UPS_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS_DATA_X,colossus)");
//   }
//   rms = dsm_structure_init(&upsStructureColossus[1],"UPS2_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS2_DATA_X,colossus)");
//   }
//   rms = dsm_structure_init(&upsStructureColossus[2],"UPS3_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS3_DATA_X,colossus)");
//   }
//   rms = dsm_structure_init(&upsStructureColossus[3],"UPS4_DATA_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(UPS4_DATA_X,colossus)");
//   }
   }

void openDsmMeteorologyStructures(void) {
  int rms;
//   rms = dsm_structure_init(&sma5Weather,"SMA_METEOROLOGY5_X");
//   if (rms != DSM_SUCCESS) {
//     dsm_error_message(rms,"dsm_structure_init(SMA_METEOROLOGY5_X)");
//   }
  rms = dsm_structure_init(&AntWeather,"ANT_WEATHER_DATA_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(ANT_WEATHER_DATA_X)");
  }
  rms = dsm_structure_init(&smaWeather,"SMA_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(SMA_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&jcmtWeather,"JCMT_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(JCMT_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&subaruWeather,"SUBARU_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(SUBARU_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&keckWeather,"KECK_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(KECK_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&irtfWeather,"IRTF_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(IRTF_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&cfhtWeather,"CFHT_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(CFHT_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&ukirtWeather,"UKIRT_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(UKIRT_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&uh88Weather,"UH88_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(UH88_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&csoWeather,"CSO_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(CSO_METEOROLOGY_X)");
  }
  rms = dsm_structure_init(&vlbaWeather,"VLBA_METEOROLOGY_X");
  if (rms != DSM_SUCCESS) {
    dsm_error_message(rms,"dsm_structure_init(VLBA_METEOROLOGY_X)");
  }
}

