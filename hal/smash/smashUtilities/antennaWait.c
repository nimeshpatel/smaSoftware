/* accepts as arguments: list of antennas, some specification of the target
 * acquire limits
 * Does not return until all antennas have arrived on source.
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include "rm.h" 
#include "dsm.h"
#include "commonLib.h"
#include "smapopt.h"
#include "astrophys.h"
#include "antennaWait.h"
#include "stderrUtilities.h"

#define ANT_LIST_SZ (SMAPOPT_MAX_ANTENNAS + 1)

#define TRUE (1)
#define FALSE (0)

static int requestedAntennas[ANT_LIST_SZ];
int timeout = 300;

void signalHandler(int signum)
{
  char errorMessage[MAX_OPMSG_SIZE+1];

  sprintf(errorMessage, "antennaWait taking more than %d seconds - antenna stalled?",
	  abs(timeout));
  sendOpMessage(OPMSG_WARNING, 11, 60, errorMessage);
  if (timeout < 0)
    exit(-1);
  else
    alarm(timeout);
}

static void usage(char *a) {
  fprintf(stderr,"Usage: %s -a <antennaList> -b <beamFraction (default=0.1)> -f <frequency> -e <errorInArcSeconds> -s <sourceName> -v \n",a);
  fprintf(stderr,"  This command will not return until all specified antennas show a tracking\n");
  fprintf(stderr,"  error less than some critical value.  The default behavior is to use all\n");
  fprintf(stderr,"  antennas in the project, and 0.1*beamsize at the observing rest frequency.\n");
  fprintf(stderr,"  Other valid combinations are: -b, -b & -f, or -s\n");
  fprintf(stderr,"  Use -v for verbose output, which prints a running summary at 1Hz.\n");
  fprintf(stderr,"  Use -o to force antennaWait to wait for offline antennas.\n");
  fprintf(stderr,"  If -s is present, it will first wait for the source to appear in RM\n");
}

int main(int argc, char *argv[]) {
  int rm_status;
  smapoptContext optCon; 
  char c;
  int rm_antlist[RM_ARRAY_SIZE];
  int s;
  int offline = FALSE;
  time_t timestamp;
  double frequencyGHz, restFrequency[2];
  double acquireLimitArcSecond;
  double acquireLimitCriterion[RM_ARRAY_SIZE];
  int i,j,ant;
  int debug = 0;
  int antennasGiven = 0;
  int help;
  double beamFraction = 0.1;
  int verbose = 0;
  int units = ANTENNA_WAIT_DEFAULT;
  char *cp;
  char *sourceName = INITIAL_SMAPOPT_STRING_ARGUMENT;
  struct sigaction action, oldAction;

  struct smapoptOption options[] = {
    {"antenna", 'a', SMAPOPT_ARG_ANTENNAS, requestedAntennas,'a',"antenna argument (comma or space delimited)"},
    {"beamfraction", 'b', SMAPOPT_ARG_DOUBLE, &beamFraction, 'b', 
     "fraction of the beam required for pointing acquisition"},
    {"error", 'e', SMAPOPT_ARG_DOUBLE, &acquireLimitArcSecond, 'e', 
     "number of arcseconds required for pointing acquisition"},
    {"frequencyGHz", 'f', SMAPOPT_ARG_DOUBLE, &frequencyGHz,'f', "observing frequency for determing fractional beamsize"},
    {"help", 'h', SMAPOPT_ARG_NONE, &help, 'h', "print the usage"},
    {"offline", 'o', SMAPOPT_ARG_NONE, &offline, 'o', 
     "include antennas which have been marked offline"},
    {"source", 's', SMAPOPT_ARG_STRING, &sourceName, 's', 
     "do not start waiting until this source is in RM on the antenna"},
    {"timeout", 't', SMAPOPT_ARG_INT, &timeout, 't',
     "time in seconds to wait before issuing an operator message"},
    {"verbose", 'v', SMAPOPT_ARG_NONE, &verbose, 'v', "print out a line as each atenna arrives"},
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 }
  };

  if((cp = strrchr(argv[0], '/')) == NULL) {
    cp = argv[0];
  } else {
    cp++;
  }
  optCon = smapoptGetContext(cp, argc, argv, options, 0);
  if (verbose) {
    if (strcmp(sourceName,INITIAL_SMAPOPT_STRING_ARGUMENT) != 0 ) {
      printf("Read source name from cmdline = %s\n",sourceName);
    } else {
      printf("Did not read a source name from the cmdline\n");
    }
  }
  s = dsm_open();
  if (s != DSM_SUCCESS) {
    dsm_error_message(s, "dsm_open");
    exit(-1);
  } 
#if 0
  s = dsm_read("m5",
	       "DSM_AS_IFLO_REST_FR_D",
	       (char *)&restFrequency,
	       &timestamp);
#else
  s = dsm_read("m5",
	       "DSM_AS_IFLO_REST_FR_V2_D",
	       (char *)restFrequency,
	       &timestamp);
#endif
  frequencyGHz = restFrequency[0]*(1.0e-9);
  if (verbose) {
    fprintf(stderr,"Read rest frequency of low-freq receiver from DSM = %f GHz\n",frequencyGHz);
  }
  dsm_close();
  while ((c = smapoptGetNextOpt(optCon)) >= 0) {
    if (debug) {
      fprintf(stderr,"parsing %c\n",c);
    }
    switch (c) {
    case 'e':
      units = ANTENNA_WAIT_ARC_SECONDS_COMMON;
      if (acquireLimitArcSecond <= 0) {
	fprintf(stderr,"Argument to -e must be positive valued\n");
	usage(argv[0]);
	exit(-2);
      }
      break;
    case 'b':
      units = ANTENNA_WAIT_BEAM_FRACTION_COMMON;
      if (beamFraction <= 0) {
	fprintf(stderr,"Argument to -b must be positive valued\n");
	usage(argv[0]);
	exit(-3);
      }
      break;
    case 'f':
      if (frequencyGHz <= 20) {
	fprintf(stderr,"Argument to -f must be > 20\n");
	usage(argv[0]);
	exit(-4);
      }
      units = ANTENNA_WAIT_BEAM_FRACTION_COMMON;
      break;
    case 'o':
      offline = TRUE;
      break;
    case 'a':
      antennasGiven = 1;
      break;
    case 'h':
      usage(argv[0]);
      exit(0);
    }
  }
  smapoptFreeContext(optCon);
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = signalHandler;
  sigaction(SIGALRM, &action, &oldAction);
  if (timeout != 0)
    alarm(abs(timeout));
  if (antennasGiven == 0) {
     /* If antenna option not given, default to those in array */
    if (verbose) {
      fprintf(stderr,"No antennas specified, will use those in the project\n");
    }
    getAntennaList(requestedAntennas);
  }
  if (!offline) {
    int dsm_status, i;
    char online_list[11];
    time_t time_stamp;
    
    dsm_status = dsm_open();
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_open");
      exit(-1);
    }
    dsm_status = dsm_read("hal9000", "DSM_ONLINE_ANTENNAS_V11_B", &online_list[0], &time_stamp);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_read");
    } else
      for (i = 0; i < 11; i++)
	if (!online_list[i]) {
	  printf("Setting %d offine\n", i);
	  requestedAntennas[i] = FALSE;
	}
  }
  /* initializing ref. mem. */
  rm_status=rm_open(rm_antlist);
  if(rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"rm_open()");
    exit(-6);
  }
  for(i = j = 0; (ant = rm_antlist[i]) != RM_ANT_LIST_END; i++) {
    if (requestedAntennas[ant]) {
      rm_antlist[j++] = ant;
    }
  }
  rm_antlist[j] = RM_ANT_LIST_END;
  switch (units) {
  case ANTENNA_WAIT_ARC_SECONDS_COMMON:
    acquireLimitCriterion[0] = acquireLimitArcSecond;
    break;
  case ANTENNA_WAIT_DEFAULT:
  case ANTENNA_WAIT_BEAM_FRACTION_COMMON:
    acquireLimitCriterion[0] = beamFraction;
    break;
  }
  antennaWait(rm_antlist,units,acquireLimitCriterion,frequencyGHz,sourceName,verbose);
  rm_close();
  return(0);
}

