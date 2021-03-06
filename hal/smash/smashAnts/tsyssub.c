/*************************************************************** 
* tsyssub.c
* 
* NAP	Originally wrote tsys.c commanding the loads through Track
* TRH   rewritten to avoid going through Track as tsysnew.c
* RWW	modifying 11/07 Made thread safe and working on diagnostics
*	12/4/07 Now contains tsys as a subroutine.  Now tsyssub.c
*
****************************************************************/
/* The function prototype is
 *
 * int tsys(int *antennaArray, int *rxListArg, int intTimeArg,
 *          int measureTrxArg, int print, int debugArg);
 *
 * antennaArray should point to an array of SMAPOPT_MAX_ANTENNAS+1 ints
 *   as returned by getAntennaList.  If it is NULL, the Antennas in
 *   the array will be used
 * rxList should point to an array of 3 ints as returned by getReceiverList.
 *   If it is NULL, the active receivers or if none, receiver 1 will be used.
 * intTime is the number of seconds to integrate, or if 0, the tsys default
 *   of 4 sec. will be used.
 * The remaining arguments are logical and the action will be taken if true.
 *
 * Tsys usually works on several antennas, so success and failure are
 * hard to define.  tsys gives the operator warnings of failures and only
 * returns failure if it was unable to accomplish anything.
 *
 * tsyssub.o must be linked with $(COMMONLIB)/tune_xdr.o and
 * $(COMMONLIB)/tune_clnt.o
 */

#include <pthread.h>
#include <rpc/rpc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "smapopt.h"
#include "rm.h"
#include "dsm.h"
#include "commonLib.h"
#include "stderrUtilities.h"
#include "tune6status.h"
#include "tune.h"
#include "smashAnts.h"

#define USE_BOTHLOADS_COMMAND 0
#define USE_ITIMER 0
#define DEBUG 0
#define NUMBER_ANTENNAS 8
#define TUNE_THREAD_PRIORITY (18)
#define PRINT_QUANTUM_LIMIT 0
#define WACKO_TEMP 1000000
#define USE_MUWATTS 1
#if USE_MUWATTS
  #define SATURATION_VOLTAGE 4.75  /* uWatt */
#else
  #define SATURATION_VOLTAGE 9.95  /* volts */
#endif

#if PRINT_QUANTUM_LIMIT
static int dsm_open_status;
#endif

static int useMuWatts = USE_MUWATTS;
static float muWattsSaturationVoltage = 4.75;
static float contDetSaturationVoltage = 9.95;
static int measureTrx;
static pthread_t measureTsysTId[NUMBER_ANTENNAS+1];
static int debug = DEBUG;
static int intTime = 4; /* default integration time per load in seconds */
static CLIENT *clp[NUMBER_ANTENNAS+1];      
static int trx1corrupted[NUMBER_ANTENNAS+1];
static int trx2corrupted[NUMBER_ANTENNAS+1];
static int tsys1corrupted[NUMBER_ANTENNAS+1];
static int tsys2corrupted[NUMBER_ANTENNAS+1];
static int rpcErrors[NUMBER_ANTENNAS+1];
static double trx[NUMBER_ANTENNAS+1], trx2[NUMBER_ANTENNAS+1];
static short hotloadType[NUMBER_ANTENNAS+1];
static int rxList[3];
static FILE *devstderr;

/* RWW Patch to make the rpc calls thread safe the way Todd does in tune.c */
rawResults *(*rr[NUMBER_ANTENNAS+1])() = { (rawResults *(*)())0,
  &rawrequest1_1, &rawrequest2_1, &rawrequest3_1, &rawrequest4_1,
  &rawrequest5_1, &rawrequest6_1, &rawrequest7_1, &rawrequest8_1,
  &rawrequest9_1, &rawrequest10_1};
#define rawrequest_1(r, c) rr[antenna](r, c)

/* tsyssub.c */
static int CallRawRequest(int antenna, char *req, int callNumber);
static void *manageRPC(void *antnumber);
static void integrate(int antenna, char *label, float *v1p,  float *v2p);
#if USE_ITIMER
static void serialCleanup(int sig);
static void callSetitimer(int which, struct itimerval *interval,  
                   struct itimerval *old_settings);
#endif
#if PRINT_QUANTUM_LIMIT
static void getFrequencies(double freq[3]);
#endif
#if 0
static void printNullPointerMessage(int antenna);
static int printedNullPointer[NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0,0,0};
#endif

static void *manageRPC(void *antnumber) {
  int rm_status;
  int antenna;
  struct timeval DebugTime;
#if USE_ITIMER
  struct sigaction action, oldAction;
  itruct sigaction action, oldAction;
  struct itimerval interval, old_settings;
#endif
  int timestamp;
  float thot,tcold,yfactor,heatedload,unheatedload;
  float heatedload2,unheatedload2, sky, sky2;
  char serverName[10];
#if 0
  float voltagearray[2];
  float voltage,voltage2;
#endif
  int i;
  short s;

#if PRINT_QUANTUM_LIMIT
  double quantum,h,k,hvk,freq;
  double freq[3];

  h = 6.63e-27;
  k = 1.38e-16;
  if (dsm_open_status == 0 && measureTrx) {
    getFrequencies(freq);
  }
#endif
  antenna = (int)antnumber;
  
  /*************************************************/
  /* create an RPC client to that antenna/hostname */
  /*************************************************/

  if (debug) {
    printf("Creating tune client call to antenna %d\n",antenna);
  }
  sprintf(serverName,"acc%d",antenna);

#if USE_ITIMER
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = serialCleanup;
  if (sigaction(SIGALRM, &action, &oldAction) != 0) {
    perror("sigaction: ");
  }
  /* it_value must be non-zero to work */
  interval.it_value.tv_sec = 5;
  interval.it_value.tv_usec = 5;
  interval.it_interval.tv_sec = 5;
  interval.it_interval.tv_usec = 5;
  callSetitimer(ITIMER_REAL, &interval, &old_settings); /* alarm on */
#endif
  if (!(clp[antenna] = clnt_create(serverName, TUNEPROG, TUNEVERS, "tcp"))) {
    clnt_pcreateerror(serverName);
    return((void *)1);
  } else {
    if (debug) {
      printf("Client created to %s\n",serverName);
    }
  }
#if USE_ITIMER
  callSetitimer(ITIMER_REAL, &old_settings, NULL); /* alarm off */
#endif
  DebugTime.tv_sec = 5;
  DebugTime.tv_usec = 0;
  if (clnt_control(clp[antenna], CLSET_TIMEOUT, (char *)&DebugTime) == FALSE) {
    printf("manageRPC(%d) - can't change client timeout value\n",antenna);
  }

  /* put in the unheated load */
  if((i = CallRawRequest(antenna, "unheatedload in", 1)) != 0) {
    goto finish;
  }

  /* read the unheated load value */
  /* for the purposes of y-factor, the unheatedload is the coldest load */
  integrate(antenna, "unheated load", &unheatedload, &unheatedload2);
  /* in this case, HOTLOAD means the ambient load, and coldload also is the
   * ambientload (compared to the heated load)
   */
  rm_status = rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
  rm_status = rm_write(antenna,"RM_HOTLOAD_LOWFREQ_VOLTS_F",&unheatedload);
  rm_status = rm_write(antenna,"RM_HOTLOAD_HIGHFREQ_VOLTS_F",&unheatedload2);
  rm_status = rm_write(antenna,"RM_HOTLOAD_LOWFREQ_VOLTS_TIMESTAMP_L",&timestamp);
  rm_status = rm_write(antenna,"RM_HOTLOAD_HIGHFREQ_VOLTS_TIMESTAMP_L",&timestamp);
#if 0
  rm_status = rm_write(antenna,"RM_UNHEATEDLOAD_TIMESTAMP_L",&timestamp);
#endif

  /* check hotload type */  
  rm_status = rm_read(antenna,"RM_HOTLOAD_STYLE_S",&hotloadType[antenna]);

  /* remove unheated load and put in hotload? */
  if (measureTrx && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
#if USE_BOTHLOADS_COMMAND
    if(CallRawRequest(antenna, "bothloads out in", 2) != 0) {
      goto finish;
    }
#else
    if(CallRawRequest(antenna, "heatedload in", 2) != 0) {
      goto finish;
    }
    if(CallRawRequest(antenna, "unheatedload out", 3) != 0) {
      goto finish;
    }
#endif
    
    integrate(antenna, "heated load", &heatedload, &heatedload2);
    if (rxList[1] != 0) {
      /* compute Trx */
      yfactor = heatedload/unheatedload;
      if (debug) {
	printf("yfactor(1) = %f\n",yfactor);
      }
      /*    printf("Yfactor = %g\n",yfactor);*/
      rm_status = rm_read(antenna,"RM_HEATEDLOAD_TEMPERATURE_F",&thot);
      rm_status = rm_read(antenna,"RM_UNHEATEDLOAD_TEMPERATURE_F",&tcold);
#define CELSIUS_TO_KELVIN 273.15
      thot += CELSIUS_TO_KELVIN;
      tcold += CELSIUS_TO_KELVIN;
      trx[antenna] = (thot-yfactor*tcold)/(yfactor-1);
      if (debug) {
	printf("hot temp = %f  ambient temp = %f\n",thot,tcold);
	printf("Trx(1) = %f\n",trx[antenna]);
      }
      rm_status = rm_write(antenna,"RM_TRX_D",&trx[antenna]);
      rm_status = rm_write(antenna,"RM_TRX_TIMESTAMP_L",&timestamp);
    }
    if (rxList[2] != 0) {
      /* compute Trx2 */
      yfactor = heatedload2/unheatedload2;
      rm_status = rm_read(antenna,"RM_HEATEDLOAD_TEMPERATURE_F",&thot);
      rm_status = rm_read(antenna,"RM_UNHEATEDLOAD_TEMPERATURE_F",&tcold);
      thot += CELSIUS_TO_KELVIN;
      tcold += CELSIUS_TO_KELVIN;
      trx2[antenna] = (thot-yfactor*tcold)/(yfactor-1);
      if (debug) {
	printf("Yfactor(2) = %g\n",yfactor);
	printf("Trx(2) = %f\n",trx2[antenna]);
      }
      rm_status = rm_write(antenna,"RM_TRX2_TIMESTAMP_L",&timestamp);
      rm_status = rm_write(antenna,"RM_TRX2_D",&trx2[antenna]);
    }
  } else {
    if (measureTrx) {
      printf("Sorry, but antenna %d does not have a two-temperature load. Trx not available.\n",antenna);
    }
  }
  /* remove both loads */
#if USE_BOTHLOADS_COMMAND
  if(CallRawRequest(antenna, "bothloads out out", 4) != 0) {
    goto finish;
  }
#else
  if(CallRawRequest(antenna, "heatedload out", 4) != 0) {
    goto finish;
  }
  if(CallRawRequest(antenna, "unheatedload out", 5) != 0) {
    goto finish;
  }
#endif

  /* while we integrate on the sky, calvaneservo will update the
   * Tsys value in RM */
  integrate(antenna, "sky", &sky, &sky2);
finish:
  rm_read(antenna,"RM_LOADS_OUT_S",&s);
  if(s == 0) {
    sleep(1);
    fprintf(devstderr,
      "tsys on ant %d finished with a load remaining out - trying to recover\n",
      antenna);
    CallRawRequest(antenna, "unheatedload out", 6);
    if(rpcErrors[antenna] < 3)
      CallRawRequest(antenna, "heatedload out", 7);
    rm_read(antenna,"RM_LOADS_OUT_S",&s);
    if(s == 0) {
      char msg[80];

      sprintf(msg,
          "TSYS WARNING!! a load on ant %d is still in the BEAM\n",
	  antenna);
      if(!debug) sendOpMessage(OPMSG_SEVERE, 19, 20, msg);
      fprintf(devstderr, "%s", msg);
      usleep(100000);
    }
  }
  clnt_destroy(clp[antenna]);
  return((void *)0);
}

int tsys(int *antennaArray, int *rxListArg, int intTimeArg, int measureTrxArg,
	int print, int debugArg) {
  int i;
  int rm_status;
  int antlist[RM_ARRAY_SIZE];
  int antenna;
  int gotantenna=0;
  int numberOfAnts;
  int ptrc;
  double tsys[NUMBER_ANTENNAS+1], tsys2[NUMBER_ANTENNAS+1];
  int dmyAntennaArray[SMAPOPT_MAX_ANTENNAS+1];
  
  for(i = 0; i < NUMBER_ANTENNAS+1; i++) {
    trx1corrupted[i] = 0;
    trx2corrupted[i] = 0;
    tsys1corrupted[i] = 0;
    tsys2corrupted[i] = 0;
    rpcErrors[i] = 0;
  }
  if(antennaArray) {
    gotantenna = 1;
  } else {
    antennaArray = dmyAntennaArray;
  }
  debug = debugArg;
  measureTrx = measureTrxArg;
  intTime = intTimeArg;
  /* initializing ref. mem. */
  rm_status = rm_open(antlist);
  if(rm_status != RM_SUCCESS && rm_status != RM_ALREADY_OPEN) {
    rm_error_message(rm_status,"rm_open()");
    return(1);
  }

  /* open the the stderr consolidator as devstderr */
  if((devstderr = fopen("/dev/stderr", "w")) == NULL) {
    printf("tsys is unable to open /dev/stderr\n");
  }
  /* Set devstderr to line buffered so each message will be printed as given */
  setvbuf(devstderr, NULL, _IOLBF, BUFSIZ);

  if (gotantenna==1) {
    if (debug) {
      printf("Saw -a\n");
    }
  } else {
    /* from the current project */
    getAntennaList(antennaArray);
  }

  numberOfAnts=0;
  for(antenna=1;antenna<NUMBER_ANTENNAS+1;antenna++) {
    if(antennaArray[antenna]==1) {
	numberOfAnts++;
    }
  }
  if (numberOfAnts == 0) {
    printf("Sorry, no antennas specified or in the project.  Nothing done.\n"); 
    fclose(devstderr);
    return(SMASH_FAILURE);
  }
  if (debug) {
    printf("Using %d antennas\n",numberOfAnts);
  }
#if PRINT_QUANTUM_LIMIT
  dsm_open_status = dsm_open();
#endif
  if(rxListArg) {
    rxList[1] = rxListArg[1];
    rxList[2] = rxListArg[2];
  } else {
    getReceiverList(rxList);
  }
  /* spawn threads for each antenna */
  if (rxList[1] == 0 && rxList[2] == 0) {
#if 0
    printf("No receivers are in use in the project. I assume you want the\n"
      "low-freq receiver.\n");
#endif
    rxList[1] = 1;
  }
  for(antenna=1; antenna<=NUMBER_ANTENNAS; antenna++) {
    if (antennaArray[antenna]==1) {
      if (debug) {
	printf("Spawning thread to ant %d\n",antenna);
      }
      if ((ptrc = pthread_create(&measureTsysTId[antenna], NULL, manageRPC,
					(void *)antenna) != 0)) {
	perror("main: pthread_create manageRPC");
      } else {
	if (debug) {
	  printf("created\n");
	}
      }
    }
  }
  sleep(1);
  /* Now wait for all the tuning threads to terminate */
  for (antenna = 1; antenna <= NUMBER_ANTENNAS; antenna++) {
    if (antennaArray[antenna] == 1) {
      int retCode;
      void *status;

      retCode = pthread_join(measureTsysTId[antenna], &status);
      if(retCode != 0 || (int)status != 0) {
	printf("The tsys thread for ant %d returned code %d, value %d\n",
	    antenna, retCode, (int)status);
      }
      if (debug) {
	printf("Ant %d thread terminated\n", antenna);
      }
    }
  }
#if PRINT_QUANTUM_LIMIT
  dsm_close();
#endif
  /* print out the Tsys value from RM to the screen */
  if(print) for (antenna=1; antenna<=8; antenna++) {
    if (antennaArray[antenna]==1) {
      printf("Ant%d: ", antenna);
      if(rpcErrors[antenna] > 1) {
	printf("Fail: Too many errors moving the loads\n");
	continue;
      }
      if (rxList[1] != 0) {
	/* print the low-freq receiver */
	rm_status=rm_read(antenna,"RM_TSYS_D",&tsys[antenna]);
	if (fabs(tsys[antenna]) > WACKO_TEMP) {
	  printf("Tsys1 =    wacko   ");
	} else {
	  printf("Tsys1 = %8.1f",tsys[antenna]);
	  if (tsys1corrupted[antenna] == 1) {
	    /*	    printf(" (corrupted)");*/
	    printf("*");
	  }
	}
	if (measureTrx && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
	  if (fabs(trx[antenna] > WACKO_TEMP)) {
	    printf(" Trx1  = wacko");
	  } else {
	    printf(" Trx1  = %8.1f",trx[antenna]);
	    if (trx1corrupted[antenna] == 1) {
	      printf("*");
	    }
	  }
	}
      }
      if (rxList[2]!=0) {
	/* print the high-freq receiver */
	rm_status=rm_read(antenna,"RM_TSYS2_D",&tsys2[antenna]);
	if (fabs(tsys2[antenna]) > WACKO_TEMP) {
	  printf("  Tsys2 = wacko   ");
	} else {
	  printf("  Tsys2 = %8.1f",tsys2[antenna]);
	  if (tsys2corrupted[antenna] == 1) {
	    printf("*");
	    /*	    printf(" (corrupted)");*/
	  }
	}
	if (measureTrx && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
	  if (fabs(trx2[antenna] > WACKO_TEMP)) {
	    printf(" Trx2 =    wacko");
	  } else {
	    printf(" Trx2  = %8.1f",trx2[antenna]);
	    if (trx2corrupted[antenna] == 1) {
	      printf("*");
	    }
	  }
	}
      }
      printf("\n");
    }
  }
  rm_close();
  fclose(devstderr);
  for(antenna=1; antenna<=NUMBER_ANTENNAS; antenna++) {
    if(antennaArray[antenna] && rpcErrors[antenna] < 1) {
      return(SMASH_SUCCESS);
    }
  }
  return(SMASH_FAILURE);
}

#if PRINT_QUANTUM_LIMIT
static void getFrequencies(double freq[3]) {
  dsm_structure smaStruct;
  char structureName[80];
  int dsm_status;
  time_t timestamp;

  strcpy(structureName, "DDS_TO_HAL_X");
  dsm_status = dsm_structure_init(&smaStruct, structureName);
  dsm_status = dsm_read("newdds",structureName,&smaStruct,&timestamp);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_structure_init(SMA_METEOROLOGY_X)");
/*    exit(17); */
  }    
  dsm_structure_get_element(&smaStruct,"FREQ_V3_D",freq);
}
#endif

#if USE_ITIMER
static void serialCleanup(int sig) {
  fprintf(devstderr, "tsys timeout creating RPC client sig %d\n", sig);
}

static void callSetitimer(int which, struct itimerval *interval,  
                   struct itimerval *old_settings) {
  int status = setitimer(which, interval, old_settings);
  if (status != 0) {
    printf("setitimer(): errno = %d = ",errno);
    switch (errno) {
    case EFAULT: printf("EFAULT\n"); break;
    case EINVAL: printf("EINVAL\n"); break;
    default: printf("unknown\n"); break;
    }
  }
}
#endif

static int CallRawRequest(int antenna, char *req, int callNumber) {
  rawRequest rawCommandRequest;
  rawResults *rawptr;

  strcpy(rawCommandRequest.requestString, req);
again:
  if (debug) {
    printf("Sending command = %s, rpcErrors = %d\n",
	rawCommandRequest.requestString, rpcErrors[antenna]);
  }
  rawptr = rawrequest_1(&rawCommandRequest,clp[antenna]);

  if(rawptr != NULL && rawptr->status == 0)
    return(0);

  rpcErrors[antenna]++;
  if (rawptr == NULL) {
    struct rpc_err load_err;         /* RPC error structure */

    clnt_geterr(clp[antenna], &load_err);
    fprintf(devstderr, "tsys: error %d%s on RPC call %d to ant %d\n",
	  load_err.re_status,
          (load_err.re_status == RPC_TIMEDOUT)? "(TIMEOUT)": "",
          callNumber, antenna);
  } else {
    fprintf(devstderr, "tsys call %d result status %d, warning %s\n",
	callNumber, rawptr->status, rawptr->warningBuffer);
  }
  if(rpcErrors[antenna] < 2) {
    sleep(1);
    goto again;
  }
  return(2);
}

static void integrate(int antenna, char *label, float *v1p,  float *v2p) {
  int i;
  float saturationVoltage;
  float voltagearray[2];

  /* syncdet2 integrates for one sec intervals and writes the result in
   * reflective memory, so after the load is in position, it is necessary
   * to wait for 2 sec to get a clean reading.  Wait one sec now and one later.
   */
  sleep(1);
  if (useMuWatts && (antenna <= 8))
    saturationVoltage = muWattsSaturationVoltage;
  else
    saturationVoltage = contDetSaturationVoltage;
  *v1p = *v2p = 0;
  for (i=0; i<intTime; i++) {
    /* this sleep waits for the next value from syncdet2 */
    sleep(1);
    if (useMuWatts && (antenna <= 8)) {
      rm_read(antenna,"RM_CONT_DET_MUWATT_V2_F",voltagearray);
    } else {
      rm_read(antenna,"RM_CONT1_DET1_F",&voltagearray[0]);
      rm_read(antenna,"RM_CONT2_DET1_F",&voltagearray[1]);
    }
    *v1p += voltagearray[0];
    *v2p += voltagearray[1];
    if (debug) {
      printf("Ant%d:%s voltages = %f %f\n", antenna, label, voltagearray[0],
          voltagearray[1]);
    }
  }
  *v1p /= intTime;
  *v2p /= intTime;
  if (*v1p > saturationVoltage) {
    printf("Ant%d: IF1 rx total power detector saturated (%5.2f) on the %s "
       "--> tsys corrupted (*)\n", antenna, *v1p, label);
    tsys1corrupted[antenna] = 1;
  } 
  if (*v2p > saturationVoltage) {
    printf("Ant%d: IF2 rx total power detector saturated (%5.2f) on the %s"
      "--> tsys corrupted (*)\n", antenna, *v2p, label);
    tsys2corrupted[antenna] = 1;
  } 
  if (debug) {
    printf("Ant%d: average %s = %f, %f volts\n", antenna, label, *v1p, *v2p);
  }
}
