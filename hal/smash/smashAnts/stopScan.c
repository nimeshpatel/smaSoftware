/***************************************************************
*
* stopScan.c
* 
* NAP 3 March 2000 revised for PowerPC
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
#include "commonLib.h"

#define OK 0
#define RUNNING 1
#define ERROR -1

#define ANT_ARRAY_SZ (sizeof(antennaArray)/sizeof(antennaArray[0]))

void usage(int exitcode, char *error, char *addl) {
 
        if (error) fprintf(stderr, "\n%s: %s\n\n", error, addl);
        fprintf(stderr, "Usage: stopScan [options] \n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
          "                  (default: all antennas)\n");
        exit(exitcode);
}


int main(int argc, char *argv[])  
{

	char c,command_n[30];
/*	char messg[100]; */
	short pmac_command_flag=0;
	short error_flag=RUNNING;
	int rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int i, antenna;
	int gotantenna=0;
	int antennaArray[SMAPOPT_MAX_ANTENNAS+1],
                antennas[SMAPOPT_MAX_ANTENNAS+1];
	int numberOfAnts=0;
        int trackStatus=0,iant;
        int tracktimestamp,timestamp;

	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
		{"antennas",'a',SMAPOPT_ARG_ANTENNAS,antennaArray,'a'},
                {NULL,0,0,NULL,0}
        };


        optCon = smapoptGetContext("stopScan", argc, argv, optionsTable,0);
 
        while ((c = smapoptGetNextOpt(optCon)) >= 0) {
 
        switch(c) {
                case 'h':
                usage(0,NULL,NULL);
                break;
 
               case 'a':
		gotantenna=1;
                break;
		}
 
        }
	for(i=0;i<30;i++) {
        command_n[i]=0x0;
        };
	command_n[0]='z'; /* send the command */
        pmac_command_flag=0;

 
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
 
        rm_status=rm_write(antenna,"RM_SMASH_TRACK_COMMAND_C30",
                                        &command_n);
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
 
        error_flag=RUNNING;
 
/*
         rm_status=rm_read(antenna,
                "RM_SMARTS_COMMAND_STATUS_S",&error_flag);
         if(rm_status != RM_SUCCESS) {
         rm_error_message(rm_status,"rm_read()");
         exit(1);
         }
 
        if(error_flag==ERROR)
        {
        printf("Error from track:\n");
        rm_status=rm_read(antenna,
 
               "RM_TRACK_MESSAGE_C100",messg);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_read()");
                exit(1);
        }
        printf("%s\n",messg);
        }
*/

	} /* if track is running */
        } /* if antenna is online */
        } /* for 8 antennas */

return(0); 
}
