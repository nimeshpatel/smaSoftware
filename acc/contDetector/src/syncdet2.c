/*********************************************************************
syncdet.c

Some changes made to allow the code to run on acc9 and acc10, which
do not have the full hardware complement.   RTM 03/26/08
 
Based on Nimesh Patel's syncdet, but uses the new cont det V/F interface.
Bob Wilson 7/9/2004
 
Last mod by Nimesh:
5 Mar 2004
Added continuum detector calibration (square-law) according to derek
and bill's coefficients which are read at start-up from a file called
contDetCalibration.conf from /instance/configFiles area.
The power values are written to new RM variables.
**********************************************************************/
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <ieeefp.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include <smem.h>
#include <pthread.h>
#include <unistd.h>
#include "rm.h"
#include "dsm.h"
#include "tsshm.h"
#include <resource.h>

#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <semaphore.h>
#include <patchPanelBits.h>
#include "smadaemon.h"
#include "contDetector.h"
#include "syncclock32.h"
#include "vme_sg_simple.h"
#include "stderrUtilities.h"

/* for hostname */
#include <sys/utsname.h>

/* Controls whether the monitor thread is run.  This may be useful
 * during initial debugging so Nimesh's syncdet can be run at the same time */
#define MONITORALL 0
/* Track the chopper's timing and derive the average period and transition
 * times for each ot the 4 transitions, averaged so autocorrelation can be
 * done chopping with the crates using this data to predict the transition
 * times */
#define DO_TSYS 1
#define DO_CHOPPER_TIMING 0
#define SYNCDET_WRITES_SCANFILE 0
#define DO_CHECK_DEAD_THREAD 0

/* Set up buffers for the samples as they come in.  The same space is used
 * whether chopping or just measuring the continuum level.  By storing
 * the samples for a second, we can look for glitches and ignore them while
 * averaging the samples.  The results will be written out once per second.
 */
/* COUNTTIME is the time in microsec for the counters to count the V/Fs */
#define COUNTTIME 10000
#define ICOUNT_MAX (1000000/COUNTTIME)
#if COUNTTIME < 100000
#    define CHECK_FLAG_MOD (100000/COUNTTIME)
#else
#    define CHECK_FLAG_MOD 1
#endif
/* The V/F converter is an AD7742, is synchronous and runs from a 4 MHz
 * oscillator.  Its output is supposed to be between 0.05*Fin and .45*Fin,
 * but it actually seems to be closser to 260kHz for zero power */
#define ZERO_COUNT (0.228*COUNTTIME)

#define CHOPMODE 0
#define FREERUNMODE CONT_DET_MODE_W

#define TRUE 1
#define FALSE 0

#define PRIORITY 100

#if DO_TSYS
#define TSYS_PRIORITY 19
#endif /* DO_TSYS */

#if MONITORALL
#define MONITOR_PRIORITY 18
/* The following define is from logTilts.c */
#define N_TILTS 4
#define SPIKETHRES 3.0

/****from yIGTuneServer**************/
#define QUAD_CHANNEL_1   0
#define STRESS_CHANNEL_1 1
#define POWER_CHANNEL_1    2
#define QUAD_CHANNEL_2   3
#define STRESS_CHANNEL_2 4
#define POWER_CHANNEL_2     5
#define MRG_RF_POWER      6
#define MRG_OP_POWER    7
#define CONT_DET1         0
#define FO_XMITTER_DET    1
#define D109              2
#define FO_TRANS_OP_ALARM 3
#define FO_TRANS_TP_ALARM 4
#define FO_RX_OP_ALARM    5
/************************************/
#endif /* MONITORALL */


/*********************************************************************/
#define DEBUG 0
#define ADC_CHANNEL 1
#define MAX 10000
/*********************************************************************/

/* global variables */
TrackServoSHM *tsshm;
pthread_attr_t thread_attr;
struct sched_param thread_sched;
#if MONITORALL
pthread_t   readADCLoopTID;
#endif /* MONITORALL */
#if DO_TSYS
pthread_t   updateTsysTID;
#endif /* DO_TSYS */
int policy = SCHED_FIFO;
int antlist[RM_ARRAY_SIZE];
int if1fd, if2fd, yigfd;
int antenna=0;
int oldContDetMode = -1, oldChoppingFlag = -1;
int sc32fd = -1;		/* File descriptor for the syncclock32 device */
struct sc32_time sctime;
int ttfd = -1;			/* File descriptor for the TrueTime device */
struct vme_sg_simple_time ttime;
int shutdownSignal = 0;
int gotQuit = 0;
short int secSamples[2][60];
int cycleCount = 0;
int prevCycleCount = 0;
int shortInPosCount = 0;
int srcPosCount = 0, refPosCount;

#if DO_CHOPPER_TIMING
/* Things for keeping track of chopper timing */
#define TR_0_1 1
#define TR_1_0 4
#define TR_0_2 2
#define TR_2_0 8
#define CHOPPER_CYCLES_PER_WRITE 40
/* the dsm variable  0-3 are transition times, 4 is period, 5 is error flag */
double dsmChopperTiming[6];
double chopperTiming[4] = {0, 0, 0, 0};
double chopperPeriod = 0.214;
double curTime;
double newTime[4];
int chopperStarting;
#endif /* DO_CHOPPER_TIMING */

int oldDay = -1;
int unixTime;
int unixTime0Today;
int sDay;
int tstate;
int trNum;
int dsmOpen = -1;
int recordTiming = 0;

/* Variables for gathering the 20 msec samples */
#define MSEC_SAMPLE_COUNT 200
#define MSEC_COUNTTIME 20000
#define SUBSAMPLE_COUNT (MSEC_COUNTTIME / COUNTTIME)
int subSample = 0, mSecSample = 0, mSecL = 0, mSecH = 0;
short int mSecSamples[2][MSEC_SAMPLE_COUNT];

/* Coefficients for correcting thedetector laws */
char detLawFileName[] = "/instance/configFiles/contDetCalibration.conf";
enum DETECTORS {C1D1 = 0, C2D1, FREQLO, FREQHI, NUMDET};
struct {
  char name[12];
  float c[4];
}
coef[NUMDET] = {
                 {"CONT1DET1", {0,1,0,0}},
                 {"CONT2DET1", {0,1,0,0}},
                 {"FreqLo", {0,1,0,0}},
                 {"FreqHi", {0,1,0,0}}
               };

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;                               /* continuum detector fd */
contDetectorioc_t cdIoctlArg = {IND, COUNTTIME};
contDetector_result_t cdOut;
int verbose = 0;

#if MONITORALL
/* for ADC values*/
int xycomfd[2];
int voltsfd;
short adcdata;
float data;
float fData[2];
#endif /* MONITORALL */

#if DO_CHECK_DEAD_THREAD
pthread_t checkDeadThreadTID;
#endif /* DO_CHECK_DEAD_THREAD */

/* Things for the Allan Variance calculation thread */
#define AV_PRIORITY 17
#define NUM_AV_SAMPLES 1200
#define AV_TIME 100000
#define MUWATT_TO_SHORT 10000
#define MAX_AV_DEPTH 10
pthread_t avThreadTID;
sem_t avSem;
int avSample = 0;
float avSamples[2][NUM_AV_SAMPLES];
double avSampleSumL = 0, avSampleSumH = 0;

/* syncdet2.c */
#if DO_CHOPPER_TIMING
void ProcessChopperTimes(void);
#endif /* DO_CHOPPER_TIMING */
void GetUnixT0Today(void);
void *readADCLoop();
float readChannel(int board,int channel);
#if DO_CHECK_DEAD_THREAD
void *CheckDeadThread(void *param);
#endif /* DO_CHECK_DEAD_THREAD */
void *avThread(void *);
static void SigIntHndlr(int);
static void SigQuitHndlr(int signo);
void ReadDetCoeff(void);
#if DO_TSYS
void updateTsys(void);
#endif /* DO_TSYS */

int main(int argc, char *argv[]) {
#if DEBUG
  FILE *fp;
#endif

  /* for chopper bits */
  int tiltFlag=0;
  int prevTiltFlag = -1;
  int chopBitsfd = -1;
  int chopPosBits;

  int nOn = 0, nOff = 0;	/* Number of on and off samples so far */
  int onSumL = 0, onSumH = 0, offSumL = 0, offSumH = 0;
  double onPwrL = 0, onPwrH = 0, offPwrL = 0, offPwrH = 0;
  double d, diffL, diffH;
  double onSsqL = 0, onSsqH = 0, offSsqL = 0, offSsqH = 0;
  int maxOnL = -1, minOnL = 1000000, maxOnH = -1, minOnH = 1000000;
  int maxOffL = -1, minOffL = 1000000, maxOffH = -1, minOffH = 1000000;
  double azTrErrSum = 0, elTrErrSum = 0;
  double dataL, dataH, rmsL, rmsH;
#if SYNCDET_WRITES_SCANFILE
  float azMod, elMod;
  double azOff, elOff;
#endif
  float syncdetChannels[2], syncdetStats[6], contDetMuWatt[2];
  float ifLoThermistors[16];
  float tempFactorL = 1, tempFactorH = 1;
  int rm_status, status;
  int iCount, curMsec, dt;
  int cdMode;
  int warned = 0;
  int RefPosMissingCount = 0;
#if SYNCDET_WRITES_SCANFILE
  short rscanFlag;
  FILE *scanfp = NULL;
#endif
  struct utsname unamebuf;	/* for hostname */
  short choppingFlag = 0;
  int avSumL, avSumH, nav;
  int i;
  int avWaitCount = 0;


  /********Initializations**************************/
  if(argc > 1 && strncmp(argv[1], "-v", 2) == 0)
    verbose++;
  DAEMONSET
  setpriority(PRIO_PROCESS,0,PRIORITY);
  tsshm = OpenShm(TSSHMNAME, TSSHMSZ);
  ReadDetCoeff();

  if(signal(SIGINT, SigIntHndlr) == SIG_ERR) {
    fprintf(stderr, "Error setting INT signal disposition\n");
    exit(SYSERR_RTN);
  }
  if(signal(SIGQUIT, SigQuitHndlr) == SIG_ERR) {
    fprintf(stderr, "Error setting QUIT signal disposition\n");
    exit(SYSERR_RTN);
  }
  if(signal(SIGTERM, SigQuitHndlr) == SIG_ERR) {
    fprintf(stderr, "Error setting TERM signal disposition\n");
    exit(SYSERR_RTN);
  }

  /* Get antenna number from hostname */
  uname (&unamebuf);
  sscanf(unamebuf.nodename, "acc%d", &antenna);
  if(antenna <= 0 || antenna > 10) {
    fprintf(stderr, "Hostname %s does not match acc1-10, I quit",
            unamebuf.nodename);
    exit(QUIT_RTN);
  }
#if MONITORALL
  /* Open all A/D channels */
 if(antenna < 9) {
  if(antenna == 1) {
    voltsfd = open("/dev/iPOptoAD16_6000", O_RDONLY);
    if (voltsfd < 0) {
      perror("Opening iPOptoAD16_6000, Can not read CD Volts");
    }
  } else {
    for (i = 0; i < 2; i++) {
      static char nodeName[] = "/dev/xVME564-4";

      xycomfd[i] = open(nodeName, O_RDONLY);
      nodeName[sizeof(nodeName) - 2]++;
      if (xycomfd[i] < 0) {
        perror("Opening XyCom, Can not read CD Volts");
      }
    }
  }	/* Cont det volts not available in ant 9 and 10 (JCMT and CSO) */

  if1fd = open("/dev/iPOptoAD16_2", O_RDONLY);
  if(if1fd<0) {
    perror("open error on /dev/iPOptoAD16_2");
    exit(-1);
  }

  yigfd = open("/dev/iPOptoAD16_1", O_RDONLY);
  if(yigfd<0) {
    perror("open error on /dev/iPOptoAD16_1");
    exit(-1);
  }

  if2fd = open("/dev/iPOptoAD16_3", O_RDONLY);
  if(if2fd<0) {
    perror("open error on /dev/iPOptoAD16_3");
    exit(-1);
  }

#if DO_CHOPPER_TIMING
  /* Open dsm for reporting chopper timing */
  if((dsmOpen = dsm_open()) != DSM_SUCCESS) {
    dsm_error_message(dsmOpen, "Dsm open failed");
  }
#endif /*DO_CHOPPER_TIMING */
 }  /* if antenna < 9 */
#endif /* MONITORALL */

  /* Open the continuum detector device */
  if((cdfd = open(cdName, O_RDONLY, 0)) < 0) {
    perror("Can not open contDetector0, I quit");
    exit(1);
  }
  ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
  /* initializing ref. mem. */
  rm_status=rm_open(antlist);
  if(rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"rm_open()");
    exit(1);
  }
  if(antenna < 9) {
    /* open and set up the UniDig device to read chopper bits */
    chopBitsfd = open("/dev/iPUniDig_D", O_RDONLY);
    if (chopBitsfd < 0) {
      perror("Can not open iPUniDig_D, I quit");
      exit(-1);
    }

    sc32fd = open("/dev/syncclock32", O_RDWR, 0);
    if(sc32fd <= 0) {
      ttfd = open("/dev/vme_sg_simple", O_RDWR, 0);
      if(ttfd <= 0) {
        fprintf(stderr,
                "Error opening TrueTime - /dev/vme_sg_simple\n");
        exit(SYSERR_RTN);
      }
    }
  }

#if DEBUG
  /*    fp=fopen("/instance/chopper.dat","w"); */
  fp = stdout;
#endif

  if((status = pthread_attr_init(&thread_attr)) != 0) {
    fprintf(stderr, "pthread_attr_init returned %s\n", strerror(status));
  }
  pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
  pthread_attr_setinheritsched(&thread_attr, PTHREAD_EXPLICIT_SCHED);

  /* end of initializations */

#if DO_TSYS
  if(antenna < 9) {
    thread_sched.sched_priority = TSYS_PRIORITY;
    if((status = pthread_attr_setschedparam(&thread_attr, &thread_sched))
          != 0) {
      fprintf(stderr, "set sched param for tsys failed - %s\n",
            strerror(status));
    }
    if((status = pthread_create(&updateTsysTID, &thread_attr, updateTsys,
               (void *) 0)) < 0) {
      fprintf(stderr, "main: pthread_create updateTsys failed - %s\n",
            strerror(status));
      exit(-1);
    }
  }
#endif /* DO_TSYS */

#if MONITORALL
  /* Starting the readADCLoop thread to read the ADC values and write
  * them to RM every second. This is a lower priority thread running
  * at priority of 16
  */
  if(antenna < 9) {
    thread_sched.sched_priority = MONITOR_PRIORITY;
    if((status = pthread_attr_setschedparam(&thread_attr, &thread_sched))
          != 0) {
      fprintf(stderr, "set sched param for avThread failed - %s\n",
            strerror(status));
    }
    if((status = pthread_create(&readADCLoopTID, &thread_attr, readADCLoop,
               (void *) 0)) < 0) {
      fprintf(stderr, "main: pthread_create readADCLoop failed - %s\n",
            strerror(status));
      exit(-1);
    }
  }
#endif /* MONITORALL */

  if(sem_init(&avSem, 1, 0) < 0)
    perror("initializing avSem");
  thread_sched.sched_priority = AV_PRIORITY;
  if((status = pthread_attr_setschedparam(&thread_attr, &thread_sched))
          != 0) {
    fprintf(stderr, "set sched param for avThread failed - %s\n",
            strerror(status));
  }
#if DO_CHECK_DEAD_THREAD
  if ((status = pthread_create(&checkDeadThreadTID, &thread_attr,
		 &CheckDeadThread, (void *) 0)) < 0) {
    fprintf(stderr, "main: pthread_create for Check Dead Thread failed"
	      "- %s\n", strerror(status));
  }
#endif /* DO_CHECK_DEAD_THREAD */
  if ((status = pthread_create(&avThreadTID, &thread_attr, &avThread,
		 (void *) 0)) < 0) {
    fprintf(stderr, "main: pthread_create for Allan Var Thread failed"
	      "- %s\n", strerror(status));
  }
  iCount = 0;

  for(;;) {

    /* This code should be executed on the first pass, but only for
     * SMA antennas */
    if((iCount++ % CHECK_FLAG_MOD) == 0 && antenna < 9)  {
      rm_status=rm_read(RM_ANT_0,"RM_CHOPPING_FLAG_S", &choppingFlag);
      if(rm_status!=RM_SUCCESS) {
        rm_error_message(rm_status,"rm_read()");
        exit(1);
      }
      /* Read Continuum detector interface mode.  This is a trick read.
      * The length of 1 asks for a readback of the output bits rather
      * than the usual input bits of the unidig.
      */
      if ((status = read(chopBitsfd, (char *)(&cdMode), 1)) < 0) {
        perror("Reading chopper mode");
        exit(-1);
      }
      cdMode &= CONT_DET_MODE_W;
      if(cdMode == CHOPMODE) {
        if(--warned <= 0) {
          fprintf(stderr, "Syncdet: no support for CHOPMODE yet\n");
          warned = 61;
        }
        sleep(1);
        continue;
      }
      warned = 0;

      if(choppingFlag != oldChoppingFlag || cdMode != oldContDetMode) {
        cdIoctlArg.mode = (cdMode == CHOPMODE)? CHOP: IND;
        cdIoctlArg.intTime = COUNTTIME;
	ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
        oldContDetMode = cdMode;
        oldChoppingFlag = choppingFlag;
	avSumL = 0; avSumH = 0; nav = 0;
	avSample = 0; avSampleSumL = 0; avSampleSumH = 0;
#if DO_CHOPPER_TIMING
        chopperStarting = 20;
        if(choppingFlag == 0) {
          for(i = 0; i < 4; i++)
            dsmChopperTiming[i] = _NAN;
        }
        recordTiming = 1;
#endif /* DO_CHOPPER_TIMING */
      }
    }
    if(read(cdfd, &cdOut, sizeof(cdOut)) < 0) {
      ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
      perror("Reading cont det");
    }
    cycleCount++;
    if(cdOut.timer != 0) {
      fprintf(stderr, "Incomplete timer count %d\n", cdOut.timer);
      continue;
    }
    if(shutdownSignal) {
      exit((gotQuit)? QUIT_RTN: NORMAL_RTN);
    }
    /* Read the TrueTime and convert the time */
    if(antenna < 9) {
      if(sc32fd > 0) {
        if((status = read(sc32fd, &sctime, sizeof(sctime))) < 0) {
          fprintf(stderr, "Error %d reading Syncclock32\n", status);
        }
        curMsec = (sDay = ((sctime.hour * 60 + sctime.min) * 60 +
                   sctime.sec)) * 1000 + sctime.usec / 1000;
        if(oldDay != sctime.yday) {
          GetUnixT0Today();
        }
	unixTime = unixTime0Today + sDay;
      } else {
        if((status = read(ttfd, &ttime, sizeof(ttime))) < 0) {
          fprintf(stderr, "Error %d reading TrueTime\n", status);
        }
        curMsec = ((ttime.hour * 60 + ttime.min) * 60 + ttime.sec) * 1000 +
                  ttime.usec / 1000;
        rm_read(RM_ANT_0, "RM_UNIX_TIME_L", &unixTime);
      }
    } else {
      struct timeval tv;
      struct timezone tz;

      gettimeofday(&tv, &tz);
      curMsec = (tv.tv_sec % 86400) * 1000 + tv.tv_usec / 1000;
      rm_read(RM_ANT_0, "RM_UNIX_TIME_L", &unixTime);
    }
    if(choppingFlag != 0) {	/* choppingFlag is always 0 on 9 and 10 */
      /* read the chopper position bits */
      if ((status = read(chopBitsfd, (char *)(&chopPosBits), 4)) < 0) {
        perror("Reading chopper position bits");
        exit(-1);
      }

#if TRIGGER_LEVEL_R | TRIGGER_PULSE_R == 0x30
      tiltFlag = 3 & (chopPosBits >> 4);
#else

      We need to change the formula above if this is reached
#endif

      if(tiltFlag == 3) {		/* Should not be in both positions */
	sendOpMessage(OPMSG_SEVERE, 19, 60,
		"Impossible chopper position at least one position bit stuck");
	goto nodata;
      }
      /* Is the data valid? */
      if(tiltFlag!=0 && tiltFlag==prevTiltFlag) {	/*  data is valid */
        if(tiltFlag==1) {
          onSumL += cdOut.lowRx;
          onSsqL += cdOut.lowRx * cdOut.lowRx;
          if(cdOut.lowRx > maxOnL)
            maxOnL = cdOut.lowRx;
          if(cdOut.lowRx < minOnL)
            minOnL = cdOut.lowRx;
          onSumH += cdOut.highRx;
          onSsqH += cdOut.highRx * cdOut.highRx;
          if(cdOut.highRx > maxOnH)
            maxOnH = cdOut.highRx;
          if(cdOut.highRx < minOnH)
            minOnH = cdOut.highRx;
          nOn++;
        } else {
          offSumL += cdOut.lowRx;
          offSsqL += cdOut.lowRx * cdOut.lowRx;
          if(cdOut.lowRx > maxOffL)
            maxOffL = cdOut.lowRx;
          if(cdOut.lowRx < minOffL)
            minOffL = cdOut.lowRx;
          offSumH += cdOut.highRx;
          offSsqH += cdOut.highRx * cdOut.highRx;
          if(cdOut.highRx > maxOffH)
            maxOffH = cdOut.highRx;
          if(cdOut.highRx < minOffH)
            minOffH = cdOut.highRx;
          nOff++;
        }
        azTrErrSum += tsshm->azTrError;
        elTrErrSum += tsshm->elTrError;

      } else if(prevTiltFlag != tiltFlag) {

#if DO_CHOPPER_TIMING
        if(chopperStarting > 10) {
          chopperStarting--;
          trNum = 0;
          tstate = 0;
          goto cs1;
        }
        curTime = unixTime0Today + sDay + sctime.usec / 1e6;
        /* printf("tf = %d pf = %d tstate = %d %.6f\n", tiltFlag, prevTiltFlag, tstate, curTime); */
        switch(tiltFlag + prevTiltFlag * 4) {
        case TR_0_1:
          if(tstate == 3) {
            ProcessChopperTimes();
            tstate = 0;
          }
          if(tstate == 0) {
            newTime[0] = curTime;
            tstate = 1;
          }
          break;
        case TR_1_0:
          newTime[1] = curTime;
          if(tstate == 1)
            tstate = 2;
          break;
        case TR_0_2:
          if(tstate == 2) {
            newTime[2] = curTime;
            tstate = 3;
          }
          break;
        case TR_2_0:
          newTime[3] = curTime;
          break;
        default:
          tstate = 0;
        }
        trNum++;
cs1:
#endif /* DO_CHOPPER_TIMING */
        prevTiltFlag = tiltFlag;
      }
    } else {	/* Not chopping, accumulate in on array */
      onSumL += cdOut.lowRx;
      avSumL += cdOut.lowRx;
      onSsqL += cdOut.lowRx * cdOut.lowRx;
      if(cdOut.lowRx > maxOnL)
        maxOnL = cdOut.lowRx;
      if(cdOut.lowRx < minOnL)
        minOnL = cdOut.lowRx;
      onSumH += cdOut.highRx;
      avSumH += cdOut.highRx;
      onSsqH += cdOut.highRx * cdOut.highRx;
      if(cdOut.highRx > maxOnH)
        maxOnH = cdOut.highRx;
      if(cdOut.highRx < minOnH)
        minOnH = cdOut.highRx;
      azTrErrSum += tsshm->azTrError;
      elTrErrSum += tsshm->elTrError;
      nOn++;
      nav++;
    }
#if DEBUG
    if(nOn > 0) {
      dataL = onSumL / nOn;
      dataH = onSumH / nOn;
    } else {
      dataL = 0;
      dataH = 0;
    }
    /*	    if(nOn > 0 && nOff > 0) { */
    fprintf(fp,"%d %d %d %d %d %.4f %.4f\n",
            nOff + nOn, prevTiltFlag, tiltFlag, cdOut.lowRx,
            cdOut.highRx, dataL, dataH);
    /* (double)offSumL/nOff, (double)offSumH/nOff); */
    /*	    } */
#endif


    /* End of data collection part.
    * Process the 20 msec data.
    */
    mSecL += cdOut.lowRx;
    mSecH += cdOut.highRx;
    if(++subSample >= SUBSAMPLE_COUNT) {
      d = (double)mSecL / MSEC_COUNTTIME;
      dataL = coef[FREQLO].c[0] + d * (coef[FREQLO].c[1] +
              d * coef[FREQLO].c[2]);
      dataL *= tempFactorL;
      mSecSamples[0][mSecSample] = (short)(dataL * MUWATT_TO_SHORT);
      d = (double)mSecH / MSEC_COUNTTIME;
      dataH = coef[FREQHI].c[0] + d * (coef[FREQHI].c[1] +
              d * coef[FREQHI].c[2]);
      dataH *= tempFactorH;
      mSecSamples[1][mSecSample] = (short)(dataH * MUWATT_TO_SHORT);
      mSecL = mSecH = subSample = 0;
      if(++mSecSample >= MSEC_SAMPLE_COUNT) {
        rm_status = rm_write(RM_ANT_0,
                    "RM_SYNCDET2_20MSEC_SAMPLES_V2_V200_S", mSecSamples);
        rm_status = rm_write(RM_ANT_0,
                    "RM_SYNCDET2_20MSEC_SAMPLES_TIMESTAMP_L", &unixTime);
        mSecSample = 0;
      }
    }

    /* Check to see if it is the end of a
     * one second cycle and time to write out the data.  End the cycle if
     * the end of the next count would be > 500 msec after Track's
     * msecCmd.  msecCmd is updated once/sec, so after outputing
     * data ~500 msec after msecCmd, wait at least 1/2 sec. before
     * considering doing it again.  There will need to be a new term
     * in this test when we start using chopper mode.
     */
    if(iCount > ICOUNT_MAX - 50) {
      if((dt = curMsec - tsshm->msecCmd) < -10000) { /* day change */
        dt += 86400000;
      }
      if(dt > 3000) { /* Track is not running if > 3 sec. */
        dt = (curMsec % 1000) - (499 - COUNTTIME/1000);
      }
    } else {
      dt = 0;
    }
    /* printf("dt = %d\n", dt); */
    if((dt > (500 - COUNTTIME/1000)) || (iCount >= (ICOUNT_MAX + 1))) {
      rm_status=rm_read(RM_ANT_0,"RM_IFLO_THERMISTORS_V16_F",
                ifLoThermistors);
      if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status, "error reading IFLO_THERMISTORS");
        exit(1);
      }
      tempFactorL = 1 + coef[FREQLO].c[3] * (ifLoThermistors[2] +
                    ifLoThermistors[3] - 66)/2;
      tempFactorH = 1 + coef[FREQHI].c[3] * (ifLoThermistors[2] +
                    ifLoThermistors[3] - 66)/2;
#if SYNCDET_WRITES_SCANFILE
      rm_status=rm_read(RM_ANT_0,"RM_PMDEL_F",&elMod);
#endif
/*      printf("nOn = %d  nOff = %d\n", nOn, nOff); */
      if(nOn > 0) {
        diffL = d = (double)onSumL / nOn;	/* Average count */
        onSsqL = onSsqL / nOn - d * d;	/* Mean Square deviation */
        d /= COUNTTIME;	/* Convert to MHz for eqn. */
        onPwrL = coef[FREQLO].c[0] + d * (coef[FREQLO].c[1] +
                 d * coef[FREQLO].c[2]);
        onPwrL *= tempFactorL;

        diffH = d = (double)onSumH / nOn;
        onSsqH = onSsqH / nOn - d * d;
        d /= COUNTTIME;	/* Convert to MHz for eqn. */
        onPwrH = coef[FREQHI].c[0] + d * (coef[FREQHI].c[1] +
                 d * coef[FREQHI].c[2]);
        onPwrH *= tempFactorH;

        azTrErrSum /= (nOn + nOff) *1000.;	/* Avg in arcsec from */
        elTrErrSum /= (nOn + nOff) *1000.;	/* milliarcsec sum */
  	if(srcPosCount > 0) srcPosCount--;
      } else {
	if(srcPosCount == 4) {
	  sendOpMessage(OPMSG_SEVERE, 19, 20,
		"On Source position flag (0x10) from chopper missing");
	  fprintf(stderr, "MAIL=adiven@sma.hawaii.edu "
		"On Source position flag (0x10) from chopper missing");
	  srcPosCount = 24;
  	}
  	if(srcPosCount < 8) srcPosCount++;
      }
      if(nOff > 0) {
        diffL -= (d = (double)offSumL / nOff);
        offSsqL = offSsqL / nOff - d * d;
        d /= COUNTTIME;	/* Convert to MHz for eqn. */
        offPwrL = coef[FREQLO].c[0] + d * (coef[FREQLO].c[1] +
                  d * coef[FREQLO].c[2]);
        offPwrL *= tempFactorL;

        diffH -= (d = (double)offSumH / nOff);
        offSsqH = offSsqH / nOff - d * d;
        d /= COUNTTIME;	/* Convert to MHz for eqn. */
        offPwrH = coef[FREQHI].c[0] + d * (coef[FREQHI].c[1] +
                  d * coef[FREQHI].c[2]);
        offPwrH *= tempFactorH;
  	if(refPosCount > 0) refPosCount--;
      } else if(choppingFlag != 0) {
	if(refPosCount == 4) {
	  sendOpMessage(OPMSG_SEVERE, 19, 20,
		"Ref position flag (0x20) from chopper missing");
	  fprintf(stderr, "MAIL=adiven@sma.hawaii.edu "
		"Ref position flag (0x20) from chopper missing");
  	}
  	if(refPosCount < 8) refPosCount++;
      }
      if(choppingFlag != 0) {

	if(nOn < iCount/4 || nOff < iCount/4) {
	  if((shortInPosCount >> 2) == 4) {
#if 1
            fprintf(stderr, "Chopper not reaching or holding position well\n");
#else
	    sendOpMessage(OPMSG_WARNING, 15, 60,
		"Chopper not reaching or holding position well");
#endif
	    shortInPosCount = 20;
	  }
	  if(shortInPosCount < 32) shortInPosCount += 4;
	} else {
#if 0
	  if(shortInPosCount > 0 && shortInPosCount < 32) shortInPosCount--;
#else
	  if(shortInPosCount > 0) shortInPosCount--;
#endif
	}
        dataL = onPwrL - offPwrL;
        /* Average level for normalizing the RMS */
        d = (onSumL + offSumL)/(nOn + nOff) - ZERO_COUNT;
        rmsL = 0.5 * (sqrt(onSsqL * (COUNTTIME * 2000)) +
                      sqrt(offSsqL * (COUNTTIME * 2000))) / d;
        dataH = onPwrH - offPwrH;
        d = (onSumH + offSumH)/(nOn + nOff) - ZERO_COUNT;
        rmsH = 0.5 * (sqrt(onSsqH * (COUNTTIME * 2000)) +
                      sqrt(offSsqH * (COUNTTIME * 2000))) / d;
        /* Combine the max and min from on and off, correcting for
         * the difference in the averages */
        if(maxOnL < maxOffL + diffL)
          maxOnL = maxOffL + diffL;
        if(minOnL > minOffL + diffL)
          minOnL = minOffL + diffL;
        if(maxOnH < maxOffH + diffH)
          maxOnH = maxOffH + diffH;
        if(minOnH > minOffH + diffH)
          minOnH = minOffH + diffH;
        contDetMuWatt[0] = 0.5 * (onPwrL + offPwrL);
        contDetMuWatt[1] = 0.5 * (onPwrH + offPwrH);
      } else {
        dataL = onPwrL;
        d = onSumL/nOn - ZERO_COUNT;
        rmsL = sqrt(onSsqL * (COUNTTIME * 2000)) / d;
        dataH = onPwrH;
        d = onSumH/nOn - ZERO_COUNT;
        rmsH = sqrt(onSsqH * (COUNTTIME * 2000)) / d;
        contDetMuWatt[0] = onPwrL;
        contDetMuWatt[1] = onPwrH;
	shortInPosCount = 0;
	srcPosCount = 0;
	refPosCount = 0;
      }
#if SYNCDET_WRITES_SCANFILE
      rm_status=rm_read(RM_ANT_0,"RM_PMDAZ_F",&azMod);
      if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status, "error reading RM_PMDAZ_F");
        exit(1);
      }
      rm_status=rm_read(RM_ANT_0,"RM_PMDEL_F",&elMod);
      if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status, "error reading RM_PMDEL_F");
        exit(1);
      }
      rm_status=rm_read(RM_ANT_0,"RM_CHART_AZOFF_ARCSEC_D",&azOff);
      if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,
            "error reading RM_CHART_AZOFF_ARCSEC_D");
        exit(1);
      }
      rm_status=rm_read(RM_ANT_0,"RM_CHART_ELOFF_ARCSEC_D",&elOff);
      if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,
            "error reading RM_CHART_ELOFF_ARCSEC_D");
        exit(1);
      }
      /* Read the command flag from rscan */
      rm_status=rm_read(RM_ANT_0,"RM_RSCAN_SYNCDET_FLAG_S", &rscanFlag);
      if(rm_status!=RM_SUCCESS) {
        rm_error_message(rm_status,"rm_read()");
        exit(1);
      }
      if(rscanFlag >= 0 && rscanFlag < 4) { /* Need to record scan data */
        static int tries = 0;

        /* Scan file not open, so recording is just starting */
        if(scanfp == NULL) {
          char fname[100];
          int fd;

          rm_status=rm_read(RM_ANT_0,"RM_RSCAN_SYNCDET_FILENAME_C100",
                    fname);
          if(rm_status != RM_SUCCESS ||
                  (fd = open(fname, O_WRONLY | O_APPEND)) < 0 ||
                  (scanfp = fdopen(fd, "a")) == NULL) {
            if(fd < 0)
              close(fd);
            if(tries++ == 2) {
              fprintf(stderr, "Can not open %s for scan data\n",
                      fname);
            }
            goto noscanwrite;
          }
          tries = 0;
        }
        fprintf(scanfp, "%10.6f %10.4f %10.4f %+6.2f %+6.2f %.3f "
                "%.3f %4.1f %4.1f %9.2f %9.2f %5.2f %5d, %5d\n",
                (tsshm->msec - 500)/(3600e3), tsshm->az / 3600000,
                tsshm->el / 3600000, azOff, elOff, dataL, dataH,
                azTrErrSum, elTrErrSum, azMod, elMod, rmsL, maxOnL, minOnL);
      } else {
        fclose(scanfp);
        tries = 0;
      }
      else {
        rm_status=rm_read(RM_ANT_0,"RM_RSCAN_SYNCDET_FLAG_S",
                  &rscanFlag);
        if(rm_status!=RM_SUCCESS) {
          rm_error_message(rm_status,"rm_read()");
          exit(1);
        }
      }
#endif /* SYNCDET_WRITES_SCANFILE */
      if(verbose) {
        printf("%6d %6d %4d %7.3f %6.2f %7.3f %6.2f\n", tsshm->msec,
               curMsec, iCount, dataL, rmsL, dataH, rmsH);
      }

      syncdetChannels[0] = dataL;
      syncdetChannels[1] = dataH;
      rm_status = rm_write(RM_ANT_0, "RM_SYNCDET2_CHANNELS_V2_F",
                  syncdetChannels);
      if(onSumL == 0) {}
      rm_status |= rm_write(RM_ANT_0, "RM_SYNCDET2_TIMESTAMP_L",
                   &unixTime);
      /* Record the samples on the half second and set the timestamp to
       * to the time of the first sample. */
      i = ((curMsec+30500)/1000) % 60;
      secSamples[0][i] = (short)(dataL * MUWATT_TO_SHORT);
      secSamples[1][i] = (short)(dataH * MUWATT_TO_SHORT);
      if(i == 59) {
	int unixTimeAtStart;

        rm_status |= rm_write(RM_ANT_0,
                     "RM_SYNCDET2_1SEC_SAMPLES_V2_V60_S", secSamples);
	unixTimeAtStart = unixTime - 59;
        rm_status = rm_write(RM_ANT_0,
                    "RM_SYNCDET2_1SEC_SAMPLES_TIMESTAMP_L", &unixTimeAtStart);
      }
      syncdetStats[0] = rmsL;
      syncdetStats[1] = minOnL;
      syncdetStats[2] = maxOnL;
      syncdetStats[3] = rmsH;
      syncdetStats[4] = minOnH;
      syncdetStats[5] = maxOnH;
      rm_status |= rm_write(RM_ANT_0, "RM_SYNCDET2_STATS_V6_F", syncdetStats);
      rm_status |= rm_write(RM_ANT_0, "RM_CONT_DET_MUWATT_V2_F", contDetMuWatt);
      if(rm_status != RM_SUCCESS)  {
        rm_error_message(rm_status,
            "error writing outputs");
      }

nodata:
      nOn = nOff = 0;
      onSumL = 0;
      onSumH = 0;
      offSumL = 0;
      offSumH = 0;
      onSsqL = 0;
      onSsqH = 0;
      offSsqL = 0;
      offSsqH = 0;
      maxOnL = -1;
      minOnL = 1000000;
      maxOnH = -1;
      minOnH = 1000000;
      maxOffL = -1;
      minOffL = 1000000;
      maxOffH = -1;
      minOffH = 1000000;
      azTrErrSum = 0;
      elTrErrSum = 0;
      iCount = 0;
    }
    if(nav >= (AV_TIME/COUNTTIME)) {
      char dummyByte;
      short dummyShort;
      static char m3State;
      static short calWheel;
      static int clearAV;

      /* Check to see if anything has changed which would disrupt the Allan
       * Variance calculation. */
      clearAV = 0;
      rm_read(RM_ANT_0, "RM_AZ_DRV_STATE_B", &dummyByte);
      if(dummyByte > 3) {		/* If slewing */
	clearAV = (1000000/AV_TIME);	/* Wait 1 sec before starting again */
      }
      rm_read(RM_ANT_0, "RM_EL_DRV_STATE_B", &dummyByte);
      if(dummyByte > 3) {		/* If slewing */
	clearAV = (1000000/AV_TIME);	/* Wait 1 sec before starting again */
      }
      rm_read(RM_ANT_0, "RM_LOADS_OUT_S", &dummyShort);
      if(dummyShort != calWheel) {
	calWheel = dummyShort;
	clearAV = (5000000/AV_TIME);	/* Wait 5 sec before starting again */
      }
      rm_read(RM_ANT_0, "RM_M3STATE_B", &dummyByte);
      if(dummyByte != m3State) {
	m3State = dummyByte;
	clearAV = (2000000/AV_TIME);	/* Wait 2 sec before starting again */
      }
      if(clearAV) {
        avSample = 0; avSampleSumL = 0; avSampleSumH = 0;
	if(clearAV > 0) clearAV--;
      } else if(avSample < NUM_AV_SAMPLES) {
        /* printf("%7.2f 0 0 0 %6d %6d\n", avSample*0.1, avSumL, avSumH); */
        d = avSumL/(double)(nav * COUNTTIME);
        avSamples[0][avSample] = d = (coef[FREQLO].c[0] +
            d * (coef[FREQLO].c[1] + d * coef[FREQLO].c[2]));
        avSampleSumL += d;
        d = avSumH/(double)(nav * COUNTTIME);
        avSamples[1][avSample] = d = (coef[FREQHI].c[0] +
            d * (coef[FREQHI].c[1] + d * coef[FREQHI].c[2]));
        avSampleSumH += d;
        avWaitCount = 0;
        avSample++;
        if(avSample == NUM_AV_SAMPLES) {
          sem_post(&avSem);
        }
      } else if(++avWaitCount > NUM_AV_SAMPLES) {
        fprintf(stderr,
                "Allan Var thread: no response, trying again\n");
        avSample = 0;
      }
      avSumL = 0;
      avSumH = 0;
      nav = 0;
    }
  } /* end of main loop */

#if DEBUG
  fclose(fp);
#endif

  return(0);
}				/* end of main Loop */

#if DO_CHOPPER_TIMING
void ProcessChopperTimes(void) {
  double delta[4], avgDelta = 0.0;
  int i;
  static int chCycleCount, badDelta = 0;

  if(chopperStarting) {
    for(i = 0; i < 4; i++) {
      chopperTiming[i] = newTime[i];
    }
    chopperStarting = 0;
    chCycleCount = 1;
    return;
  }
missedOne:
  for(i = 0; i < 4; i++) {
    delta[i] = newTime[i] - (chopperTiming[i] += chopperPeriod);
    avgDelta += delta[i];
  }
  if(avgDelta > chopperPeriod * 3) {
    badDelta += 1000;
    goto missedOne;
  }
  for(i = 0; i < 4; i++) {
    chopperTiming[i] += delta[i] / 30;
  }
  chopperPeriod += avgDelta / 8000;
  if((++chCycleCount %CHOPPER_CYCLES_PER_WRITE) == 0) {
    trNum += badDelta - CHOPPER_CYCLES_PER_WRITE * 4;
    if(verbose)
      printf("%d  %.5f %.5f %.5f %.5f %.6f %d\n", chCycleCount,
             chopperTiming[0], chopperTiming[1], delta[1], delta[2],
             chopperPeriod, trNum + badDelta);
    for(i = 0; i < 4; i++) {
      dsmChopperTiming[i] = chopperTiming[i];
    }
    dsmChopperTiming[4] = chopperPeriod;
    dsmChopperTiming[5] = trNum;
    trNum = 0;
    badDelta = 0;
    recordTiming = 1;
  }
}
#endif /* DO_CHOPPER_TIMING */

void GetUnixT0Today(void) {
  struct tm tm_time;

  bzero((char *)&tm_time, sizeof(tm_time));
  tm_time.tm_year = sctime.year - 1900;
  tm_time.tm_mon = 0;
  tm_time.tm_mday = sctime.yday;
  unixTime0Today = mktime(&tm_time);
  oldDay = sctime.yday;
}

#if MONITORALL
void *readADCLoop() {
  /* for ADC values*/
  int status;
  float fDummy=0.;
  float cont1Power=0.,cont2Power=0.;
  int i;
  double voltage;

  while(1) {

    /* The following ADC channels are for patch-panel channels 4 and 5 */

    if(antenna == 1) {
     if(voltsfd > 0) {
      for (i = 0; i < 2; i++) {
        *((int *)&data) = i + N_TILTS;
        if(read(voltsfd, (char *)(&data), 4) != 4) {
          fprintf(stderr, "syncdet2: Bad read of ContDet Volts ch %d\n",
                  i + N_TILTS);
        } else {
          fData[i] = -data;
        }
      }
     }
    }
    if(xycomfd[0] > 0) {
      for (i = 0; i < 2; i++) {
        if(read(xycomfd[i], (char *)(&adcdata), 2) != 2) {
          fprintf(stderr, "syncdet2: Bad read of XyCom ch %d read\n",
                  i + N_TILTS);
        } else {
          fData[i] = ((float)adcdata)*10.0/32768.0;
        }
      }
    } /* if antenna = 9 or 10, cont det volts not available */


    voltage = (double) fData[0];
    status = rm_write(RM_ANT_0, "RM_TOTAL_POWER_VOLTS_D", &voltage);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write()");
      exit(-1);
    }
    voltage = (double)fData[1];
    status = rm_write(RM_ANT_0, "RM_TOTAL_POWER_VOLTS2_D", &voltage);
    if (status != RM_SUCCESS) {
      rm_error_message(status, "rm_write()");
      exit(-1);
    }

    if(if1fd >= 0) {
      fDummy = readChannel(if1fd,FO_RX_OP_ALARM);
      status = rm_write(RM_ANT_0, "RM_109_200_RX1_ALARM_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-8)");
        exit(-1);
      }
      fDummy = readChannel(if1fd,FO_TRANS_TP_ALARM);
      status = rm_write(RM_ANT_0, "RM_XMIT1_TEMP_ALARM_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-7)");
        exit(-1);
      }
      fDummy = readChannel(if1fd,FO_TRANS_OP_ALARM);
      status = rm_write(RM_ANT_0, "RM_XMIT1_OP_ALARM_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-6)");
        exit(-1);
      }
      fDummy = readChannel(if1fd,D109);
      status = rm_write(RM_ANT_0, "RM_109MHZ1_POWER_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-5)");
        exit(-1);
      }
      fDummy = readChannel(if1fd,FO_XMITTER_DET);
      status = rm_write(RM_ANT_0, "RM_CONT1_DET2_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-4)");
        exit(-1);
      }
      fDummy = readChannel(if1fd,CONT_DET1);
      status = rm_write(RM_ANT_0, "RM_CONT1_DET1_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: "
            "rm_write(cont1det1)");
        exit(-1);
      }

      cont1Power= coef[C1D1].c[0] +coef[C1D1].c[1]*fDummy +
                  coef[C1D1].c[2]*fDummy*fDummy;
      status = rm_write(RM_ANT_0, "RM_CONT1_DET1_POWER_MUWATT_F",
               &cont1Power);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: "
            "rm_write(cont1det1power)");
        exit(-1);
      }
    }

    if(yigfd >= 0) {
      fDummy = readChannel(yigfd, MRG_RF_POWER);
      status = rm_write(RM_ANT_0, "RM_MRG_RF_POWER_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "Yig RF Power: rm_write(-8)");
        exit(-1);
      }
      fDummy = readChannel(yigfd, MRG_OP_POWER);
      status = rm_write(RM_ANT_0, "RM_MRG_OP_POWER_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "Yig OP Power: rm_write(-8)");
        exit(-1);
      }
    }

    if(if2fd >= 0) {
      fDummy = readChannel(if2fd,FO_RX_OP_ALARM);
      status = rm_write(RM_ANT_0, "RM_109_200_RX2_ALARM_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-8)");
        exit(-1);
      }
      fDummy = readChannel(if2fd,FO_TRANS_TP_ALARM);
      status = rm_write(RM_ANT_0, "RM_XMIT2_TEMP_ALARM_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-7)");
        exit(-1);
      }
      fDummy = readChannel(if2fd,FO_TRANS_OP_ALARM);
      status = rm_write(RM_ANT_0, "RM_XMIT2_OP_ALARM_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-6)");
        exit(-1);
      }
      fDummy = readChannel(if2fd,D109);
      status = rm_write(RM_ANT_0, "RM_109MHZ2_POWER_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-5)");
        exit(-1);
      }
      fDummy = readChannel(if2fd,FO_XMITTER_DET);
      status = rm_write(RM_ANT_0, "RM_CONT2_DET2_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-4)");
        exit(-1);
      }
      fDummy = readChannel(if2fd,CONT_DET1);
      status = rm_write(RM_ANT_0, "RM_CONT2_DET1_F", &fDummy);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-3)");
        exit(-1);
      }
      cont2Power= coef[C2D1].c[0] +coef[C2D1].c[1]*fDummy +
                  coef[C2D1].c[2]*fDummy*fDummy;
      status = rm_write(RM_ANT_0, "RM_CONT2_DET1_MUWATT_F", &cont2Power);
      if (status != RM_SUCCESS) {
        rm_error_message(status, "yIGServer.stressLoop: rm_write(-3)");
        exit(-1);
      }
    }
#if DO_CHOPPER_TIMING
    if(recordTiming && dsmOpen == DSM_SUCCESS) {
      for(i =1; i <= 12; i++) {
        static char host[8] = "crate  ";
        static char warned[12];
        char msg[24];
        int dsm_status;

        sprintf(&host[5], "%d", i);
        dsm_status = dsm_write(host, "DSM_CHOPPER_TIMING_V6_D",
                     dsmChopperTiming);
        if(dsm_status != DSM_SUCCESS && !warned) {
          sprintf(msg, "dsm_write to %s", host);
          dsm_error_message(dsm_status, msg);
          warned[i - 1] = 1;
        } else {
          warned[i-1] = 0;
        }
      }
      recordTiming = 0;
    }
#endif /* DO_CHOPPER_TIMING */
    usleep(100000);
  } /* end of while loop */
  pthread_detach(readADCLoopTID);
  pthread_exit((void *) 0);
}


float readChannel(int board,int channel) {
  int   errorCode;
  float buff;

  *((int *)&buff) = channel;
  if ((errorCode = read(board, &buff, 4)) != 4) {
    fprintf(stderr, "Error %d returned from read on A/D\n", errorCode);
    return(0.0);
  }
  return(buff);
}

/****************************end syncdet.c******************************/
#endif /* MONITORALL */

void ReadDetCoeff(void) {
  FILE *fp;
  float tcoef[4];
  int i, n;
  char line[80], str[20];

  /* read in the continuum detector calibration coefficients */
  fp = fopen(detLawFileName, "r");
  if(fp == NULL) {
    perror("Opening the det coefficients file");
    exit(QUIT_RTN);
  }

  while(fgets(line, sizeof(line), fp) != NULL) {
    tcoef[0] = tcoef[2] = tcoef[3] = 0;
    tcoef[1] = 1;
    n=sscanf(line, "%s %f %f %f %f", str, tcoef, tcoef+1, tcoef+2, tcoef+3);
    if(n == 0 || *str == '#')
      continue;
    for(i = 0; i < NUMDET; i++) {
      if(strncmp(coef[i].name, str, strlen(coef[i].name)) == 0) {
        memcpy(coef[i].c, tcoef, sizeof(tcoef));
        goto foundline;
      }
    }
    fprintf(stderr, "syncdet2: unrecognized line in %s -\n%s",
            detLawFileName, line);

foundline:
  }
  fclose(fp);
  if(verbose) {
    for(i = 0; i < NUMDET; i++) {
      printf("%s  %10.5f %10.5f %10.5f %10.5f\n", coef[i].name,
             coef[i].c[0], coef[i].c[1], coef[i].c[2], coef[i].c[3]);

    }
  }
}

#if DO_CHECK_DEAD_THREAD
void *CheckDeadThread(void *param) {
  /*    unsigned int timeStamp, unixTime; */
  contDetectorioc_t myCdIoctlArg;

  sleep(10);
  prevCycleCount = cycleCount;
  for(;;) {
    static int warned = 0;

    sleep(1);
    if(cycleCount == prevCycleCount) {
      ioctl(cdfd, CONTDET_GETMODE, &myCdIoctlArg);
      if(ioctl(cdfd, CONTDET_RELEASEWAIT, NULL) < 0) {
        if(warned < 2) {
          fprintf(stderr, "syncdet2 stalled: releasing semaphore"
		    " failed status %d sem %d\n",
		    myCdIoctlArg.mode, myCdIoctlArg.intTime);
          warned++;
        }
      } else {
        if(warned == 0) {
          fprintf(stderr, "syncdet2 stalled: status %d  waitSem %d,"
		    " semaphore released\n",
		    myCdIoctlArg.mode, myCdIoctlArg.intTime);
          warned = 1;
        }
      }
    } else {
      warned = 0;
    }
    prevCycleCount = cycleCount;
  }
}
#endif /* DO_CHECK_DEAD_THREAD */

void *avThread(void *param) {
  double ssqL, ssqH, expvar, d;
  float av[2][MAX_AV_DEPTH];
  int stride, depth, nSamp, i;

  expvar = 1/sqrt(AV_TIME*2e3);	/* 1/sqrt(B*T), AV_TIME is usec */
  for(;;) {
    if(sem_wait(&avSem) < 0)
      perror("Waiting on avSem");
    ;
    nSamp = NUM_AV_SAMPLES;
    depth = 0;
    /* convert sample sums to smaple averages */
    avSampleSumL /= NUM_AV_SAMPLES;
    avSampleSumH /= NUM_AV_SAMPLES;
    /* printf("avgs: %8.5f %8.5f\n", avSampleSumL, avSampleSumH); */
    /* Normalize samples by dividing by the average and then subtract
     * one to get the normalized deviation from the average */
    for(i = 0; i < NUM_AV_SAMPLES; i++) {
      avSamples[0][i] = avSamples[0][i] / avSampleSumL - 1;
      avSamples[1][i] = avSamples[1][i] / avSampleSumH - 1;
    }
    for(stride = 1; stride < nSamp; stride *= 2) {
      if(depth >= MAX_AV_DEPTH) {
        av[0][MAX_AV_DEPTH - 1] = -999;
        av[1][MAX_AV_DEPTH - 1] = -888;
        break;
      }
      ssqL = ssqH = 0;
      nSamp -= stride;
      for(i = 0; i < nSamp; i++) {
        d = avSamples[0][i] - avSamples[0][i + stride];
        ssqL +=  d * d;
        avSamples[0][i] = (avSamples[0][i] +
            avSamples[0][i + stride])/2;
        d = avSamples[1][i] - avSamples[1][i + stride];
        ssqH +=  d * d;
        avSamples[1][i] = (avSamples[1][i] +
            avSamples[1][i + stride])/2;
      }
      av[0][depth] = sqrt(ssqL / (nSamp * 2)) / expvar;
      av[1][depth] = sqrt(ssqH / (nSamp * 2)) / expvar;
      /* printf("%6.2f  %7.3f %7.3f\n", stride * (AV_TIME/1e6), av[0][depth], av[1][depth]); */
      depth++;
    }
    rm_write(RM_ANT_0, "RM_SYNCDET2_ALLAN_VARIANCE_V2_V10_F", av);
    avSampleSumL = avSampleSumH = 0;
    avSample = 0;
  }
}

#if DO_TSYS
void updateTsys(void) {

  short loadsOut = 1;
  int loadsOutOk;
  int rm_status;
/*  static double sky_volts, sky_volts2; */
  float hot_load_volts, hot_load_volts2;
  float contDetMuWatt[2], ambient_load_temperature;
  double tsys, tsys2;
  short wheel;
  int timestamp;
#define sky_volts (contDetMuWatt[0])
#define sky_volts2 (contDetMuWatt[1])
#define USE_AVERAGING 0
#if USE_AVERAGING
#define AVG_LENGTH 2
  static int cycles = 0;
  static double tempArray[AVG_LENGTH];
  static double hotloadArray[AVG_LENGTH];
  static double hotloadArray2[AVG_LENGTH];
  static double skyArray[AVG_LENGTH];
  static double skyArray2[AVG_LENGTH];
  int i;
#endif

 for(;;) {
  /* compute and update the Tsys in RM */
  rm_status = rm_read(RM_ANT_0, "RM_LOADS_OUT_S", &loadsOut);
  if(loadsOut) {
    if(loadsOutOk < 1) loadsOutOk++;
  } else {
    loadsOutOk = -1;
  }

  /* don't update the sky voltage if any load is in the way */ 
  if(loadsOutOk > 0) {

    /* is syncdet2 stale?  (unixTime is from the syncdet thread) */
    rm_status=rm_read(RM_ANT_0,"RM_UNIX_TIME_L",&timestamp);
    if(timestamp - unixTime > 60) {
      rm_status=rm_read(RM_ANT_0,"RM_CONT1_DET1_POWER_MUWATT_F",
	&contDetMuWatt[0]);
      rm_status=rm_read(RM_ANT_0,"RM_CONT2_DET1_MUWATT_F",
	&contDetMuWatt[1]);
    } else {
      rm_status=rm_read(RM_ANT_0,"RM_CONT_DET_MUWATT_V2_F",&contDetMuWatt);
    }
  } else {
    goto TSYSEND;
  }
  /* At this point contDetMuWatt contains the low and high freq receiver
   * outputs on the sky. */
  rm_status=rm_read(RM_ANT_0,"RM_HOTLOAD_LOWFREQ_VOLTS_F",&hot_load_volts);
  rm_status=rm_read(RM_ANT_0,"RM_HOTLOAD_HIGHFREQ_VOLTS_F",&hot_load_volts2);
  /* Calvaneservo writes this after protecting against wacko values */
  rm_status=rm_read(RM_ANT_0,"RM_UNHEATEDLOAD_TEMPERATURE_F",
		    &ambient_load_temperature);

  /* This next block should be elliminated */
  rm_status=rm_write(RM_ANT_0,"RM_SKY_LOWFREQ_VOLTS_F",&contDetMuWatt[0]);
  rm_status=rm_write(RM_ANT_0,"RM_SKY_HIGHFREQ_VOLTS_F",&contDetMuWatt[1]);
  rm_status=rm_write(RM_ANT_0,"RM_SKY_LOWFREQ_VOLTS_TIMESTAMP_L", &timestamp);
  rm_status=rm_write(RM_ANT_0,"RM_SKY_HIGHFREQ_VOLTS_TIMESTAMP_L", &timestamp);

#if USE_AVERAGING
  if (cycles == 0) {
    /* fill the array with the first value */
    cycles = 1;
    for (i=0; i<avgLength; i++) {
      tempArray[i] = ambient_load_temperature;
      hotloadArray[i] = hot_load_volts;
      hotloadArray2[i] = hot_load_volts2;
      skyArray[i] = sky_volts;
      skyArray2[i] = sky_volts2;
    }
  } else {
    /* boxcar average the most recent values */
    for (i=0; i<(avgLength-1); i++) {
      tempArray[i] = tempArray[i+1];
      hotloadArray[i] = hotloadArray[i+1];
      hotloadArray2[i] = hotloadArray2[i+1] ;
      skyArray[i] = skyArray[i+1]; 
      skyArray2[i] = skyArray2[i+1]; 
    }
    tempArray[avgLength-1] = ambient_load_temperature;
    hotloadArray[avgLength-1] = hot_load_volts;
    hotloadArray2[avgLength-1] = hot_load_volts2;
    skyArray[avgLength-1] = sky_volts;
    skyArray2[avgLength-1] = sky_volts2;
    ambient_load_temperature = average(tempArray,avgLength);
    hot_load_volts =  average(hotloadArray,avgLength);
    hot_load_volts2 = average(hotloadArray2,avgLength);
    sky_volts = average(skyArray,avgLength);
    sky_volts2 = average(skyArray2,avgLength);
    if (DEBUG) {
      printf("avg(%d) load temp = %f (%f %f %f %f %f)\n",avgLength,ambient_load_temperature,
	     tempArray[0], tempArray[1], tempArray[2], tempArray[3], tempArray[4]);
    }
  }
#endif

  tsys = (273.15+(double)ambient_load_temperature) 
    * (contDetMuWatt[0]/(hot_load_volts-contDetMuWatt[0]));
  tsys2 = (273.15+(double)ambient_load_temperature) 
    * (contDetMuWatt[1]/(hot_load_volts2-contDetMuWatt[1]));
  if (DEBUG) {
     printf("tsys = %f\n",tsys);
  }
#if 1
  rm_status=rm_write(RM_ANT_0,"RM_TSYS_D",&tsys);
  rm_status=rm_write(RM_ANT_0,"RM_TSYS2_D",&tsys2);
  rm_status=rm_write(RM_ANT_0,"RM_TSYS_TIMESTAMP_L",&timestamp);
#else
printf("muwatt = %.3f ambLoad = %.2f hlVolts = %.3f Tsys = %.2f\n",
contDetMuWatt[0], ambient_load_temperature, hot_load_volts, tsys);
#endif
TSYSEND:
  sleep(1);
 }
}

#if USE_AVERAGING
double average(double *v, int n) {
  int i;
  double sum = 0;
  for (i=0; i<n; i++) {
    sum += v[i];
  }
  return(sum/n);
}
#endif /* USE_AVERAGING */

#endif /* DO_TSYS */

/* Subroutine to handle SIGINT (^C) interrupts and shut down gracefully */
static void SigIntHndlr(int signo) {
  shutdownSignal = 1;
}

/* Subroutine to handle SIGQUIT, SIGTERM & SIGHUP interrupts and shut
 * down gracefully */
static void SigQuitHndlr(int signo) {
  shutdownSignal = 1;
  gotQuit = 1;
}
