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

/* for hostname */
#include <sys/utsname.h>

#define CHOPSHORTTIME 10000
#define FREERUNTIME 100000
#define CHOPMODE 0
#define FREERUNMODE CONT_DET_MODE_W

#define TRUE 1
#define FALSE 0

/* The following define is from logTilts.c */
#define N_TILTS 4
#define PRIORITY 16
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


/*********************************************************************/
#define DEBUG 1
#define ADC_CHANNEL 1
#define MAX 10000
/*********************************************************************/

/* global variables */
TrackServoSHM *tsshm;
pthread_t   readADCLoopTID;
int policy = SCHED_FIFO;
short choppingFlag=0;
int antlist[RM_ARRAY_SIZE];
int contADC2,contADC3;
float cont1c0,cont1c1,cont1c2;
float cont2c0,cont2c1,cont2c2;
int antenna=0;
int oldContDetMode = -1, oldChoppingFlag = -1;

/* Things for the continuum detector device */
char cdName[] = "/dev/contDetector0";
int cdfd;                               /* continuum detector fd */
contDetectorioc_t cdIoctlArg;
contDetector_result_t cdOut;

/* for ADC values*/
int fdADC[N_TILTS+2],nRead;
int fD;
short adcdata;
float data;
float fData[N_TILTS+3];

/* syncdet2.c */
void *readADCLoop();
float readChannel(int board,int channel);

int main(int argc, char *argv[]) {
#if DEBUG
    FILE *fp;
#endif

    FILE *fpContDetCal;

    /* for chopper bits */
    int fdChopper;
    int readData;

    char nodeName[80];

    int i;
    int icount=1;
    double syncdetvolts=0.0;
    double syncdetvolts2=0.0;
    double syncdetVoltsArray[6];
    short tilt_flag=0;
    int prevInPosTilt, prevTiltFlag;
    int onSum1, onSum2, offSum1, offSum2;
    int onTime, offTime;
    int rm_status,status;
    int servomsec;
    int cdMode;
    int warned = 0;

    /* for hostname */
    struct utsname unamebuf;

    short choppingFlag;
    char dummyString[20];


    /* end of variable declarations */

    /********Initializations**************************/

    setpriority(PRIO_PROCESS,0,PRIORITY);

    tsshm = OpenShm(TSSHMNAME, TSSHMSZ);

    /* read in the continuum detector calibration coefficients */
    fpContDetCal = fopen("/instance/configFiles/contDetCalibration.conf","r");
    if(fpContDetCal == NULL) {
        fprintf(stderr,"Failed to open the cont.det. calibration coefficients file.\n");
        exit(QUIT_RTN);
    }
    fscanf(fpContDetCal,"%s %f %f %f",dummyString,&cont1c0,&cont1c1,&cont1c2);
    fscanf(fpContDetCal,"%s %f %f %f",dummyString,&cont2c0,&cont2c1,&cont2c2);
    fclose(fpContDetCal);


    /* Open all A/D channels: */

    /* get antenna number from hostname */
    uname (&unamebuf);
    if(!strcmp(unamebuf.nodename,"acc1")) {
        antenna=1;
        sprintf(nodeName, "/dev/iPOptoAD16_6000");
        fD = open(nodeName, O_RDONLY);
        if (fD < 0) {
            perror("Open");
            exit(-1);
        }

    } /* if acc1 */
    else {
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

#if DEBUG
    fp=fopen("/instance/chopper.dat","w");
#endif

    /* end of initializations */

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

    while(1) {

        /* first check the chopping status. If 0, do nothing. */
        rm_status=rm_read(RM_ANT_0,"RM_CHOPPING_FLAG_S",
                          &choppingFlag);
        if(rm_status!=RM_SUCCESS) {
            rm_error_message(rm_status,"rm_read()");
            exit(1);
        }

	/* read the chopper bits */
	if ((status = read(fdChopper, (char *)(&readData), 4)) < 0) {
		perror("Reading chopper bits");
                exit(-1);
	}
	if((readData & CONT_DET_MODE_W) == CHOPMODE) {
	    if(--warned == 0) {
		fprintf(stderr, "No support for CHOPMODE yet\n");
		warned = 11;
	    }
	    sleep(5);
	    continue;
	}
	warned = 0;
	if(choppingFlag != oldChoppingFlag || (cdMode =
		(readData & CONT_DET_MODE_W)) != oldContDetMode) {
	    cdIoctlArg.mode = (cdMode == CHOPMODE)? CHOP: IND;
	    if(choppingFlag) {
		cdIoctlArg.intTime = CHOPSHORTTIME;
	    } else {
		cdIoctlArg.intTime = FREERUNTIME;
	    }
	    ioctl(cdfd, CONTDET_SETMODE, &cdIoctlArg);
	    oldContDetMode = cdMode;
	    oldChoppingFlag = choppingFlag;
	}
	servomsec=tsshm->msec;
        if(choppingFlag==1) {

#if TRIGGER_LEVEL_R | TRIGGER_PULSE_R == 0x30
	    tilt_flag = 3 & (readData >> 4);
#else
               CHANGE THE EXPRESSION ABOVE
#endif
	    if(read(cdfd, &cdOut, sizeof(cdOut)) != sizeof(cdOut)) {
		perror("Reading cont det");
	    }

            /* ignore all data with tilt_flag=0 (chopper still moving)  or
	     * with the tilt flag changed during the integration
	     */
            if(tilt_flag!=0 && tilt_flag==prevTiltFlag) {
		if(tiltFlag != prevInPosTilt) {
		    if(tiltFlag == 1) {
		    }
		    prevInPosTile = tiltFlag;
		}
		if(tilt_flag==1) {
		    onSum1 += cdOut.lowRx;
		    onSum2 += cdOut.highRx;
		    ontime += cdOut.timer;
		} else {
		    offSum1 += cdOut.lowRx;
		    offSum2 += cdOut.highRx;
		    offtime += cdOut.timer;
		}

	    }/* if prevtiltflag=tiltflag*/

                else { /* if chopper status has changed */

                    icount=1; /* reset the averaging flag to 1 because the
                                                chopper status has changed */
                    if(tilt_flag==1) {
                        on_minus_off=on_avg-off_avg;
                        on_minus_off2=on_avg2-off_avg2;
                        syncdetvolts=(double)on_minus_off;
                        syncdetvolts2=(double)on_minus_off2;

                        syncdetVoltsArray[0]=syncdetvolts;
                        syncdetVoltsArray[1]=syncdetvolts2;
                        if((fabs(syncdetvolts)<SPIKETHRES)&&(fabs(syncdetvolts2)<SPIKETHRES))
                            rm_status=rm_write(RM_ANT_0,"RM_SYNCDET_CHANNELS_V2_D",
                                               &syncdetVoltsArray);
                        if(rm_status!=RM_SUCCESS) {
                            rm_error_message(rm_status,"rm_write()");
                            exit(1);
                        }

                    } /* if tiltflag=1 */
                }

                previous_tilt_flag=tilt_flag;

            }/* if tilt_flag!=0 */

#if DEBUG
            fprintf(fp,"%d %d %d %f %f %f %f %f %f %f %f %f %f %d %d\n",
	      icount,previous_tilt_flag, tilt_flag,adc_volts,adc_volts2,on_avg,
              on_avg2,off_avg, off_avg2,on_minus_off,on_minus_off2,syncdetvolts,
              syncdetvolts2,servomsec, tilt_flag);
#endif

        } else {	/* if choppingFlag==1 */
            sleep (1);
	}
    } /* end of while loop */
    close(fdChopper);
    /* This should be a for loop over all of the ADC chanels, but closing is not
     * needed since the program is dying at this point.
     *	close(fdADC); */

#if DEBUG

    fclose(fp);
#endif

    return(0);
}				/* end of main Loop */

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
                    fprintf(stderr, "logTilts: Got %d bytes on channel %d instead of
                            4\n",
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

        cont1Power= cont1c0 +cont1c1*fDummy + cont1c2*fDummy*fDummy;
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
        cont2Power= cont2c0 +cont2c1*fDummy + cont2c2*fDummy*fDummy;
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
