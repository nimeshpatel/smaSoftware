/***************************************************************
*
* writeTilts.c
* NAP 14 April 2005
* tiltmeter writing ON/OFF
*
****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <smapopt.h>
#include "rm.h"
#include "commonLib.h"

#define OK 0
#define RUNNING 1
#define ERROR -1

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: tol [options] <-c or --command (on/off)>\n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n");
        exit(exitcode);
}


void main(int argc, char *argv[])  
{
	char c,*command,command_n[30];
/*      char messg[100]; */

	short pmac_command_flag=0;
	int gotc=0,rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int i,antenna;
	int gotantenna=0,antennaArray[SMAPOPT_MAX_ANTENNAS+1],
		antennas[SMAPOPT_MAX_ANTENNAS+1];
        int trackStatus=0,iant;
        int tracktimestamp,timestamp;
	int numberOfAnts=0;
	int writeTiltsFlag=0;


	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a'},
                {"command",'c',SMAPOPT_ARG_STRING,&command,'c'},
                {NULL,0,0,NULL,0}
        };

	 for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };
	command_n[1]=0x0; /* send the command */
        pmac_command_flag=0;



 if(argc<2) usage(-1,"Insufficient number of arguments","A command is  required.");
 
        optCon = smapoptGetContext("trackCommand", argc, argv, optionsTable,0);
 
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
                break;
                }
 
 
        }
 
 if(gotc!=1) usage(-2,"No command specified","An  on/off command is required .\n");
 
        if(c<-1) {
        fprintf(stderr, "%s: %s\n",
                smapoptBadOption(optCon, SMAPOPT_BADOPTION_NOALIAS),
                smapoptStrerror(c));
        }
         smapoptFreeContext(optCon);

	if(!strcmp(command,"on")) {
	 writeTiltsFlag=1;
	}
	else if(!strcmp(command,"off")) {
	 writeTiltsFlag=0;
	}
	else { 
	usage(-3,"Wrong command specified","Command should be either on or off.\n");
	}
 
              /* initializing ref. mem. */
        rm_status=rm_open(antlist);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_open()");
                exit(1);
        }
 
	if(gotantenna==1) {
             numberOfAnts=0;
                 for(iant=1;iant<ANT_ARRAY_SZ;iant++) {
                    if(antennaArray[iant]==1) {
                    antennas[numberOfAnts]=iant;
                    numberOfAnts++;
                    }
                 }
		 if (numberOfAnts < 1) {
		   printf("No antennas specified.  Nothing done.\n");
		   exit(1);
		 }
        }
        else {
	getAntennaList(antennaArray);
        }

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

        rm_status=rm_write(antenna,"RM_TILT_RECORDTILTS_FLAG_L",
                                        &writeTiltsFlag);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
 
 
	} /* if track is running */
        } /* if antenna is online */
        } /* for 8 antennas */

 
}
