/*************************************************************** 
*
* tsys.c
* 
* TRH   rewritten to avoid going through Track
*
****************************************************************/

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
#include "tune6status.h"
#include "tune.h"

#define USE_BOTHLOADS_COMMAND 0
#define DEBUG 0
#define NUMBER_ANTENNAS 10
#define TUNE_THREAD_PRIORITY (18)
#define PRINT_QUANTUM_LIMIT 0
#define WACKO_TEMP 1000000
#define USE_MUWATTS 1
#if USE_MUWATTS
  #define SATURATION_VOLTAGE 4.75  /* uWatt */
#else
  #define SATURATION_VOLTAGE 9.95  /* volts */
#endif

void callSetitimer(int which, struct itimerval *interval,  
                   struct itimerval *old_settings);
void printNullPointerMessage(int antenna);
void usage(int exitcode, char *error, char *addl);
static void serialCleanup(int sig);
void getFrequencies(double freq[3]);
int processResponse(rawResults *result, int antenna);
void *manageRPC(void *antnumber);
int dsm_open_status;
int receiverTempFlag;
pthread_t tuneRxTId[NUMBER_ANTENNAS+1];
int debug = DEBUG;
int integrate = 4; /* default integration time per load in seconds */
CLIENT *clp[NUMBER_ANTENNAS+1];      
int trx1corrupted[NUMBER_ANTENNAS+1];
int trx2corrupted[NUMBER_ANTENNAS+1];
int tsys1corrupted[NUMBER_ANTENNAS+1];
int tsys2corrupted[NUMBER_ANTENNAS+1];
double trx[NUMBER_ANTENNAS+1], trx2[NUMBER_ANTENNAS+1];
short hotloadType[NUMBER_ANTENNAS+1];
int rxList[3];
int printedNullPointer[NUMBER_ANTENNAS+1] = {0,0,0,0,0,0,0,0,0};
#if 0
float thermometerOffset = 0.0; /* this is now implemented in calvaneservo */
/*      thot += thermometerOffset; */
#endif

#if 1
/* RWW Patch to make the rpc calls thread safe the way Todd does in tune.c */
rawResults *(*rr[9])() = { (rawResults *(*)())0,
  &rawrequest1_1, &rawrequest2_1, &rawrequest3_1, &rawrequest4_1,
  &rawrequest5_1, &rawrequest6_1, &rawrequest7_1, &rawrequest8_1};
#define rawrequest_1(r, c) rr[antenna](r, c)
#endif

/* tsysnew.c */
int CallRawRequest(int antenna, char *req);

void usage(int exitcode, char *error, char *addl) {
  if (error) {
    fprintf(stderr, "\n%s: %s\n\n", error, addl);
  }
  printf("Usage: tsys [options] \n"
	  "[options] include:\n"
	  "  -d or --debug   print debugging info\n"
	  "  -h or --help    this help\n"
	  "  -i or --integration     time in seconds (default = %d)\n",
	  integrate);
  printf("  -r or --receiverTemp    also compute the receiver temperature by using the "
         "                          heated load\n"
	 "  -a<n> or --antenna <n> (n is the antenna number)\n"
         "                  (default: all antennas)\n");
  exit(exitcode);
}

void *manageRPC(void *antnumber) {
  int rm_status;
  int antenna;
#if 0
  CLIENT *cl;      
#endif
  struct timeval DebugTime;
  rawRequest rawCommandRequest;
  struct sigaction action, oldAction;
  struct itimerval interval, old_settings;
  int timestamp;
  float thot,tcold,yfactor,hotload,coldload;
  float hotload2,coldload2, sky, sky2;
  char serverName[10];
  rawResults *rawptr;
  float voltagearray[2];
  float voltage,voltage2;
  int i, response;

#if PRINT_QUANTUM_LIMIT
  double quantum,h,k,hvk,freq;
  double freq[3];

  h = 6.63e-27;
  k = 1.38e-16;
  if (dsm_open_status == 0 && receiverTempFlag) {
    getFrequencies(freq);
  }
#endif
  antenna = (int)antnumber;
#if 0
  antenna = *((int *)antnumber);
  trx1corrupted[antenna] = 0;
  trx2corrupted[antenna] = 0;
  tsys1corrupted[antenna] = 0;
  tsys2corrupted[antenna] = 0;
#endif
  
  /*************************************************/
  /* create an RPC client to that antenna/hostname */
  /*************************************************/

  if (debug) {
    fprintf(stderr,"Creating tune client call to antenna %d\n",antenna);
  }
  sprintf(serverName,"acc%d",antenna);

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
  if (!(clp[antenna] = clnt_create(serverName, TUNEPROG, TUNEVERS, "tcp"))) {
    clnt_pcreateerror(serverName);
    return((void *)1);
  } else {
    if (debug) {
      printf("Client created to %s\n",serverName);
    }
  }
  callSetitimer(ITIMER_REAL, &old_settings, NULL); /* alarm off */
  DebugTime.tv_sec = 5;
  DebugTime.tv_usec = 0;
  if (clnt_control(clp[antenna], CLSET_TIMEOUT, (char *)&DebugTime) == FALSE) {
    fprintf(stderr,"manageRPC(%d) - can't change client timeout value\n",antenna);
  }

  /* put in the unheated load */
  if(CallRawRequest(antenna, "unheatedload in") != 0) {
    return((void *)2); 
  }

  /* read the unheated load value */
  /* for the purposes of y-factor, the unheatedload is the coldest load */
  /* syncdet2 integrates for one sec intervals and writes the result in
   * reflective memory, so after the load is in position, it is necessary
   * to wait for 2 sec to get a clean reading.  Wait one sec now and one later.
   */
  sleep(1);
  coldload = coldload2 = 0;
  for (i=0; i<integrate; i++) {
    /* do the sleep first to be sure the system has settled */
    sleep(1);
#if USE_MUWATTS
    rm_read(antenna,"RM_CONT_DET_MUWATT_V2_F",voltagearray);
    voltage  = voltagearray[0];
    voltage2 = voltagearray[1];
#else
    rm_read(antenna,"RM_CONT1_DET1_F",&voltage);
    rm_read(antenna,"RM_CONT2_DET1_F",&voltage2);
#endif
    coldload += voltage/integrate;
    coldload2 += voltage2/integrate;
    if (debug) {
      printf("ambient load voltage = %f\n",voltage);
    }
  }
  if (coldload > SATURATION_VOLTAGE) {
    printf("Ant%d: IF1 rx total power detector saturated on the unheated load --> tsys corrupted (*)\n",antenna);
    tsys1corrupted[antenna] = 1;
  } 
  if (coldload2 > SATURATION_VOLTAGE) {
    printf("Ant%d: IF2 rx total power detector saturated on the unheated load --> tsys corrupted (*)\n",antenna);
    tsys2corrupted[antenna] = 1;
  } 
  if (debug) {
    printf("average unheated load = %f volts\n",coldload);
  }
  /* in this case, HOTLOAD means the ambient load, and coldload also is the
   * ambientload (compared to the heated load)
   */
  rm_status = rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
  rm_status = rm_write(antenna,"RM_HOTLOAD_LOWFREQ_VOLTS_F",&coldload);
  rm_status = rm_write(antenna,"RM_HOTLOAD_HIGHFREQ_VOLTS_F",&coldload2);
  rm_status = rm_write(antenna,"RM_HOTLOAD_LOWFREQ_VOLTS_TIMESTAMP_L",&timestamp);
  rm_status = rm_write(antenna,"RM_HOTLOAD_HIGHFREQ_VOLTS_TIMESTAMP_L",&timestamp);
  rm_status = rm_write(antenna,"RM_UNHEATEDLOAD_TIMESTAMP_L",&timestamp);

  /* check hotload type */  
  rm_status = rm_read(antenna,"RM_HOTLOAD_STYLE_S",&hotloadType[antenna]);
#if 0
  if (receiverTempFlag && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
    /* remove unheated load and put in hotload */
#if USE_BOTHLOADS_COMMAND
    strcpy(rawCommandRequest.requestString,  "bothloads out in");
#else
    strcpy(rawCommandRequest.requestString,  "heatedload in");
#endif
    if (debug) {
      printf("Sending command = %s\n",rawCommandRequest.requestString);
    }
    rawptr = rawrequest_1(&rawCommandRequest,cl);
    response = processResponse(rawptr,antenna);
    if (response < 0) {
      clnt_destroy(cl);
      return((void *)3); 
    }
    
    strcpy(rawCommandRequest.requestString,  "unheatedload out");
    if (debug) {
      printf("Sending command = %s\n",rawCommandRequest.requestString);
    }
    rawptr = rawrequest_1(&rawCommandRequest,cl);
    response = processResponse(rawptr,antenna);
    if (response < 0) {
      clnt_destroy(cl);
      return((void *)4); 
    }

    /* read the heatedload value */
    /* syncdet2 integrates for one sec intervals and writes the result in
     * reflective memory, so after the load is in position, it is necessary
     * to wait for 2 sec to get a clean reading.  Wait one sec now and one later
     */
    sleep(1);
    hotload = hotload2 = 0;
    for (i=0; i<integrate; i++) {
    /* do the sleep first to be sure the system has settled */
      sleep(1);
#if USE_MUWATTS
      rm_read(antenna,"RM_CONT_DET_MUWATT_V2_F",voltagearray);
      voltage  = voltagearray[0];
      voltage2 = voltagearray[1];
#else
      rm_read(antenna,"RM_CONT1_DET1_F",&voltage);
      rm_read(antenna,"RM_CONT2_DET1_F",&voltage2);
#endif
      hotload += voltage/integrate;
      hotload2 += voltage2/integrate;
      if (debug) {
	printf("heated load voltage = %f\n",voltage);
      }
    }

    if (hotload > SATURATION_VOLTAGE) {
      printf("Ant%d: IF1 rx total power detector saturated on the heated load --> trx corrupted (*)\n",antenna);
      trx1corrupted[antenna] = 1;
    } 
    if (hotload2 > SATURATION_VOLTAGE) {
      printf("Ant%d: IF2 rx total power detector saturated on the heated load --> trx corrupted (*)\n",antenna);
      trx2corrupted[antenna] = 1;
    } 
    if (debug) {
      printf("average heatedload = %f volts\n",hotload);
    }
    if (rxList[1] != 0) {
      /* compute Trx */
      yfactor = hotload/coldload;
      if (debug) {
	printf("yfactor(1) = %f\n",yfactor);
      }
      /*    fprintf(stderr,"Yfactor = %g\n",yfactor);*/
      rm_status = rm_read(antenna,"RM_HEATEDLOAD_TEMPERATURE_F",&thot);
      rm_status = rm_read(antenna,"RM_UNHEATEDLOAD_TEMPERATURE_F",&tcold);
#define CELSIUS_TO_KELVIN 273.15
      thot += CELSIUS_TO_KELVIN;
#if 0
      if (debug) {
	printf("Using offset of %+f degrees for the hotload temperature\n");
      }
#endif
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
      yfactor = hotload2/coldload2;
      rm_status = rm_read(antenna,"RM_HEATEDLOAD_TEMPERATURE_F",&thot);
      rm_status = rm_read(antenna,"RM_UNHEATEDLOAD_TEMPERATURE_F",&tcold);
      thot += CELSIUS_TO_KELVIN;
      tcold += CELSIUS_TO_KELVIN;
      trx2[antenna] = (thot-yfactor*tcold)/(yfactor-1);
      if (debug) {
	fprintf(stderr,"Yfactor(2) = %g\n",yfactor);
	printf("Trx(2) = %f\n",trx2[antenna]);
      }
      rm_status = rm_write(antenna,"RM_TRX2_TIMESTAMP_L",&timestamp);
      rm_status = rm_write(antenna,"RM_TRX2_D",&trx2[antenna]);
    }

  } else {
    if (receiverTempFlag) {
      printf("Sorry, but antenna %d does not have a two-temperature load. Trx not available.\n",antenna);
    }
  }
#endif
  /* remove both loads */
  if(CallRawRequest(antenna, "heatedload out") != 0) {
    return((void *)2); 
  }
  if(CallRawRequest(antenna, "unheatedload out") != 0) {
    return((void *)2); 
  }
#if 0
#if USE_BOTHLOADS_COMMAND
  strcpy(rawCommandRequest.requestString,  "bothloads out");
#else
  strcpy(rawCommandRequest.requestString,  "heatedload out"); 
#endif
  if (debug) {
    printf("Sending command = %s\n",rawCommandRequest.requestString);
  }
  rawptr = rawrequest_1(&rawCommandRequest,cl);
  response = processResponse(rawptr,antenna);
  if (response < 0) {
    clnt_destroy(cl);
    return((void *)5); 
  }

  /* make sure that the unheated load is out */
  strcpy(rawCommandRequest.requestString,  "unheatedload out");
  if (debug) {
    printf("Sending command = %s\n",rawCommandRequest.requestString);
  }
  rawptr = rawrequest_1(&rawCommandRequest,cl);
  response = processResponse(rawptr,antenna);
#endif
  clnt_destroy(clp[antenna]);
  if (response < 0) return((void *)6); 

  /* while we integrate on the sky, calvaneservo will update the Tsys value in RM */
  /* syncdet2 integrates for one sec intervals and writes the result in
   * reflective memory, so after the load is in position, it is necessary
   * to wait for 2 sec to get a clean reading.  Wait one sec now and one later.
   */
  sleep(1);
  sky = 0;
  sky2 = 0;
  for (i=0; i<2; i++) {
    /* do the sleep first to be sure the system has settled */
    sleep(1);
#if USE_MUWATTS
    rm_read(antenna,"RM_CONT_DET_MUWATT_V2_F",voltagearray);
    voltage  = voltagearray[0];
    voltage2 = voltagearray[1];
#else
    rm_read(antenna,"RM_CONT1_DET1_F",&voltage);
    rm_read(antenna,"RM_CONT2_DET1_POWER_MUWATT_F",&voltage2);
#endif
    sky += voltage/2;
    sky2 += voltage2/2;
    if (debug) {
      printf("sky voltage = %f\n",voltage);
    }
  }
  if (debug) {
    printf("Average sky = %f\n",sky);
  }
  return((void *)0);
}

int main(int argc, char *argv[]) {
  char c;
#if 0
  int offsetFlag=0;
  pthread_t thread[NUMBER_ANTENNAS+1];
  int rpcThreadArg = 0; /* argument to send to new thread */
#endif
  int rm_status;
  int antlist[RM_ARRAY_SIZE];
  smapoptContext optCon;
  int antenna;
#if 0
  int antptr[NUMBER_ANTENNAS+1];
#endif
  int gotantenna=0;
  int antennaArray[SMAPOPT_MAX_ANTENNAS+1];
  int antennas[SMAPOPT_MAX_ANTENNAS+1];
  int numberOfAnts;
  int iant, ant;
  int ptrc;
#if 0
  pthread_attr_t attr;
  struct sched_param fifo_param;
#endif
  double tsys[NUMBER_ANTENNAS+1], tsys2[NUMBER_ANTENNAS+1];

  struct  smapoptOption optionsTable[] = {
    {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
    {"debug",'d',SMAPOPT_ARG_NONE,&debug,'d'},
    {"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a'},
    {"integration",'i',SMAPOPT_ARG_INT,&integrate,'i'},
    {"receiverTemp",'r',SMAPOPT_ARG_NONE,&receiverTempFlag,'r'},
    {NULL,0,0,NULL,0}
  };

  optCon = smapoptGetContext("tsys", argc, argv, optionsTable,0);
  
  while ((c = smapoptGetNextOpt(optCon)) >= 0) {
    switch(c) {
    case 'h':
      usage(0,NULL,NULL);
      break;
    case 'a':
      gotantenna=1;
      break;
    case 'r':
      receiverTempFlag=1;
      break;
    case 'd':
      debug = 1;
      break;
    }
  }
  if (c < -1) {
    fprintf(stderr, "%s: %s\n",
	    smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
	    smapoptStrerror(c));
  }
  smapoptFreeContext(optCon);
  
  /* initializing ref. mem. */
  rm_status = rm_open(antlist);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"rm_open()");
    exit(1);
  }

  if (gotantenna==1) {
    if (debug) {
      printf("Saw -a\n");
    }
    numberOfAnts=0;
    for(iant=1;iant<NUMBER_ANTENNAS+1;iant++) {
      if(antennaArray[iant]==1) {
	antennas[numberOfAnts]=iant;
	numberOfAnts++;
      }
    }
    if (numberOfAnts < 1) {
      printf("No antennas specified.  Nothing done.\n");
      exit(1);
    }
    if (debug) {
      printf("Saw %d antennas\n",numberOfAnts);
    }
  } else {
    /* from the current project */
    getAntennaList(antennaArray);
    numberOfAnts=0;
    for(iant=1; iant<=NUMBER_ANTENNAS; iant++) {
      if (antennaArray[iant]==1) {
	numberOfAnts++;
      }
    }
    if (numberOfAnts == 0) {
      printf("Sorry, but no antennas are in use in the project.\n");
      exit(0);
    }
  }
#if PRINT_QUANTUM_LIMIT
  dsm_open_status = dsm_open();
#endif
  getReceiverList(rxList);
  /* spawn threads for each antenna */
  if (rxList[1] == 0 && rxList[2] == 0) {
    /*
    printf("No receivers are in use in the project. I assume you want the low-freq receiver.\n");
    */
    rxList[1] = 1;
  }
  for(iant=1; iant<=NUMBER_ANTENNAS; iant++) {
    if (antennaArray[iant]==1) {
      if (debug) {
	printf("Spawning thread to ant %d\n",iant);
      }
      if ((ptrc = pthread_create(&tuneRxTId[iant], NULL, manageRPC,
					(void *)iant) != 0)) {
#if 0
      antptr[iant] = iant;
					(void *)&antptr[iant]) != 0)) {
#endif
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
  for (ant = 1; ant <= NUMBER_ANTENNAS; ant++) {
    if (antennaArray[ant] == 1) {
      int retCode;
      void *status;

      retCode = pthread_join(tuneRxTId[ant], &status);
      if(retCode != 0 || (int)status != 0) {
	fprintf(stderr,
	    "The tsys thread for ant %d returned code %d, value %d\n",
	    ant, retCode, (int)status);
      }
      if (debug) {
	printf("Ant %d thread terminated\n", ant);
      }
    }
  }
#if PRINT_QUANTUM_LIMIT
  dsm_close();
#endif
  /* print out the Tsys value from RM to the screen */
  for (antenna=1; antenna<=8; antenna++) {
    if (antennaArray[antenna]==1) {
      if (rxList[1] != 0 && rxList[2]==0) {
	/* only the low-freq receiver */
	rm_status=rm_read(antenna,"RM_TSYS_D",&tsys[antenna]);
	if (fabs(tsys[antenna]) > WACKO_TEMP) {
	  printf("Ant%d: Tsys =    wacko",antenna);
	} else {
	  printf("Ant%d: Tsys = %8.1f",antenna,tsys[antenna]);
	  if (tsys1corrupted[antenna] == 1) {
	    /*	    printf(" (corrupted)");*/
	    printf("*");
	  }
	}
	if (receiverTempFlag && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
	  if (fabs(trx[antenna] > WACKO_TEMP)) {
	    printf(" Trx  = wacko");
	  } else {
	    printf(" Trx  = %8.1f",trx[antenna]);
	    if (trx1corrupted[antenna] == 1) {
	      printf("*");
	    }
	  }
	}
	printf("\n");
      }
      if (rxList[1] != 0 && rxList[2]!=0) {
	/* both receivers */
	rm_status=rm_read(antenna,"RM_TSYS_D",&tsys[antenna]);
	rm_status=rm_read(antenna,"RM_TSYS2_D",&tsys2[antenna]);
	printf("Ant%d: Tsys = ",antenna);
	printf("%8.1f",tsys[antenna]);
	if (tsys1corrupted[antenna] == 1) {
	  printf("*");
	}
	printf(" %8.1f",tsys2[antenna]);
	if (tsys2corrupted[antenna] == 1) {
	  printf("*");
	}
	if (receiverTempFlag && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
	  printf(" Trx  = ");
	  printf("%8.1f",trx[antenna]);
	  if (trx1corrupted[antenna] == 1) {
	    printf("*");
	  } else {
	    printf(" ");
	  }
	  printf("%8.1f",trx2[antenna]);
	  if (trx2corrupted[antenna] == 1) { 
	    printf("*  ");
	  }
	}
	printf("\n");
      }
      if (rxList[1] == 0 && rxList[2]!=0) {
	/* only the high-freq receiver */
	rm_status=rm_read(antenna,"RM_TSYS2_D",&tsys2[antenna]);
	if (fabs(tsys2[antenna]) > WACKO_TEMP) {
	  printf("Ant%d: Tsys = wacko\n",antenna);
	} else {
	  printf("Ant%d: Tsys = %8.1f",antenna,tsys2[antenna]);
	  if (tsys2corrupted[antenna] == 1) {
	    printf("*");
	    /*	    printf(" (corrupted)");*/
	  }
	  if (receiverTempFlag && hotloadType[antenna] == NEW_HOTLOAD_STYLE) {
	    printf(" Trx  = %8.1f",trx2[antenna]);
	    if (trx2corrupted[antenna] == 1) {
	      printf("*");
	    }
	  }
	  printf("\n");
	}
      }
    }
  }
  rm_close();
  return(0);
}

void getFrequencies(double freq[3]) {
  dsm_structure smaStruct;
  char structureName[80];
  int dsm_status;
  time_t timestamp;

  strcpy(structureName, "DDS_TO_HAL_X");
  dsm_status = dsm_structure_init(&smaStruct, structureName);
  dsm_status = dsm_read("newdds",structureName,&smaStruct,&timestamp);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status,"dsm_structure_init(SMA_METEOROLOGY_X)");
    exit(17);
  }    
  dsm_structure_get_element(&smaStruct,"FREQ_V3_D",freq);
}

int processResponse(rawResults *result, int antenna) {
  FILE *fp;

  if(result != NULL && result->status == 0) return(0);
  fp = fopen("/dev/stderr", "w");
  if (result == NULL) {
    if (printedNullPointer[antenna] == 0) {
      printNullPointerMessage(antenna);
      printedNullPointer[antenna] = 1;
    }
    fprintf(fp, "tsys ant %d NULL pointer\n", antenna);
  } else {
    fprintf(fp, "tsys result status %d, warning %s\n", result->status,
	result->warningBuffer);
  }
  fclose(fp);
    return(-1);
  /*
    printf("Status: %s\n",result->resultString);
    if (result->status == 0) {
      printf("%s",result->warningBuffer);
    }
  return(0);
  */
}

void printNullPointerMessage(int antenna) {

  printf("Received a null pointer from antenna %d.  This usually means that the RPC client\n", antenna);
  printf("timeout limit was reached.  As such, the microcontroller will not respond to\n");
  printf("your next command until it completes the previous command.  Be patient. It is\n");
  printf("possible that tune6 is stale on antenna %d, in which case acc%d must be rebooted.\n",antenna,antenna);
}

static void serialCleanup(int sig) {
  FILE *fp;

  if((fp = fopen("/dev/stderr", "w")) == NULL) {
    fprintf(stderr, "WARNING Timeout could not be written to /dev/stderr\n");
  } else {
    fprintf(fp, "tsys RPC call timeout sig %d\n", sig);
    fclose(fp);
  }
}

void callSetitimer(int which, struct itimerval *interval,  
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

int CallRawRequest(int antenna, char *req) {
  rawRequest rawCommandRequest;
  rawResults *rawptr;
  int response;

  strcpy(rawCommandRequest.requestString, req);
  if (debug) {
    printf("Sending command = %s\n",rawCommandRequest.requestString);
  }
  rawptr = rawrequest_1(&rawCommandRequest,clp[antenna]);
  response = processResponse(rawptr,antenna);
  if (response < 0) {
    clnt_destroy(clp[antenna]);
    return(2); 
  }
  return(0);
}
