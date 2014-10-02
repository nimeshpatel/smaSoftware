/**********************************************************************
syncdet.c
 
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
#include <termio.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>
#include <smem.h>
#include <pthread.h>
#include <unistd.h>
#include "rm.h"
#include "tsshm.h"
#include <resource.h>

#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <patchPanelBits.h>
#include "smadaemon.h"
#include "contDetector.h"
#include "vme_sg_simple.h"

/* for hostname */
#include <sys/utsname.h>

/* Controls whether the monitor thread is run.  This may be useful
 * during initial debugging so Nimesh's syncdet can be run at the same time */
#define MONITORALL 0

/* Set up buffers for the samples as they come in.  The same space is used
 * whether chopping or just measuring the continuum level.  By storing
 * the samples for a second, we can look for glitches and ignore them while
 * averaging the samples.  The results will be written out once per second.
 */
/* COUNTTIME is the time in microsec for the counters to count the V/Fs */
#define COUNTTIME 5000
#define ICOUNT_MAX (1000000/COUNTTIME)
#if COUNTTIME < 100000
#    define CHECK_FLAG_MOD (100000/COUNTTIME)
#else
#    define CHECK_FLAG_MOD 1
#endif

#if 0
/* Allocate enough buffer space for a second of data, but have some
 * spare room in the buffer in case on and off times differ */
#define BUFLEN (1200000/COUNTTIME)
/* If the integration time is < 30 msec, a short will not overflow */
#if COUNTTIME > 30000
int lo[BUFLEN];	/* for the "low frequency" receiver */
int hi[BUFLEN];	/* for the "high frequency" receiver */
#else

unsigned short lo[BUFLEN];
unsigned short hi[BUFLEN];
#endif
#define LOWON lo
#define LOWOFF (&lo[BUFLEN/2])
#define HIGHON hi
#define HIGHOFF (&hi[BUFLEN/2])
#endif

#define CHOPMODE 0
#define FREERUNMODE CONT_DET_MODE_W

#define TRUE 1
#define FALSE 0

#define PRIORITY 20

#if MONITORALL
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
pthread_t   readADCLoopTID;
int policy = SCHED_FIFO;
int antlist[RM_ARRAY_SIZE];
int contADC2,contADC3;
int antenna=0;
int oldContDetMode = -1, oldChoppingFlag = -1;
int ttfd;			/* File descriptor for the TrueTime device */
struct vme_sg_simple_time ttime;

/* Coefficients for correcting thedetector laws */
char detLawFileName[] = "/instance/configFiles/contDetCalibration.conf";
enum DETECTORS {C1D1 = 0, C2D1, FREQLO, FREQHI, NUMDET};
struct {
    char name[12];
    float c[4];
} coef[NUMDET] = {
    {"CONT1DET1", {0,1,0,0}},
    {"CONT2DET1", {0,1,0,0}},
    {"FreqLo", {0,1,0,0}},
    {"FreqHi", {0,1,0,0}}
};

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;                               /* continuum detector fd */
contDetectorioc_t cdIoctlArg;
contDetector_result_t cdOut;
int verbose = 0;

#if MONITORALL
/* for ADC values*/
int fdADC[N_TILTS+2],nRead;
int fD;
short adcdata;
float data;
float fData[N_TILTS+3];

/* syncdet2.c */
void *readADCLoop();
float readChannel(int board,int channel);

#endif /* MONITORALL */
void ReadDetCoeff(void);

int main(int argc, char *argv[]) {
#if DEBUG
    FILE *fp;
#endif

    /* for chopper bits */
    int fdChopper;
    int chopPosBits;
#if MONITORALL

    char nodeName[80];
#endif /* MONITORALL */

    int tiltFlag=0;
    int prevTiltFlag = -1;
    int nOn = 0, nOff = 0;	/* Number of on and off samples so far */
    int onSumL = 0, onSumH = 0, offSumL = 0, offSumH = 0;
    double onPwrL = 0, onPwrH = 0, offPwrL = 0, offPwrH = 0;
    double diffL, diffH;
    double onSsqL = 0, onSsqH = 0, offSsqL = 0, offSsqH = 0;
    int maxOnL = -1, minOnL = 1000000, maxOnH = -1, minOnH = 1000000;
    int maxOffL = -1, minOffL = 1000000, maxOffH = -1, minOffH = 1000000;
    double azTrErrSum = 0, elTrErrSum = 0;
    double dataL, dataH, rmsL, rmsH;
    float azMod, elMod;
    double azOff, elOff;
    float syncdetChannels[2], syncdetStats[6], contDetMuWatt[2];
    int rm_status,status;
    int iCount, curMsec, dt;
    int cdMode;
    int warned = 0;
    short rscanFlag;
    FILE *scanfp = NULL;
    struct utsname unamebuf;	/* for hostname */
    short choppingFlag;
    int unixTime;

    /********Initializations**************************/
    if(argc > 1 && strncmp(argv[1], "-v", 2) == 0) verbose++;
    DAEMONSET
    setpriority(PRIO_PROCESS,0,PRIORITY);
    tsshm = OpenShm(TSSHMNAME, TSSHMSZ);

    ReadDetCoeff();
#if MONITORALL
    /* Open all A/D channels, first get antenna number from hostname */
    uname (&unamebuf);
    if(!strcmp(unamebuf.nodename,"acc1")) {
        antenna=1;
        sprintf(nodeName, "/dev/iPOptoAD16_6000");
        fD = open(nodeName, O_RDONLY);
        if (fD < 0) {
            perror("Open");
            exit(-1);
        }
    } else {	/* if acc1 */
        for (i = 0; i <= (N_TILTS+2); i++) {
            sprintf(nodeName, "/dev/xVME564-%d", i);
            fdADC[i] = open(nodeName, O_RDONLY);
            if (fdADC[i] < 0) {
                perror("Open");
                exit(-1);
            }
        }
    } /* if other than acc1 */

    sprintf(nodeName, "/dev/iPOptoAD16_3");
    contADC3 = open(nodeName, O_RDONLY);
    if(contADC3<0) {
        perror("open error on /dev/iPOptoAD16_3");
        exit(-1);
    }

    sprintf(nodeName, "/dev/iPOptoAD16_2");
    contADC2 = open(nodeName, O_RDONLY);
    if(contADC2<0) {
        perror("open error on /dev/iPOptoAD16_2");
        exit(-1);
    }
#endif /* MONITORALL */


    /* open and set up the UniDig device to read chopper bits */
    fdChopper = open("/dev/iPUniDig_D", O_RDONLY);
    if (fdChopper < 0) {
        perror("Can not open iPUniDig_D, I quit");
        exit(-1);
    }
    /* Open the continuum detector device */
    if((cdfd = open(cdName, O_RDONLY, 0)) < 0) {
        perror("Can not open contDetector0, I quit");
        exit(1);
    }

    /* initializing ref. mem. */
    rm_status=rm_open(antlist);
    if(rm_status != RM_SUCCESS) {
        rm_error_message(rm_status,"rm_open()");
        exit(1);
    }
    ttfd = open("/dev/vme_sg_simple", O_RDWR, 0);
    if(ttfd <= 0) {
	fprintf(stderr, "Error opening TrueTime - /dev/vme_sg_simple\n");
	exit(SYSERR_RTN);
    }

#if DEBUG
    /*    fp=fopen("/instance/chopper.dat","w"); */
    fp = stdout;
#endif

    /* end of initializations */

#if MONITORALL
    /* Starting the readADCLoop thread to read the ADC values and write
    * them to RM every second. This is a lower priority thread running
    * at priority of 16
    */
    if (pthread_create(&readADCLoopTID, NULL, readADCLoop, (void *) 0) < 0) {
        perror("main: pthread_create readADCLoop");
        exit(-1);
    }

    /* If the -s option is given, syncdet will not do synchronous detection
    * and can be run in conjunction with the new one using the official
    * continuum detector.  I will keep its rm stuff up to date.
    */
    if(argc >= 2 && strcmp(argv[1], "-s") == 0) {
        fprintf(stderr, "Sync detecting is suppressed\n");
        while(1)
            sleep(10000);
    }
#endif /* MONITORALL */

    iCount = 0;
    for(;;) {

	/* This code should be executed on the first pass */
        if((iCount++ % CHECK_FLAG_MOD) == 0) {
            rm_status=rm_read(RM_ANT_0,"RM_CHOPPING_FLAG_S", &choppingFlag);
            if(rm_status!=RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
            }
            /* Read Continuum detector interface mode.  This is a trick read.
            * The length of 1 asks for a readback of the output bits rather
            * than the usual input bits of the unidig.
            */
            if ((status = read(fdChopper, (char *)(&cdMode), 1)) < 0) {
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
		ioctl(cdfd, EIOCSETMODE, &cdIoctlArg);
		oldContDetMode = cdMode;
		oldChoppingFlag = choppingFlag;
            }
        }
        if(read(cdfd, &cdOut, sizeof(cdOut)) < 0) {
            ioctl(cdfd, EIOCSETMODE, &cdIoctlArg);
            perror("Reading cont det");
        }
	/* Read the TrueTime and convert the time */
	if((status = read(ttfd, &ttime, sizeof(ttime))) < 0) {
	    fprintf(stderr, "Error %d reading TrueTime\n", status);
	}
	curMsec = ((ttime.hour * 60 + ttime.min) * 60 + ttime.sec) * 1000 +
		(ttime.usec + 500) / 1000;
        if(choppingFlag != 0) {
            /* read the chopper position bits */
            if ((status = read(fdChopper, (char *)(&chopPosBits), 4)) < 0) {
                perror("Reading chopper position bits");
                exit(-1);
            }

#if TRIGGER_LEVEL_R | TRIGGER_PULSE_R == 0x30
            tiltFlag = 3 & (chopPosBits >> 4);
#else
            We need to change the formula above if this is reached
#endif
            /*
             * If the chopper position has changed, this is the place to
             * keep track of it for forecasting chopper times for chopped
             * autocorrelation.
             */

            /* Is the data valid? */
            if(tiltFlag!=0 && tiltFlag==prevTiltFlag) {
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

            }	/*  data is valid */
            prevTiltFlag = tiltFlag;
        } else {	/* Not chopping, accumulate in on array */
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
            azTrErrSum += tsshm->azTrError;
            elTrErrSum += tsshm->elTrError;
            nOn++;
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


        /* End of dta collection part.  Check to see if it is the end of a
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
        if((dt > (500 - COUNTTIME/1000)) || (iCount >= (ICOUNT_MAX + 1))) {
            if(nOn > 0) {
		double d;

		diffL = d = (double)onSumL / nOn;	/* Average count */
		onSsqL = onSsqL / nOn - d * d;	/* Mean Square deviation */
		d /= COUNTTIME;	/* Convert to MHz for eqn. */
		onPwrL = coef[FREQLO].c[0] + d * (coef[FREQLO].c[1] +
			d * (coef[FREQLO].c[2] + d * coef[FREQLO].c[3]));

		diffH = d = (double)onSumH / nOn;
		onSsqH = onSsqH / nOn - d * d;
		d /= COUNTTIME;	/* Convert to MHz for eqn. */
		onPwrH = coef[FREQHI].c[0] + d * (coef[FREQHI].c[1] +
			d * (coef[FREQHI].c[2] + d * coef[FREQHI].c[3]));

                azTrErrSum /= (nOn + nOff) *1000.;	/* Avg in arcsec from */
                elTrErrSum /= (nOn + nOff) *1000.;	/* milliarcsec sum */
            } else {
		goto nodata;
	    }
            if(nOff > 0) {
		double d;

		diffL -= (d = (double)offSumL / nOff);
		offSsqL = offSsqL / nOff - d * d;
		d /= COUNTTIME;	/* Convert to MHz for eqn. */
		offPwrL = coef[FREQLO].c[0] + d * (coef[FREQLO].c[1] +
			d * (coef[FREQLO].c[2] + d * coef[FREQLO].c[3]));

		diffH -= (d = (double)offSumH / nOff);
		offSsqH = offSsqH / nOff - d * d;
		d /= COUNTTIME;	/* Convert to MHz for eqn. */
		offPwrH = coef[FREQHI].c[0] + d * (coef[FREQHI].c[1] +
			d * (coef[FREQHI].c[2] + d * coef[FREQHI].c[3]));
            }
            if(choppingFlag != 0) {

                dataL = onPwrL - offPwrL;
                rmsL = 0.5 * (sqrt(onSsqL * (COUNTTIME * 2000)) +
			sqrt(offSsqL * (COUNTTIME * 2000))) * (nOn + nOff) /
			(onSumL + offSumL);
                dataH = onPwrH - offPwrH;
                rmsH = 0.5 * (sqrt(onSsqH * (COUNTTIME * 2000)) +
			sqrt(offSsqH * (COUNTTIME * 2000))) * (nOn + nOff) /
			(onSumH + offSumH);
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
                rmsL = sqrt(onSsqL * (COUNTTIME * 2000)) * nOn / onSumL;
                dataH = onPwrH;
                rmsH = sqrt(onSsqH * (COUNTTIME * 2000)) * nOn / onSumH;
                contDetMuWatt[0] = onPwrL;
                contDetMuWatt[1] = onPwrH;
            }
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
#if 0
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
            } else {
                rm_status=rm_read(RM_ANT_0,"RM_RSCAN_SYNCDET_FLAG_S",
			&rscanFlag);
                if(rm_status!=RM_SUCCESS) {
		    rm_error_message(rm_status,"rm_read()");
		    exit(1);
                }
            }
#endif
	    if(verbose) {
	        printf("%6d %6d %6d %4d %7.3f %6.2f %5d\n", tsshm->msec,
		    tsshm->msecCmd, curMsec, iCount, dataL,
		    rmsL, maxOnL - minOnL);
	    }

            syncdetChannels[0] = dataL;
            syncdetChannels[1] = dataH;
	    rm_read(RM_ANT_0, "RM_UNIX_TIME_L", &unixTime);
            rm_status = rm_write(RM_ANT_0, "RM_SYNCDET2_CHANNELS_V2_F",
                                 syncdetChannels);
	    rm_write(RM_ANT_0, "RM_SYNCDET2_TIMESTAMP_L", &unixTime);
            syncdetStats[0] = rmsL;
            syncdetStats[1] = minOnL;
            syncdetStats[2] = maxOnL;
            syncdetStats[3] = rmsH;
            syncdetStats[4] = minOnH;
            syncdetStats[5] = maxOnH;
            rm_status |= rm_write(RM_ANT_0, "RM_SYNCDET2_STATS_V6_F",
                                  syncdetStats);
            rm_status |= rm_write(RM_ANT_0, "RM_CONT_DET_MUWATT_V2_F",
                                  contDetMuWatt);
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
    } /* end of main loop */

#if DEBUG
    fclose(fp);
#endif

    return(0);
}				/* end of main Loop */

#if MONITORALL
void *readADCLoop() {
    /* for ADC values*/
    int status;
    float fDummy=0.;
    float cont1Power=0.,cont2Power=0.;
    int i;
    double voltage;

    setpriority(PRIO_PROCESS, (0), (19));
    while(1) {

        /* The following ADC channels are for patch-panel channels 4 and 5
                */
        if(antenna!=1) {

            for (i = 0; i <= (N_TILTS+1); i++) {
                nRead = read(fdADC[i], (char *)(&adcdata), 2);
                if (nRead != 2)
                    fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 2\n",
                            nRead,i);
                else
                    fData[i] = ((float)adcdata)*10.0/32768.0;
                /*
                printf("%f %f %f %f TP 1: %f TP 2:  %f \n",
                       fData[0], fData[1], fData[2], fData[3], fData[4], fData[5]);
                */
            }
        } /* if antenna other than 1 */
        else {
            for (i = 0; i <= (N_TILTS+1); i++) {
                *((int *)&data) = i;
                nRead = read(fD, (char *)(&data), 4);
                if (nRead != 4)
                    fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of 4\n",
                            nRead, i);
                fData[i] = -data;
            }


        } /* if antenna = 1 */


        voltage = (double) fData[4];
        status = rm_write(RM_ANT_0, "RM_TOTAL_POWER_VOLTS_D", &voltage);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "rm_write()");
            exit(-1);
        }
        voltage = (double)fData[5];
        status = rm_write(RM_ANT_0, "RM_TOTAL_POWER_VOLTS2_D", &voltage);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "rm_write()");
            exit(-1);
        }

        fDummy = readChannel(contADC2,FO_RX_OP_ALARM);
        status = rm_write(RM_ANT_0, "RM_109_200_RX1_ALARM_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-8)");
            exit(-1);
        }
        fDummy = readChannel(contADC2,FO_TRANS_TP_ALARM);
        status = rm_write(RM_ANT_0, "RM_XMIT1_TEMP_ALARM_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-7)");
            exit(-1);
        }
        fDummy = readChannel(contADC2,FO_TRANS_OP_ALARM);
        status = rm_write(RM_ANT_0, "RM_XMIT1_OP_ALARM_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-6)");
            exit(-1);
        }
        fDummy = readChannel(contADC2,D109);
        status = rm_write(RM_ANT_0, "RM_109MHZ1_POWER_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-5)");
            exit(-1);
        }
        fDummy = readChannel(contADC2,FO_XMITTER_DET);
        status = rm_write(RM_ANT_0, "RM_CONT1_DET2_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-4)");
            exit(-1);
        }
        fDummy = readChannel(contADC2,CONT_DET1);
        status = rm_write(RM_ANT_0, "RM_CONT1_DET1_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(cont1det1)");
            exit(-1);
        }

        cont1Power= coef[C1D1].c[0] +coef[C1D1].c[1]*fDummy +
		coef[C1D1].c[2]*fDummy*fDummy;
        status = rm_write(RM_ANT_0, "RM_CONT1_DET1_POWER_MUWATT_F", &cont1Power);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(cont1det1power)");
            exit(-1);
        }

        fDummy = readChannel(contADC3,FO_RX_OP_ALARM);
        status = rm_write(RM_ANT_0, "RM_109_200_RX2_ALARM_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-8)");
            exit(-1);
        }
        fDummy = readChannel(contADC3,FO_TRANS_TP_ALARM);
        status = rm_write(RM_ANT_0, "RM_XMIT2_TEMP_ALARM_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-7)");
            exit(-1);
        }
        fDummy = readChannel(contADC3,FO_TRANS_OP_ALARM);
        status = rm_write(RM_ANT_0, "RM_XMIT2_OP_ALARM_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-6)");
            exit(-1);
        }
        fDummy = readChannel(contADC3,D109);
        status = rm_write(RM_ANT_0, "RM_109MHZ2_POWER_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-5)");
            exit(-1);
        }
        fDummy = readChannel(contADC3,FO_XMITTER_DET);
        status = rm_write(RM_ANT_0, "RM_CONT2_DET2_F", &fDummy);
        if (status != RM_SUCCESS) {
            rm_error_message(status, "yIGServer.stressLoop: rm_write(-4)");
            exit(-1);
        }
        fDummy = readChannel(contADC3,CONT_DET1);
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
