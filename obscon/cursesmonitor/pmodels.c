#include <stdio.h>
#include <math.h>
#include <curses.h>
#include<string.h>
#include <time.h>
#include "rm.h"
#include "monitor.h"

#define DESC_STRING_SIZE 256

void pmodelscreen(
	double *azdc, double *azcollimation, double *azelaxistilt,
	double *azaztiltsin, double *azaztiltcos,
	double *azaztiltsin2, double *azaztiltcos2,
	double *azencsin, double *azenccos,
	double *azencsin2, double *azenccos2,
	double *azencsin3, double *azenccos3,
	double *azrms,
	double *eldc, double *elsag, double *elsin,
	double *elaztiltsin, double *elaztiltcos,
	double *elaztiltsin2, double *elaztiltcos2,
	double *elrms , int *trackstatus, int *antlist);

int pmodelspage(int icount,int *antlist){

	int rms;
	int trackStatus[20];

	extern double radian;
	double Double;

	double azdc[9], azcollimation[9], azelaxistilt[9];
	double azaztiltsin[9], azaztiltcos[9];
	double azaztiltsin2[9], azaztiltcos2[9];
	double azencsin[9], azenccos[9];
	double azencsin2[9], azenccos2[9];
	double azencsin3[9], azenccos3[9];
	double azrms[9];
	double eldc[9], elsag[9],elsin[9];
	double elaztiltsin[9], elaztiltcos[9];
	double elaztiltsin2[9], elaztiltcos2[9];
	double elrms[9]; 

	int i;
	int tracktimestamp,timeStamp;

	/* END of variable declarations*/

	trackStatus[1]=0;
	trackStatus[2]=0;
	trackStatus[3]=0;
	trackStatus[4]=0;
	trackStatus[5]=0;
	trackStatus[6]=0;
	trackStatus[7]=0;
	trackStatus[8]=0;


	if((icount % 20) == 1) {
	initscr();
#ifdef LINUX
	clear();
#endif
 	move(1,1);
	refresh();
	}

	for(i=1;i<9;i++) {

	rms=rm_read(i,"RM_UNIX_TIME_L",&timeStamp);
	rms|=rm_read(i,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
        if(abs(tracktimestamp-timeStamp)>3L) trackStatus[i]=0;
        if(abs(tracktimestamp-timeStamp)<=3L) trackStatus[i]=1;
	if(rms!=RM_SUCCESS) trackStatus[i]=0;

	  rms=rm_read(i,"RM_AZDC_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azdc[i]=Double;
	  	else azdc[i]=0.;
	  rms=rm_read(i,"RM_AZCOLLIMATION_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azcollimation[i]=Double;
	  	else azcollimation[i]=0.;
	  rms=rm_read(i,"RM_AZELAXISTILT_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azelaxistilt[i]=Double;
	  	else azelaxistilt[i]=0.;
	  rms=rm_read(i,"RM_AZAZTILTSIN_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azaztiltsin[i]=Double;
	  	else azaztiltsin[i]=0.;
	  rms=rm_read(i,"RM_AZAZTILTCOS_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azaztiltcos[i]=Double;
	  	else azaztiltcos[i]=0.;
	  rms=rm_read(i,"RM_AZAZTILTSIN2_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azaztiltsin2[i]=Double;
	  	else azaztiltsin2[i]=0.;
	  rms=rm_read(i,"RM_AZAZTILTCOS2_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azaztiltcos2[i]=Double;
	  	else azaztiltcos2[i]=0.;
	  rms=rm_read(i,"RM_AZENCSIN_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azencsin[i]=Double;
	  	else azencsin[i]=0.;
	  rms=rm_read(i,"RM_AZENCCOS_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azenccos[i]=Double;
	  	else azenccos[i]=0.;
	  rms=rm_read(i,"RM_AZENCSIN2_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azencsin2[i]=Double;
	  	else azencsin2[i]=0.;
	  rms=rm_read(i,"RM_AZENCCOS2_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azenccos2[i]=Double;
	  	else azenccos2[i]=0.;
	  rms=rm_read(i,"RM_AZENCSIN3_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azencsin3[i]=Double;
	  	else azencsin3[i]=0.;
	  rms=rm_read(i,"RM_AZENCCOS3_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azenccos3[i]=Double;
	  	else azenccos3[i]=0.;
	  rms=rm_read(i,"RM_AZRMS_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) azrms[i]=Double;
	  	else azrms[i]=0.;
	  rms=rm_read(i,"RM_ELDC_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) eldc[i]=Double;
	  	else eldc[i]=0.;
	  rms=rm_read(i,"RM_ELSAG_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elsag[i]=Double;
	  	else elsag[i]=0.;
	  rms=rm_read(i,"RM_ELSIN_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elsin[i]=Double;
	  	else elsin[i]=0.;
	  rms=rm_read(i,"RM_ELAZTILTSIN_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elaztiltsin[i]=Double;
	  	else elaztiltsin[i]=0.;
	  rms=rm_read(i,"RM_ELAZTILTCOS_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elaztiltcos[i]=Double;
	  	else elaztiltcos[i]=0.;
	  rms=rm_read(i,"RM_ELAZTILTSIN2_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elaztiltsin2[i]=Double;
	  	else elaztiltsin2[i]=0.;
	  rms=rm_read(i,"RM_ELAZTILTCOS2_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elaztiltcos2[i]=Double;
	  	else elaztiltcos2[i]=0.;
	  rms=rm_read(i,"RM_ELRMS_ARCSEC_D",&Double);
	  if(trackStatus[i]==1) elrms[i]=Double;
	  	else elrms[i]=0.;

	}

	pmodelscreen(azdc, azcollimation, azelaxistilt,
	azaztiltsin, azaztiltcos, azaztiltsin2, azaztiltcos2,
	azencsin, azenccos, azencsin2, azenccos2, azencsin3, 
	azenccos3, azrms, eldc, elsag, elsin,elaztiltsin, elaztiltcos,
	elaztiltsin2, elaztiltcos2,  elrms ,trackStatus,antlist);
	    
	return 0;
}

void pmodelscreen(
	double *azdc, double *azcollimation, double *azelaxistilt,
	double *azaztiltsin, double *azaztiltcos,
	double *azaztiltsin2, double *azaztiltcos2,
	double *azencsin, double *azenccos,
	double *azencsin2, double *azenccos2,
	double *azencsin3, double *azenccos3,
	double *azrms,
	double *eldc, double *elsag, double *elsin,
	double *elaztiltsin, double *elaztiltcos,
	double *elaztiltsin2, double *elaztiltcos2,
	double *elrms, int *trackstatus, int *antlist) {
  int j, rm_status;
  long rightnow;
  int feed,rms;
  char modeldate[10];
	extern int defaultTiltFlag[20];
  
  move(0,0);
  printw("Antenna      ");
  for(j=1;j<9;j++) {
	if(defaultTiltFlag[j]==1) printw("  %dT   ",j);
	else printw("   %d    ",j);
  }
  move(1,0);
  printw("Feed offsets");
  for (j=1;j<9;j++) {
    rms=rm_read(j,"RM_FEED_L",&feed);
    if (feed <200 || feed > 999) {
      printw(" wacko  ");
    } else {
      printw(" %3dGHz ",feed);
    }
  }
  move(2,0);
  printw("AzDC      ");
  for(j=1;j<9;j++) {
    if (fabs(azdc[j]) > 3600) {
      standout();
    }
    printw(" %6.1f ",azdc[j]);
    standend();
  }
  move(3,0);
  printw("AzColl    ");
  for(j=1;j<9;j++) printw(" %6.1f ",azcollimation[j]);
  move(4,0);
  printw("ElTilt    ");
  for(j=1;j<9;j++) printw(" %6.1f ",azelaxistilt[j]);
  move(5,0);
  printw("AAzTltSin ");
  for(j=1;j<9;j++) {
    if (fabs(azaztiltsin[j]) > 60) {
      standout();
    }
    printw(" %6.1f ",azaztiltsin[j]);
    standend();
  }
  move(6,0);
  printw("AAzTltCos ");
  for(j=1;j<9;j++) {
    if (fabs(azaztiltcos[j]) > 60) {
      standout();
    }
    printw(" %6.1f ",azaztiltcos[j]);
    standend();
  }
  move(7,0);
  printw("AAzTltSin2");
  for(j=1;j<9;j++) printw(" %6.1f ",azaztiltsin2[j]);
  move(8,0);
  printw("AAzTltCos2");
  for(j=1;j<9;j++) printw(" %6.1f ",azaztiltcos2[j]);
  move(9,0);
  printw("AzEncSin  ");
  for(j=1;j<9;j++) printw(" %6.1f ",azencsin[j]);
  move(10,0);
  printw("AzEncCos  ");
  for(j=1;j<9;j++) printw(" %6.1f ",azenccos[j]);
  move(11,0);
  printw("AzEncSin2 ");
  for(j=1;j<9;j++) printw(" %6.1f ",azencsin2[j]);
  move(12,0);
  printw("AzEncCos2 ");
  for(j=1;j<9;j++) printw(" %6.1f ",azenccos2[j]);
  move(13,0);
  printw("AzEncSin3 ");
  for(j=1;j<9;j++) printw(" %6.1f ",azencsin3[j]);
  move(14,0);
  printw("AzEncCos3 ");
  for(j=1;j<9;j++) printw(" %6.1f ",azenccos3[j]);
  move(15,0);
  printw("ElDc       ");
  for(j=1;j<9;j++) {
    if (fabs(eldc[j]) > 3600) {
      standout();
    }
    printw(" %6.1f ",eldc[j]);
    standend();
  }
  move(16,0);
  printw("ElSag      ");
  for(j=1;j<9;j++) printw(" %6.1f ",elsag[j]);
  move(17,0);
  printw("ElSin      ");
  for(j=1;j<9;j++) printw(" %6.1f ",elsin[j]);
  move(18,0);
  printw("EAzTltSin ");
  for(j=1;j<9;j++) printw(" %6.1f ",elaztiltsin[j]);
  move(19,0);
  printw("EAzTltCos ");
  for(j=1;j<9;j++) printw(" %6.1f ",elaztiltcos[j]);
  move(20,0);
  printw("EAzTltSin2");
  for(j=1;j<9;j++) printw(" %6.1f ",elaztiltsin2[j]);
  move(21,0);
  printw("EAzTltCos2");
  for(j=1;j<9;j++) printw(" %6.1f ",elaztiltcos2[j]);
  move(22,0);
  printw("Az/ElRms     ");
  for(j=1;j<9;j++) printw("%.1f/%.1f ",azrms[j],elrms[j]);
  move(23,0);
  printw("RModelDate");
  rm_status = rm_read(antlist[0], "RM_UNIX_TIME_L", &rightnow);
  for(j=1;j<9;j++) {
    printw(" ");
    if(trackstatus[j]==1) {
      rm_read(j,"RM_MODELDATE_C10",modeldate);
      if (oldDate(modeldate,rightnow)) {
	standout();
      }
    } else {
      sprintf(modeldate," %s ","   *   ");
    }
    printw("%s",modeldate);
    standend();
  }
  refresh();
}
