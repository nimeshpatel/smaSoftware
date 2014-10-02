#include <curses.h>
#include <rpc/rpc.h>
#include <math.h>
#include <malloc.h>
#include <stdarg.h>
#include <stdio.h>
#include <termio.h>
#include <time.h>
#include "esma.h"
#include "dsm.h"
#include "rm.h"
#include "monitor.h"
#include "optics.h"
#include "c1DC.h"

#define N_ANTS 10
int nAnts = 7;

#define NMAX 50000
#define ALPHA 1.0
#define BETA 0.5
#define GAMMA 2.0

int antennaMapping[N_ANTS+1];

#define GET_PSUM for (j=1;j<=ndim;j++) { for (i=1,sum=0.0;i<=mpts;i++)\
                                                sum += p[i][j]; psum[j]=sum;}

float tSys[11][2];

float amp[N_ANTS+1][N_ANTS+1];

void amoeba(p,y,ndim,ftol,funk,nfunk)
     float **p,y[],ftol,(*funk)();
     int ndim,*nfunk;
{
  int i,j,ilo,ihi,inhi,mpts=ndim+1;
  float ytry,ysave,sum,rtol,amotry(),*psum,*vector();
  void nrerror(),free_vector();

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
	    /*
	    if (*nfunk >= NMAX) nrerror("Too many iterations in AMOEBA");
	    */
	    if (*nfunk >= NMAX) break;
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
        free_vector(psum,1,ndim);
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

float fit(float *eff)
{
  int i, j;
  float value = 0.0;
  float item;

  for (i = 1; i < nAnts+1; i++)
    for (j = i+1; j <= nAnts+1; j++) {
      item = amp[i][j] - sqrt(tSys[i][0]*tSys[j][0])*eff[i]*eff[j];
      value += item*item;
    }
  value = sqrt(value);
  return(value);
}

static dsm_structure lastScanVisibilities;

char string1[100];

void makeRealData(int sideband)
{
  int status, i, j;
  float amplitudes[11][11][2];
  time_t timestamp;

  status = dsm_read("m5", "LAST_SCAN_VISIBILITES_X", &lastScanVisibilities,
                    &timestamp);
  if (status != DSM_SUCCESS) {
    dsm_error_message(status, "structure read");
    exit(-1);
  }

  strcpy(string1, ctime(&timestamp));
  string1[strlen(string1)-1] = (char)0;

  status = dsm_structure_get_element(&lastScanVisibilities,
                                     "AMP_V11_V11_V2_F",
                                     &amplitudes[0][0][0]);
  if (status != DSM_SUCCESS) {
    dsm_error_message(status, "get_element (amp)");
    exit(-1);
  }
  status = dsm_structure_get_element(&lastScanVisibilities,
                                     "SCAN_TSYS_V11_V2_F",
                                     &tSys[0][0]);
  if (status != DSM_SUCCESS) {
    dsm_error_message(status, "get_element (Tsys)");
    exit(-1);
  }
  for (i = 1; i < nAnts; i++)
    for (j = i; j <= nAnts; j++) {
      switch (sideband) {
      case 0:
        amp[i][j] = amplitudes[antennaMapping[i]][antennaMapping[j]][0] +
          amplitudes[antennaMapping[i]][antennaMapping[j]][1];
        break;
      case -1:
        amp[i][j] = amplitudes[antennaMapping[i]][antennaMapping[j]][0];
        break;
      case 1:
        amp[i][j] = amplitudes[antennaMapping[i]][antennaMapping[j]][1];
        break;
      default:
        fprintf(stderr, "Illegal sideband code (%d) passed to makeRealData\n",
                sideband);
        exit(-1);
      }
      
      if (isnan(amp[i][j]))
        amp[i][j] = 0.0;
      amp[i][j] *= sqrt(tSys[i][0]*tSys[j][0]);
      amp[j][i] = amp[i][j];
    }
  /*
  for (i = 1; i < nAnts; i++)
    for (j = i+1; j <= nAnts; j++)
      printf("%d-%d:\t%f\n", antennaMapping[i], antennaMapping[j],
             amp[i][j]);
  */
}


void solveGains(int count) {
  static int firstCall = TRUE;
  int ant1, ant2, nfunk, rms, sb;
  float **p, maxEff;
  float y[N_ANTS+2];
  float effLSB[N_ANTS+1], effUSB[N_ANTS+1], effDSB[N_ANTS+1];
  double dummyDouble;
  int antennasInArray[N_ANTS+1];
  time_t system_time;

  if (firstCall) {
    rms = dsm_structure_init(&lastScanVisibilities,"LAST_SCAN_VISIBILITES_X");
    if (rms != DSM_SUCCESS) {
      dsm_error_message(rms,"dsm_structure_init(LAST_SCAN_VISIBILITES_X)");
      exit(-1);
    }
    firstCall = FALSE;
  }
  if ((count % 60) == 1) {
    /*
      Initialize Curses Display
    */
    initscr();
    clear();
    move(1,1);
    refresh();
  }
  move(0, 12);
  system_time = time(NULL);
  printw("Relative Antenna Efficiencies on %s",
	 asctime(gmtime(&system_time)));
  getAntennaList(&antennasInArray[0]);
  nAnts = 0;
  for (ant1 = 1; ant1 <= N_ANTS; ant1++)
    if (antennasInArray[ant1]) {
      antennaMapping[nAnts+1] = ant1;
      nAnts++;
    }

  p = (float **)malloc((nAnts+2)*sizeof(float *));
  for (ant1 = 1; ant1 <= nAnts+1; ant1++)
    p[ant1] = (float *)malloc((nAnts+1)*sizeof(float));
  for (sb = -1; sb < 2; sb++) {
    makeRealData(sb);
    if (sb == -1) {
      move(1, 12);
      printw("Values are from a scan stored at %s\n", string1);
    }
    for (ant1 = 1; ant1 <= nAnts; ant1++)
      p[1][ant1] = 1.0e-3;
    for (ant1 = 2; ant1 <= nAnts+1; ant1++)
      for (ant2 = 1; ant2 <= nAnts; ant2++) {
        p[ant1][ant2] = 1.0e-3;
        if ((ant1-1) == ant2)
          p[ant1][ant2] += 0.1e-3;
      }
    for (ant1 = 1; ant1 <= nAnts+1; ant1++)
      y[ant1] = fit(p[ant1]);
    nfunk = 0;
    amoeba(p, y, nAnts, 2.0e-5, fit, &nfunk);
    for (ant1 = 2; ant1 <= nAnts+1; ant1++)
      for (ant2 = 1; ant2 <= nAnts; ant2++) {
        if ((ant1-1) == ant2)
          p[ant1][ant2] += p[ant1][ant2]*0.1;
      }
    for (ant1 = 1; ant1 <= nAnts+1; ant1++)
      y[ant1] = fit(p[ant1]);
    nfunk = 0;
    amoeba(p, y, nAnts, 2.0e-5, fit, &nfunk);
    /* Normalize efficiencies to highest value */
    maxEff = 0.0;
    for (ant1 = 1; ant1 <= nAnts; ant1++)
      if ((maxEff < p[1][ant1]) && (antennaMapping[ant1] < 9))
        maxEff = p[1][ant1];
    if (maxEff == 0.0)
      maxEff = 1.0;
    for (ant1 = 1; ant1 <= nAnts; ant1++) {
      p[1][ant1] /= maxEff;
      if (p[1][ant1] < 0.0)
        p[1][ant1] = 0.0;
      /*
      p[1][ant1] *= p[1][ant1];
      */
    }
    for (ant1 = 1; ant1 <= nAnts; ant1++)
      switch(sb) {
      case -1:
        effLSB[ant1] = p[1][ant1];
        break;
      case 0:
        effDSB[ant1] = p[1][ant1];
        break;
      case 1:
        effUSB[ant1] = p[1][ant1];
        break;
      }
  }
  move(3,23);
  printw("Ant     LSB      USB      DSB");
  for (ant1 = 1; ant1 <= nAnts; ant1++) {
    move(4+ant1, 23);
    printw("%2d     %5.3f    %5.3f    %5.3f\n",
           antennaMapping[ant1], effLSB[ant1], effUSB[ant1], effDSB[ant1]);
  }
  move(0,79);
  refresh();
}
