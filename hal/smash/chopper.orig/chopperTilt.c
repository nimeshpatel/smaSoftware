/***************************************************************
*
* chopperTilt.c
* 
* NAP 26 August 2000 for the chopper pmac on antenna-4
  copied from chopperX.c
* NAP 27 Feb 2001 default action: all antennas.
* Revised on 26 Jun 2001 to command only those antennas which
*  are specified by the project command- if no antenna switch is given.
*
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <smapopt.h>
#include "rm.h"

#define OK 0
#define RUNNING 1
#define ERROR -1

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
      fprintf(stderr, "Usage: chopperX [options] --cts (or -c) <displacement: -cts or --tilt (or -t) <absolute angle on sky in arcseconds>\n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
          "                  (default: all antennas)\n");
        exit(exitcode);
}


void main(int argc, char *argv[])  
{

	char c,messg[100],command_n[30],*counts,*tiltangle;
	short pmac_command_flag=0;
	char pmaccommand[30];
	short error_flag=RUNNING;
	int gotc=0,rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int counts_int=0;
	float tilt_float;
	int  i,antenna;
	FILE *fp;
	int gotantenna=0,antennaArray[]={0,0,0,0,0,0,0,0,0};
        char line[BUFSIZ];
        int trackStatus=0,iant,ipad;
        int tracktimestamp,timestamp;
	short antennaStatus = 0;


	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"antennas",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {"counts",'c',SMAPOPT_ARG_STRING,&counts,'c'},
                {"tilt",'t',SMAPOPT_ARG_STRING,&tiltangle,'t'},
                {NULL,0,0,NULL,0}
        };


	

 if(argc<2) usage(-1,"Insufficient number of arguments","At least
tilt angle (cts or arcseconds) required.");
 
        optCon = smapoptGetContext("chopperTilt", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
 
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
               case 'a':
		gotantenna=1;
                break;
 
                case 'c':
                gotc=1;
		counts_int=atoi(counts);
                break;

                case 't':
                gotc=1;
		tilt_float=atof(tiltangle);
		tilt_float=tilt_float*10000./60.;
		counts_int=(int)tilt_float;
                break;
                }
        }
        pmac_command_flag=1;

 
 if(gotc!=1) usage(-2,"No displacement specified","Specify counts or mm to move.\n");
 
        if(c<-1) {
        fprintf(stderr, "%s: %s\n",
                smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
                smapoptStrerror(c));
        }
         smapoptFreeContext(optCon);
 
              /* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }

	
	if(gotantenna==1) {
        antennaArray[antenna]=1;
        }
        else {
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

	for(iant=1;iant<=8;iant++) {

        if(antennaArray[iant]==1) {
        antenna=iant;

	sprintf(pmaccommand,"chopperCommand2 -a %d -c p21=%d",antenna,counts_int);
        system(pmaccommand);
        sleep(1);
        sprintf(pmaccommand,"chopperCommand2 -a %d -c p2=5",antenna);
        system(pmaccommand);
        } /* if antenna is online */
        } /* for 8 antennas */
}
