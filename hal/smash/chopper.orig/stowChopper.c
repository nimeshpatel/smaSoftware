/***************************************************************
*
* stowChopper.c
* 
* NAP 6 March 2000 revised for PowerPC
* NAP 23 Aug 2000 revised for Antenna-4's chopper pmac
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
        fprintf(stderr, "Usage: stowChopper [options] \n"
                "[options] include:\n"
                "  -h or --help    this help\n"
                "  -a<n> or --antenna <n> (n is the antenna number)\n"
          "                  (default: all antennas)\n");
        exit(exitcode);
}


void main(int argc, char *argv[])  
{

	char c,messg[100],command_n[30];
	short pmac_command_flag=0;
	short error_flag=RUNNING;
	int rm_status,antlist[RM_ARRAY_SIZE];
	smapoptContext optCon;
	int i, antenna;
	FILE *fp;
	int gotantenna=0,antennaArray[]={0,0,0,0,0,0,0,0,0};
        char line[BUFSIZ];
        int trackStatus=0,iant,ipad;
        int tracktimestamp,timestamp;
	short antennaStatus = 0;

	 struct  smapoptOption optionsTable[] = {
                {"help",'h',SMAPOPT_ARG_NONE,0,'h'},
                {"antennas",'a',SMAPOPT_ARG_INT,&antenna,'a'},
                {NULL,0,0,NULL,0}
        };


        optCon = smapoptGetContext("stowChopper", argc, argv, optionsTable,0);
 
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
        pmac_command_flag=1;

 
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

	strcpy(command_n, "p2=2");
 
   /* send the command */
        rm_status=rm_write(antenna,"RM_CHOPPER_SMASH_COMMAND_C30",
                                        &command_n);
        if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write()");
                exit(1);
        }
 
 

rm_status=rm_write_notify(antenna,"RM_CHOPPER_SMASH_COMMAND_FLAG_S",
                                        &pmac_command_flag);
 
     if(rm_status != RM_SUCCESS) {
                rm_error_message(rm_status,"rm_write_notify()");
                exit(1);
        }
 
        } /* if antenna is online */
        } /* for 8 antennas */

}
