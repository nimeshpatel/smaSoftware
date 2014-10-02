#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <utsname.h>
#include <ctype.h>
#include "stripchart.h"
#include "smapopt.h"
#include "rm.h"
#include "tek_driv.h"
#include "coldload.h"
#define INITIAL_SMAPOPT_STRING_ARGUMENT "000000000"

char rmvar[RM_NAME_LENGTH];
int antennaNumber;
int runningOnHal9000 = 0;

void main(int argc, char *argv[]) {
  char *ptr;
  int help = 0;
  float bigBuffer[BIG_BUFFER_SIZE];
  float bigBufferRx1[BIG_BUFFER_SIZE];
  int rm_list[RM_ARRAY_SIZE];
  smapoptContext optCon; 
#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)
  int antennaArgument[ANT_LIST_SZ];
  int antennasGiven = 0;
  int DEBUG = 0;
  int startSecond, stopSecond;
  int rc, points, i, rms, narg, timestamp, npoints;
  int rx = 0;
  float avg, avgrx1;
  struct utsname unamebuf;
  int rmAnt;

  struct smapoptOption options[] = {
  {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, antennaArgument, 'a',"antenna argument (this supercedes -s)"},
  {"receiver",'r', SMAPOPT_ARG_INT, &rx, 'i', "which receiver to plot: 0=low-freq, 1=high-freq (default=0)"},
  {"debug",'D', SMAPOPT_ARG_NONE, &DEBUG, 0, "print debugging messages"},
  SMAPOPT_AUTOHELP
  { NULL, '\0', 0, NULL, 0 }
  };

  optCon = smapoptGetContext(argv[0], argc, argv, options, 0);
  if (DEBUG) {
    fprintf(stdout,"Finished smapoptGetContext()\n");
    fflush(stdout);
  }

  while ((rc = smapoptGetNextOpt(optCon)) >= 0) {
    switch(rc) {
    case 'a':
      antennasGiven = 1;
      break;
    case 'h':
      smapoptPrintHelp(optCon, stdout, 0);
      exit(0);
    }
  }
  if (DEBUG) {
    for (i=0; i<=8; i++) {
      fprintf(stderr,"antennaArgument[i] = %d\n",antennaArgument[i]);
    }
  }
  if (help) {
    smapoptPrintHelp(optCon, stdout, 0);
    return;
  }
  /************************************************************/
  /* figure out which antenna/hostname to send the command to */
  /************************************************************/
  uname(&unamebuf);
  if(strcmp(unamebuf.nodename,"hal9000") != 0) {
    runningOnHal9000 = 0;
    sscanf(unamebuf.nodename,"acc%d",&antennaNumber);
    /*
    printf("We are not running on hal9000. We are running on acc%d\n",antennaNumber);
    */
  } else {
    runningOnHal9000 = 1;
    printf("We are running on hal9000\n");
  }

  if (antennasGiven == 0) {
    if (runningOnHal9000==1) {
      printf("You must give an antenna number\n");
      return;
    }
  } else {
    if (DEBUG) {
      printf("We have seen the -a option\n");
    }
    if (runningOnHal9000==1) {
      for (i=1; i<=8; i++) {
	if (antennaArgument[i] == 1) {
	  antennaNumber = i;
	  break;
	}
      }
    } else {
      printf("It makes no sense to give -a when running from this program from an antenna computer\n");
      return;
    }
  }

  ptr = getenv("TERM");
  if (ptr == NULL) {
    printf("Sorry, but the terminal type is not defined.  You must run this command from an xterm window.\n");
    return;
  }
  if (!present(ptr,"xterm")) {
    printf("Sorry, but the terminal type is %s.  You must run this command from an xterm window.\n",ptr);
    return;
  }
  if (runningOnHal9000 && (antennaNumber <1 || antennaNumber >8)) {
    printf("Invalid antenna number = %d\n",antennaNumber);
    exit(0);
  }
  rms = rm_open(rm_list);
  printf("Go put the cold load in front of the ");
  printf(" ambient load for awhile (10-15 seconds),\n");
  printf("then come back here and hit control-C\n");
  npoints = stripchart(antennaNumber, rx, bigBuffer, bigBufferRx1);

input:
  printf("Now, judging from the graph, enter the start and stop time for the cold load:\n");
  narg = scanf("%d %d",&startSecond, &stopSecond);
  if (narg<2) {
    printf("You must enter two numbers, separated by spaces.\n");
    goto input;
  }
  if (startSecond<0 || stopSecond<0) {
    printf("Both numbers must be positive.\n");
    goto input;
  }
  if (stopSecond<startSecond) {
    printf("The first number must be less than the second.\n");
    goto input;
  }
  if (stopSecond >= npoints) {
    printf("You tried to select more data points than you measured!\n");
    goto input;
  }
  avg = avgrx1 = 0;
  points = 0;
  for (i=startSecond; i<=stopSecond; i++) {
    points++;
    avg += bigBuffer[i];
    avgrx1 += bigBufferRx1[i];
  }  
  avgrx1 /= points;
  avg /= points;
  printf("Averaging %d points, writing average values (%f,%f) to RM.  Thanks!\n",
	 points,avg,avgrx1);
  strcpy(rmvar,"RM_UNIX_TIME_L");
  if (runningOnHal9000 == 1) {
    rmAnt = antennaNumber;
  } else {
    rmAnt = 0;
  }
  rms = call_rm_read(rmAnt,rmvar,&timestamp);
  strcpy(rmvar,"RM_COLDLOAD_LOWFREQ_VOLTS_F");
  rms = call_rm_write(rmAnt,rmvar,&avg);
  strcpy(rmvar,"RM_COLDLOAD_HIGHFREQ_VOLTS_F");
  rms = call_rm_write(rmAnt,rmvar,&avgrx1);
  strcpy(rmvar,"RM_COLDLOAD_LOWFREQ_VOLTS_TIMESTAMP_L");
  call_rm_write(rmAnt,rmvar,&timestamp);
  strcpy(rmvar,"RM_COLDLOAD_HIGHFREQ_VOLTS_TIMESTAMP_L");
  call_rm_write(rmAnt,rmvar,&timestamp);
}

int call_rm_write(int ant, char *name, void *value) {
  char statusString[150];
  int rm_status;
  int debug = 0;
  if (debug) fprintf(stderr,"Entered call_rm_write()\n");
  sprintf(statusString,"call_rm_write(%s)",name);
  if (debug) fprintf(stderr,"calling rm_write(%s), ptr=0x%08x\n",name,value);
  rm_status = rm_write(ant,name,value);
  if (debug) fprintf(stderr,"finished rm_write()\n");
  checkReflectiveMemoryStatus(rm_status,statusString);
  return(rm_status);
}



