/**********************************************************************		
syncdet.c

Nimesh Patel
5 Dec 2002
A temporary C code to allow synchronous detection of 
continuum detector volts with chopping secondary until
the new hardware/software becomes available.
Segments from Taco's codes: logTilt.c and printChopperBits.c
have been included here.
11 Dec 2002, added tsshm to get msec from servo for timestamping.
(presently this feature is useful only in DEBUG mode but later it will get used
with the data for better synchronizing with azoff/eloff)
17 Feb 2003, added changes for antenna-1, where xycom does not work.
Reading the continuum detector voltage through IPs using code copied
from Taco's new logTiltsAcc1.c.

18 Apr 2003
Added Taco's real-time clock timer for better than usleep timing.
Without this, the while loop was holding up the CPU (on acc1 only for some reason).
Putting a usleep of 100 microseconds helped but the actual delay was perhaps
much larger as pointed out by Bob. (As much as 20ms). Scans did not come out OK
with usleep(100).

6 oct 2003
Added additional channels for simultaneous monitoring.

15 oct 2003
removed all the differences for antenna-1- and all references to
logTilts related stuff (xycom adc) since the floor-based continuum
system is now removed from from all antennas. Also removed the boxcar
averaging which i suspect was causing some spikes instead of helping.

10 Nov 2003
Since yesterday, syncdet stopped working on acc1 giving an open error
on /dev/iPOptoAD16_6000. There must have been a change in the hardware
and this special condition for antenna-1 is no longer necessary- so
commenting it out and making the code work the same way for all
antennas. NaP.
                                
15 Nov 2003
Created a dual-threaded version. The previous version of syncdet would
now be the main thread which will sleep unless chopping_status
is 1. In the background, a 1-sec while loop will update the continuum
detector and other monitoring variables in RM (which were previously
getting updated by yIGTuneServer. Thus there should be no conflict
between syncdet and yIGTuneServer. 

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



/* the following includes and defines are from chopperBits.c*/
#include <sys/file.h>
#include <sys/types.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "iPUniDig_D.h"
#include "smadaemon.h"

/* for hostname */
#include <sys/utsname.h>

#define SLEEPTIME_NANOSEC 100000 

#define TRUE 1
#define FALSE 0


#define SBC_RESET_W      0x000001
#define SERVO_ESTOP_W    0x000002
#define OVERTEMP_R       0x000004
#define CHOPPER_SYNC_W   0x000008
#define TRIGGER_LEVEL_R  0x000010
#define TRIGGER_PULSE_R  0x000020
#define CHOPPER_RESET_W  0x000040
#define TRIGGER_SPARE_R  0x000080
#define ESTOP_BYPASS_R   0x000100
#define WRITE_BITS (SBC_RESET_W | SERVO_ESTOP_W | CHOPPER_SYNC_W | CHOPPER_RESET_W)
#define READ_BITS (OVERTEMP_R | TRIGGER_LEVEL_R | TRIGGER_PULSE_R | TRIGGER_SPARE_R | ESTOP_BYPASS_R)
/**/

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
struct      sched_param param;
pthread_attr_t  attr;
int policy = SCHED_FIFO;
short choppingFlag=0;
int antlist[RM_ARRAY_SIZE];
int contADC2,contADC3;
float cont1c0,cont1c1,cont1c2;
float cont2c0,cont2c1,cont2c2;
	int antenna=0;
        /* for ADC values*/
        int fdADC[N_TILTS+2],nRead;
	int fD;
        short adcdata;
	float data;
	float fData[N_TILTS+3];


void realtimeSignalHandler(int sig, siginfo_t *extra, void *cruft);
void *readADCLoop();
float readChannel(int board,int channel);

int
main(int argc, char *argv[])
{
#if DEBUG
	FILE *fp;
#endif
	FILE *fpContDetCal;

        /* for chopper bits */
        int fdChopper;
        int readMask = READ_BITS;
        int writeMask = WRITE_BITS;
        int readData;
        /**/
        
	char nodeName[80];
        
        int             i;
	int 		icount=1;
        float adc_volts,adc_volts2;
	double sumsyncdetvolts=0.0,syncdetvolts=0.0;
	double sumsyncdetvolts2=0.0,syncdetvolts2=0.0;
	double syncdetVoltsArray[6];
        int boxcarCount=0;
	short tilt_flag=0,previous_tilt_flag;
	double on_avg=0.,off_avg=0.,on_minus_off=0.;
	double on_avg2=0.,off_avg2=0.,on_minus_off2=0.;
/*
	float adc_volts_array[MAX];
	short tilt_flag_array[MAX];
*/
	int rm_status,status;
        int chopperTrigger[2];
	int servomsec;

	/* for hostname */
	struct utsname unamebuf;

	/* for real-time clock timer signal handling */
	
	struct sigevent timerEvent;
	struct sigaction sa;
	sigset_t allsigs;
	struct sigaction action, oldAction;
	struct timespec realtimeClockRes;
	struct itimerspec its;
	timer_t updateTimer;

	float buff1,buff2;

	short choppingFlag=0;
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
            printf("Error returned from open - /dev/iPUniDig_D , errno = %d\n", errno);
            exit(-1);
        }
#if 0
        if ((status = ioctl(fdChopper, IOCTL_SETIN, &readMask)) < 0) {
            printf("Error %d returned from 2nd ioctl call, errno = %d\n",
		    status, errno);
            exit(-1);
        }
#endif
        if ((status = ioctl(fdChopper, IOCTL_DEBUG_OFF, " ")) < 0) {
            printf("Error %d returned from 1st ioctl call, errno = %d\n",
		    status, errno);
            exit(-1);
        }  
  

  /* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }

	previous_tilt_flag=1;

#if DEBUG
	fp=fopen("/instance/chopper.dat","w");
#endif

	
	  /* Create and configure a timer */

#if 0
	  if (sigemptyset(&sa.sa_mask) != 0)
	    perror("sigemptyset");
	  sa.sa_flags = SA_SIGINFO;
	  sa.sa_sigaction = realtimeSignalHandler;
	  if (sigaction(SIGRTMIN, &sa, NULL) < 0)
	    perror("sigaction");
	  /* Get system clock resolution */
	  if (clock_getres(CLOCK_REALTIME, &realtimeClockRes) < 0)
	    perror("tracking thread - clock_getres");
	  /* Create a timer based on the CLOCK_REALTIME clock */
	  its.it_interval.tv_sec = 0;
	  its.it_interval.tv_nsec = SLEEPTIME_NANOSEC;
	  its.it_value = its.it_interval;
	  timerEvent.sigev_notify = SIGEV_SIGNAL;
	  timerEvent.sigev_signo = SIGRTMIN;
	  timerEvent.sigev_value.sival_ptr = (void *)&updateTimer;
	  if (timer_create(CLOCK_REALTIME, &timerEvent, &updateTimer) < 0)
	    perror("timer_create");
	  /* Make timer relative, goes off at end of interval */
	  if (timer_settime(updateTimer, 0, &its, NULL) < 0)
	      perror("timer_settime");
	  if (sigemptyset(&allsigs) != 0)
	    perror("sigemptyset");
#endif

/* end of initializations */

/* Starting the readADCLoop thread to read the ADC values and write
them to RM every second. This is a lower priority thread running at priority of
16 */
        pthread_attr_init(&attr);
        if (pthread_create(&readADCLoopTID, &attr, readADCLoop,
                         (void *) 0) == -1) {
        perror("main: pthread_create readADCLoop");
        exit(-1);
        }
        param.sched_priority=16;
        pthread_attr_setschedparam(&attr,&param);
        pthread_setschedparam(readADCLoopTID,policy,&param);

	/* If the -s option is given, syncdet will not do synchronous detection
	 * and can be run in conjunction with the new one using the official
	 * continuum detector.  I will keep its rm stuff up to date.
	 */
	if(argc >= 2 && strcmp(argv[1], "-s") == 0) {
	    fprintf(stderr, "Sync detecting is suppressed\n");
	    while(1) sleep(10000);
	}

	while(1) {
       
	/* first check the chopping status. If 0, do nothing. */ 
            rm_status=rm_read(RM_ANT_0,"RM_CHOPPING_FLAG_S",
			&choppingFlag);
                  if(rm_status!=RM_SUCCESS) {
                  rm_error_message(rm_status,"rm_read()");
                  exit(1);
                  }
     if(choppingFlag==1) { 

       /* read the chopper bits */
        if ((status = read(fdChopper, (char *)(&readData), 4)) < 0) {
                fprintf(stderr, "Error %d returned from read, errno = %d\n", status,
                errno);
                exit(-1);
        }

	servomsec=tsshm->msec;

        chopperTrigger[0]=(readData&TRIGGER_LEVEL_R)?1:0;
        chopperTrigger[1]=(readData&TRIGGER_PULSE_R)?1:0;
        
        if (chopperTrigger[0]==1) tilt_flag=1;
        if (chopperTrigger[1]==1) tilt_flag=2;
        if ((chopperTrigger[0]==0)&&(chopperTrigger[1]==0)) tilt_flag=0;


      adc_volts  = readChannel(contADC2,CONT_DET1);
      adc_volts2 = readChannel(contADC3,CONT_DET1);

	
	 /* ignore all data with tilt_flag=0 (chopper still moving) */
         
	if(tilt_flag!=0) {

            if(tilt_flag==previous_tilt_flag) {
            
                  if(tilt_flag==1){
                  on_avg=(((double)icount-1)*on_avg+
			(double)adc_volts)/(double)icount;
                  on_avg2=(((double)icount-1)*on_avg2+
			(double)adc_volts2)/(double)icount;
                  icount++;
                  }
            
                  if(tilt_flag==2) {
                  off_avg=(((double)icount-1)*off_avg+
			(double)adc_volts)/(double)icount;
                  off_avg2=(((double)icount-1)*off_avg2+
			(double)adc_volts2)/(double)icount;
                  icount++;
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
	fprintf(fp,"%d %d %d %f %f %lf %lf %lf %lf %lf %lf %lf %lf %d %d %d\n",
	icount,previous_tilt_flag, tilt_flag,adc_volts,adc_volts2,on_avg,
	on_avg2,off_avg, off_avg2,on_minus_off,on_minus_off2,syncdetvolts,
	syncdetvolts2,servomsec, chopperTrigger[0],chopperTrigger[1]);
#endif


/*
	sigsuspend(&allsigs);
*/
         } /* if choppingFlag==1 */
	if(choppingFlag==0) sleep (1);
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


void *readADCLoop()
{
        /* for ADC values*/
	int status;
	float fDummy=0.;
	float cont1Power=0.,cont2Power=0.;
	int i;
	double voltage;

      while(1)
      {

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


float readChannel(int board,int channel)
{
  int   errorCode;
  float buff;

  *((int *)&buff) = channel;
  if ((errorCode = read(board, &buff, 4)) != 4) {
    fprintf(stderr, "Error %d returned from read on A/D\n", errorCode);
    return(0.0);
  }
  return(buff);
}

void realtimeSignalHandler(int sig, siginfo_t *extra, void *cruft)
{
  int noverflow;

  noverflow = timer_getoverrun(*(timer_t *)extra->si_value.sival_ptr);
  if (noverflow != 0) {
  fprintf(stderr, "realtimeSignalHandler: timer overflow (%d ticks)\n", noverflow);
    fflush(stderr);
  }
  return;
}

/****************************end syncdet.c******************************/
