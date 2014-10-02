/*    point.c                                                                */
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
#include "nr.h"
#include "nrutil.h"
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
#define WUDGY 100
#define NDIM 64

int scanLength = 5;
int successiveNullPointers = 0;
int planetMode = FALSE;
int rm_status,antlist[RM_ARRAY_SIZE];
int antennaList[N_ANTENNAS];
int referenceList[N_ANTENNAS];
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
int i_grid = 0;
int n_size = 64;
int n_start = 0;
int n_Bore = 64;
int j,k;

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
float amp[NDIM*NDIM][N_ANTENNAS][N_ANTENNAS][2], phase[NDIM*NDIM][N_ANTENNAS][N_ANTENNAS][2];
int cAntennas[N_ANTENNAS];
float data[5][N_ANTENNAS][N_ANTENNAS];
float model[N_ANTENNAS][N_ANTENNAS];
double startAzOff, startElOff,CurrentAzOff, CurrentElOff,azoff,eloff,az_step,el_step;

time_t timestamp, start_time, cur_time, Start_time;

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

FILE *ipnfp;                 /* KS: file pointer to the ipoint data file */ 
int antcode = 0;             /* KS: antenna code */
void recordIpointData(FILE *fp, int antcode, int pointid);   /* KS: */


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
    dsm_error_message(errorCode, text);
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
  short mode;
  int dsm_status;

  mode = 1;  
  dsm_status = dsm_write("m5", "DSM_AS_POINTING_MODE_S",
			 (void *)&mode);
  if (dsm_status != DSM_SUCCESS) {
    dsm_error_message(dsm_status, "dsm_write");
    perror("point: dsm write of DSM_AS_POINTING_MODE_S");
  }

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

/*
  Restore the antenna offsets that were present when this program
  began.
*/
void restoreInitialOffsets(int azOnly)
{
  int ant;

  if (gotGoodOffsets) {
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant])
	newAzOff(ant, oldAzO[ant]);
    if (!azOnly) {
      sleep(TRACK_LATENCY);
      for (ant = 1; ant < N_ANTENNAS; ant++)
	if (antennaList[ant])
	  newElOff(ant, oldElO[ant]);
    }
  }
}

void abortCleanup(void)
{
  fprintf(stderr,
	  "Executing emergency cleanup function\n");
  restoreCorrelatorState();
  restoreInitialOffsets(FALSE);
  if ((antcode != 0) && (useMir))
    recordIpointData(ipnfp,antcode,0);  /* KS: abort ipoint info recording */
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
  s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (void *)&corNum, &timestamp);
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
    s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (void *)&corNum, &timestamp);
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
		 (void *)status, &timestamp);
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
		 (void *)coh, &timestamp);
    s = dsm_read("m5", "DSM_AS_SCAN_AMP_V11_V11_V2_F",
		 (void *)tAmp, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_AMP_V11_V11_V2_F)");  
    s = dsm_read("m5", "DSM_AS_SCAN_PHASE_V11_V11_V2_F",
		 (void *)tPhase, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_PHASE_V11_V11_V2_F)");
    s = dsm_read("m5", "DSM_AS_ANT_STATUS_V11_L",
		 (void *)status, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_ANT_STATUS_V11_L)");
    s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (void *)&corNum, &timestamp);
    dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_CORNUM_L)");
    s = dsm_read("m5", "DSM_AS_SCAN_MIRNUM_L", (void *)&mirNum, &timestamp);
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
		     (void *)&corNum, &timestamp);
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

/*
   KS:
   This function records information needed to reduce ipoint data in MIR.
   The information is written into a file.
*/
void recordIpointData(FILE *fp, int antcode, int pointid)
{
  int     s;
  long    kMirNum; /* integ number in MIR */

  s = dsm_read("m5", "DSM_AS_SCAN_MIRNUM_L", (void *)&kMirNum, &timestamp);
  dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_MIRNUM_L)");

  fprintf(fp,"%8d  %8d  %1d\n", kMirNum, antcode, pointid);
}

/*-----------------------------------------*/

main(int argc, char **argv)
{
  short solSysFlag, mode;
  short rxSB;
  int sb,rc, ant, status, i, a1, a2, point, ok;
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
  int dsm_status;
  double fRest, lambda;
  double planetDiameter;
  char *sideband = "nothing";
  int antennas[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int reference[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  char mirdir[81];      /* KS: name of the MIR data directory */
  char filename[256];          /* KS: fname */
  char file_name[80];
  char * ipnfile = "ipn.dat"; /* KS: name of the ipoint data file */
  time_t now;
  struct tm *nowValues,ts;
  FILE *antLog[N_ANTENNAS];
  FILE *fp_raw_data;
  struct ph antPH[N_ANTENNAS];
  struct sigaction action, oldAction;
  smapoptContext optCon;
  struct smapoptOption options[] = {
    {"antennas", 'a', SMAPOPT_ARG_ANTENNAS, &antennas[0], 0, "Specify a list of antennas to move"},
    {"reference", 'r', SMAPOPT_ARG_ANTENNAS, &reference[0], 0, "Specify a list of reference antennas"},
    {"block", 'b', SMAPOPT_ARG_BLOCK, &block, 0, "Block where line appears"},
    {"chunk", 'c', SMAPOPT_ARG_CHUNK, &chunk, 0, "Chunk where line appears"},
    {"Center", 'C', SMAPOPT_ARG_INT, &center, 0, "Channel of line center"},
    {"debug", 'd', SMAPOPT_ARG_NONE, &debug, 0, "Turn on debugging messages"},
    {"integation", 'i', SMAPOPT_ARG_INT, &integration, 0, "Integration time per point in seconds"},
    {"log", 'l', SMAPOPT_ARG_NONE, &log, 0, "Print results in the SMAsh log file"},
    {"Log", 'L', SMAPOPT_ARG_NONE, &Log, 0, "Store results in a standard pointing log"},
    {"mir", 'm', SMAPOPT_ARG_NONE, &useMir, 0, "Store addition info for offline analysis"},
    {"offset", 'o', SMAPOPT_ARG_DOUBLE, &offsetStep, 0, "Offset step size in arc seconds"},
/*    {"partial", 'p', SMAPOPT_ARG_NONE, &partial, 0, "Just apply 1/2 of derived offset values"}, */
    {"quiet", 'q', SMAPOPT_ARG_NONE, &quiet, 0, "Suppress normal status messages"},
/*    {"repetitions", 'r', SMAPOPT_ARG_INT, &repetitions, 0, "Number of pointing cycles"}, */
/*    {"stuff", 's', SMAPOPT_ARG_NONE, &stuff, 0, "Apply derived offsets at end of all reps."}, */
    {"Sideband", 'S', SMAPOPT_ARG_STRING, &sideband, 0, "Sideband containing line"},
    {"type", 't', SMAPOPT_ARG_NONE, &type, 0, "Type out the results"},
    /*
    {"unwise", 'u', SMAPOPT_ARG_NONE, &unwise, 0, "Do fit even if center point isn't strongest"},
    */
    {"width", 'w', SMAPOPT_ARG_INT, &width, 0, "Width of spectral line"},
    {"size", 'n', SMAPOPT_ARG_INT, &n_size, 0, "Raster size"}, 
    {"start", 's', SMAPOPT_ARG_INT, &n_start, 0, "Starting raster grid position"}, 
    {"Bore", 'B', SMAPOPT_ARG_INT, &n_Bore, 0, "Bore-sighting interval"}, 
    SMAPOPT_AUTOHELP
    { NULL, '\0', 0, NULL, 0 },
    "This command does interferometric celestial holography. It is essentially a copy of Taco's ipoint but for some
changes."
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
    mode = 2;
    dsm_status = dsm_write("m5", "DSM_AS_POINTING_MODE_S",
			   (void *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("point: dsm write of DSM_AS_POINTING_MODE_S");
    }
    mode = sidebandFlag;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_SIDEBAND_S",
			(void *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_SIDEBAND_S");
    }
    mode = block;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_BLOCK_S",
			   (void *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_BLOCK_S");
    }
    mode = chunk;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_CHUNK_S",
			   (void *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_CHUNK_S");
    }
    mode = center - width/2;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_BCHAN_S",
			   (void *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_BCHAN_S");
    }
    mode = center + width/2;
    dsm_status = dsm_write("m5", "DSM_AS_LINE_ECHAN_S",
			   (void *)&mode);
    if (dsm_status != DSM_SUCCESS) {
      dsm_error_message(dsm_status, "dsm_write");
      perror("dataCatcher: dsm write of DSM_AS_LINE_ECHAN_S");
    }
  } else {
    mode = 1;
    dsm_status = dsm_write("m5", "DSM_AS_POINTING_MODE_S",
			   (void *)&mode);
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
		    (void *)&fRest,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_AS_IFLO_REST_FR_D");
    exit(-1);
  }
  status = dsm_read("m5", "DSM_AS_IFLO_SIDEBAND_S", (void *)&rxSB, &timestamp);
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
  if (nAntennas < 3) {
    fprintf(stderr, "Only %d antennas are in the project list - 3 or more are needed\n",
	    nAntennas);
    exit(-1);
  } else if (debug) {
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
  rm_status = rm_read(lowestAntennaNumber, "RM_SOLSYS_FLAG_S",
		      &solSysFlag);
  if (rm_status != RM_SUCCESS) {
    rm_error_message(rm_status,"RM_SOLSYS_FLAG_S");
    abortCleanup();
    exit(-1);
  }
  if (debug)
    printf("solSysFlag = %d\n", solSysFlag);
  for (a1 = 0; a1 < N_ANTENNAS; a1++)
    for (a2 = 0; a2 < N_ANTENNAS; a2++)
      model[a1][a2] = 1.0;
  if (solSysFlag == 1) {
    double uMeters[N_ANTENNAS][N_ANTENNAS],
      vMeters[N_ANTENNAS][N_ANTENNAS];

    rm_status = rm_read(lowestAntennaNumber, "RM_PLANET_DIAMETER_ARCSEC_D",
			&planetDiameter);
    if (rm_status != RM_SUCCESS) {
      rm_error_message(rm_status,"RM_PLANET_DIAMETER_ARCSEC_D");
      abortCleanup();
      exit(-1);
    }
    if (debug)
      printf("The planet diameter is %f\n", planetDiameter);
    status = dsm_read("newdds", "DSM_DDS_HAL_U_V11_D",
		      (void *)&uMeters, &timestamp);
    if (status != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(status, "dsm_read - DSM_DDS_HAL_U_V11_D");
      exit(-1);
    }
    status = dsm_read("newdds", "DSM_DDS_HAL_V_V11_D",
		      (void *)&vMeters, &timestamp);
    if (status != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(status, "dsm_read - DSM_DDS_HAL_V_V11_D");
      exit(-1);
    }
    if (debug)
      printf("lambda = %f, fRest = %e\n", lambda, fRest);
/*    for (a1 = 0; a1 < N_ANTENNAS; a1++)
      for (a2 = 0; a2 < N_ANTENNAS; a2++)
	model[a1][a2] = fabs(diskVisibility((float)planetDiameter,
					     (float)(uMeters[a1] - uMeters[a2]),
					     (float)(vMeters[a1] - vMeters[a2]),
					     lambda
					     ));
 XXXXXXXXXXXX */
  }
  if (debug)
    for (a1 = 0; a1 < N_ANTENNAS; a1++) {
      for (a2 = 0; a2 < N_ANTENNAS; a2++)
	if (antennaList[a1] && antennaList[a2] && (a1 != a2))
	  printf("%d-%d: %f ", a1, a2, model[a1][a2]);
      printf("\n");
    }
  /*
  nAntennaGains = (nAntennas * (nAntennas-1)) / 2;
  */
  nAntennaGains = nAntennas;
  nParameters = nAntennaGains + 2*nAntennas;

  /* KS: open file to write i-point informatioon */
  if (useMir) {
    status = dsm_read("m5",
		      "DSM_AS_FILE_NAME_C80", 
		      mirdir,
		      &timestamp);
    if (status != DSM_SUCCESS) {
      if (PRINT_DSM_ERRORS)
	dsm_error_message(status, "dsm_read - DSM_AS_IFLO_REST_FR_D");
      exit(-1);
    }
    if (debug)
      printf("mir directory is \"%s\"\n", mirdir);
    if (mirdir != "null") {
      strcpy(filename,mirdir);
      strcat(filename,ipnfile);
      if (!quiet)
	printf("ipoint data file=%s\n",filename);
      if ((ipnfp=fopen(filename,"a"))==NULL) {
	printf(" ipoint data file open error\n");
	abortCleanup();
	exit(1);
      }
      for (ant = 1; ant <= N_ANTENNAS; ant++)
	if (antennaList[ant])
	  antcode = antcode + pow(2,ant-1); 
    }
  }
  if (debug) {
    printf("Put the correlator into the proper state for pointing,\n");
    printf("MIR output on, FITS output off, plotting off, 2 second scans\n");
  }
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
		    (void *)&savedPlot,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_CH_PLOT_ERRORS_L");
  }
  status = dsm_read(lowestCrateName,
		    "DSM_CH_IDL_ERRORS_L", 
		    (void *)&savedIDL,
		    &timestamp);
  if (status != DSM_SUCCESS) {
    if (PRINT_DSM_ERRORS)
      dsm_error_message(status, "dsm_read - DSM_CH_IDL_ERRORS_L");
  }
  status = dsm_read(lowestCrateName,
		    "DSM_CH_SCAN_LENGTH_D", 
		    (void *)&savedScanLength,
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
  if (debug)
    printf("Collect the original offsets from each antenna\n");
  for (ant = 1; ant < N_ANTENNAS; ant++)
    if (antennaList[ant]) {
      status = rm_read(ant, "RM_AZOFF_D", &oldAzO[ant]);
      if(status != RM_SUCCESS) {
	rm_error_message(status, "rm_read(RM_AZOFF_D)");
	abortCleanup();
	exit(1);
      }
      status = rm_read(ant, "RM_ELOFF_D", &oldElO[ant]);
      if(status != RM_SUCCESS) {
	rm_error_message(status, "rm_read(RM_ELOFF_D)");
	abortCleanup();
	exit(1);
      }
      if (!quiet)
	printf("For antenna #%d, intitial offsets: %f %f\n",
	       ant, oldAzO[ant], oldElO[ant]);
    }
  gotGoodOffsets = TRUE;
  if (repetitions <= 0)
    repetitions = 1;
/* Following part for raster; replaces 5-point */
/* do raster */
/* Raster starts */
        az_step=7734.0e9/fRest;el_step=az_step; /*23.3 for 332 GHz, and 33.3 for 232.4 GHz*/
  	if (offsetStep != -1.0) {
		az_step=offsetStep;
		el_step=offsetStep;
	}
        start_time = time(NULL);
        ts = *localtime(&start_time);
        sprintf(file_name, "/data/engineering/cholo/%02d%02d_%02d%02d.cholo",ts.tm_mon+1,ts.tm_mday,ts.tm_hour,ts.tm_min);
        printf("fRest=%4.1fGHz grid=%4.1f:%4.1f filename %s\n",(fRest/1.0e9),az_step,el_step,file_name);
        fp_raw_data = fopen(file_name,"w");
        if(fp_raw_data==NULL){
                printf("cannot open the data file\n");
                exit(1);
        }
        start_time = time(NULL);
        Start_time = start_time;
/* n_size is az size; n_start is el size */
        startAzOff=-((float)n_size/2.)*az_step;
        startElOff=-((float)n_start/2.)*el_step;
	if (n_size == 1){startAzOff = 0.0;}
	if (n_start == 1) {startElOff = 0.0;} 
	printf("Do boresight\n");
    	for (ant = 1; ant < N_ANTENNAS; ant++)
      		if (antennaList[ant] && !reference[ant]){
			CurrentAzOff=oldAzO[ant];
			CurrentElOff=oldElO[ant];
       			newAzOff(ant,CurrentAzOff);
			sleep(TRACK_LATENCY);
			newElOff(ant,CurrentElOff);
	}
	point=i_grid;
	getFluxes(i_grid);
  	for (sb = 0; sb < 2; sb++)
    	  for (a1 = 1; a1 < N_ANTENNAS; a1++)
      	    for (a2 = 1; a2 < N_ANTENNAS; a2++)
       	      if ((a1 < a2) && antennaList[a1] && antennaList[a2]) {
/*       		 	printf("Bore:amp[%d][%d][%d][%d] = %f\tphase[%d][%d][%d][%d] = %f %f %f\n",
               		point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
               		point, a1, a2, sb, phase[point][a1][a2][sb],
			azoff, eloff); */
fprintf(fp_raw_data,"Bore:A[%d][%d][%d][%d] = %6.2f\tP[%d][%d][%d][%d] = %6.2f %6.2f %6.2f %6.2f %6.2f\n", 
			point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
                        point, a1, a2, sb, phase[point][a1][a2][sb],
                        azoff, eloff, CurrentAzOff, CurrentElOff);
                        fflush(fp_raw_data);
		}
	azoff=startAzOff;
	eloff=startElOff;
    	for (ant = 1; ant < N_ANTENNAS; ant++)
      		if (antennaList[ant] && !reference[ant]){
			CurrentAzOff=azoff+oldAzO[ant];
       			newAzOff(ant, CurrentAzOff);
			sleep(TRACK_LATENCY);
			CurrentElOff=eloff+oldElO[ant];
        		newElOff(ant, CurrentElOff);
		}
        printf ("Ready to start map; hit Return to continue:");
        fgetc(stdin);
        printf ("Start raster! \n");
/* Elevation loop */
/* n_start is used as elevation size; n_size is used as azimuth size */
        for (j=0; j<n_start; j++){  /*step through rows*/
		for (k=0; k<n_size; k++){ /* do a row */
			i_grid=j*n_size+k;	
			point=i_grid;
			printf("Doing point %d: azoff = %f; eloff = %f \n", i_grid, azoff,eloff);
			getFluxes(i_grid);
			azoff+=az_step;
    			for (ant = 1; ant < N_ANTENNAS; ant++)
      				if (antennaList[ant] && !reference[ant]){
					CurrentAzOff=azoff+oldAzO[ant];
       					newAzOff(ant, CurrentAzOff);
				}
  			for (sb = 0; sb < 2; sb++)
    			  for (a1 = 1; a1 < N_ANTENNAS; a1++)
      			    for (a2 = 1; a2 < N_ANTENNAS; a2++)
        		      if ((a1 < a2) && antennaList[a1] && antennaList[a2]) {
/*            			printf("Grid:amp[%d][%d][%d][%d] = %f\tphase[%d][%d][%d][%d] = %f %f %f\n",
                   		point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
                   		point, a1, a2, sb, phase[point][a1][a2][sb],
				azoff, eloff); */
fprintf(fp_raw_data,"Grid:A[%d][%d][%d][%d] = %6.2f\tP[%d][%d][%d][%d] = %6.2f %6.2f %6.2f %6.2f %6.2f\n", 
			point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
                        point, a1, a2, sb, phase[point][a1][a2][sb],
                        azoff, eloff, CurrentAzOff, CurrentElOff);
                        fflush(fp_raw_data);
        	              }
		}
                cur_time=time(NULL);
                printf ("%3dth line: Stop time: %d time taken: %d\n", j, cur_time, cur_time-start_time);
/* do bore every so many grid points rather than rows */
		if( ((i_grid+1) % n_Bore) == 0.0 ){
			printf("Do boresight\n");
    			for (ant = 1; ant < N_ANTENNAS; ant++)
      				if (antennaList[ant] && !reference[ant]){
					CurrentAzOff=oldAzO[ant];
					CurrentElOff=oldElO[ant];
       					newAzOff(ant,CurrentAzOff);
					sleep(TRACK_LATENCY);
	  				newElOff(ant,CurrentElOff);
				}
			getFluxes(i_grid);
  			for (sb = 0; sb < 2; sb++)
    			  for (a1 = 1; a1 < N_ANTENNAS; a1++)
      			    for (a2 = 1; a2 < N_ANTENNAS; a2++)
        		      if ((a1 < a2) && antennaList[a1] && antennaList[a2]) {
/*            			printf("Bore:amp[%d][%d][%d][%d] = %f\tphase[%d][%d][%d][%d] = %f %f %f\n",
                   		point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
                   		point, a1, a2, sb, phase[point][a1][a2][sb],
				azoff, eloff); */
fprintf(fp_raw_data,"Bore:A[%d][%d][%d][%d] = %6.2f\tP[%d][%d][%d][%d] = %6.2f %6.2f %6.2f %6.2f %6.2f\n", 
			point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
                      	point, a1, a2, sb, phase[point][a1][a2][sb],
                       	azoff, eloff, CurrentAzOff, CurrentElOff);
                        fflush(fp_raw_data);
			      }
		}
		eloff+=el_step;
    		for (ant = 1; ant < N_ANTENNAS; ant++)
      			if (antennaList[ant] && !reference[ant]){
				azoff=startAzOff;
				CurrentAzOff=azoff+oldAzO[ant];
				newAzOff(ant, CurrentAzOff);
				sleep(TRACK_LATENCY);
				CurrentElOff=eloff+oldElO[ant];
				newElOff(ant,CurrentElOff);
			}
	}
			printf("Do boresight\n");
    			for (ant = 1; ant < N_ANTENNAS; ant++)
      				if (antennaList[ant] && !reference[ant]){
					CurrentAzOff=oldAzO[ant];
					CurrentElOff=oldElO[ant];
					newAzOff(ant,CurrentAzOff);
					sleep(TRACK_LATENCY);
					newElOff(ant,CurrentElOff);
				}
			getFluxes(i_grid);
  			for (sb = 0; sb < 2; sb++)
    			  for (a1 = 1; a1 < N_ANTENNAS; a1++)
      			    for (a2 = 1; a2 < N_ANTENNAS; a2++)
        		      if ((a1 < a2) && antennaList[a1] && antennaList[a2]) {
/*            			printf("Bore:amp[%d][%d][%d][%d] = %f\tphase[%d][%d][%d][%d] = %f %f %f\n",
                   		point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0,
                   		point, a1, a2, sb, phase[point][a1][a2][sb],
				azoff, eloff); */
fprintf(fp_raw_data,"Bore:A[%d][%d][%d][%d] = %6.2f\tP[%d][%d][%d][%d] = %6.2f %6.2f %6.2f %6.2f %6.2f\n", 
			point, a1, a2, sb, amp[point][a1][a2][sb]*10000.0, 
			point, a1, a2, sb, phase[point][a1][a2][sb],
                       	azoff, eloff, CurrentAzOff, CurrentElOff);
                       	fflush(fp_raw_data);
			      }
        printf ("Start: %d Stop: %d Time taken: %d secs\n", Start_time, cur_time, cur_time - Start_time);
	printf("Done!\n");
/* Above part for raster; replaces 5-point */
 restore:
  restoreInitialOffsets(FALSE);
  restoreCorrelatorState();
  exit(OK);
}
