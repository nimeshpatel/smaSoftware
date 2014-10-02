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

int scanLength = 5;
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
float amp[5][N_ANTENNAS][N_ANTENNAS][2], phase[5][N_ANTENNAS][N_ANTENNAS][2];
int cAntennas[N_ANTENNAS];
float data[5][N_ANTENNAS][N_ANTENNAS];
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

FILE *ipnfp;                 /* KS: file pointer to the ipoint data file */ 
int antcode = 0;             /* KS: antenna code */
void recordIpointData(FILE *fp, int antcode, int pointid);   /* KS: */


/*
  point.c uses the Numerical Recipies amoeba function to minimize a function
  some of the NR routines have been pasted into the code here.
*/

/*
  B E G I N   S W I P E D   N U M E R I C A L   R E C I P I E S   C O D E
*/
#define NMAX 5000
#define ALPHA 1.0
#define BETA 0.5
#define GAMMA 2.0

#define GET_PSUM for (j=1;j<=ndim;j++) { for (i=1,sum=0.0;i<=mpts;i++)\
         sum += p[i][j]; psum[j]=sum;}

void nrerror(error_text)
char error_text[];
{
  fprintf(stderr,"Numerical Recipes run-time error...\n");
  fprintf(stderr,"%s\n",error_text);
  fprintf(stderr,"...now exiting to system...\n");
  abortCleanup();
  exit(1);
}

float *vector(nl,nh)
int nl,nh;
{
    float *v;

    v=(float *)malloc(WUDGY + (unsigned) (nh-nl+1)*sizeof(float));
    if (!v) nrerror("allocation failure in vector()");
    return v-nl;
}

void free_vector(v,nl,nh)
float *v;
int nl,nh;
{
    free((char*) (v+nl));
}

void amoeba(p,y,ndim,ftol,funk,nfunk)
float **p,y[],ftol,(*funk)();
int ndim,*nfunk;
{
    int i,j,ilo,ihi,inhi,mpts=ndim+1;
    float ytry,ysave,sum,rtol,amotry(),*psum,*vector();
    void nrerror(),free_vector();
    char abortMessage[100];

    psum=vector(1,ndim);
    *nfunk=0;
    GET_PSUM
    for (;;) {
     ilo=1;
     ihi = y[1]>y[2] ? (inhi=2,1) : (inhi=1,2);
     for (i=1;i<=mpts;i++) {
      if (y[i] < y[ilo]) ilo=i;
      if (y[i] > y[ihi]) {
       inhi=ihi;
       ihi=i;
      } else if (y[i] > y[inhi])
       if (i != ihi) inhi=i;
     }
     rtol=2.0*fabs(y[ihi]-y[ilo])/(fabs(y[ihi])+fabs(y[ilo]));
     if (rtol < ftol) break;
     if (*nfunk >= (NMAX/2))
       ftol *= 3.0;
     if (*nfunk >= NMAX) {
       sprintf(abortMessage, "Too many iterations in AMOEBA, rtol = %f, ftol = %f\n",
	       rtol, ftol);
       nrerror(abortMessage);
     }
     ytry=amotry(p,y,psum,ndim,funk,ihi,nfunk,-ALPHA);
     if (ytry <= y[ilo])
      ytry=amotry(p,y,psum,ndim,funk,ihi,nfunk,GAMMA);
     else if (ytry >= y[inhi]) {
      ysave=y[ihi];
      ytry=amotry(p,y,psum,ndim,funk,ihi,nfunk,BETA);
      if (ytry >= ysave) {
       for (i=1;i<=mpts;i++) {
        if (i != ilo) {
         for (j=1;j<=ndim;j++) {
          psum[j]=0.5*(p[i][j]+p[ilo][j]);
          p[i][j]=psum[j];
         }
         y[i]=(*funk)(psum);
        }
       }
       *nfunk += ndim;
       GET_PSUM
      }
     }
    }
    /*
    free_vector(psum,1,ndim);
    */
}

float amotry(p,y,psum,ndim,funk,ihi,nfunk,fac)
float **p,*y,*psum,(*funk)(),fac;
int ndim,ihi,*nfunk;
{
    int j;
    float fac1,fac2,ytry,*ptry,*vector();
    void nrerror(),free_vector();

    ptry=vector(1,ndim);
    fac1=(1.0-fac)/ndim;
    fac2=fac1-fac;
    for (j=1;j<=ndim;j++) ptry[j]=psum[j]*fac1-p[ihi][j]*fac2;
    ytry=(*funk)(ptry);
    ++(*nfunk);
    if (ytry < y[ihi]) {
     y[ihi]=ytry;
     for (j=1;j<=ndim;j++) {
      psum[j] += ptry[j]-p[ihi][j];
      p[ihi][j]=ptry[j];
     }
    }
    free_vector(ptry,1,ndim);
    return ytry;
}

#undef ALPHA
#undef BETA
#undef GAMMA
#undef NMAX

float bessj1(x)
     float x;
{
  float ax,z;
  double xx,y,ans,ans1,ans2;
  
  if ((ax=fabs(x)) < 8.0) {
    y=x*x;
    ans1=x*(72362614232.0+y*(-7895059235.0+y*(242396853.1
					      +y*(-2972611.439+y*(15704.48260+y*(-30.16036606))))));
    ans2=144725228442.0+y*(2300535178.0+y*(18583304.74
					   +y*(99447.43394+y*(376.9991397+y*1.0))));
    ans=ans1/ans2;
  } else {
    z=8.0/ax;
    y=z*z;
    xx=ax-2.356194491;
    ans1=1.0+y*(0.183105e-2+y*(-0.3516396496e-4
			       +y*(0.2457520174e-5+y*(-0.240337019e-6))));
    ans2=0.04687499995+y*(-0.2002690873e-3
			  +y*(0.8449199096e-5+y*(-0.88228987e-6
						 +y*0.105787412e-6)));
    ans=sqrt(0.636619772/ax)*(cos(xx)*ans1-z*sin(xx)*ans2);
    if (x < 0.0) ans = -ans;
  }
  return ans;
}

/*
  E N D   O F   S W I P E D   N U M E R I C A L   R E C I P I E S   C O D E
*/

float diskVisibility(float diameter, float u, float v, float lambda)
{
  int status;
  float baselineLambda, beta, uL, vL, uvDist, x;

  if (lambda == 0)
    return(1.0);
  uL = u/lambda;
  vL = v/lambda;
  uvDist = sqrt(uL*uL + vL*vL);
  beta = uvDist * diameter*0.5 * ARCSEC_TO_RAD;
  x = 2.0 * M_PI * beta;
  if (x == 0)
    return(1.0);
  else
    return(bessj1(x)/(x/2.0));
}


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
  This function gathers the data for one of the five points.   The
  "point" argument specifies which of the 5 points is being observed
  (0 through 4).
*/
void newGetFluxes(int point)
{
  int s, sb, a1, a2, ok, nScans, scanCount;
  long firstScan, corNum, mirNum, good, status[N_ANTENNAS];
  float tAmp[N_ANTENNAS][N_ANTENNAS][2], tPhase[N_ANTENNAS][N_ANTENNAS][2],
    real[N_ANTENNAS][N_ANTENNAS][2], imag[N_ANTENNAS][N_ANTENNAS][2];

  /*
  sleep(scanLength/2);
  */
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
  /*
  if (point == 0) {
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
  }
  */

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
  printf("Pausing the correlator\x0d");
  correlatorPause(TRUE);
  usleep(2500000);
  printf("Resuming the correlator\x0d");
  correlatorPause(FALSE);
  while (scanCount < nScans) {
    if (scanCount == 0) {
      s = dsm_read("m5", "DSM_AS_SCAN_CORNUM_L", (void *)&corNum, &timestamp);
      dsmErrorProcess(s, "dsm_read(DSM_AS_SCAN_CORNUM_L)");
      firstScan = corNum;
    }
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
    firstScan = corNum;
    if (!quiet) {
      printf("Summing scan %d of %d                                 \x0d",
	     scanCount+1, nScans);
      fflush(stdout);
    }
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
  This function returns the estimated flux you would get if the true
  flux for that baselines was "fullFlux", and the antennas had pointing
  errors of (aO1, eO1), (aO2, eO2)
*/
float estimatedBaselineFlux(float fullFlux,
			    float aO1,
			    float eO1,
			    float aO2,
			    float eO2)
{
  int status;
  float value;
  double u[11], v[11];

  value = fullFlux*beam(aO1,eO1)*beam(aO2, eO2);
  if (value > 1000.0)
    printf("Strange value %f from %f %f %f %f %f\n", value,
	   fullFlux, aO1, eO1, aO2, eO2);
  return(value);
}

/*
  This function is the function that the NR amoeba function will try to
  minimize.   It returns the RMS of the difference between the measured
  flux on all baselines and all 5 points, and the theoretical flux for
  those baselines and points assuming a specific set of ideal baseline
  fluxes and pointing errors.
*/
float meritFunction(float *parameters)
{
  int point, base, a1, a2;
  float value, tValue;

  /*
  if (debug) {
    printf("In merit function - passed\n");
    for (a1 = 1; a1 <= nParameters; a1++)
      printf("%f ", parameters[a1]);
    printf("\n");
  }
  */
  value = 0.0;
  base = 0;
  for (a1 = 0; a1 < (nAntennas-1); a1++)
    for (a2 = a1+1; a2 < nAntennas; a2++) {
      /*
      printf("In merit function, a1 = %d, a2 = %d, base = %d\n", a1, a2, base);
      */
      tValue = estimatedBaselineFlux(parameters[1+a1]*parameters[1+a2], 
				     parameters[1+nAntennaGains+(2*a1)],
				     parameters[1+nAntennaGains+(2*a1)+1],
				     parameters[1+nAntennaGains+(2*a2)],
				     parameters[1+nAntennaGains+(2*a2)+1]) -
	data[2][a1][a2];
      tValue *= model[a1][a2];
      value += tValue*tValue;
      tValue = estimatedBaselineFlux(parameters[1+a1]*parameters[1+a2], 
				     parameters[1+nAntennaGains+(2*a1)],
				     parameters[1+nAntennaGains+(2*a1)+1]-offsetStep,
				     parameters[1+nAntennaGains+(2*a2)],
				     parameters[1+nAntennaGains+(2*a2)+1]-offsetStep) -
	data[0][a1][a2];
      tValue *= model[a1][a2];
      value += tValue*tValue;
      tValue = estimatedBaselineFlux(parameters[1+a1]*parameters[1+a2], 
				     parameters[1+nAntennaGains+(2*a1)],
				     parameters[1+nAntennaGains+(2*a1)+1]+offsetStep,
				     parameters[1+nAntennaGains+(2*a2)],
				     parameters[1+nAntennaGains+(2*a2)+1]+offsetStep) -
	data[1][a1][a2];
      tValue *= model[a1][a2];
      value += tValue*tValue;
      tValue = estimatedBaselineFlux(parameters[1+a1]*parameters[1+a2], 
				     parameters[1+nAntennaGains+(2*a1)]-offsetStep,
				     parameters[1+nAntennaGains+(2*a1)+1],
				     parameters[1+nAntennaGains+(2*a2)]-offsetStep,
				     parameters[1+nAntennaGains+(2*a2)+1]) -
	data[3][a1][a2];
      tValue *= model[a1][a2];
      value += tValue*tValue;
      tValue = estimatedBaselineFlux(parameters[1+a1]*parameters[1+a2], 
				     parameters[1+nAntennaGains+(2*a1)]+offsetStep,
				     parameters[1+nAntennaGains+(2*a1)+1],
				     parameters[1+nAntennaGains+(2*a2)]+offsetStep,
				     parameters[1+nAntennaGains+(2*a2)+1]) -
	data[4][a1][a2];
      tValue *= model[a1][a2];
      value += tValue*tValue;
      base++;
    }
  value = sqrt(value);
  return(value);
}

/*
  This function builds the data structures that amoeba needs, calls amoeba,
  and derives pointing offsets using the results amoeba supplies.
*/

void deriveOffsets()
{
  int a1, a2, antPtr, i, j, nCalls, sideband, try;
  static float **p, *y;
  float newAzOffsets[N_ANTENNAS], newElOffsets[N_ANTENNAS];

  if (debug)
    printf("In deriveOffsets - fill in the various arrays\n");
  antPtr = 0;
  for (a1 = 0; a1 < N_ANTENNAS; a1++) {
    newAzOffsets[a1] = newElOffsets[a1] = 0.0;
    if (a1 > 0)
      if (antennaList[a1])
	cAntennas[antPtr++] = a1;
  }
  if (firstRep) {
    /*
      Now make the big array for amoeba:
      Need nParameters+1 rows each nParameters long.
    */
    p = (float **)malloc(WUDGY + (nParameters+2)*sizeof(float **));
    if (p == NULL) {
      perror("deriveOffsets: ,malloc error on p");
      abortCleanup();
      exit(-1);
    }
    for (i = 1; i < (nParameters+2); i++) {
      p[i] = (float *)malloc(WUDGY + (nParameters+1)*sizeof(float));
      if (p[i] == NULL) {
	perror("deriveOffsets: malloc of p[i]");
	abortCleanup();
	exit(-1);
      }
    }
    y = (float *)malloc(WUDGY + (nParameters+2)*sizeof(float));
    if (y == NULL) {
      perror("deriveOffsets malloc of y");
      abortCleanup();
      exit(-1);
    }
  }
  for (sideband = 0; sideband < 2; sideband++) {
    if (debug)
      printf("Sideband check: line = %d, sideband = %d, sidebandFlag = %d, cond = %d\n",
	     line, sideband, sidebandFlag,((!line) || ((sideband == 0) && (sidebandFlag == -1)) ||
					   (sideband == sidebandFlag)));
	     
    if ((!line) || ((sideband == 0) && (sidebandFlag == -1)) ||
	(sideband == sidebandFlag)) {
      for (a1 = 0; a1 < antPtr-1; a1++)
	for (a2 = a1+1; a2 < antPtr; a2++) {
	  data[0][a1][a2] = amp[0][cAntennas[a1]][cAntennas[a2]][sideband];
	  data[1][a1][a2] = amp[1][cAntennas[a1]][cAntennas[a2]][sideband];
	  data[2][a1][a2] = amp[2][cAntennas[a1]][cAntennas[a2]][sideband];
	  data[3][a1][a2] = amp[3][cAntennas[a1]][cAntennas[a2]][sideband];
	  data[4][a1][a2] = amp[4][cAntennas[a1]][cAntennas[a2]][sideband];
	}
      /*
	amoeba is called twice, with differing parameters.   See the
	NR documentation for the reason.
      */
      if (debug)
	printf("Starting try loop\n");
      for (try = 0; try < 2; try++) {
	/*
	  Now fill the big array in - each row has one set of
	  parameters, and all but the first have one parameter
	  tweeked by a characteristic size step.
	*/

	for (i = 1+try; i < (nParameters+2); i++) {
	  j = 1+try;
	  /*
	    First, fill in the initial values for antenna gains.   As a first
	    guess for A(n), I'll use the average value of the flux at the central position
	    for all baselines that include A(n)
	  */
	  /*
	  for (a1 = 0; a1 < antPtr-1; a1++)
	    for (a2 = a1+1; a2 < antPtr; a2++)
	      p[i][j++] = data[2][a1][a2];
	  */
	  for (a1 = 0; a1 < antPtr; a1++) {
	    int ii, jj;
	    float nn;

	    nn = 0.0;
	    p[i][j] = 0.0;
	    for (ii = 0; ii < antPtr-1; ii++)
	      for (jj = ii+1; jj < antPtr; jj++)
		if ((ii == a1) || (jj == a1)) {
		  nn += 1.0;
		  p[i][j] += data[2][ii][jj];
		}
	    if (nn == 0.0) {
	      fprintf(stderr,
		      "Error setting initial gain guess for antenna %d - no data\n",
		      a1);
	      abortCleanup();
	      exit(-1);
	    } else {
	      p[i][j] = sqrt(p[i][j]/nn);
	      j++;
	    }
	  }
	  for (a1 = 0; a1 < antPtr; a1++) {
	    p[i][j+(2*a1)] = 0.0;
	    p[i][j+(2*a1)+1] = 0.0;
	  }
	}
	j = 1+try;
	for (i = 2; i < (nAntennaGains+2); i++)
	  p[i][j++] /= 2.0;
	for (i = nAntennaGains+2; i < (nParameters+2); i++)
	  p[i][j++] = hWHM/2.0;
	if (debug) {
	  printf("Array p for downhill simplex:\n");
	  for (i = 1; i < (nParameters+2); i++) {
	    printf("%d: ", i);
	    for (j = 1; j < (nParameters+1); j++)
	      printf("%f ", p[i][j]);
	    printf("\n");
	  }
	}
	for (i = 1+try; i < (nParameters+2); i++)
	  y[i] = meritFunction(p[i]);
	if (debug) {
	  printf("y array:\n");
	  for (i = 1; i < (nParameters+2); i++)
	    printf("%f ", y[i]);
	  printf("\n");
	}
	if (debug)
	  printf("Calling amoeba, sb = %d\n", sideband);
	amoeba(p, y, nParameters, 4.0e-5, meritFunction, &nCalls);
      }
      for (i = 1; i < (nParameters+2); i++)
	for (a1 = 0; a1 < nAntennas; a1++) {
	  newAzOffsets[a1] += p[i][1+nAntennaGains+(2*a1)];
	  newElOffsets[a1] += p[i][2+nAntennaGains+(2*a1)];
	}
      /*
      for (a1 = 0; a1 < nAntennas; a1++) {
	newAzOffsets[a1] /= (float)(nParameters+1);
	newElOffsets[a1] /=  (float)(nParameters+1);
      }
      */
      if (debug) {
	printf("amoeba returned after %d function calls\n",
	       nCalls);
	printf("y array:\n");
	for (i = 1; i < (nParameters+2); i++)
	  printf("%f ", y[i]);
	printf("\n");
	printf("Array p for downhill simplex:\n");
	for (i = 1; i < (nParameters+2); i++) {
	  printf("%d: ", i);
	  for (j = 1; j < (nParameters+1); j++)
	    printf("%f ", p[i][j]);
	  printf("\n");
	}
      }
    }
  }
  for (a1 = 0; a1 < nAntennas; a1++) {
    newAzOffsets[a1] /= (float)(nParameters+1);
    newElOffsets[a1] /=  (float)(nParameters+1);
    if (!line) {
      newAzOffsets[a1] /= 2.0;
      newElOffsets[a1] /= 2.0;
    }
    repAzO[rep][cAntennas[a1]] = oldAzO[cAntennas[a1]]-newAzOffsets[a1];
    repElO[rep][cAntennas[a1]] = oldElO[cAntennas[a1]]-newElOffsets[a1];
  }
  if (debug || type) {
    printf("Suggested new offsets:\n");
    for (a1 = 0; a1 < nAntennas; a1++)
      if (!badAntenna[cAntennas[a1]])
	printf("Ant %d: %5.1f %5.1f\n", cAntennas[a1],
	       oldAzO[cAntennas[a1]]-newAzOffsets[a1], oldElO[cAntennas[a1]]-newElOffsets[a1]);
  }
  if (debug) {
    printf("Antenna gains:\n");
    for (a1 = 1; a1 < nAntennas+1; a1++)
      printf("Ant %d: %f\n", cAntennas[a1-1], p[1][a1]);
  }
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
  int dsm_status;
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
    for (a1 = 0; a1 < N_ANTENNAS; a1++)
      for (a2 = 0; a2 < N_ANTENNAS; a2++)
	model[a1][a2] = fabs(diskVisibility((float)planetDiameter,
					     (float)(uMeters[a1] - uMeters[a2]),
					     (float)(vMeters[a1] - vMeters[a2]),
					     lambda
					     ));
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

  /* KS: open file to write i-point information */
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

	  antPH[ant]=readHeader(ant, rmOpenFlag, rmOpenFlag);
	}
      }
    sleep(TRACK_LATENCY);
    if (Log)
      for (ant = 1; ant < N_ANTENNAS; ant++)
	if (antennaList[ant]) {
	  int rmOpenFlag = 0;
	  
	  antPH[ant]=readHeader(ant, rmOpenFlag, rmOpenFlag);
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
    if (useMir && (mirdir != "null")) {
      recordIpointData(ipnfp,antcode,0);  /* KS: ipoint info recording */
      fclose(ipnfp);                      /* KS: */
    }
  bypass:
    /*
      Now make sure the center position whas strongest:
    */
    ok = 1;
    for (point = 0; point < 5; point++)
      if (point != 2)
	for (a1 = 1; a1 < N_ANTENNAS; a1++)
	  for (a2 = 1; a2 < N_ANTENNAS; a2++)
	    if ((a1 < a2) && antennaList[a1] && antennaList[a2])
	      if ((amp[point][a1][a2][0] > amp[2][a1][a2][0]) ||
		    (amp[point][a1][a2][1] > amp[2][a1][a2][1]))
		ok = 0;
    /*
    if (debug)
      if (ok)
	printf("Center point was strongest - will fit\n");
    if (!ok) {
      if (unwise) {
	ok = 1;
	if (!quiet)
	  printf("Center point is not strongest, but unwise is set, so will fit anyway\n");
      } else {
	fprintf(stderr, "Center point was NOT strongest - won't derive new offsets\n");
	rep--;
      }
    }
    */
    ok = TRUE;
    if ((debug) || (!ok)) {
      printf("Final flux results\n");
      printf("Lower sideband:\n");
      for (point = 0; point < 5; point++) {
	printf("%d ", point);
	for (a1 = 1; a1 < N_ANTENNAS; a1++)
	  for (a2 = 1; a2 < N_ANTENNAS; a2++)
	    if ((a1 < a2) && antennaList[a1] && antennaList[a2])
	      printf("%d-%d: (%6.2f,%6.1f) ",
		     a1, a2, amp[point][a1][a2][0]*10000.0,
		     phase[point][a1][a2][0]);
	printf("\n");
      }
      printf("Upper sideband:\n");
      for (point = 0; point < 5; point++) {
	printf("%d ", point);
	for (a1 = 1; a1 < N_ANTENNAS; a1++)
	  for (a2 = 1; a2 < N_ANTENNAS; a2++)
	    if ((a1 < a2) && antennaList[a1] && antennaList[a2])
	      printf("%d-%d: (%6.2f,%6.1f) ",
		     a1, a2, amp[point][a1][a2][1]*10000.0,
		     phase[point][a1][a2][1]);
	printf("\n");
      }
    }
    if (maxCoh < (MIN_MAX_COH/sqrt(cohScans))) {
      fprintf(stderr,
	      "The average coherence at the strongest point was only %f\n",
	      maxCoh);
      fprintf(stderr,
	      "So the signal/noise is too low to derive new offsets.\n");
      ok = 0;
    }
    for (a1 = 1; a1 < N_ANTENNAS; a1++)
      if (antennaList[a1]) {
	/*
	printf("Ant %d Max. Coh = %f, position %d\n", a1,
	       maxCohAnt[a1],
	       maxCohPos[a1]);
	*/
	if (maxCohAnt[a1] < (MIN_MAX_COH * 0.75)) {
	  fprintf(stderr, "The maximum coherence on any antenna %d baseline is %f\n",
		  a1, maxCohAnt[a1]);
	  fprintf(stderr, "That's less than the minumum allowable value of %f\n",
		  MIN_MAX_COH * 0.75);
	  fprintf(stderr, "So no offsets will be derived for this antenna\n");
	  badAntenna[a1] = TRUE;
	}
      }
    if (ok)
      deriveOffsets();
    else
      goto restore;;
    firstRep = FALSE;
    if (rep < (repetitions-1))
      sleep(TRACK_LATENCY);
  }
  if (!quiet)
    printf("\n");
  for (ant = 1; ant < N_ANTENNAS; ant++)
    newAzO[ant] = newElO[ant] = 0.0;
  for (rep = 0; rep < repetitions; rep++) {
    if (debug || type)
      printf("Repetition %d:\n", rep+1);
    if (log)
      SMAshLogPrintf("Repetition %d:\n", rep+1);
    for (a1 = 1; a1 < N_ANTENNAS; a1++) {
      if (antennaList[a1] && (!badAntenna[a1])) {
	if (debug || type)
	  printf("Ant %d\tAzOff = %5.1f\tElOff = %5.1f\tdeltaAz = %5.1f\tdeltaEl = %5.1f\n",
		 a1, repAzO[rep][a1], repElO[rep][a1],
		 repAzO[rep][a1]-oldAzO[a1], repElO[rep][a1]-oldElO[a1]);
	if (log)
	  SMAshLogPrintf("Ant %d\tAzOff = %5.1f\tElOff = %5.1f\tdeltaAz = %5.1f\tdeltaEl = %5.1f\n",
			 a1, repAzO[rep][a1], repElO[rep][a1],
			 repAzO[rep][a1]-oldAzO[a1], repElO[rep][a1]-oldElO[a1]);
	newAzO[a1] += repAzO[rep][a1];
	newElO[a1] += repElO[rep][a1];
      }
    }
  }
  if (repetitions > 1) {
    if (debug || type)
      printf("Average:\n");
    if (log)
      SMAshLogPrintf("Average:\n");
    for (a1 = 1; a1 < N_ANTENNAS; a1++) {
      if (antennaList[a1] && (!badAntenna[a1])) {
	newAzO[a1] /= (float)repetitions;
	newElO[a1] /= (float)repetitions;
	if (debug || type)
	  printf("Ant %d\tAzOff = %5.1f\tElOff = %5.1f\tdeltaAz = %5.1f\tdeltaEl = %5.1f\n",
		 a1, newAzO[a1], newElO[a1], newAzO[a1]-oldAzO[a1], newElO[a1]-oldElO[a1]);
	if (log)
	  SMAshLogPrintf("Ant %d\tAzOff = %5.1f\tElOff = %5.1f\tdeltaAz = %5.1f\tdeltaEl = %5.1f\n",
			 a1, newAzO[a1], newElO[a1], newAzO[a1]-oldAzO[a1], newElO[a1]-oldElO[a1]);
      }
    }
  }
  if (Log) {
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant]) {
	struct offsets poffsets;

	if (!badAntenna[ant]) {
	  poffsets.azoff = newAzO[ant];
	  poffsets.azoffErr = 1.0;
	  poffsets.eloff = newElO[ant];
	  poffsets.eloffErr = 1.0;
	  printHeaderAndOffsets(antLog[ant], ant,
				antPH[ant], poffsets,"somecomment");
	}
	fclose(antLog[ant]);
      }
  }
  /*
    Send the new offsets to the antennas if that was requested
  */
  if (stuff) {
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if (antennaList[ant] && (!badAntenna[ant])) {
	if (!partial)
	  newAzOff(ant, newAzO[ant]);
	else
	  newAzOff(ant, (newAzO[ant]+oldAzO[ant])/2.0);
      }
    sleep(TRACK_LATENCY);
    for (ant = 1; ant < N_ANTENNAS; ant++)
      if ((antennaList[ant]) && (!badAntenna[ant])) {
	if (!partial)
	  newElOff(ant, newElO[ant]);
	else
	  newElOff(ant, (newElO[ant]+oldElO[ant])/2.0);
      }
  }
 restore:
  restoreCorrelatorState();
  exit(OK);
}
