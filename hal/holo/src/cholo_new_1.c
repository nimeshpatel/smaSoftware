/*    cholo.c                                                                */
/*    First version Nov. 25, 2002                                            */
/*    KS added -m option. Dec.10, 2002                                       */
/*                                                                           */
/*   This program is a little interferometric five-point routine that runs   */
/* on hal9000.   It gets its flux measurements from dataCatcher via DSM, so  */
/* dataCatcher must be running in order for this program to work.  This pro- */
/* gram is primarily intended for doing a quick pointing check before or     */
/* during an astronomical track.                                             */
/*                                                                           */

#include <malloc.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <string.h>  /* KS: */
#include <rpc/rpc.h>
#include <unistd.h>
#define NRANSI 1
#include "astrophys.h"
#include "dsm.h"
#include "rm.h"
#include "smapopt.h"
#include "integration.h"
#include "standard.h"
#include "ph.h"

#define N_ANTENNAS 11
#define NULL_POINTER_LIMIT 2
#define OK 0
#define ERROR -1
#define RUNNING 1
#define DEG_TO_RAD (M_PI/180.0)
#define ARCSEC_TO_RAD (M_PI/(180.0 * 3600.0))
#define TRACK_LATENCY 4
#define PRINT_DSM_ERRORS 1
#define MIN_MAX_COH 0.1

int scanLength = 30;
int successiveNullPointers = 0;
int planetMode = FALSE;
int rm_status,antlist[RM_ARRAY_SIZE];
int antennaList[N_ANTENNAS];
int badAntenna[N_ANTENNAS] = {0,0,0,0,0,0,0,0,0,0,0};
int debug = FALSE;
int useMir = FALSE;
int quiet = FALSE;
int type = FALSE;
int integration = 60;
int nAntennas = 0;
int nAntennaGains, rep;
int nParameters;
int firstRep = TRUE;
int repetitions = 1;
int sidebandFlag = 0;
int line = FALSE;

double hWHM = -1.0;
double offsetStep = -1.0;
double oldAzO[N_ANTENNAS], oldElO[N_ANTENNAS];
float repAzO[100][N_ANTENNAS], repElO[100][N_ANTENNAS];
float maxCoh = -1.0e31;
float maxCohAnt[N_ANTENNAS] = {-1.0e31, -1.0e31, -1.0e31, -1.0e31, -1.0e31,
			       -1.0e31, -1.0e31, -1.0e31, -1.0e31, -1.0e31,
			       -1.0e31};
int maxCohPos[N_ANTENNAS];
float cohScans = 0.0;
float newAzO[N_ANTENNAS], newElO[N_ANTENNAS];
float amp[64][N_ANTENNAS][N_ANTENNAS][2], phase[64][N_ANTENNAS][N_ANTENNAS][2];
int cAntennas[N_ANTENNAS];
float data[64][N_ANTENNAS][N_ANTENNAS];
float model[N_ANTENNAS][N_ANTENNAS];

time_t timestamp;

int savedPlot, savedIDL;
double savedScanLength;
intgCommand command;
int activeCrate[13];
char crateName[10], lowestCrateName[10];
int lowestActiveCrate, crate;
CLIENT *cl[13];
intgCommand cmd;
intgParameters newParams;
int correlatorFuckedUp = FALSE;
int gotGoodOffsets = FALSE;

void abortCleanup(void);

void recordIpointData(FILE *fp, int antcode, int pointid);   /* KS: */
FILE *ipnfp;                 /* KS: file pointer to the ipoint data file */ 
int antcode = 0;             /* KS: antenna code */


statusCheck(intgStatus *status)
/*
  Check the return status of a request to the integration server on the
  correlator crate.
*/
{
  if (status == NULL) {
    if (successiveNullPointers++ > NULL_POINTER_LIMIT) {
      fprintf(stderr,
              "Too many (%d) succesive NULL pointers received - aborting\n",
              successiveNullPointers);
      exit(-1);
    }
  } else {
    successiveNullPointers = 0;
    if (status->status != INTG_SUCCESS)
      printf("integration server returned error status, reason = %d\n",
             status->reason);
  }
}

/*
  This function is essentially just a function version of the SMAsh
  eloff command.
*/
void newElOff(int ant, double elOff)
{
  short pmac_command_flag = 0;
  short error_flag = RUNNING;
  int i;
  char command_n[30], message[100];

  for (i = 0; i < 30; i++)
    command_n[i] = (char)0;
  command_n[0] = 'P';
  rm_status = rm_write(ant, "RM_SMASH_ELOFF_ARCSEC_D", &elOff);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newElOff first write");
    abortCleanup();
    exit(-1);
  }
  rm_status = rm_write(ant, "RM_SMASH_TRACK_COMMAND_C30", &command_n);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newElOff second write");
    abortCleanup();
    exit(-1);
  }
  rm_status = rm_write_notify(ant, "RM_SMARTS_PMAC_COMMAND_FLAG_S",
			      &pmac_command_flag);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newElOff third write");
    abortCleanup();
    exit(-1);
  }  
  rm_status = rm_read(ant, "RM_SMARTS_COMMAND_STATUS_S",
		      &error_flag);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"RM_SMARTS_COMMAND_STATUS_S");
    abortCleanup();
    exit(-1);
  }
  if (error_flag == ERROR) {
    fprintf(stderr, "newElOff: error from Track:\n");
    rm_status = rm_read(ant, "RM_TRACK_MESSAGE_C100", message);
    if (rm_status != RM_SUCCESS) {
      rm_error_message(rm_status,"RM_TRACK_MESSAGE_C100");
      abortCleanup();
      exit(-1);
    }
    fprintf(stderr, "%s\n", message);
  }
}

/*
  This function is essentially just a function version of the SMAsh
  azoff command.
*/
void newAzOff(int ant, double azOff)
{
  short pmac_command_flag = 0;
  short error_flag = RUNNING;
  int i;
  char command_n[30], message[100];

  for (i = 0; i < 30; i++)
    command_n[i] = (char)0;
  command_n[0] = 'O';
  rm_status = rm_write(ant, "RM_SMASH_AZOFF_ARCSEC_D", &azOff);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newAzOff first write");
    abortCleanup();
    exit(-1);
  }
  rm_status = rm_write(ant, "RM_SMASH_TRACK_COMMAND_C30", &command_n);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newAzOff second write");
    abortCleanup();
    exit(-1);
  }
  rm_status = rm_write_notify(ant, "RM_SMARTS_PMAC_COMMAND_FLAG_S",
			      &pmac_command_flag);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newAzOff third write");
    abortCleanup();
    exit(-1);
  }  
  rm_status = rm_read(ant, "RM_SMARTS_COMMAND_STATUS_S",
		      &error_flag);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"newAzOff first read");
    abortCleanup();
    exit(-1);
  }
  if (error_flag == ERROR) {
    fprintf(stderr, "newAzOff: error from Track:\n");
    rm_status = rm_read(ant, "RM_TRACK_MESSAGE_C100", message);
    if (rm_status != RM_SUCCESS) {
      rm_error_message(rm_status,"newAzOff first read");
      abortCleanup();
      exit(-1);
    }
    fprintf(stderr, "%s\n", message);
  }
}

/*
  Check for error returns from DSM calls
*/
void dsmErrorProcess(int errorCode, char *text)
{
  if (errorCode != DSM_SUCCESS) {
    dsm_error_message(errorCode, "dsm_write");
    fprintf(stderr, "errorCode = %d\n", errorCode);
  }
}

void correlatorPause(int pause)
{
  intgCommand pauseRequest;

  if (pause)
    pauseRequest.command = INTG_PAUSE_ON;
  else
    pauseRequest.command = INTG_PAUSE_OFF;
  for (crate = 1; crate < 13; crate++)
    if (cl[crate])
      statusCheck(intgcommand_1(&pauseRequest, cl[crate]));
}

/*
  Restore the initial correlator state (integration time, plotting, etc.
*/
void restoreCorrelatorState(void)
{
  if (correlatorFuckedUp) {
    correlatorPause(FALSE);
    if (savedIDL < 0) {
      command.command = INTG_DONT_IDL;
      for (crate = 1; crate < 13; crate++)
	if (cl[crate])
	  statusCheck(intgcommand_1(&command, cl[crate]));
    }
    if (savedPlot >= 0) {
      command.command = INTG_DISPLAY_ON;
      for (crate = 1; crate < 13; crate++)
	if (cl[crate])
	  statusCheck(intgcommand_1(&command, cl[crate]));
    }
    newParams.time = (int)savedScanLength;
    newParams.scanNumber = 0;
    sprintf(newParams.sourceName, "0");
      for (crate = 1; crate < 13; crate++)
	if (cl[crate])
	  statusCheck(intgsetparams_1(&newParams, cl[crate]));
  }
}

void abortCleanup(void)
{
  int dsm_status;
  short mode = 1;
  
  dsm_status = dsm_write("m5", "DSM_AS_POINTING_MODE_S",
			 (char *)&mode);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status, "dsm_write");
    perror("point: dsm write of DSM_AS_POINTING_MODE_S");
  }
  fprintf(stderr,
	  "Executing emergency cleanup function\n");
  restoreCorrelatorState();
/*
  restoreInitialOffsets(FALSE); 
  if ((antcode != 0) && (useMir))
*/
}

/*
  Signal handler to catch ^C and restore initial correlator state and
  antenna offsets before exiting.
*/
void signalHandler(int signum) {
  if (signum == SIGINT) {  
    fprintf(stderr,"Received ^C - will try to restore initial state.\n");
    abortCleanup();
    exit(-1);
  }
  else { 
    fprintf(stderr,"point received unexpected signal #%d\n",signum);
  }
}

/*
  This function gathers the data for one of the five points.   The
  "point" argument specifies which of the 5 points is being observed
  (0 through 4).
*/
void getFluxes(int point)
{
  int s, sb, a1, a2, ok, nScans, scanCount;
  long firstScan, corNum, mirNum, good, status[N_ANTENNAS];
  float tAmp[N_ANTENNAS][N_ANTENNAS][2], tPhase[N_ANTENNAS][N_ANTENNAS][2],
    real[N_ANTENNAS][N_ANTENNAS][2], imag[N_ANTENNAS][N_ANTENNAS][2],
    coh[N_ANTENNAS][N_ANTENNAS][2], aveCoh, nCohValues;

  sleep(scanLength/2);
  /*
    Calculate how many of the short scans must be summed to get the
    requested per-point integration time.
  */
  nScans = integration/scanLength;
  if (integration % scanLength)
    nScans++;
  if (nScans < 1)
    nScans = 1;
  for (sb = 0; sb < 2; sb++)
    for (a1 = 0; a1 < N_ANTENNAS; a1++)
      for (a2 = 0; a2 < N_ANTENNAS; a2++)
	real[a1][a2][sb] = imag[a1][a2][sb] = 0.0;
  scanCount = 0;
  s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (char *)&corNum, &timestamp);
  dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_CORNUM_L)");

  /*
    Wait for the next scan to come in
  */
  if (!quiet) {
    printf("Waiting for next scan to come in\x0d");
    fflush(stdout);
  }
  firstScan = corNum;
  while (firstScan == corNum) {
    long halTime;

    halTime = time(NULL);
    s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (char *)&corNum, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_CORNUM_L)");
    if (abs((long)timestamp - halTime) > 300) {
      fprintf(stderr,
	      "DSM scan information not updated in > 5 minutes.    I think\n");
      fprintf(stderr, "dataCatcher is not running so I'll abort here.\n");
      abortCleanup();
      exit(-1);
    }
    usleep(100000);
  }

  /*
    Now wait for a scan to come in for which all the baselines are flagged
    as good.
  */

  if (!quiet) {
    printf("Now wait for the first good scan at this position\x0d");
    fflush(stdout);
  }
  do {
    s = dsm_read("m5", "DSM_AS_ANT_STATUS_V11_L",
		 (char *)status, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_ANT_STATUS_V11_L)");
    ok = 1;
    for (a1 = 1; a1 < N_ANTENNAS; a1++)
      for (a2 = 1; a2 < N_ANTENNAS; a2++)
	if ((a1 < a2) && antennaList[a1] && antennaList[a2])
	  if ((status[a1] != 1) || (status[a2] != 1))
	    ok = 0;
    usleep(100000);
  } while (!ok);
  aveCoh = nCohValues = 0.0;
  while (scanCount < nScans) {
    if (!quiet) {
      printf("Summing scan %d of %d                                 \x0d",
	     scanCount+1, nScans);
      fflush(stdout);
    }
    s = dsm_read("m5", "DSM_AS_SCAN_CORR_V11_V11_V2_F",
		 (char *)coh, &timestamp);
    s = dsm_read("m5", "DSM_AS_SCAN_AMP_V11_V11_V2_F",
		 (char *)tAmp, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_AMP_V11_V11_V2_F)");  
    s = dsm_read("m5", "DSM_AS_SCAN_PHASE_V11_V11_V2_F",
		 (char *)tPhase, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_PHASE_V11_V11_V2_F)");
    s = dsm_read("m5", "DSM_AS_ANT_STATUS_V11_L",
		 (char *)status, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_ANT_STATUS_V11_L)");
    s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (char *)&corNum, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_CORNUM_L)");
    s = dsm_read("m5", "DSM_AS_SCAN_MIRNUM_L", (char *)&mirNum, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_MIRNUM_L)");
    if (debug && useMir)
      printf("Corr #%d, MIR #%d\n", corNum, mirNum);
    for (sb = 0; sb < 2; sb++) {
      for (a1 = 1; a1 < N_ANTENNAS; a1++)
	for (a2 = 1; a2 < N_ANTENNAS; a2++)
	  if ((a1 < a2) && antennaList[a1] && antennaList[a2]) {
	    /*
	      Convert amp & phase produced by dataCatcher into
	      real and imaginary for easier vector addition
	    */
	    aveCoh += coh[a1][a2][sb];
	    if (maxCohAnt[a1] < coh[a1][a2][sb]) {
	      maxCohAnt[a1] = coh[a1][a2][sb];
	      maxCohPos[a1] = point;
	    }
	    if (maxCohAnt[a2] < coh[a1][a2][sb]) {
	      maxCohAnt[a2] = coh[a1][a2][sb];
	      maxCohPos[a2] = point;
	    }
	    nCohValues += 1.0;
	    real[a1][a2][sb] +=
	      tAmp[a1][a2][sb] * (float)cos(DEG_TO_RAD*(double)tPhase[a1][a2][sb]);
	    imag[a1][a2][sb] +=
	      tAmp[a1][a2][sb] * (float)sin(DEG_TO_RAD*(double)tPhase[a1][a2][sb]);
	    if (debug)
	      printf("tAmp[%d][%d][%d] = %f\t tPhase[%d][%d][%d[] = %f\n",
		     a1, a2, sb, tAmp[a1][a2][sb]*10000.0,
		     a1, a2, sb, tPhase[a1][a2][sb]);
	  }
    }
    scanCount++;
    /*
      If we need more scans, loop here until a new scan comes.
    */
    if (scanCount < nScans) {
      firstScan = corNum;
      while (firstScan == corNum) {
	s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L",
		     (char *)&corNum, &timestamp);
	dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_CORNUM_L)");
	usleep(100000);
      }
    }
  }
  if (nCohValues > 0.0)
    aveCoh /= nCohValues;
  if (maxCoh < aveCoh)
    maxCoh = aveCoh;
  if (debug)
    printf("\nAverage coherence = %f\n", aveCoh);

  /*
    We're done summing the data - now convert back to amp & phase
  */
  for (sb = 0; sb < 2; sb++)
    for (a1 = 1; a1 < N_ANTENNAS; a1++)
      for (a2 = 1; a2 < N_ANTENNAS; a2++)
	if ((a1 < a2) && antennaList[a1] && antennaList[a2]) {
	  amp[point][a1][a2][sb] =
	    (float)sqrt((double)(real[a1][a2][sb]*real[a1][a2][sb] +
				 imag[a1][a2][sb]*imag[a1][a2][sb]));
	  phase[point][a1][a2][sb] =
	    (float)atan2((double)real[a1][a2][sb],
			 (double)imag[a1][a2][sb])/DEG_TO_RAD;
	  if (debug)
	    printf("Final amp[%d][%d][%d][%d] = %f\tphase[%d][%d][%d][%d] = %f\n",
		   point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
		   point, a1, a2, sb, phase[point][a1][a2][sb]);
	}
  cohScans = nScans;
}

/*
  This function returns the value of a normalized gaussian beam
  at offset position azOff, elOff
*/
float beam(float azOff, float elOff)
{
  float r2, value;

  r2 = ((azOff*azOff)+(elOff*elOff))/(hWHM*hWHM);
  value = exp(-0.693147*r2);
  return(value);
}

/*-----------------------------------------*/

main(int argc, char **argv)
{
  short solSysFlag;
  short rxSB;
  int rc, ant, status, i, a1, a2, point, ok;
  int lowestAntennaNumber;
  int help = FALSE;
  int usage = FALSE;
  int stuff = FALSE;
  int partial = FALSE;
  int log = FALSE;
  int Log = FALSE;
  int unwise = TRUE;
  int antennasSpecified = FALSE;
  int chunk = -1;
  int block = -1;
  int center = -1;
  int width = -1;
  double fRest, lambda;
  double planetDiameter;
  char *sideband = "nothing";
  int antennas[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char mirdir[81];      /* KS: name of the MIR data directory */
  char filename[256];          /* KS: fname */
  char * ipnfile = "ipn.dat"; /* KS: name of the ipoint data file */
  time_t now;
  struct tm *nowValues;
  FILE *antLog[N_ANTENNAS];
  struct ph antPH[N_ANTENNAS];
  struct sigaction action, oldAction;
  smapoptContext optCon;
  struct smapoptOption options[] = {
    {"antennas", 'a', SMAPOPT_ARG_ANTENNAS, &antennas[0], 0, "Specify a list of antennas to move"},
    {"block", 'b', SMAPOPT_ARG_BLOCK, &block, 0, "Block where line appears"},
    {"chunk", 'c', SMAPOPT_ARG_CHUNK, &chunk, 0, "Chunk where line appears"},
    {"Center", 'C', SMAPOPT_ARG_INT, &center, 0, "Channel of line center"},
    {"debug", 'd', SMAPOPT_ARG_NONE, &debug, 0, "Turn on debugging messages"},
    {"integation", 'i', SMAPOPT_ARG_INT, &integration, 0, "Integration time per point in seconds"},
    {"log", 'l', SMAPOPT_ARG_NONE, &log, 0, "Print results in the SMAsh log file"},
    {"Log", 'L', SMAPOPT_ARG_NONE, &Log, 0, "Store results in a standard pointing log"},
    {"mir", 'm', SMAPOPT_ARG_NONE, &useMir, 0, "Store addition info for offline analysis"},
    {"offset", 'o', SMAPOPT_ARG_DOUBLE, &offsetStep, 0, "Offset step size in arc seconds"},
    {"partial", 'p', SMAPOPT_ARG_NONE, &partial, 0, "Just apply 1/2 of derived offset values"},
    {"quiet", 'q', SMAPOPT_ARG_NONE, &quiet, 0, "Suppress normal status messages"},
    {"repetitions", 'r', SMAPOPT_ARG_INT, &repetitions, 0, "Number of pointing cycles"},
    {"stuff", 's', SMAPOPT_ARG_NONE, &stuff, 0, "Apply derived offsets at end of all reps."},
    {"Sideband", 'S', SMAPOPT_ARG_STRING, &sideband, 0, "Sideband containing line"},
    {"type", 't', SMAPOPT_ARG_NONE, &type, 0, "Type out the results"},
    /*
    {"unwise", 'u', SMAPOPT_ARG_NONE, &unwise, 0, "Do fit even if center point isn't strongest"},
    */
    {"width", 'w', SMAPOPT_ARG_INT, &width, 0, "Width of spectral line"},
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 },
    "This command does interferometric pointing.   It may be executed within a script\nand can be told to apply the derived offsets automatically."
  };   
  intgCommand request;

  optCon = smapoptGetContext(argv[0], argc, argv, options, SMAPOPT_CONTEXT_EXPLAIN);
  
  if ((rc = smapoptGetNextOpt(optCon)) < -1) {
    fprintf(stderr, "point: bad argument %s: %s\n", 
            smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS), 
	    smapoptStrerror(rc));
    return 2;
  }

  /* 
     KS: set useMir true regardless of the -m option. The option is kept in
     case we need it later.
  */
  useMir = TRUE;

  
  if (help) {
    smapoptPrintHelp(optCon, stdout, 0);
    return 0;
  } if (usage) {
    smapoptPrintUsage(optCon, stdout, 0);
    return 0;
  }
  Log = TRUE;
  action.sa_flags = 0;
  sigemptyset(&action.sa_mask);
  action.sa_handler = signalHandler;
  sigaction(SIGINT, &action, &oldAction);
  status = dsm_open();
  if(status != DSM_SUCCESS) {
    dsmErrorProcess(status, "dsm_open");
    exit(-1);
  }

  if (strcmp(sideband, "nothing")) {
    switch (sideband[0]) {
    case 'l':
      sidebandFlag = -1;
      break;
    case 'u':
      sidebandFlag = 1;
      break;
    default:
      fprintf(stderr, "Illegal sideband \"%s\" specified - aborting\n");
      exit(-1);
    }
  }
  /*
  printf("width = %d, chunk = %d, block = %d, sideband = \"%s\"\n",
	 width, chunk, block, sideband);
  */
  if ((width >= 0) || (chunk > 0) || (block > 0) || strcmp(sideband, "nothing"))
    line = TRUE;
  else
    line = FALSE;
  /*
  printf("%d %d %d %d %d\n",
	 (width >= 0), (chunk > 0) ,(block > 0) ,strcmp(sideband, "nothing"), sidebandFlag);
  */
  if (line && ((width < 0) || (chunk < 0) || (block < 0) ||
      ((sidebandFlag != -1) && (sidebandFlag != 1)))) {
    fprintf(stderr,
	    "In line pointing you must specify a block (-b), chunk (-c), width (-w) and sideband (-S)\n");
    exit(-1);
  }
  if (line && (center <= 0))
    center = 64;
  if (debug && line)
    printf("Line mode pointing: block %d, chunk %d, center %d, width %d\n",
	   block, chunk, center, width);
  if (line) {
    int dsm_status;
    short mode = 2;
    
    dsm_status = dsm_write("m5", "DSM_AS_POINTING_MODE_S",
			   (char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("point: dsm write of DSM_AS_POINTING_MODE_S");
    }
    mode = sidebandFlag;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_SIDEBAND_S",
			(char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_SIDEBAND_S");
    }
    mode = block;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_BLOCK_S",
			   (char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_BLOCK_S");
    }
    mode = chunk;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_CHUNK_S",
			   (char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_CHUNK_S");
    }
    mode = center - width/2;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_BCHAN_S",
			   (char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_BCHAN_S");
    }
    mode = center + width/2;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_ECHAN_S",
			   (char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_ECHAN_S");
    }
  } else {
    int dsm_status;
    short mode = 1;
    
    dsm_status = dsm_write("m5", "DSM_AS_POINTING_MODE_S",
			   (char *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("point: dsm write of DSM_AS_POINTING_MODE_S");
    }
  }
  lowestActiveCrate = getCrateList(&activeCrate[0]);
  if (lowestActiveCrate < 0) {
    fprintf(stderr, "Error returned by getCrateList - aborting\n");
    exit(-1);
  } else if (lowestActiveCrate == 0) {
    fprintf(stderr, "No crates are active - aborting\n");
    exit(-1);
  } else
    sprintf(lowestCrateName, "crate%d", lowestActiveCrate);

  status = dsm_read("m5",
		    "DSM_AS_IFLO_REST_FR_D",
		    (char *)&fRest,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_AS_IFLO_REST_FR_D");
    exit(-1);
  }
  status = dsm_read("m5", "DSM_AS_IFLO_SIDEBAND_S", (char *)&rxSB, &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_AS_IFLO_SIDEBAND_S");
    exit(-1);
  }
  if (rxSB == 1)
    fRest += 5.0e9;
  else
    fRest -= 5.0e9;
  if (fRest != 0.0)
    lambda = SPEED_OF_LIGHT/fRest;
  else {
    fprintf(stderr, "LO frequency works out to 0 Hz - aborting\n");
    exit(-1);
  }
  
  if (hWHM == -1) {  
    hWHM = 1.2 * lambda / 6.0;
    hWHM *= 1800.0 * (180.0/M_PI);
  }
  if (debug)
    printf("hWHM = %f\n", hWHM);
  if (offsetStep == -1.0) {
    offsetStep = hWHM;
    if (!quiet)
      printf("No offset step given, will use Half Width at Half Maximum: %5.1f\"\n",
	     offsetStep);
  }
  for (ant = 1; ant < N_ANTENNAS; ant++)
    if (antennas[ant])
      antennasSpecified = TRUE;

  if (antennasSpecified) {
    fprintf(stderr, "Sorry - you cannot yet specify an explicit list of antennas - aborting\n");
    exit(-1);
  }
  if (debug) {
    printf("%s was passed the following parameters:\n", argv[0]);
    printf("stuff = %d\tpartial = %d\tdebug = %d\tintegration = %d\n",
	   stuff, partial, debug, integration);
    printf("log = %d\t\toffset = %f\ttype = %d\n",
	   log, offsetStep, type);
    if (antennasSpecified) {
      printf("The following antennas were explicitly specified to move:\n");
      for (ant = 1; ant < N_ANTENNAS; ant++)
	if (antennas[ant])
	  printf("%d ", ant);
      printf("\n");
    }
  }
  if (debug)
    printf("Get antenna list\n");
  lowestAntennaNumber = getAntennaList(antennaList);
  if (Log) {
    time(&now);
    nowValues = gmtime(&now);
  }
  for (ant = 1; ant < N_ANTENNAS; ant++)
    if (antennaList[ant]) {
      char antLogName[100];

      if (Log) {
	sprintf(antLogName, "/data/engineering/ipoint/ant%d/%02d%02d%02d", ant,
		nowValues->tm_year - 100,
		nowValues->tm_mon + 1,
		nowValues->tm_mday);
	antLog[ant] = fopen(antLogName, "a");
	if (antLog[ant] == NULL) {
	  perror("Opening antenna pointing log file");
	  exit(-1);
	}
      }
      nAntennas++;
    } else if (antennasSpecified) {
      char antLogName[100];

      if (antennas[ant]) {
	fprintf(stderr,
		"Antenna %d was not included in the project command - you cannot point it.\n",
		ant);
	exit(-1);
      }
      if (Log) {
	sprintf(antLogName, "/data/engineering/ipoint/ant%d", ant);
	antLog[ant] = fopen(antLogName, "a");
	if (antLog[ant] == NULL) {
	  perror("Opening antenna pointing log file");
	  exit(-1);
	}
      }
    }
   if (debug) {
    printf("Number of antennas = %d (", nAntennas);
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant])
	printf("%d ", ant);
    printf(")\n");
  }
  rm_status = rm_open(antlist);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"rm_open()");
    abortCleanup();
    exit(1);
  }

  /*
  nAntennaGains = (nAntennas * (nAntennas-1)) / 2;
  */
  nAntennaGains = nAntennas;
  nParameters = nAntennaGains + 2*nAntennas;

/*  if (debug) {
    printf("Put the correlator into the proper state for pointing,\n");
    printf("MIR output on, FITS output off, plotting off, 2 second scans\n");
  } */
  for (crate = 1; crate < 13; crate++) {
    if (activeCrate[crate]) {
      sprintf(crateName, "crate%d", crate);
      if (debug)
	printf("Trying to connect to %s\n", crateName);
      if (!(cl[crate] = clnt_create(crateName, INTGPROG, INTGVERS, "tcp"))) {
	clnt_pcreateerror(crateName);
      }
    } else
      cl[crate] = NULL;
  }
  
  status = dsm_read(lowestCrateName,
		    "DSM_CH_PLOT_ERRORS_L", 
		    (char *)&savedPlot,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_CH_PLOT_ERRORS_L");
  }
  status = dsm_read(lowestCrateName,
		    "DSM_CH_IDL_ERRORS_L", 
		    (char *)&savedIDL,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_CH_IDL_ERRORS_L");
  }
  status = dsm_read(lowestCrateName,
		    "DSM_CH_SCAN_LENGTH_D", 
		    (char *)&savedScanLength,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_CH_SCAN_LENGTH_D");
  }
  command.command = INTG_DONT_STORE;
  for (crate = 1; crate < 13; crate++)
    if (cl[crate])
      statusCheck(intgcommand_1(&command, cl[crate]));
  command.command = INTG_DO_IDL;
  for (crate = 1; crate < 13; crate++)
    if (cl[crate])
  statusCheck(intgcommand_1(&command, cl[crate]));
  command.command = INTG_DISPLAY_OFF;
  for (crate = 1; crate < 13; crate++)
    if (cl[crate])
      statusCheck(intgcommand_1(&command, cl[crate]));
  newParams.time = scanLength;
  newParams.scanNumber = 0;
  sprintf(newParams.sourceName, "0");
  for (crate = 1; crate < 13; crate++)
    if (cl[crate])
      statusCheck(intgsetparams_1(&newParams, cl[crate]));
  correlatorFuckedUp = TRUE;
  if (repetitions <= 0)
    repetitions = 1;
/* Raster starts
	newAzOff(ant, oldAzO[ant]+az_step);
	newElOff(ant, oldElO[ant]+el_step);
   startAz
   startEl */ 
	az_step=7734.0/freq;el_step=az_step; /*23.3 for 332 GHz, and 33.3 for 232.4 GHz*/
        start_time = time(NULL);
        ts = *localtime(&start_time);
        printf ("Start raster! \n");
        start_time = time(NULL);
        Start_time = start_time;
        printf ("Starting raster on antenna %d\n", ant_scn);
        azoff=((float)n_size/2.+1.)*az_step;
        eloff=((float)n_size/2.)*el_step - n_start*el_step;
	newAzOff(ant, oldAzO[ant]+azoff);
	newElOff(ant, oldElO[ant]+eloff);
        for (j=n_start; j<n_size; j++){  /*do one row*/
                i=0;
        currentAzoff=0;
                start_time = time(NULL);
                sprintf(command, "azoff -a %d", ant_scn);
                printf ("%s \n", command);

                sleep(1);
                istatus=system (command);

                printf("%6d %6d  %6f\n",istatus,currentAzoff,azoff);
                printf("%6d  %6f \n",currentAzoff,azoff); fflush(stdout);
/*              while((1.0*currentAzoff)>-azoff){*/
                while((1.0*currentAzoff)>(-(azoff+offsetunit))){
                        for (i_delay=1; i_delay<n_delay; i_delay++){}
                        adc0_rd=read(adc0_fd, amp_vvm[1].byte, count);
                        if(adc0_rd<=0){
                                printf("cannot read device 0\n");
                                exit(1);
                                close(adc0_fd);
                                close(adc1_fd);
                        }
 /* usleep(1); */
                        adc1_rd=read(adc1_fd, pha_vvm[1].byte, count);
                        if(adc1_rd<=0){
                                printf("cannot read device 0\n");
                                exit(1);
                                close(adc0_fd);
                                close(adc1_fd);
                        }
/* read encoders using encoderClient */
                        encoderClient(ant_scn,&az_enc,&el_enc,&m1009,&m1010);

                        az_read[i]=az_enc;el_read[i]=el_enc;
                        amp_read[i]=amp_vvm[1].word;
                        phase_read[i]=pha_vvm[1].word;

                        rm_status=rm_read(ant_scn,"RM_AZOFF_D",&currentAzoffD);
                        if(rm_status != RM_SUCCESS) {
                        rm_error_message(rm_status,"rm_read()");
                        exit(1);
                        }/*if end*/
                        currentAzoff=(short)currentAzoffD;
/* printf("line# %3d %3d %4d %6.2f %6.2f %6.2f %5d %d\n",j,i,currentAzoff,azoff,az_enc,el_enc,amp_read[i],icount); fflush(stdout); */

                        i++;
                }/*while end*/

                sprintf(command, "stopScan -a %d", ant_scn);
                system (command);
                sleep(1);
                printf ("command: %s \n", command);
        /*      system (command);
                usleep(10); */
                eloff = eloff - el_step;
                sprintf(command, "eloff -a %d -s %f", ant_scn, eloff);
                system (command);
                sleep(1);
                printf ("command: %s ", command);fflush(stdout);
        /*      system (command);
                usleep(10); */
                sprintf(command, "azoff -a %d -s %f", ant_scn, azoff+offsetunit*3);
                system (command);
                sleep(1);
                printf ("command: %s ", command);fflush(stdout);
        /*      system (command); */

                for(k=1;k<i;k++){
                        fprintf(fp_raw_data,"%10.6f %10.6f %6d %6d %3d\n" ,\
                        el_read[k],az_read[k],amp_read[k],phase_read[k],j);
                        fflush(fp_raw_data);
                }/*for end*/
                /* Place for including online dispaly command */
                printf ("Flush the data\n");
                cur_time=time(NULL);
                printf ("%3dth line: Stop time: %d time taken: %d secs samples: %d\n", \
                j, cur_time, cur_time-start_time, i);
                sleep(4);
/*              sprintf(command, "azoff -a %d -s %f", ant_scn, azoff);
                printf ("command: %s ", command);fflush(stdout);
                system (command);
                usleep(10);
                sprintf(command, "eloff -a %d -s %f", ant_scn, eloff);
                printf ("command: %s ", command);fflush(stdout);
                system (command);
                usleep(10);*/
        }/* for end; El loop done */

        printf ("Start: %d Stop: %d Time taken: %d secs\n", Start_time, cur_time, cur_time - Start_time);
        sprintf(command, "azoff -a %d -s 0", ant_scn);
        system (command);
        usleep(100000);
        printf ("command: %s ", command);
        sprintf(command, "eloff -a %d -s 0", ant_scn);
        system (command);
        usleep(100000);
        printf ("command: %s\n", command);

/* The following part does 5-point; replaced it with raster above
  for (rep = 0; rep < repetitions; rep++) {
    if ((repetitions > 1) && (!quiet))
      printf("Starting cycle %d of %d\n", rep+1, repetitions);
    if ((rep !=0) && (mirdir != "null") && useMir)          /* KS */
      if ((ipnfp=fopen(filename,"a"))==NULL) {
        printf(" ipoint data file open error\n");
        abortCleanup();
        exit(1);
      }     
    if (!quiet)
      printf("Now go to the first offset position (-El)\n");
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant])
	newElOff(ant, oldElO[ant]-offsetStep);
    if (useMir && (mirdir != "null"))
      recordIpointData(ipnfp,antcode,5);  /* KS: ipoint info recording */
    getFluxes(0);
    if (!quiet)
      printf("Now go to the second offset position (+El)\n");
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant])
	newElOff(ant, oldElO[ant]+offsetStep);
    if (useMir && (mirdir != "null"))
      recordIpointData(ipnfp,antcode,4);  /* KS: ipoint info recording */
    getFluxes(1);
    if (!quiet)
      printf("Now go to the center position\n");
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant]) {
	newElOff(ant, oldElO[ant]);
	if (Log) {
	  int rmOpenFlag = 0;

	  sleep(TRACK_LATENCY);
	  antPH[ant]=readHeader(ant, rmOpenFlag, rmOpenFlag);
	}
      }
    if (useMir && (mirdir != "null"))
      recordIpointData(ipnfp,antcode,3);  /* KS: ipoint info recording */
    getFluxes(2);
    if (!quiet)
      printf("Now go to the third offset position (-Az)\n");
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant])
	newAzOff(ant, oldAzO[ant]-offsetStep);
    if (useMir && (mirdir != "null"))
      recordIpointData(ipnfp,antcode,2);  /* KS: ipoint info recording */
    getFluxes(3);
    if (!quiet)
      printf("Now go to the forth offset position (+Az)\n");
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant])
	newAzOff(ant, oldAzO[ant]+offsetStep);
    if (useMir && (mirdir != "null"))
      recordIpointData(ipnfp,antcode,1);  /* KS: ipoint info recording */
    getFluxes(4);
    if ((rep < (repetitions-1)) || (!stuff)) {
      if (!quiet)
	printf("Now return to the center\n");
      restoreInitialOffsets(TRUE);
    }

    firstRep = FALSE;
    if (rep < (repetitions-1))
      sleep(TRACK_LATENCY);
  }
*/
  for (ant = 1; ant < N_ANTENNAS; ant++)
    newAzO[ant] = newElO[ant] = 0.0;
 restore:
  restoreCorrelatorState();
  exit(OK);
}
