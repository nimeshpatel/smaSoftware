/***************************************************************
*
* observe.c
* 
* SMAsh command for tracking an astronomical source
* NAP 
* 1 March 2000
*
* 5 June 2001: added inputs for source specs and offsets
* This version is for the PowerPC
* Command line arguments are now parsed using the smapopt library.
* Revised on 26 Jun 2001 to command only those antennas which
*  are specified by the project command- if no antenna switch is given.
****************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <smapopt.h>
#include "rm.h"

#define OK     0
#define ERROR -1
#define RUNNING 1

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: observe [options] --source <sourcename>\n"
                "[options] include:\n"
		"[--ra or -r <hh:mm:ss.sss> --dec or -d <+/-dd:mm:dd.ddd>\n"
		"--epoch -e <1950./2000.>\n"
		"--velocity or -v <velocity km/s (only vlsr for now)>]\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
                "                  (default: all online antennas)\n");
        exit(exitcode);
}

void main(int argc, char *argv[])  
{
	FILE *fp;
	char c,*source,Source[34],messg[100], command_n[30];
	short i, pmac_command_flag=0; 
	short error_flag=RUNNING, sourcelength;
	int gotsource=0,rm_status,antlist[RM_ARRAY_SIZE];
	int gotantenna=0,gotcoordinates=0,gotvelocity=0;
	smapoptContext optCon;	
	int antenna;
	int antennaArray[]={0,0,0,0,0,0,0,0,0};
	int iant,ipad;
	int tracktimestamp,timestamp;
	char line[BUFSIZ];
	int trackStatus=0;
	short antennaStatus = 0;

	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"antenna",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {"source",'s',SMAPOPT_ARG_STRING,&source,'s'},
                {NULL,0,0,NULL,0}
        };

 if(argc<2) usage(-1,"Insufficient number of arguments","At least
source- name required.");
 
        optCon = smapoptGetContext("observe", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
                case 'a':
		gotantenna=1;
                break;
 
                case 's':
                gotsource=1;
                break;
 
                }
 
        }
 
        if(gotsource!=1) usage(-2,"No source specified","Source name is required .\n");
 
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

	sourcelength=strlen(source);
	strcpy(Source,source);
	for(i=0;i<30;i++) {
	command_n[i]=0x0;
	}
	command_n[0]='n';/*0x6e;*/ /* send 'n' command */
	command_n[1]=0x0;
	pmac_command_flag=0;

	for(iant=1;iant<=8;iant++) {

	if(antennaArray[iant]==1) {
	antenna=iant;

	  /* check if track is running on this antenna */
        
        rm_status=rm_read(antenna,"RM_UNIX_TIME_L",&timestamp);
	rm_status=rm_read(antenna,"RM_TRACK_TIMESTAMP_L",&tracktimestamp);
        if(abs(tracktimestamp-timestamp)>3L) {
	trackStatus=0;
	printf("Track is not running on antenna %d.\n",antenna);
	}
        if(abs(tracktimestamp-timestamp)<=3L) trackStatus=1;

	if(trackStatus==1) {

	
	rm_status=rm_write(antenna,"RM_SMARTS_SOURCE_LENGTH_S",
				&sourcelength);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
	rm_status=rm_write(antenna,"RM_SMARTS_SOURCE_C34",Source);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }

	rm_status=rm_write(antenna,"RM_SMASH_TRACK_COMMAND_C30",
					command_n);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
	
	rm_status=rm_write_notify(antenna,"RM_SMARTS_PMAC_COMMAND_FLAG_S",
					&pmac_command_flag);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write_notify()");
                exit(1);
        }

	} /* if track is running */
	} /* if antenna is online */
	} /* for 8 antennas */
}
