/* recordTilts.c
2 March 2005
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>
#include "rm.h"
#include "dsm.h"
#include "smapopt.h"
#include "ph.h"

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))

#define SCALE_FACTOR_TILT1 200.5
#define SCALE_FACTOR_TILT2 201.8

void signalHandler(int signum);
void *antennaThread(void *antennas);

int fileWriteFlag=1;
int quitFlag=0;
pthread_t antennaThreadTID[10];
pthread_attr_t *attr;
char *scantype = "noscan";
char *givenFileName="                                                                                                                        ";
int channel[2];
int gotfilename=0;
int unixtime = 0;
char killfile[80];

void usage(int exitcode, char *error, char *addl) {

        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: recordTilts [options] \n"
                "[options] include:\n"
                "--scantype -s <noscan,azccw,azcw,elup,eldown>\n"
                "--filename -f <filename>\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                "                  (default: all online antennas)\n");
        exit(exitcode);
}


int main(int argc, char **argv)
{
	int i,iant,numberOfAnts,antennaArray[SMAPOPT_MAX_ANTENNAS+1];
	int antennas[SMAPOPT_MAX_ANTENNAS+1];
	int waiting;
	char c;
	int gotantennas=0,gotscantype=0;
	int rm_status, antlist[RM_ARRAY_SIZE];/*variables for encoderClient*/
	short antennaStatus=0;
	smapoptContext optCon;
	struct sigaction action, old_action; int sigactionInt;
	struct  smapoptOption optionsTable[] = {
		{"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a',
		"a comma-separated list of antennas"},
		{"filename",'f',SMAPOPT_ARG_STRING,&givenFileName,'f',
		"filename: file to write data into (optional), must be
		less than 120 characters in length, and include a full path if
		required."},
		{"scantype",'s',SMAPOPT_ARG_STRING,&scantype,'s',
		"scantype string: azscan/elscan, default is noscan."},
		{"unixtime",'u',SMAPOPT_ARG_INT,&unixtime,'u',
		"unixtime: only respond to kill file with this timetag (optional)"},
		SMAPOPT_AUTOHELP
		{NULL,'\0',0,NULL,0},
		"This program records the tiltmeter readings into \n
	         an ascii file with UTC time-stamp and other antenna info.\n
		 Some of the header variables may have incorrect/invalid values."
	};

	optCon = smapoptGetContext("recordTilts", argc, argv, optionsTable,
			SMAPOPT_CONTEXT_EXPLAIN);


	while ((c = smapoptGetNextOpt(optCon)) >= 0) { 

	  switch(c) {

	  case 's' :
	    gotscantype=1;		
	    break;
	    
	  case 'a':
	    gotantennas=1;
	    break;
	    
	  case 'f':
	    gotfilename=1;
	    break;
	    
	  }
	}



	/* signal handler for control C */
	action.sa_flags=0;
	sigemptyset(&action.sa_mask);
	action.sa_handler = signalHandler;
	sigactionInt = sigaction(SIGUSR1,&action, &old_action);

	/* initializing ref. mem. */
	rm_status=rm_open(antlist);
	if(rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status,"rm_open()");
	  exit(1);
	}

	if(gotantennas==1) {
	numberOfAnts=0;
	for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
	  if(antennaArray[iant]==1) {
	    antennas[numberOfAnts]=iant;
	    numberOfAnts++;
	  }
	}	
	} else {
	 /* check which antennas are having ONLINE status (as set
           by the project command */
        for(iant=1;iant<=8;iant++) {
        i=0;
        while(antlist[i] != RM_ANT_LIST_END)
                if(iant == antlist[i++]) {
                rm_read(iant,"RM_ANTENNA_STATUS_S",&antennaStatus);
                if(antennaStatus==1) antennaArray[iant]=1; 
                if(antennaStatus==0) antennaArray[iant]=0;
                }
        }
	}

	numberOfAnts=0;
	for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
	  if(antennaArray[iant]==1) {
	    antennas[numberOfAnts]=iant;
	    numberOfAnts++;
	  }
	}	

	if (unixtime == 0) {
	  strcpy(killfile,"/global/killrecordTilts");
	} else {
	  sprintf(killfile,"/global/killrecordTilts.%d",unixtime);
	}
	
	for (iant=0; iant<numberOfAnts; iant++) {
	  if (pthread_create(&antennaThreadTID[iant], attr, antennaThread,
			     (void *) &antennas[iant]) == -1) {
	    perror("pthread_create antennaThread");
	  } else {
	    fprintf(stderr,"Created thread for antenna %d\n",antennas[iant]);
	  }
	}
	waiting = 0;
/* Wait for all antennas to finish, but not longer than 30 seconds after the first
 * one has finished. 
 */
	while(quitFlag < numberOfAnts && waiting < 30) {
	  sleep(1);
	  if (quitFlag > 0) {
	    waiting++;
	  }
	}
	if (waiting == 30) {
	  fprintf(stderr,"Sorry, %d antennas failed to record data\n",numberOfAnts-quitFlag);
	}
	fprintf(stderr,"Detached thread for antenna ");
	for (iant=0; iant<numberOfAnts; iant++) {
	  pthread_detach(&antennaThreadTID[iant]);
	  fprintf(stderr,"%d ",antennas[iant]);
	}
	fprintf(stderr,"\n");
	/* close ref. mem. */
	rm_status=rm_close(); 
	if(rm_status != RM_SUCCESS) {
	  rm_error_message(rm_status,"rm_close()");
	  exit(1);
	}
	fprintf(stderr,"All antennas finished. Main thread of recordTilts is deleting the killfile and exiting\n");
	remove(killfile);
	return(0);
}

void *antennaThread(void *antenna) {
  FILE *fp;
	FILE *fp1,*fp2,*fp3,*fp4,*fp5,*fp6,*fp7,*fp8;
	char filename[120];
	char somecomment[1000];
	char emsg[80];
	int i,rm_status,ant_num;
	float az_deg,el_deg;
	float utc_hours;
	time_t new_time;
	char timestamp[50];
	struct tm *Time;
	struct ph p;
	float tilts[4], tiltx, tilty, tiltxbase, tiltybase;
	double tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY;
	float newtiltx,newtilty,newTilts[4];
	int recordTiltsFlag=0;
		int tiltflag;
		float pmdaz,pmdel;

	ant_num = *((int *)antenna);

	new_time=time(NULL);
	Time=localtime(&new_time);
	strftime(timestamp,100,"%d%b%y_%H%M%S",Time);
	if(gotfilename==0) {
	  sprintf(filename,
		  "/data/engineering/tilt/ant%d/ant%d.%s.%s.tilt", 
		  ant_num,ant_num,scantype,timestamp);
	}
	else {
	  strcpy(filename,givenFileName);
	}
	
	if(ant_num==1) {
	  if ((fp1=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==2) {
	  if ((fp2=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==3) {
	  if ((fp3=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==4) {
	  if ((fp4=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==5) {
	  if ((fp5=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==6) {
	  if ((fp6=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==7) {
	  if ((fp7=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}
	if(ant_num==8) {
	  if ((fp8=fopen(filename,"w"))==NULL){
	    fprintf(stderr,"cannot open data file:%s\n",filename);
	    exit(1);
	  } 
	}


	sprintf(somecomment,"%s", filename);
		
	if(ant_num==1) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp1,"#");
	  printHeader(fp1,p,somecomment);
	}
	if(ant_num==2) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp2,"#");
	  printHeader(fp2,p,somecomment);
	}
	if(ant_num==3) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp3,"#");
	  printHeader(fp3,p,somecomment);
	}
	if(ant_num==4) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp4,"#");
	  printHeader(fp4,p,somecomment);
	}
	if(ant_num==5) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp5,"#");
	  printHeader(fp5,p,somecomment);
	}
	if(ant_num==6) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp6,"#");
	  printHeader(fp6,p,somecomment);
	}
	if(ant_num==7) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp7,"#");
	  printHeader(fp7,p,somecomment);
	}
	if(ant_num==8) {
	  readHeader(ant_num,0,1,&p);
	      fprintf(fp8,"#");
	  printHeader(fp8,p,somecomment);
	}
	printf("Will listen for killfile = %s\n",killfile);
	fp = fopen(killfile,"r");
	if (fp != NULL) {
	  fprintf(stderr,"The file %s was seen.  You must delete this first.\n",killfile);
	  exit(0);
	}
	while (fopen(killfile,"r") == NULL &&fileWriteFlag==1) {
	  while (fopen(killfile,"r") == NULL&&fileWriteFlag==1){

	      rm_status=rm_read(ant_num,"RM_TILT_RECORDTILTS_FLAG_L",&recordTiltsFlag);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_TILT_RECORDTILTS_FLAG_L (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	if(recordTiltsFlag==1) {
	      rm_status=rm_read(ant_num,"RM_UTC_HOURS_F",&utc_hours);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_UTC_HOURS_F (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_ACTUAL_AZ_DEG_F",&az_deg);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_ACTUAL_AZ_DEG_F (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_ACTUAL_EL_DEG_F",&el_deg);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_ACTUAL_EL_DEG_F (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_TILT_VOLTS_V4_F",&tilts);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_ACTUAL_EL_DEG_F (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_TILT_XTRA_TESTPOINTS_V4_F",&newTilts);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_ACTUAL_EL_DEG_F (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_AFT_FOREWARD_TILT_UPPER_ARCSEC_D",&tiltScaledX);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_AFT_FORWARD_TILT_UPPER_ARCSEC_D (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_AFT_FOREWARD_TILT_LOWER_ARCSEC_D",&tiltScaledBaseX);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_AFT_FORWARD_TILT_LOWER_ARCSEC_D (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D",&tiltScaledY);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }
	      rm_status=rm_read(ant_num,"RM_LEFT_RIGHT_TILT_LOWER_ARCSEC_D",&tiltScaledBaseY);
	      if(rm_status != RM_SUCCESS) {
		sprintf(emsg,"error reading RM_LEFT_RIGHT_TILT_UPPER_ARCSEC_D (antenna %d)",ant_num);
		rm_error_message(rm_status,emsg);
		exit(1);
	      }

		rm_status=rm_read(ant_num,"RM_PMDAZ_F",&pmdaz);
		rm_status=rm_read(ant_num,"RM_PMDEL_F",&pmdel);
	rm_status=rm_read(ant_num,"RM_TILT_CORRECTION_FLAG_L",&tiltflag);

		tiltx=tilts[0]/2.0*SCALE_FACTOR_TILT1;
		tilty=tilts[1]/2.0*SCALE_FACTOR_TILT2;
		tiltxbase=tilts[2]/2.0*SCALE_FACTOR_TILT1;
		tiltybase=tilts[3]/2.0*SCALE_FACTOR_TILT2;
		newtiltx=newTilts[0]/2.0*SCALE_FACTOR_TILT1;
		newtilty=newTilts[1]/2.0*SCALE_FACTOR_TILT2;
	    
	    if(ant_num==1) {
	      fprintf(fp1,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp1);
	    }
	    if(ant_num==2) {
	      fprintf(fp2,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp2);
	    }
	    if(ant_num==3) {
	      fprintf(fp3,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp3);
	    }
	    if(ant_num==4) {
	      fprintf(fp4,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp4);
	    }
	    if(ant_num==5) {
	      fprintf(fp5,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp5);
	    }
	    if(ant_num==6) {
	      fprintf(fp6,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp6);
	    }
	    if(ant_num==7) {
	      fprintf(fp7,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp7);
	    }
	    if(ant_num==8) {
	      fprintf(fp8,"%10.6f %10.4f %10.4f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.9f %.2f %.2f %d\n",
utc_hours,az_deg,el_deg,tilts[0],tilts[1],tilts[2],tilts[3],newTilts[0],newTilts[1],tiltx,tilty,tiltxbase,tiltybase,tiltScaledX,tiltScaledY,tiltScaledBaseX,tiltScaledBaseY,newtiltx, newtilty,pmdaz,pmdel,tiltflag);
	      fflush(fp8);
	    }
	  }  /*recordTiltsFlag ON */
		sleep(1);
	    
	    i++;
	    
	  } /* end while save flag is on */
	  
	    if(ant_num==1) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp1,"#");
	      printHeader(fp1,p,somecomment);
	      fflush(fp1);
	      fclose(fp1);
	    }
	    if(ant_num==2) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp2,"#");
	      printHeader(fp2,p,somecomment);
	      fflush(fp2);
	      fclose(fp2);
	    }
	    if(ant_num==3) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp3,"#");
	      printHeader(fp3,p,somecomment);
	      fflush(fp3);
	      fclose(fp3);
	    }
	    if(ant_num==4) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp4,"#");
	      printHeader(fp4,p,somecomment);
	      fflush(fp4);
	      fclose(fp4);
	    }
	    if(ant_num==5) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp5,"#");
	      printHeader(fp5,p,somecomment);
	      fflush(fp5);
	      fclose(fp5);
	    }
	    if(ant_num==6) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp6,"#");
	      printHeader(fp6,p,somecomment);
	      fflush(fp6);
	      fclose(fp6);
	    }
	    if(ant_num==7) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp7,"#");
	      printHeader(fp7,p,somecomment);
	      fflush(fp7);
	      fclose(fp7);
	    }
	    if(ant_num==8) {
	      readHeader(ant_num,0,1,&p);
	      fprintf(fp8,"#");
	      printHeader(fp8,p,somecomment);
	      fflush(fp8);
	      fclose(fp8);
	    }
	    quitFlag++;

	  sleep(1);
	} /* end while */
	return(0);
}

void signalHandler(int signum)
{
if(signum==SIGUSR1) {
  printf("Got signal.\n");
  fileWriteFlag=0;
 }
}


